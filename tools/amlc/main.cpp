/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <ProgramOptions.hxx>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <memory>

#include <any/version.h>
#include <any/asm.h>
#include <any/scheduler.h>
#include <any/db.h>
#include <any/loader.h>
#include <any/actor.h>
#include <any/errno.h>
#include <any/std.h>
#include <any/std_io.h>
#include <any/std_string.h>
#include <any/std_buffer.h>
#include <any/std_buffer_bits.h>
#include <any/std_array.h>
#include <any/std_tuple.h>
#include <any/std_table.h>

#if defined(ALINUX) || defined(AAPPLE)
#include <unistd.h>
#endif

#include "compiler.h"

struct address_t
{
    std::string host;
    uint16_t port;
};

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

    std::cout << " -> " << o << "\n";
}

static std::string pid_string(aactor_t* a, apid_t pid)
{
    std::stringstream ss;
    apid_idx_t idx = apid_idx(a->owner->idx_bits, pid);
    apid_gen_t gen = apid_gen(a->owner->idx_bits, a->owner->gen_bits, pid);
    ss << "<" << idx << "." << gen << ">";
    return ss.str();
}

static void on_panic(aactor_t* a, void*)
{
    aint_t ev_idx = any_top(a);
    std::cout << pid_string(a, ascheduler_pid(a->owner, a)) << " ";
    if (a->frame->pt) {
        aprototype_t& chunk = a->frame->pt->chunk->prototypes[0];
        std::cout << chunk.strings + chunk.header->source << ":";
        if (a->frame->ip < a->frame->pt->header->num_instructions) {
            std::cout << a->frame->pt->source_lines[a->frame->ip] << " ";
        } else {
            std::cout << a->frame->pt->source_lines[a->frame->ip - 1] << " ";
        }
    }
    std::cout << "panic - ";
    if (any_type(a, ev_idx).type == AVT_STRING) {
        std::cout << any_to_string(a, ev_idx) << "\n";
    } else {
        std::cout << "unknown fatal error\n";
    }
}

static void on_unresolved(
    aloader_t*, const char* module, const char* name, void*)
{
    std::cout << "unresolved import `" << module << ':' << name << "`\n";
}

static void execute(
    const std::string& module, const std::string& name,
    int8_t idx_bits, int8_t gen_bits, aint_t cstack_sz,
    const std::vector<std::string>& chunks,
    address_t* debug, int32_t max_conns, bool alive,
    int32_t realtime_resolution)
{
    aerror_t ec;
    ascheduler_t s;
    adb_t db;

    ec = ascheduler_init(&s, idx_bits, gen_bits, &myalloc, NULL);
    if (ec != AERR_NONE) {
        error("failed to init scheduler %d", ec);
    }
    ascheduler_on_panic(&s, &on_panic, NULL);
    aloader_on_unresolved(&s.loader, &on_unresolved, NULL);

    if (debug) {
        ec = adb_init(
            &db, &myalloc, NULL, &s,
            debug->host.c_str(), debug->port, (aint_t)max_conns);
        if (ec != AERR_NONE) {
            error("failed to init debug service %d", ec);
        } else {
            std::cout <<
                "debug service attached\n" <<
                " -> at " << debug->host << ":" << debug->port << "\n";
        }
    }

    astd_lib_add(&s.loader);
    astd_lib_add_io(
        &s.loader, [](void*, const char* str) { std::cout << str; }, NULL);
    astd_lib_add_string(&s.loader);
    astd_lib_add_buffer(&s.loader);
	astd_lib_add_buffer_bits(&s.loader);
    astd_lib_add_array(&s.loader);
    astd_lib_add_tuple(&s.loader);
    astd_lib_add_table(&s.loader);

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
    std::cout << " -> pid = " << pid_string(a, ascheduler_pid(&s, a)) << "\n";
    any_import(a, module.c_str(), name.c_str());
    ascheduler_start(&s, a, 0);

    while (alive || ascheduler_num_processes(&s) > 0) {
        if (debug) adb_run_once(&db);
        ascheduler_run_once(&s);
#ifdef AWINDOWS
        Sleep(std::max<DWORD>(realtime_resolution/1000, 1));
#else
        usleep((useconds_t)realtime_resolution);
#endif
    }

    if (debug) adb_cleanup(&db);
    ascheduler_cleanup(&s);
}

int entry(int argc, char** argv)
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

        p["debug"]
            .abbreviation('d')
            .description("run with debug service at host:port")
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

        p["max_conns"]
            .description("maximum number of debug connections")
            .type(po::i32)
            .fallback(64);

        p["alive"]
            .description("keep AMLC execution session alive")
            .type(po::void_);

        p["rt_res"]
            .description("real-time resolution hint in us")
            .type(po::i32)
            .fallback(1);

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
                std::unique_ptr<address_t> debug;
                if (p["debug"].was_set()) {
                    debug.reset(new address_t());
                    auto d = p["debug"].get().string;
                    auto colon = d.find_first_of(':');
                    debug->host = d.substr(0, colon);
                    auto port_str = d.substr(colon + 1);
                    debug->port = (uint16_t)atoi(port_str.c_str());
                }
                execute(
                    e.substr(0, sep), e.substr(sep + 1),
                    (int8_t)p["idx_bits"].get().i32,
                    (int8_t)p["gen_bits"].get().i32,
                    (aint_t)p["cstack_sz"].get().i32,
                    p[""].to_vector<po::string>(),
                    debug.get(), p["max_conns"].get().i32,
                    p["alive"].was_set(),
                    p["rt_res"].get().i32);
            }
            return 0;
        }
    } catch (const std::exception& e) {
        std::cerr << "uncaught exception: " << e.what() << "\n";
        return -1;
    }
}

int main(int argc, char** argv)
{
#ifdef AWINDOWS
    WSADATA wsa_data;
    int ws_err = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ws_err != 0) {
        error("WSAStartup failed %d", ws_err);
    }
#endif
    entry(argc, argv);
#ifdef AWINDOWS
    WSACleanup();
#endif
}
