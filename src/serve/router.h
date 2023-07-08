#ifndef TRAMA_ROUTER_H
#define TRAMA_ROUTER_H

#include <chttp/serve.h>
#include "template.h"
#include "../core/core.h"

typedef struct {
    trama_t *trama;
    trama_template_cache_t cache;
    char pool[65536];
} trama_router_t;

trama_status_t trama_router_init(trama_router_t *router, trama_t *trama);
void           trama_router_free(trama_router_t *router);
void trama_router_respond(trama_router_t *router, 
                          chttp_request_t *req, 
                          chttp_string_t req_body, 
                          chttp_response_t *res, 
                          chttp_responsebody_t *res_body);

#endif