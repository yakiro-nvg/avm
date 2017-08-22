/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "compiler.h"

#include <any/asm.h>

#include <ctype.h>
#include <exception>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <map>

using namespace std;

struct amlc_value_t
{
    string literal;
    aint_t idx;
};

typedef vector<amlc_value_t> amlc_pool;

struct amlc_prototype_ctx_t
{
    amlc_pool arguments;
    amlc_pool imports;
    amlc_pool constants;
    amlc_pool nesteds;
    amlc_pool pseudo_nesteds;
};

struct amlc_ctx_t;

typedef function<void(
    amlc_ctx_t&, amlc_prototype_ctx_t&)> amlc_opcode_handler_t;

struct amlc_ctx_t
{
    aasm_t* a;
    bool verbose;
    string filename;
    const char* s;
    int line;
    int column;
    stringstream ss;
    map<string, amlc_opcode_handler_t> opcode_handlers;
};

static void error(amlc_ctx_t& ctx, const char* fmt, ...)
{
    va_list args;
    char buf[512];
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ctx.ss.str(string());
    ctx.ss << ctx.filename;
    ctx.ss << '@' << ctx.line << ':' << ctx.column << '\n' << buf;
    throw exception(ctx.ss.str().c_str());
}

static amlc_pool::const_iterator pool_find(
    const amlc_pool& pool, const string& literal)
{
    return find_if(pool.begin(), pool.end(), [&](const amlc_value_t& v) {
        return v.literal == literal;
    });
}

static amlc_pool::const_iterator pool_find(
    const amlc_pool& pool, aint_t idx)
{
    return find_if(pool.begin(), pool.end(), [&](const amlc_value_t& v) {
        return v.idx == idx;
    });
}

static amlc_pool::iterator pool_find(amlc_pool& pool, const string& literal)
{
    return find_if(pool.begin(), pool.end(), [&](const amlc_value_t& v) {
        return v.literal == literal;
    });
}

static amlc_pool::iterator pool_find(amlc_pool& pool, aint_t idx)
{
    return find_if(pool.begin(), pool.end(), [&](const amlc_value_t& v) {
        return v.idx == idx;
    });
}

static aint_t pool_push(
    amlc_pool& pool, const string& literal, function<aint_t()> push)
{
    auto i = pool_find(pool, literal);
    if (i != pool.end()) {
        return i->idx;
    } else {
        amlc_value_t v;
        v.idx = push();
        v.literal = literal;
        pool.push_back(move(v));
        return v.idx;
    }
}

static inline aint_t pool_push_argument(amlc_pool& pool, const string& literal)
{
    return pool_push(pool, literal, [&] { return -(aint_t)pool.size() - 1; });
}

static inline aint_t pool_take_argument(
    amlc_ctx_t& ctx, amlc_pool& pool, const string& literal)
{
    return pool_push(pool, literal, [&]() -> int {
        error(ctx, "undefined argument `%s`", literal.c_str());
        return 0;
    });
}

static inline aint_t pool_push_import(
    amlc_pool& pool, aasm_t* a, const string& module, const string& name)
{
    auto literal = module + ':' + name;
    return pool_push(pool, literal, [&] {
        return aasm_add_import(a, module.c_str(), name.c_str());
    });
}

static inline aint_t pool_push_integer_constant(
    amlc_pool& pool, aasm_t* a, const string& literal, aint_t v)
{
    return pool_push(pool, literal, [&] {
        return aasm_add_constant(a, ac_integer(v));
    });
}

static inline aint_t pool_push_string_constant(
    amlc_pool& pool, aasm_t* a, const string& v)
{
    return pool_push(pool, v, [&] {
        return aasm_add_constant(
            a, ac_string(aasm_string_to_ref(a, v.c_str())));
    });
}

static inline aint_t pool_push_real_constant(
    amlc_pool& pool, aasm_t* a, const string& literal, areal_t v)
{
    return pool_push(pool, literal, [&] {
        return aasm_add_constant(a, ac_real(v));
    });
}

