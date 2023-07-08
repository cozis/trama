#ifndef CHTTP_REQUEST_H
#define CHTTP_REQUEST_H

#include <chttp/parse.h>
#include <chttp/BearSSL/bearssl.h>

#define CHTTP_STREAM_BUFFER_SIZE 256

typedef struct {
    bool using_ssl;
    
    // SSL stuff
    br_ssl_client_context sc;
    br_x509_minimal_context xc;
    unsigned char iobuf[BR_SSL_BUFSIZE_BIDI];
    br_sslio_context ioc;

    int fd;
    size_t bf_remaining;
    size_t fd_remaining;
    size_t total_bytes;
    char buffer[CHTTP_STREAM_BUFFER_SIZE];
} chttp_stream_t;

int  chttp_read(chttp_stream_t *stream, void *dst, size_t max);
void chttp_close(chttp_stream_t *stream);

typedef struct {
    chttp_error_t *err;
    size_t trust_anchors_num;
    const br_x509_trust_anchor *trust_anchors;
} chttp_request_config_t;

bool chttp_request(chttp_method_t method, const char *url, 
                   chttp_header_t *headers, size_t num_headers,
                   const char *req_body, size_t req_body_len,
                   chttp_response_t *res, chttp_stream_t *res_body, 
                   chttp_request_config_t *config);

#endif