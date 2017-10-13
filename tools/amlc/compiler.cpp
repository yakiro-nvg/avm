/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include "compiler.h"

#include <any/asm.h>

#include <ctype.h>
#include <math.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <map>

struct amlc_value_t
{
    std::string literal;
    aint_t idx;
};

typedef std::vector<amlc_value_t> amlc_pool_t;

struct amlc_prototype_ctx_t
{
    aint_t num_arguments;
    aint_t num_variables;
    amlc_pool_t locals;
    amlc_pool_t imports;
    amlc_pool_t constants;
    amlc_pool_t nesteds;
    amlc_pool_t pseudo_nesteds;
    amlc_pool_t label_addresses;
    std::vector<aint_t> pseudo_jumps;
};

struct amlc_ctx_t;

typedef std::function<void(
    amlc_ctx_t&, amlc_prototype_ctx_t&)> amlc_opcode_handler_t;

typedef std::map<std::string, amlc_opcode_handler_t> opcode_handlers_t;

struct amlc_ctx_t
{
    aasm_t* a;
    bool verbose;
    std::string filename;
    const char* s;
    int line;
    int column;
    std::stringstream ss;
    opcode_handlers_t opcode_handlers;
};

static void error(amlc_ctx_t& ctx, const char* fmt, ...)
{
    va_list args;
    char buf[512];
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ctx.ss.str(std::string());
    ctx.ss << ctx.filename;
    ctx.ss << '@' << ctx.line << ':' << ctx.column << '\n' << buf;
    throw std::logic_error(ctx.ss.str().c_str());
}

static amlc_pool_t::const_iterator pool_find(
    const amlc_pool_t& pool, const std::string& literal)
{
    return find_if(pool.begin(), pool.end(), [&](const amlc_value_t& v) {
        return v.literal == literal;
    });
}

static amlc_pool_t::const_iterator pool_find(
    const amlc_pool_t& pool, aint_t idx)
{
    return find_if(pool.begin(), pool.end(), [&](const amlc_value_t& v) {
        return v.idx == idx;
    });
}

static amlc_pool_t::iterator pool_find(
    amlc_pool_t& pool, const std::string& literal)
{
    return find_if(pool.begin(), pool.end(), [&](const amlc_value_t& v) {
        return v.literal == literal;
    });
}

static amlc_pool_t::iterator pool_find(amlc_pool_t& pool, aint_t idx)
{
    return find_if(pool.begin(), pool.end(), [&](const amlc_value_t& v) {
        return v.idx == idx;
    });
}

static aint_t pool_push(
    amlc_pool_t& pool, const std::string& literal, std::function<aint_t()> push)
{
    auto i = pool_find(pool, literal);
    if (i != pool.end()) {
        return i->idx;
    } else {
        amlc_value_t v;
        v.idx = push();
        v.literal = literal;
        pool.push_back(std::move(v));
        return v.idx;
    }
}

static inline void pool_push_argument(
    amlc_ctx_t& ctx, amlc_pool_t& pool,
    aint_t& num_arguments, const std::string& literal)
{
    if (pool_find(pool, literal) != pool.end()) {
        error(ctx, "duplicated symbol `%s`", literal.c_str());
    } else {
        pool_push(pool, literal, [&] { return -(++num_arguments); });
    }
}

static inline void pool_push_variable(
    amlc_ctx_t& ctx, amlc_pool_t& pool,
    aint_t& num_variables, const std::string& literal)
{
    if (pool_find(pool, literal) != pool.end()) {
        error(ctx, "duplicated symbol `%s`", literal.c_str());
    } else {
        pool_push(pool, literal, [&] { return num_variables++; });
    }
}

static inline void pool_push_label(
    amlc_ctx_t& ctx, amlc_pool_t& pool,
    aint_t address, const std::string& literal)
{
    if (pool_find(pool, literal) != pool.end()) {
        error(ctx, "duplicated symbol `%s`", literal.c_str());
    } else {
        pool_push(pool, literal, [&] { return address; });
    }
}

static inline aint_t pool_take_local(
    amlc_ctx_t& ctx, amlc_pool_t& pool, const std::string& literal)
{
    return pool_push(pool, literal, [&]() -> int {
        error(ctx, "undefined symbol `%s`", literal.c_str());
        return 0;
    });
}

static inline aint_t pool_push_label_address(
    amlc_ctx_t& ctx, amlc_pool_t& pool, const std::string& literal)
{
    return pool_push(pool, literal, [&] { return pool.size(); });
}

static inline aint_t pool_push_import(
    amlc_pool_t& pool, aasm_t* a,
    const std::string& module, const std::string& name)
{
    auto literal = module + ':' + name;
    return pool_push(pool, literal, [&] {
        return aasm_add_import(a, module.c_str(), name.c_str());
    });
}

static inline aint_t pool_push_integer_constant(
    amlc_pool_t& pool, aasm_t* a, const std::string& literal, aint_t v)
{
    return pool_push(pool, literal, [&] {
        return aasm_add_constant(a, ac_integer(v));
    });
}

