#ifndef CHTTP_PARSE_H
#define CHTTP_PARSE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <chttp/error.h>

#define CHTTP_MAX_HEADERS 24

typedef struct {
    const char *str;
    size_t len;
} chttp_string_t;

#define CHTTP_STRING_NEW(s, l) \
    ((chttp_string_t) { .str=(s), .len=((int) (l)) < 0 ? (int) strlen(s) : (int) (l) })

#define CHTTP_STRING_FROM_LITERAL(s) \
    ((chttp_string_t) { .str=(s), .len=sizeof(s)-1 })

typedef enum {
    CHTTP_METHOD_GET,
    CHTTP_METHOD_PUT,
    CHTTP_METHOD_POST,
    CHTTP_METHOD_HEAD,
    CHTTP_METHOD_PATCH,
    CHTTP_METHOD_TRACE,
    CHTTP_METHOD_DELETE,
} chttp_method_t;

typedef enum {
    CHTTP_HOSTMODE_NAME,
    CHTTP_HOSTMODE_IPV4,
    CHTTP_HOSTMODE_IPV6,
} chttp_hostmode_t;

typedef uint32_t chttp_ipv4_t;
typedef struct {
    uint16_t data[8];
} chttp_ipv6_t;

typedef struct {
    chttp_hostmode_t mode;
    union {
        chttp_ipv4_t   ipv4;
        chttp_ipv6_t   ipv6;
        chttp_string_t name;
    };
    bool  no_port;
    uint16_t port;
} chttp_host_t;

typedef struct {
    chttp_host_t host;
    chttp_string_t path;
    chttp_string_t query;
    chttp_string_t schema;
    chttp_string_t fragment;
    chttp_string_t username;
    chttp_string_t password;
} chttp_url_t;

typedef struct {
    chttp_string_t name;
    chttp_string_t body;
} chttp_header_t;

typedef struct {
    int major, minor;
    chttp_url_t url;
    chttp_method_t method;
    chttp_header_t headers[CHTTP_MAX_HEADERS];
    size_t     num_headers;
} chttp_request_t;

typedef struct {
    int major, minor;
    int status;
    chttp_header_t headers[CHTTP_MAX_HEADERS];
    size_t     num_headers;
} chttp_response_t;

bool chttp_parse_ipv4(const char *src, size_t len, uint32_t *out, chttp_error_t *err);

bool chttp_parse_url(const char *src, size_t len, chttp_url_t *url, chttp_error_t *err);
bool chttp_parse_request(const char *src, size_t len, chttp_request_t *out, chttp_error_t *err);
bool chttp_parse_response(const char *src, size_t len, chttp_response_t *out, chttp_error_t *err);

chttp_header_t *chttp_get_request_header(chttp_request_t req, const char *name);
chttp_header_t *chttp_get_response_header(chttp_response_t res, const char *name);

bool chttp_get_request_content_length(chttp_request_t req, size_t *out, chttp_error_t *error);
bool chttp_get_response_content_length(chttp_response_t res, size_t *out, chttp_error_t *error);

bool chttp_string_match(chttp_string_t str1, const char *str2);

#endif /* CHTTP_PARSE_H */