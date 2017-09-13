/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#include <any/db.h>

#include <any/scheduler.h>

#define REQUEST_BUFF_SZ 2048
#define IO_BUFF_SZ 8192

static AINLINE void*
aalloc(
    adb_t* self, void* old, const aint_t sz)
{
    return self->alloc(self->alloc_ud, old, sz);
}

static int
dispatch(
    struct wby_con* con, void* ud)
{
    // TODO
    return 0;
}

static int
websocket_connect(
    struct wby_con* con, void* ud)
{
    // TODO
    return 0;
}

static void
websocket_connected(
    struct wby_con* con, void* ud)
{
    // TODO
}

static int
websocket_frame(
    struct wby_con* con, const struct wby_frame* frame, void* ud)
{
    // TODO
    return 0;
}

static void
websocket_closed(
    struct wby_con* con, void* ud)
{
    // TODO
}

static void
on_throw(
    aactor_t* a, void* ud)
{
    // TODO
}

static void
on_spawn(
    aactor_t* a, void* ud)
{
    // TODO
}

static void
on_exit(
    aactor_t* a, void* ud)
{
    // TODO
}

static int32_t
on_step(
    aactor_t* a, void* ud)
{
    // TODO
    return TRUE;
}

static void
config_callback(
    struct wby_config* cfg)
{
    cfg->dispatch     = &dispatch;
    cfg->ws_connect   = &websocket_connect;
    cfg->ws_connected = &websocket_connected;
    cfg->ws_frame     = &websocket_frame;
    cfg->ws_closed    = &websocket_closed;
}

static void
config(
    struct wby_config* cfg, void* ud,
    const char* address, uint16_t port, aint_t max_connections)
{
    cfg->userdata = ud;
    cfg->address = address;
    cfg->port = port;
    cfg->connection_max = (unsigned int)max_connections;
    cfg->request_buffer_size = REQUEST_BUFF_SZ;
    cfg->io_buffer_size = IO_BUFF_SZ;
    config_callback(cfg);
}

static void
attach(
    ascheduler_t* target, adb_t* db)
{
    ascheduler_on_throw(target, &on_throw, db);
    ascheduler_on_spawn(target, &on_spawn, db);
    ascheduler_on_exit (target, &on_exit,  db);
    ascheduler_on_step (target, &on_step,  db);
}

static void
detach(
    ascheduler_t* target)
{
    ascheduler_on_throw(target, NULL, NULL);
    ascheduler_on_spawn(target, NULL, NULL);
    ascheduler_on_exit (target, NULL, NULL);
    ascheduler_on_step (target, NULL, NULL);
}

aerror_t
adb_init(
    adb_t* self, aalloc_t alloc, void* alloc_ud, ascheduler_t* target,
    const char* address, uint16_t port, aint_t max_connections)
{
    aerror_t ec;
    wby_size wby_buff_sz;
    struct wby_config cfg;
    memset(&cfg, 0, sizeof(cfg));
    config(&cfg, self, address, port, max_connections);
    memset(self, 0, sizeof(adb_t));
    self->alloc = alloc;
    self->alloc_ud = alloc_ud;
    self->target = target;
    wby_init(&self->wby, &cfg, &wby_buff_sz);
    self->wby_buff = aalloc(self, NULL, (aint_t)wby_buff_sz);
    if (self->wby_buff == NULL) {
        ec = AERR_FULL;
        goto failed;
    }
    if (wby_start(&self->wby, self->wby_buff) != 0) {
        ec = AERR_RUNTIME;
        goto failed;
    }
    attach(target, self);
    return AERR_NONE;
failed:
    if (self->wby_buff) {
        aalloc(self, self->wby_buff, 0);
        self->wby_buff = NULL;
    }
    return ec;
}

void
adb_cleanup(
    adb_t* self)
{
    if (self->wby_buff == NULL) return;
    detach(self->target);
    wby_stop(&self->wby);
    aalloc(self, self->wby_buff, 0);
    self->wby_buff = NULL;
}

void
adb_run_once(
    adb_t* self)
{
    wby_update(&self->wby, FALSE);
}