static inline aint_t pool_push_nested(
    amlc_pool& pool, aasm_t* a, const string& literal)
{
    return pool_push(pool, literal, [&] { return aasm_push(a); });
}

static inline aint_t pool_push_pseudo_nested(
    amlc_pool& pool, const string& literal)
{
    return pool_push(pool, literal, [&] { return pool.size(); });
}

static void resolve_pseudo_nesteds(
    amlc_ctx_t& ctx, ainstruction_t* instructions, aint_t num,
    const amlc_pool& nesteds, const amlc_pool& pseudos)
{
    for (aint_t i = 0; i < num; ++i) {
        ainstruction_t& ai = instructions[i];
        if (ai.b.opcode != AOC_CLS) continue;
        auto pi = pool_find(pseudos, (aint_t)ai.cls.idx);
        auto ni = pool_find(nesteds, pi->literal);
        if (ni == nesteds.end()) {
            error(ctx, "undefined closure `%s`", pi->literal.c_str());
        }
        ai.cls.idx = ni->idx;
    }
}

static inline string character(char c)
{
    string s;
    switch (c) {
    case '\n':
        s = "\\n";
        break;
    case '\t':
        s = "\\t";
        break;
    case 0:
        s = "<EOF>";
        break;
    default:
        s = c;
        break;
    }
    return move(s);
}

static inline void advance(amlc_ctx_t& ctx)
{
    ++ctx.s;
    ++ctx.column;
}

static inline bool issymbol(char c)
{
    return c == '_';
}

static inline bool isbareword(char c)
{
    return isalpha(c) || isdigit(c) || issymbol(c);
}

static inline void match(amlc_ctx_t& ctx, char c)
{
    if (*ctx.s != c) {
        error(ctx, "expected `%s`, saw `%s`",
            character(c).c_str(), character(*ctx.s).c_str());
    }
    advance(ctx);
}

static inline void match(amlc_ctx_t& ctx, const char* s)
{
    int len = (int)strlen(s);
    if ((int)strlen(ctx.s) < len || memcmp(s, ctx.s, len) != 0) {
        auto end = strchr(ctx.s, '\n');
        if (end) {
            error(ctx, "expected `%s`, saw `%.*s`", s, (int)(end - ctx.s), ctx.s);
        } else {
            error(ctx, "expected `%s`, saw `%s`", s, ctx.s);
        }
    }
    ctx.s += len;
    ctx.column += len;
}

static string lookahead(amlc_ctx_t& ctx, int nchars)
{
    ctx.ss.str(string());
    for (int i = 0; i < nchars; ++i) {
        if (!isalpha(ctx.s[i])) {
            error(ctx, "expect alphabet, saw `%s`", character(*ctx.s).c_str());
        }
        ctx.ss << ctx.s[i];
    }
    return ctx.ss.str();
}

static void skip_whitespace(amlc_ctx_t& ctx, bool line)
{
    while (isspace(*ctx.s) || *ctx.s == '/' || *ctx.s == ',') {
        if (*ctx.s == '\n') {
            if (line == false) return;
            ++ctx.line;
            ctx.column = 1;
            ++ctx.s;
        } else if (isspace(*ctx.s)) {
            if (*ctx.s == '\t') {
                ctx.column += 3;
            }
            advance(ctx);
        } else if (*ctx.s == ';') { // semicolon style comment
            while (*ctx.s && *ctx.s != '\n') {
                advance(ctx);
            }
            ++ctx.line;
            ctx.column = 1;
            ++ctx.s;
        } else if (*ctx.s == '/') {
            if (ctx.s[1] == '/') { // C++ style comment
                while (*ctx.s && *ctx.s != '\n') {
                    advance(ctx);
                }
                ++ctx.line;
                ctx.column = 1;
                ++ctx.s;
            } else if (ctx.s[1] == '*') { // C style comment
                ctx.s += 2;
                while (*ctx.s && !(*ctx.s == '*' && ctx.s[1] == '/')) {
                    if (*ctx.s == '\n') {
                        ++ctx.line;
                        ctx.column = 1;
                        ++ctx.s;
                    } else {
                        advance(ctx);
                    }
                }
                match(ctx, '*');
                match(ctx, '/');
            } else {
                return;
            }
        } else {
            return;
        }
    }
}

