#ifndef TRAMA_SERVE_H
#define TRAMA_SERVE_H

#include <chttp/serve.h>
#include "router.h"
#include "../core/core.h"

typedef chttp_server_handle_t trama_serve_handle_t;
void trama_serve(trama_t *trama, const char *addr, uint16_t port, trama_serve_handle_t *handle);
void trama_serve_stop(trama_serve_handle_t *handle);

#endif