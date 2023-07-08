#ifndef TRAMA_TEMPLATE_H
#define TRAMA_TEMPLATE_H

#include "tinytemplate.h"
#include "../core/core.h"

#define TRAMA_MAX_TEMPLATE_CACHE 32
#define TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL 16

typedef struct {
    char    *name;
    char    *data;
    uint64_t time;
    tinytemplate_instr_t *program;
} trama_template_t;

typedef struct {
    size_t count;
    trama_template_t entries[TRAMA_MAX_TEMPLATE_CACHE];
} trama_template_cache_t;

typedef enum {
    TRAMA_TEMPLATE_PARAMTYPE_INT,
    TRAMA_TEMPLATE_PARAMTYPE_FLOAT,
    TRAMA_TEMPLATE_PARAMTYPE_STRING,
    TRAMA_TEMPLATE_PARAMTYPE_USER,
    TRAMA_TEMPLATE_PARAMTYPE_USERITER,
    TRAMA_TEMPLATE_PARAMTYPE_POST,
    TRAMA_TEMPLATE_PARAMTYPE_POSTITER,
    TRAMA_TEMPLATE_PARAMTYPE_GROUP,
    TRAMA_TEMPLATE_PARAMTYPE_GROUPITER,
} trama_template_paramtype_t;

typedef struct {
    const char *name;
    trama_template_paramtype_t type;
    union {
        trama_user_t  iter_user;
        trama_post_t  iter_post;
        trama_group_t iter_group;
    };
    union {
        int64_t     as_int;
        double      as_float;
        const char *as_string;
        trama_user_t  *as_user;
        trama_post_t  *as_post;
        trama_group_t *as_group;
        trama_useriter_t  *as_useriter;
        trama_postiter_t  *as_postiter;
        trama_groupiter_t *as_groupiter;
    };
} trama_template_param_t;

typedef struct {
    size_t count;
    bool  failed;
    trama_template_param_t array[TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL];
} trama_template_params_t;

void trama_template_cache_init(trama_template_cache_t *cache);
void trama_template_cache_free(trama_template_cache_t *cache);

void trama_template_params_init(trama_template_params_t *params);
void trama_template_params_free(trama_template_params_t *params);
void trama_template_params_setUser(trama_template_params_t *params, const char *name, trama_user_t *user);
void trama_template_params_setPost(trama_template_params_t *params, const char *name, trama_post_t *post);
void trama_template_params_setGroup(trama_template_params_t *params, const char *name, trama_group_t *group);
void trama_template_params_setUserIter(trama_template_params_t *params, const char *name, trama_useriter_t *iter);
void trama_template_params_setPostIter(trama_template_params_t *params, const char *name, trama_postiter_t *iter);
void trama_template_params_setGroupIter(trama_template_params_t *params, const char *name, trama_groupiter_t *iter);
void trama_template_params_setString(trama_template_params_t *params, const char *name, const char *str);
void trama_template_params_setInteger(trama_template_params_t *params, const char *name, int64_t value);
void trama_template_params_setFloating(trama_template_params_t *params, const char *name, double value);

trama_template_t *trama_template_load(trama_template_cache_t *cache, const char *file, char *errmsg, size_t errmax);
int               trama_template_eval(trama_template_t *template, trama_template_params_t *params, char *dst, size_t max, char *errmsg, size_t errmax);

#endif