static string match_symbol(amlc_ctx_t& ctx)
{
    ctx.ss.str(string());
    if (isalpha(*ctx.s) == false) {
        error(ctx, "expect alphabet, saw `%s`", character(*ctx.s).c_str());
    }
    while (isbareword(*ctx.s)) {
        ctx.ss << *ctx.s;
        advance(ctx);
    }
    return ctx.ss.str();
}

static string match_number(amlc_ctx_t& ctx, bool& integer, aint_t& i, areal_t& r)
{
    ctx.ss.str("");
    aint_t sign = 1;
    if (*ctx.s == '-') {
        sign = -1;
        ctx.ss << *ctx.s;
        advance(ctx);
    }
    aint_t intp = 0;
    if (isdigit(*ctx.s)) {
        intp = (*ctx.s - '0');
        ctx.ss << *ctx.s;
        advance(ctx);
        while (isdigit(*ctx.s)) {
            intp = 10 * intp + (*ctx.s - '0');
            ctx.ss << *ctx.s;
            advance(ctx);
        }
    }
    if (*ctx.s == 'e' || *ctx.s == 'E' || *ctx.s == '.') {
        integer = false;
        aint_t fracp = 0;
        aint_t fracdiv = 1;
        if (*ctx.s == '.') {
            ctx.ss << *ctx.s;
            advance(ctx);
            if (isdigit(*ctx.s) == false) {
                error(ctx, "expected digit, saw `%s`",
                    character(*ctx.s).c_str());
            }
            while (isdigit(*ctx.s)) {
                fracp = 10 * fracp + (*ctx.s - '0');
                fracdiv *= 10;
                ctx.ss << *ctx.s;
                advance(ctx);
            }
        }
        int esign = 1;
        int ep = 0;
        if (*ctx.s == 'e' || *ctx.s == 'E') {
            ctx.ss << *ctx.s;
            advance(ctx);
            if (*ctx.s == '+') {
                ctx.ss << *ctx.s;
                advance(ctx);
            } else if (*ctx.s == '-') {
                esign = -1;
                ctx.ss << *ctx.s;
                advance(ctx);
            } if (isdigit(*ctx.s)) {
                ep = (*ctx.s - '0');
                ctx.ss << *ctx.s;
                advance(ctx);
            } else {
                error(ctx, "unexpected character `%s`",
                    character(*ctx.s).c_str());
            }
            while (isdigit(*ctx.s)) {
                ep = ep * 10 + (*ctx.s - '0');
                ctx.ss << *ctx.s;
                advance(ctx);
            }
        }
        r = (areal_t)sign *
            ((areal_t)intp + (areal_t)fracp / (areal_t)fracdiv) *
            (areal_t)pow(10.0, (double)esign * (double)ep);
    } else if (isspace(*ctx.s) == false && *ctx.s != 0) {
        error(ctx, "expected digit, saw `%s`", character(*ctx.s).c_str());
    } else {
        i = sign * intp;
        integer = true;
    }
    return ctx.ss.str();
}

static aint_t match_integer(amlc_ctx_t& ctx)
{
    bool integer; aint_t i; areal_t r;
    auto literal = match_number(ctx, integer, i, r);
    if (integer == false) {
        error(ctx, "expected integer, saw `%s`", literal);
    }
    return i;
}

