/*
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
*/
#include "serve.h"

static void callback(chttp_request_t *req, 
                     chttp_string_t req_body,
                     chttp_response_t *res, 
                     chttp_responsebody_t *res_body,
                     void *userp)
{
    trama_router_t *router = userp;
    trama_router_respond(router, req, req_body, res, res_body);
}

void trama_serve_stop(trama_serve_handle_t *handle)
{
    chttp_quit(handle);
}

void trama_serve(trama_t *trama, const char *addr, uint16_t port,
                 trama_serve_handle_t *handle)
{
    trama_router_t router;
    trama_router_init(&router, trama);
    
    chttp_error_t error;
    chttp_server_config_t config = chttp_get_default_server_configs();
    bool ok = chttp_serve(addr, port, callback, &router, handle, config, &error);
    if (!ok)
        trama_log(trama, "%s", error.msg);

    trama_router_free(&router);
}