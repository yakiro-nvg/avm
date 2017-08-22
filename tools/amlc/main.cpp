/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <ProgramOptions.hxx>
#include <fstream>
#include <vector>

#include <any/version.h>
#include <any/asm.h>
#include <any/scheduler.h>
#include <any/loader.h>
#include <any/actor.h>
#include <any/errno.h>
#include <any/std_libs.h>
#include <any/gc_string.h>

#include "compiler.h"

using namespace std;

static void* myalloc(void*, void* old, aint_t sz)
{
    return realloc(old, (size_t)sz);
}

static void error(const char* fmt, ...)
{
    va_list args;
    char buf[512];
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    throw exception(buf);
}

static string file_name_without_extension(const string& path)
{
    auto base_filename = path.substr(path.find_last_of("/\\") + 1);
    string::size_type const p(base_filename.find_last_of('.'));
    return base_filename.substr(0, p);
}

static void print_version()
{
    cout <<
        "amlc version 1.0" << ", " <<
        "target AVM " << AVERSION_MAJOR << "." << AVERSION_MINOR <<
        " [" << AVERSION_NAME << "]\n";
}

static void compile(const string& i, const string& o, bool verbose)
{
    cout << "compiling " << i << "\n";

    ifstream is;
    is.open(i, fstream::in);
    if (!is.is_open()) {
        error("failed to open `%s`", i);
    }
    is.seekg(0, fstream::end);
    auto sz = (size_t)is.tellg();
    is.seekg(0, fstream::beg);
    vector<char> buf;
    buf.resize(sz + 1);
    is.read(buf.data(), sz);
    buf[sz] = NULL;
    is.close();

    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    if (aasm_load(&a, NULL) != AERR_NONE) {
        error("failed to load aasm_t");
    }
    amlc_compile(&a, buf.data(), i.c_str(), verbose);
    aasm_save(&a);
    ofstream os;
    os.open(o, fstream::out | fstream::binary | fstream::trunc);
    os.write((const char*)a.chunk, a.chunk_size);
    os.close();
    aasm_cleanup(&a);

    cout << "    -> " << o << "\n";
}

static void on_panic(aactor_t* a)
{
    aint_t ev_idx = any_count(a) - 1;
    if (any_type(a, ev_idx).type == AVT_STRING) {
        cout << "[" << ascheduler_pid(a->owner, a) << "] panic - " <<
            any_to_string(a, ev_idx) << "\n";
    } else {
        cout << "[" << ascheduler_pid(a->owner, a) << "] panic - " <<
            "unknown fatal error" << "\n";
    }
}

static void on_unresolved(const char* module, const char* name)
{
    cout << "unresolved import `" << module << ':' << name << "`\n";
}

static void execute(
    const string& module, const string& name,
    int8_t idx_bits, int8_t gen_bits, aint_t cstack_sz,
    const vector<string>& chunks)
{
    ascheduler_t s;
    aerror_t ec;

    ec = ascheduler_init(&s, idx_bits, gen_bits, &myalloc, NULL);
    if (ec != AERR_NONE) {
        error("failed to init scheduler %d", ec);
    }
    ascheduler_on_panic(&s, &on_panic);
    aloader_on_unresolved(&s.loader, &on_unresolved);

    astd_lib_add_io(
        &s.loader, [](void*, const char* str) { cout << str; }, NULL);

    for (size_t i = 0; i < chunks.size(); ++i) {
        auto& c = chunks[i];
        ifstream is;
        is.open(c, fstream::in | fstream::binary);
        if (!is.is_open()) {
            error("failed to open `%s`", c.c_str());
        }
        is.seekg(0, fstream::end);
        auto sz = (size_t)is.tellg();
        is.seekg(0, fstream::beg);
        auto* chunk = (achunk_header_t*)myalloc(NULL, NULL, sz);
        is.read((char*)chunk, sz);
        is.close();
        cout << "add " << c << "\n";
        ec = aloader_add_chunk(&s.loader, chunk, sz, &myalloc, NULL);
        if (ec != AERR_NONE) {
            error("failed to add chunk %d", ec);
        }
    }

    ec = aloader_link(&s.loader, TRUE);
    if (ec != AERR_NONE) {
        error("failed to link %d", ec);
    }
    cout << "linking success\n";

    aactor_t* a;
    cout << "spawn " << module << ":" << name << "\n";
    ec = ascheduler_new_actor(&s, cstack_sz, &a);
    if (ec != AERR_NONE) {
        error("failed to create actor %d", ec);
    }
    cout << "    -> pid = " << ascheduler_pid(&s, a) << "\n";
    any_find(a, module.c_str(), name.c_str());
    ascheduler_start(&s, a, 0);

    while (ascheduler_num_processes(&s) > 0) {
        ascheduler_run_once(&s);
    }

    ascheduler_cleanup(&s);
}

int main(int argc, char** argv)
{
    try {
        po::parser p;

        p["help"]
            .abbreviation('h')
            .description("print this help screen")
            .type(po::void_)
            .callback([&] { cout << p << "\n"; });

        p["version"]
            .abbreviation('v')
            .description("display compiler version information")
            .type(po::void_)
            .callback(&print_version);

        p["compile"]
            .abbreviation('c')
            .description("compile an AML source file")
            .type(po::string);

        p["execute"]
            .abbreviation('e')
            .description("run with entry point")
            .type(po::string);

        p["verbose"]
            .abbreviation('V')
            .description("display debugging log")
            .type(po::void_);

        p["output"]
            .abbreviation('o')
            .description("place the output into file")
            .type(po::string);

        p["idx_bits"]
            .description("number of index bits")
            .type(po::i32)
            .fallback(8);

        p["gen_bits"]
            .description("number of generation bits")
            .type(po::i32)
            .fallback(24);

        p["cstack_sz"]
            .description("size of entry point native stack in bytes")
            .type(po::i32)
            .fallback(4096000);

        p[""];

        if (!p(argc, argv)) {
            return 1;
        } else {
            if (p["compile"].was_set()) {
                auto i = p["compile"].get().string;
                if (i.length() <= 0) {
                    throw exception("input missing");
                }
                string o;
                if (p["output"].was_set()) {
                    o = p["output"].get().string;
                } else {
                    o = file_name_without_extension(i) + ".avmc";
                }
                compile(i, o, p["verbose"].available());
            }
            if (p["execute"].was_set()) {
                auto e = p["execute"].get().string;
                if (e.length() <= 0) {
                    throw exception("entry point missing");
                }
                auto sep = e.find_first_of(':');
                if (sep == string::npos) {
                    throw exception(
                        ("bad entry `" + e + "`, missing `:`").c_str());
                }
                if (sep == e.length() - 1) {
                    throw exception(
                        ("bad entry `" + e + "`, missing function").c_str());
                }
                execute(
                    e.substr(0, sep), e.substr(sep + 1),
                    (int8_t)p["idx_bits"].get().i32,
                    (int8_t)p["gen_bits"].get().i32,
                    (aint_t)p["cstack_sz"].get().i32,
                    p[""].to_vector<po::string>());
            }
            return 0;
        }
    } catch (const exception& e) {
        cerr << "uncaught exception: " << e.what() << "\n";
        return -1;
    }
}