static inline aint_t pool_push_string_constant(
    amlc_pool_t& pool, aasm_t* a, const std::string& v)
{
    return pool_push(pool, v, [&] {
        return aasm_add_constant(
            a, ac_string(aasm_string_to_ref(a, v.c_str())));
    });
}

static inline aint_t pool_push_real_constant(
    amlc_pool_t& pool, aasm_t* a, const std::string& literal, areal_t v)
{
    return pool_push(pool, literal, [&] {
        return aasm_add_constant(a, ac_real(v));
    });
}

static inline aint_t pool_push_nested(
    amlc_pool_t& pool, aasm_t* a, const std::string& literal)
{
    return pool_push(pool, literal, [&] { return aasm_push(a); });
}

static inline aint_t pool_push_pseudo_nested(
    amlc_pool_t& pool, const std::string& literal)
{
    return pool_push(pool, literal, [&] { return pool.size(); });
}

static void resolve_pseudo_jumps(
    amlc_ctx_t& ctx, ainstruction_t* instructions,
    amlc_pool_t& locals, amlc_pool_t& addresses, std::vector<aint_t>& pseudos)
{
    for (size_t i = 0; i < pseudos.size(); ++i) {
        aint_t off = pseudos[i];
        auto& ins = instructions[off];
        auto lbl_itr = pool_find(addresses, ins.jmp.displacement);
        auto lbl_adr = pool_take_local(ctx, locals, lbl_itr->literal);
        ins.jmp.displacement = lbl_adr - off - 1;
    }
}

static void resolve_pseudo_nesteds(
    amlc_ctx_t& ctx, ainstruction_t* instructions, aint_t num,
    const amlc_pool_t& nesteds, const amlc_pool_t& pseudos)
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

static inline std::string character(char c)
{
    std::string s;
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
    return std::move(s);
}

static inline void advance(amlc_ctx_t& ctx)
{
    ++ctx.s;
    ++ctx.column;
}

static inline bool issymbol(char c)
{
    return c == '_' || c == '-';
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

static void skip_whitespace(amlc_ctx_t& ctx, bool line)
{
    while (isspace(*ctx.s) || *ctx.s == '/' || *ctx.s == '%') {
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
        } else if (*ctx.s == '%') { // percent sign style comment
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

static std::string match_symbol(amlc_ctx_t& ctx)
{
    ctx.ss.str(std::string());
    if (isalpha(*ctx.s) == false) {
        error(ctx, "expect alphabet, saw `%s`", character(*ctx.s).c_str());
    }
    while (isbareword(*ctx.s)) {
        ctx.ss << *ctx.s;
        advance(ctx);
    }
    return ctx.ss.str();
}

static std::string match_number(
    amlc_ctx_t& ctx, bool& integer, aint_t& i, areal_t& r)
{
    ctx.ss.str(std::string());
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
        error(ctx, "expected integer, saw `%s`", literal.c_str());
    }
    return i;
}

static std::string match_string(amlc_ctx_t& ctx)
{
    ctx.ss.str(std::string());
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
    aasm_emit(ctx.a, ai_nop(), ctx.line);
}

static void match_brk(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_brk(), ctx.line);
}

static void match_pop(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_pop(match_integer(ctx)), ctx.line);
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
    aasm_emit(ctx.a, ai_ldk(idx), ctx.line);
}

static void match_nil(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_nil(), ctx.line);
}

static void match_ldb(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    auto sym = match_symbol(ctx);
    if (sym != "true" && sym != "false") {
        error(ctx, "expected boolean, saw `%s`", sym.c_str());
    }
    aasm_emit(ctx.a, ai_ldb(sym == "true" ? TRUE : FALSE), ctx.line);
}

static void match_lsi(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_lsi(match_integer(ctx)), ctx.line);
}

static void match_llv(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    aint_t idx;
    if (isalpha(*ctx.s)) {
        idx = pool_take_local(ctx, pctx.locals, match_symbol(ctx));
    } else {
        idx = match_integer(ctx);
    }
    aasm_emit(ctx.a, ai_llv(idx), ctx.line);
}

static void match_slv(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    aint_t idx;
    if (isalpha(*ctx.s)) {
        idx = pool_take_local(ctx, pctx.locals, match_symbol(ctx));
    } else {
        idx = match_integer(ctx);
    }
    aasm_emit(ctx.a, ai_slv(idx), ctx.line);
}

static void match_imp(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    std::stringstream name;
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
    aasm_emit(ctx.a, ai_imp(idx), ctx.line);
}

static void match_cls(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    std::stringstream name;
    name << match_symbol(ctx);
    match(ctx, '/');
    name << '/' << match_integer(ctx);
    aint_t idx = pool_push_pseudo_nested(pctx.pseudo_nesteds, name.str());
    aasm_emit(ctx.a, ai_cls(idx), ctx.line);
}

static aint_t match_jump_disp(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    aint_t dsp;
    if (isalpha(*ctx.s)) {
        dsp = pool_push_label_address(
            ctx, pctx.label_addresses, match_symbol(ctx));
        auto pt = aasm_prototype(ctx.a);
        pctx.pseudo_jumps.push_back(pt->num_instructions);
    }
    else {
        dsp = match_integer(ctx);
    }
    return dsp;
}

