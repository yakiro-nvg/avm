/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <ProgramOptions.hxx>
#include <fstream>
#include <vector>
#include <stdexcept>

#include <any/version.h>
#include <any/asm.h>
#include <any/scheduler.h>
#include <any/loader.h>
#include <any/actor.h>
#include <any/errno.h>
#include <any/std_io.h>
#include <any/std_string.h>
#include <any/std_buffer.h>

#include "compiler.h"

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
    throw std::logic_error(buf);
}

static std::string file_name_without_extension(const std::string& path)
{
    auto base_filename = path.substr(path.find_last_of("/\\") + 1);
    std::string::size_type const p(base_filename.find_last_of('.'));
    return base_filename.substr(0, p);
}

static void print_version()
{
    std::cout <<
        "amlc version 1.0" << ", " <<
        "target AVM " << AVERSION_MAJOR << "." << AVERSION_MINOR <<
        " [" << AVERSION_NAME << "]\n";
}

static void compile(const std::string& i, const std::string& o, bool verbose)
{
    std::cout << "compiling " << i << "\n";

    std::ifstream is;
    is.open(i, std::fstream::in);
    if (!is.is_open()) {
        error("failed to open `%s`", i.c_str());
    }
    is.seekg(0, std::fstream::end);
    auto sz = (size_t)is.tellg();
    is.seekg(0, std::fstream::beg);
    std::vector<char> buf;
    buf.resize(sz + 1);
    is.read(buf.data(), sz);
    buf[sz] = '\0';
    is.close();

    aasm_t a;
    aasm_init(&a, &myalloc, NULL);
    if (aasm_load(&a, NULL) != AERR_NONE) {
        error("failed to load aasm_t");
    }
    amlc_compile(&a, buf.data(), i.c_str(), verbose);
    aasm_save(&a);
    std::ofstream os;
    os.open(o, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    os.write((const char*)a.chunk, a.chunk_size);
    os.close();
    aasm_cleanup(&a);

    std::cout << "    -> " << o << "\n";
}

static void on_panic(aactor_t* a)
{
    aint_t ev_idx = any_check_index(a, any_count(a) - 1);
    if (any_type(a, ev_idx).type == AVT_STRING) {
        std::cout << "[" << ascheduler_pid(a->owner, a) << "] panic - " <<
            any_to_string(a, ev_idx) << "\n";
    } else {
        std::cout << "[" << ascheduler_pid(a->owner, a) << "] panic - " <<
            "unknown fatal error" << "\n";
    }
}

static void on_unresolved(const char* module, const char* name)
{
    std::cout << "unresolved import `" << module << ':' << name << "`\n";
}

static void execute(
    const std::string& module, const std::string& name,
    int8_t idx_bits, int8_t gen_bits, aint_t cstack_sz,
    const std::vector<std::string>& chunks)
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
        &s.loader, [](void*, const char* str) { std::cout << str; }, NULL);

    astd_lib_add_buffer(&s.loader);

    for (size_t i = 0; i < chunks.size(); ++i) {
        auto& c = chunks[i];
        std::ifstream is;
        is.open(c, std::fstream::in | std::fstream::binary);
        if (!is.is_open()) {
            error("failed to open `%s`", c.c_str());
        }
        is.seekg(0, std::fstream::end);
        auto sz = (size_t)is.tellg();
        is.seekg(0, std::fstream::beg);
        auto* chunk = (achunk_header_t*)myalloc(NULL, NULL, sz);
        is.read((char*)chunk, sz);
        is.close();
        std::cout << "add " << c << "\n";
        ec = aloader_add_chunk(&s.loader, chunk, sz, &myalloc, NULL);
        if (ec != AERR_NONE) {
            error("failed to add chunk %d", ec);
        }
    }

    ec = aloader_link(&s.loader, TRUE);
    if (ec != AERR_NONE) {
        error("failed to link %d", ec);
    }
    std::cout << "linking success\n";

    aactor_t* a;
    std::cout << "spawn " << module << ":" << name << "\n";
    ec = ascheduler_new_actor(&s, cstack_sz, &a);
    if (ec != AERR_NONE) {
        error("failed to create actor %d", ec);
    }
    std::cout << "    -> pid = " << ascheduler_pid(&s, a) << "\n";
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
            .callback([&] { std::cout << p << "\n"; });

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
                    error("input missing");
                }
                std::string o;
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
                    error("entry point missing");
                }
                auto sep = e.find_first_of(':');
                if (sep == std::string::npos) {
                    error(
                        ("bad entry `" + e + "`, missing `:`").c_str());
                }
                if (sep == e.length() - 1) {
                    error(
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
    } catch (const std::exception& e) {
        std::cerr << "uncaught exception: " << e.what() << "\n";
        return -1;
    }
}
