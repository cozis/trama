#ifndef CHTTP_SERVE_H
#define CHTTP_SERVE_H

#include <chttp/parse.h>

typedef void *chttp_server_handle_t;

typedef struct {
    void  *data;
    size_t size;
    _Bool close;
} chttp_responsebody_t;

typedef struct {
    _Bool        reuse_addr;
    unsigned int max_parallel;
    unsigned int backlog;
} chttp_server_config_t;

typedef void (*chttp_servefn_t)(chttp_request_t *req, chttp_string_t req_body,
                                chttp_response_t *res, chttp_responsebody_t *res_body,
                                void *userp);

bool chttp_serve(const char *addr, uint16_t port, 
                 chttp_servefn_t callback, void *userp, 
                 chttp_server_handle_t *handle, 
                 chttp_server_config_t config,
                 chttp_error_t *error);

void chttp_quit(chttp_server_handle_t handle);
chttp_server_config_t chttp_get_default_server_configs();
void chttp_append_response_header(chttp_response_t *res, const char *name, const char *body);
void chttp_append_response_header_format(chttp_response_t *res, const char *name, const char *format, ...);
int chttp_pathcmp(chttp_string_t path, const char *fmt, ...);
int chttp_vpathcmp(chttp_string_t path, const char *fmt, va_list va);

#endif