static void match_jmp(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    aasm_emit(ctx.a, ai_jmp(match_jump_disp(ctx, pctx)), ctx.line);
}

static void match_jin(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    aasm_emit(ctx.a, ai_jin(match_jump_disp(ctx, pctx)), ctx.line);
}

static void match_ivk(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_ivk(match_integer(ctx)), ctx.line);
}

static void match_ret(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_ret(), ctx.line);
}

static void match_snd(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_snd(), ctx.line);
}

static void match_rcv(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_rcv(match_integer(ctx)), ctx.line);
}

static void match_rmv(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_rmv(), ctx.line);
}

static void match_rwd(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_rwd(), ctx.line);
}

static void match_add(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_add(), ctx.line);
}

static void match_sub(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_sub(), ctx.line);
}

static void match_mul(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_mul(), ctx.line);
}

static void match_div(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_div(), ctx.line);
}

static void match_not(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_not(), ctx.line);
}

static void match_eq(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_eq(), ctx.line);
}

static void match_lt(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_lt(), ctx.line);
}

static void match_le(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_le(), ctx.line);
}

static void match_gt(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_gt(), ctx.line);
}

static void match_ge(amlc_ctx_t& ctx, amlc_prototype_ctx_t&)
{
    aasm_emit(ctx.a, ai_ge(), ctx.line);
}

static void match_variables(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    match(ctx, '[');
    skip_whitespace(ctx, true);
    while (*ctx.s != ']') {
        skip_whitespace(ctx, true);
        if (pctx.num_variables != 0) {
            match(ctx, ',');
            skip_whitespace(ctx, true);
        }
        pool_push_variable(
            ctx, pctx.locals, pctx.num_variables, match_symbol(ctx));
        skip_whitespace(ctx, true);
    }
    match(ctx, ']');
}

static void match_label(
    amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx, const std::string& literal)
{
    match(ctx, ':');
    skip_whitespace(ctx, true);
    auto pt = aasm_prototype(ctx.a);
    pool_push_label(
        ctx, pctx.locals, pt->num_instructions, literal);
}

static aint_t match_prototype(amlc_ctx_t& ctx);

static void match_nested_prototype(amlc_ctx_t& ctx, amlc_prototype_ctx_t& pctx)
{
    std::stringstream name;
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
    pctx.num_arguments = 0;
    pctx.num_variables = 0;

    match(ctx, '(');
    while (*ctx.s != ')') {
        skip_whitespace(ctx, true);
        if (pctx.num_arguments != 0) {
            match(ctx, ',');
            skip_whitespace(ctx, true);
        }
        pool_push_argument(
            ctx, pctx.locals, pctx.num_arguments, match_symbol(ctx));
        skip_whitespace(ctx, true);
    }
    skip_whitespace(ctx, true);
    match(ctx, ')');
    skip_whitespace(ctx, true);

    auto sym = match_symbol(ctx);
    if (sym == "var") {
        skip_whitespace(ctx, true);
        match_variables(ctx, pctx);
        skip_whitespace(ctx, true);
        sym = match_symbol(ctx);
    }

    do {
        skip_whitespace(ctx, true);
        if (sym == "end") {
            skip_whitespace(ctx, true);
            break;
        } else if (sym == "def") {
            match_nested_prototype(ctx, pctx);
            skip_whitespace(ctx, true);
            continue;
        }
        auto handler = ctx.opcode_handlers.find(sym);
        if (handler == ctx.opcode_handlers.end()) {
            if (*ctx.s == ':') {
                match_label(ctx, pctx, sym);
                skip_whitespace(ctx, true);
            } else {
                error(ctx, "unexpected symbol `%s`", sym.c_str());
            }
        } else {
            handler->second(ctx, pctx);
            skip_whitespace(ctx, true);
        }
        sym = match_symbol(ctx);
    } while (true);

    auto pt = aasm_prototype(ctx.a);
    auto cu = aasm_resolve(ctx.a);

    resolve_pseudo_jumps(
        ctx, cu.instructions,
        pctx.locals, pctx.label_addresses, pctx.pseudo_jumps);

    resolve_pseudo_nesteds(
        ctx, cu.instructions, pt->num_instructions,
        pctx.nesteds, pctx.pseudo_nesteds);

    pt->num_local_vars = pctx.num_variables;

    return (aint_t)pctx.num_arguments;
}

static void match_module_prototype(amlc_ctx_t& ctx)
{
    std::stringstream name;
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
    p->source = aasm_string_to_ref(ctx.a, ctx.filename.c_str());
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
    ADD_HANDLER(brk);
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
    ADD_HANDLER(add);
    ADD_HANDLER(sub);
    ADD_HANDLER(mul);
    ADD_HANDLER(div);
    ADD_HANDLER(not);
    ADD_HANDLER(eq);
    ADD_HANDLER(lt);
    ADD_HANDLER(le);
    ADD_HANDLER(gt);
    ADD_HANDLER(ge);

#undef  ADD_HANDLER

    match_module(ctx);
}