static string match_string(amlc_ctx_t& ctx)
{
    ctx.ss.str("");
    match(ctx, '"');
    for (;;) {
        if (*ctx.s == 0 || *ctx.s == '"') {
            break;
        } else if (*ctx.s < 32) {
            error(ctx, "bad string character 0x%x", *ctx.s);
        }  else if (*ctx.s == '\\') {
            advance(ctx);
            char c = *ctx.s;
            advance(ctx);
            switch (c) {
            case '"':
            case '\\':
            case '/':
                ctx.ss << c;
                break;
            case 'b':
                ctx.ss << '\b';
                break;
            case 'f':
                ctx.ss << '\f';
                break;
            case 'n':
                ctx.ss << '\n';
                break;
            case 'r':
                ctx.ss << '\r';
                break;
            case 't':
                ctx.ss << '\t';
                break;
            default:
                error(ctx, "unexpected character `%s`",
                    character(*ctx.s).c_str());
            }
        } else {
            ctx.ss << *ctx.s;
            advance(ctx);
        }
    }
    match(ctx, '"');
    return ctx.ss.str();
}

static void match_nop(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_nop());
}

static void match_pop(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_pop(match_integer(ctx)));
}

static void match_ldk(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    aint_t idx;
    if (*ctx.s == '"') {
        idx = pool_push_string_constant(
            pctx.constants, ctx.a, match_string(ctx));
    } else if (*ctx.s == '-' || isdigit(*ctx.s)) {
        bool integer; aint_t i; areal_t r;
        auto literal = match_number(ctx, integer, i, r);
        if (integer) {
            idx = pool_push_integer_constant(pctx.constants, ctx.a, literal, i);
        } else {
            idx = pool_push_real_constant(pctx.constants, ctx.a, literal, r);
        }
    } else {
        error(ctx, "unexpected character `%s`", character(*ctx.s).c_str());
    }
    aasm_emit(ctx.a, ai_ldk(idx));
}

static void match_nil(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_nil());
}

static void match_ldb(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    auto sym = match_symbol(ctx);
    if (sym != "true" && sym != "false") {
        error(ctx, "expected boolean, saw `%s`", sym.c_str());
    }
    aasm_emit(ctx.a, ai_ldb(sym == "true" ? TRUE : FALSE));
}

static void match_lsi(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_lsi(match_integer(ctx)));
}

static void match_llv(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    aint_t idx;
    if (isalpha(*ctx.s)) {
        idx = pool_take_argument(ctx, pctx.arguments, match_symbol(ctx));
    } else {
        idx = match_integer(ctx);
    }
    aasm_emit(ctx.a, ai_llv(idx));
}

static void match_slv(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_slv(match_integer(ctx)));
}

static void match_imp(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    stringstream name;
    auto module = match_symbol(ctx);
    match(ctx, ':');
    name << match_symbol(ctx);
    match(ctx, '/');
    name << '/';
    if (*ctx.s == '*') {
        match(ctx, '*');
        name << '*';
    } else {
        name << match_integer(ctx);
    }
    aint_t idx = pool_push_import(pctx.imports, ctx.a, module, name.str());
    aasm_emit(ctx.a, ai_imp(idx));
}

static void match_cls(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    stringstream name;
    name << match_symbol(ctx);
    match(ctx, '/');
    name << '/' << match_integer(ctx);
    aint_t idx = pool_push_pseudo_nested(pctx.pseudo_nesteds, name.str());
    aasm_emit(ctx.a, ai_cls(idx));
}

static void match_jmp(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_jmp(match_integer(ctx)));
}

static void match_jin(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_jin(match_integer(ctx)));
}

static void match_ivk(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_ivk(match_integer(ctx)));
}

static void match_ret(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_ret());
}

static void match_snd(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_snd());
}

static void match_rcv(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_rcv(match_integer(ctx)));
}

static void match_rmv(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_rmv());
}

static void match_rwd(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_rwd());
}

static aint_t match_prototype(amlc_ctx_t& ctx);

static void match_nested_prototype(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    stringstream name;
    match(ctx, "def");
    skip_whitespace(ctx, true);
    name << match_symbol(ctx);
    pool_push_nested(pctx.nesteds, ctx.a, name.str());
    skip_whitespace(ctx, true);
    aint_t nargs = match_prototype(ctx);
    skip_whitespace(ctx, true);
    auto i = pool_find(pctx.nesteds, name.str());
    name << '/' << nargs;
    i->literal = name.str();
    aasm_pop(ctx.a);
}

static aint_t match_prototype(amlc_ctx_t& ctx)
{
    amlc_prototype_ctx_t pctx;

    match(ctx, '(');
    while (*ctx.s != ')') {
        skip_whitespace(ctx, true);
        if (pctx.arguments.size() > 0) {
            match(ctx, ',');
            skip_whitespace(ctx, true);
        }
        pool_push_argument(pctx.arguments, match_symbol(ctx));
    }
    skip_whitespace(ctx, true);
    match(ctx, ')');
    skip_whitespace(ctx, true);

    for (;;) {
        auto word = lookahead(ctx, 3);
        if (word == "end") {
            match(ctx, "end");
            skip_whitespace(ctx, true);
            break;
        } else if (word == "def") {
            match_nested_prototype(ctx, pctx);
            skip_whitespace(ctx, true);
            continue;
        }
        auto handler = ctx.opcode_handlers.find(word);
        if (handler == ctx.opcode_handlers.end()) {
            error(ctx, "unexpected symbol `%s`", word.c_str());
        } else {
            match(ctx, word.c_str());
            skip_whitespace(ctx, true);
            handler->second(ctx, pctx);
            skip_whitespace(ctx, true);
        }
    }

    auto pt = aasm_prototype(ctx.a);
    auto cu = aasm_resolve(ctx.a);

    resolve_pseudo_nesteds(
        ctx, cu.instructions, pt->num_instructions,
        pctx.nesteds, pctx.pseudo_nesteds);

    return (aint_t)pctx.arguments.size();
}

static void match_module_prototype(amlc_ctx_t& ctx)
{
    stringstream name;
    match(ctx, "def");
    skip_whitespace(ctx, true);
    name << match_symbol(ctx);
    aasm_module_push(ctx.a, name.str().c_str());
    skip_whitespace(ctx, true);
    aint_t nargs = match_prototype(ctx);
    skip_whitespace(ctx, true);
    auto pt = aasm_prototype(ctx.a);
    name << '/' << nargs;
    pt->symbol = aasm_string_to_ref(ctx.a, name.str().c_str());
    aasm_pop(ctx.a);
}

static void match_module(amlc_ctx_t& ctx)
{
    match(ctx, "module");
    skip_whitespace(ctx, false);
    aasm_prototype_t* p = aasm_prototype(ctx.a);
    p->symbol = aasm_string_to_ref(ctx.a, match_symbol(ctx).c_str());
    skip_whitespace(ctx, true);
    while (*ctx.s != 0) {
        match_module_prototype(ctx);
        skip_whitespace(ctx, true);
    }
}

void amlc_compile(
    aasm_t* a, const char* buf, const char* filename, bool verbose)
{
    amlc_ctx_t ctx;
    ctx.a = a;
    ctx.filename = filename;
    ctx.s = buf;
    ctx.column = 1;
    ctx.line = 1;
    ctx.verbose = verbose;

#define ADD_HANDLER(n) ctx.opcode_handlers[#n] = &match_##n

    ADD_HANDLER(nop);
    ADD_HANDLER(pop);
    ADD_HANDLER(ldk);
    ADD_HANDLER(nil);
    ADD_HANDLER(ldb);
    ADD_HANDLER(lsi);
    ADD_HANDLER(llv);
    ADD_HANDLER(slv);
    ADD_HANDLER(imp);
    ADD_HANDLER(cls);
    ADD_HANDLER(jmp);
    ADD_HANDLER(jin);
    ADD_HANDLER(ivk);
    ADD_HANDLER(ret);
    ADD_HANDLER(snd);
    ADD_HANDLER(rcv);
    ADD_HANDLER(rmv);
    ADD_HANDLER(rwd);

#undef  ADD_HANDLER

    match_module(ctx);
}