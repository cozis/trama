#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "template.h"

void 
trama_template_params_init(trama_template_params_t *params)
{
    params->count = 0;
    params->failed = false;
}

void 
trama_template_params_free(trama_template_params_t *params)
{
    (void) params;
    // Nothing to be done!
}

void 
trama_template_params_setUser(trama_template_params_t *params, 
                              const char *name, trama_user_t *user)
{
    if (params->failed)
        return;

    if (params->count == TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL) {
        params->failed = true;
        return;
    }

    trama_template_param_t *param = params->array + params->count;
    param->name = name;
    param->type = TRAMA_TEMPLATE_PARAMTYPE_USER;
    param->as_user = user;
    params->count++;
}

void 
trama_template_params_setPost(trama_template_params_t *params, 
                              const char *name, trama_post_t *post)
{
    if (params->failed)
        return;

    if (params->count == TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL) {
        params->failed = true;
        return;
    }

    trama_template_param_t *param = params->array + params->count;
    param->name = name;
    param->type = TRAMA_TEMPLATE_PARAMTYPE_POST;
    param->as_post = post;
    params->count++;
}

void 
trama_template_params_setGroup(trama_template_params_t *params, 
                               const char *name, trama_group_t *group)
{
    if (params->failed)
        return;

    if (params->count == TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL) {
        params->failed = true;
        return;
    }

    trama_template_param_t *param = params->array + params->count;
    param->name = name;
    param->type = TRAMA_TEMPLATE_PARAMTYPE_GROUP;
    param->as_group = group;
    params->count++;
}

void 
trama_template_params_setUserIter(trama_template_params_t *params, 
                                  const char *name, trama_useriter_t *iter)
{
    if (params->failed)
        return;

    if (params->count == TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL) {
        params->failed = true;
        return;
    }

    trama_template_param_t *param = params->array + params->count;
    param->name = name;
    param->type = TRAMA_TEMPLATE_PARAMTYPE_USERITER;
    param->as_useriter = iter;
    params->count++;
}

void 
trama_template_params_setPostIter(trama_template_params_t *params, 
                                  const char *name, trama_postiter_t *iter)
{
    if (params->failed)
        return;

    if (params->count == TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL) {
        params->failed = true;
        return;
    }

    trama_template_param_t *param = params->array + params->count;
    param->name = name;
    param->type = TRAMA_TEMPLATE_PARAMTYPE_POSTITER;
    param->as_postiter = iter;
    params->count++;
}

void 
trama_template_params_setGroupIter(trama_template_params_t *params, 
                                   const char *name, trama_groupiter_t *iter)
{
    if (params->failed)
        return;

    if (params->count == TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL) {
        params->failed = true;
        return;
    }

    trama_template_param_t *param = params->array + params->count;
    param->name = name;
    param->type = TRAMA_TEMPLATE_PARAMTYPE_GROUPITER;
    param->as_groupiter = iter;
    params->count++;
}

void 
trama_template_params_setInteger(trama_template_params_t *params, 
                                 const char *name, int64_t value)
{
    if (params->failed)
        return;

    if (params->count == TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL) {
        params->failed = true;
        return;
    }

    trama_template_param_t *param = params->array + params->count;
    param->name = name;
    param->type = TRAMA_TEMPLATE_PARAMTYPE_INT;
    param->as_int = value;
    params->count++;
}

void 
trama_template_params_setFloating(trama_template_params_t *params, 
                                  const char *name, double value)
{
    if (params->failed)
        return;

    if (params->count == TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL) {
        params->failed = true;
        return;
    }

    trama_template_param_t *param = params->array + params->count;
    param->name = name;
    param->type = TRAMA_TEMPLATE_PARAMTYPE_FLOAT;
    param->as_float = value;
    params->count++;
}

void 
trama_template_params_setString(trama_template_params_t *params, 
                                const char *name, const char *str)
{
    if (params->failed)
        return;

    if (params->count == TRAMA_TEMPLATE_MAX_PARAMS_PER_EVAL) {
        params->failed = true;
        return;
    }

    trama_template_param_t *param = params->array + params->count;
    param->name = name;
    param->type = TRAMA_TEMPLATE_PARAMTYPE_STRING;
    param->as_string = str;
    params->count++;
}

void
trama_template_cache_init(trama_template_cache_t *cache)
{
    cache->count = 0;
}

void
trama_template_cache_free(trama_template_cache_t *cache)
{
    for (size_t i = 0; i < cache->count; i++) {
        free(cache->entries[i].name);
        free(cache->entries[i].program);
    }
}

static bool 
load_file(const char *file, 
          char **name, char **body,
          char *errmsg, size_t errmax)
{
    FILE *stream = fopen(file, "rb");
    if (stream == NULL) {
        snprintf(errmsg, errmax, "Couldn't open file");
        return false;
    }

    fseek(stream, 0, SEEK_END);
    long temp = ftell(stream);
    fseek(stream, 0, SEEK_SET);
    size_t body_len = temp;

    size_t name_len = strlen(file);
    void *memory = malloc(name_len + body_len + 2);
    if (memory == NULL) {
        snprintf(errmsg, errmax, "Out of memory");
        fclose(stream);
        return false;
    }
    
    *name = memory;
    *body = *name + name_len + 1;

    strcpy(*name, file);
    if (fread(*body, 1, body_len, stream) != body_len || ferror(stream)) {
        snprintf(errmsg, errmax, "Failed to read file");
        fclose(stream);
        free(memory);
        return false;
    }
    (*body)[body_len] = '\0';
    return true;
}

static bool 
get_last_modification_time(const char *file, uint64_t *time, 
                           char *errmsg, size_t errmax)
{
    int code;
#ifndef _WIN32
    struct stat buffer;
    code = stat(file, &buffer);
#else
    struct _stat buffer;
    code = _stat(file, &buffer);
#endif

    if(code) {
        snprintf(errmsg, errmax, "Couldn't query [%s] for last modification time", file);
        return false;
    }
    *time = buffer.st_mtime;
    return true;
}

static trama_template_t*
find_template(trama_template_cache_t *cache,
              const char *file)
{
    for (size_t i = 0; i < cache->count; i++) {
        trama_template_t *entry = cache->entries + i;
        if (!strcmp(file, entry->name))
            return entry;
    }
    return NULL;
}

trama_template_t*
trama_template_load(trama_template_cache_t *cache, 
                    const char *file, 
                    char *errmsg, size_t errmax)
{
    trama_template_t *entry = find_template(cache, file);
    if (entry) {
        // The template is already in the cache.
        // If it wasn't modified since being loaded,
        // serve it, else evict it.

        char msg[256];
        uint64_t last_modification_time;
        if (!get_last_modification_time(entry->name, &last_modification_time, msg, sizeof(msg))) {
            fprintf(stderr, "%s\n", msg); // TODO: Log in a proper way
            last_modification_time = entry->time;
        }

        if (last_modification_time < entry->time) {
            // Template is nice and fresh
            fprintf(stderr, "Serving cached version of %s\n", file);
            return entry;
        }

        fprintf(stderr, "Evicting %s\n", file);
        // Remove this entry
        free(entry->name);
        free(entry->program);
        *entry = cache->entries[cache->count-1];
        cache->count--;
    }

    if (cache->count == TRAMA_MAX_TEMPLATE_CACHE) {
        snprintf(errmsg, errmax, "Cache entry limit reached");
        return NULL;
    }

    entry = cache->entries + cache->count;
    if (!load_file(file, &entry->name, &entry->data, errmsg, errmax))
        return NULL;

    size_t size = 128;
    tinytemplate_instr_t *program = malloc(size * sizeof(tinytemplate_instr_t));
    if (program == NULL) {
        snprintf(errmsg, errmax, "Out of memory");
        free(entry->name);
        return NULL;
    }

    for (bool done = false; !done;) {
        tinytemplate_status_t status;
        status = tinytemplate_compile(entry->data, strlen(entry->data), 
                                      program, size, NULL, errmsg, errmax);
        if (status == TINYTEMPLATE_STATUS_DONE)
            done = true;
        else if (status == TINYTEMPLATE_STATUS_EMEMORY) {
            size *= 2;
            void *temp = realloc(program, size * sizeof(tinytemplate_instr_t));
            if (temp == NULL) {
                snprintf(errmsg, errmax, "Out of memory");
                free(entry->name);
                free(program);
                return NULL;
            }
            program = temp;
        } else {
            // Error message was already reported
            free(entry->name);
            free(program);
            return NULL;
        }
    }
    entry->time = time(NULL);
    entry->program = program;
    cache->count++;
    return entry;
}

typedef struct {
    char  *dst;
    size_t max;
    size_t out;
    trama_template_params_t *params;
} eval_context_t;

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

static void
append_output(void *data, const char *src, size_t len)
{
    eval_context_t *context = data;
    size_t copying = MIN(len, context->max - context->out);
    memcpy(context->dst + context->out, src, copying);
    context->out += copying;
}

static bool 
get_property(void *data, const char *key, size_t len,
             tinytemplate_type_t *type, 
             tinytemplate_value_t *value);

static bool 
next_item(void *data, tinytemplate_type_t *type, 
          tinytemplate_value_t *value)
{
    trama_template_param_t *param = data;
    
    switch (param->type) {
        
        case TRAMA_TEMPLATE_PARAMTYPE_USERITER:
        if (trama_nextUser(param->as_useriter, &param->iter_user) != TRAMA_STATUS_OK)
            return false;
        *type = TINYTEMPLATE_TYPE_DICT;
        value->as_dict.data = param;
        value->as_dict.get  = get_property;
        return true;

        case TRAMA_TEMPLATE_PARAMTYPE_POSTITER:
        if (trama_nextPost(param->as_postiter, &param->iter_post) != TRAMA_STATUS_OK)
            return false;
        *type = TINYTEMPLATE_TYPE_DICT;
        value->as_dict.data = param;
        value->as_dict.get  = get_property;
        return true;
        
        case TRAMA_TEMPLATE_PARAMTYPE_GROUPITER:
        if (trama_nextGroup(param->as_groupiter, &param->iter_group) != TRAMA_STATUS_OK)
            return false;
        *type = TINYTEMPLATE_TYPE_DICT;
        value->as_dict.data = param;
        value->as_dict.get  = get_property;
        return true;

        default:
        break;
    }
    return false;
}

static bool
get_property_of_group(trama_group_t *group, 
                      const char *key, size_t len,
                      tinytemplate_type_t *type, 
                      tinytemplate_value_t *value)
{
    if (len == 4 && !strncmp(key, "name", len)) {
        *type = TINYTEMPLATE_TYPE_STRING;
        value->as_string.str = group->name;
        value->as_string.len = strlen(group->name);
        return true;
    }
    if (len == 4 && !strncmp(key, "desc", len)) {
        *type = TINYTEMPLATE_TYPE_STRING;
        value->as_string.str = group->desc;
        value->as_string.len = strlen(group->desc);
        return true;
    }
    return false;
}

static bool
get_property_of_user(trama_user_t *user, 
                     const char *key, size_t len,
                     tinytemplate_type_t *type, 
                     tinytemplate_value_t *value)
{
    if (len == 4 && !strncmp(key, "name", len)) {
        *type = TINYTEMPLATE_TYPE_STRING;
        value->as_string.str = user->name;
        value->as_string.len = strlen(user->name);
        return true;
    }
    if (len == 4 && !strncmp(key, "pass", len)) {
        *type = TINYTEMPLATE_TYPE_STRING;
        value->as_string.str = user->pass;
        value->as_string.len = strlen(user->pass);
        return true;
    }
    return false;
}

static bool
get_property_of_post(trama_post_t *post, 
                     const char *key, size_t len,
                     tinytemplate_type_t *type, 
                     tinytemplate_value_t *value)
{
    if (len == 2 && !strncmp(key, "id", len)) {
        *type = TINYTEMPLATE_TYPE_INT;
        value->as_int = post->id;
        return true;
    }
    if (len == 5 && !strncmp(key, "group", len)) {
        *type = TINYTEMPLATE_TYPE_STRING;
        value->as_string.str = post->group;
        value->as_string.len = strlen(post->group);
        return true;
    }
    if (len == 5 && !strncmp(key, "title", len)) {
        *type = TINYTEMPLATE_TYPE_STRING;
        value->as_string.str = post->title;
        value->as_string.len = strlen(post->title);
        return true;
    }
    if (len == 6 && !strncmp(key, "author", len)) {
        *type = TINYTEMPLATE_TYPE_STRING;
        value->as_string.str = post->author;
        value->as_string.len = strlen(post->author);
        return true;
    }
    if (len == 4 && !strncmp(key, "body", len)) {
        *type = TINYTEMPLATE_TYPE_STRING;
        value->as_string.str = post->body;
        value->as_string.len = post->size;
        return true;
    }
    if (len == 6 && !strncmp(key, "upvote", len)) {
        *type = TINYTEMPLATE_TYPE_INT;
        value->as_int = post->upvote;
        return true;
    }
    if (len == 8 && !strncmp(key, "downvote", len)) {
        *type = TINYTEMPLATE_TYPE_INT;
        value->as_int = post->downvote;
        return true;
    }
    return false;
}

static bool 
get_property(void *data, const char *key, size_t len,
             tinytemplate_type_t *type, 
             tinytemplate_value_t *value)
{
    trama_template_param_t *param = data;
    switch (param->type) {

        case TRAMA_TEMPLATE_PARAMTYPE_INT:
        *type = TINYTEMPLATE_TYPE_INT;
        value->as_int = param->as_int;
        break;

        case TRAMA_TEMPLATE_PARAMTYPE_FLOAT:
        *type = TINYTEMPLATE_TYPE_FLOAT;
        value->as_float = param->as_float;
        break;

        case TRAMA_TEMPLATE_PARAMTYPE_STRING:
        *type = TINYTEMPLATE_TYPE_STRING;
        value->as_string.str = param->as_string;
        value->as_string.len = strlen(param->as_string);
        break;

        case TRAMA_TEMPLATE_PARAMTYPE_USERITER:  return get_property_of_user(&param->iter_user,  key, len, type, value);
        case TRAMA_TEMPLATE_PARAMTYPE_POSTITER:  return get_property_of_post(&param->iter_post,  key, len, type, value);
        case TRAMA_TEMPLATE_PARAMTYPE_GROUPITER: return get_property_of_group(&param->iter_group, key, len, type, value);
        case TRAMA_TEMPLATE_PARAMTYPE_USER:  return get_property_of_user(param->as_user, key, len, type, value);
        case TRAMA_TEMPLATE_PARAMTYPE_POST:  return get_property_of_post(param->as_post, key, len, type, value);
        case TRAMA_TEMPLATE_PARAMTYPE_GROUP: return get_property_of_group(param->as_group, key, len, type, value);
        
        default:
        break;
    }
    return false;
}

static bool 
get_param(void *data, const char *key, size_t len,
          tinytemplate_type_t *type, tinytemplate_value_t *value)
{
    eval_context_t *context = data;
    
    for (size_t i = 0; i < context->params->count; i++) {
        trama_template_param_t *param = context->params->array + i;
        size_t param_name_len = strlen(param->name);
        if (param_name_len == len && !strncmp(key, param->name, len)) {
            switch (param->type) {
                
                case TRAMA_TEMPLATE_PARAMTYPE_INT:
                *type = TINYTEMPLATE_TYPE_INT;
                value->as_int = param->as_int;
                break;
                
                case TRAMA_TEMPLATE_PARAMTYPE_FLOAT:
                *type = TINYTEMPLATE_TYPE_FLOAT;
                value->as_float = param->as_float;
                break;

                case TRAMA_TEMPLATE_PARAMTYPE_STRING:
                *type = TINYTEMPLATE_TYPE_STRING;
                value->as_string.str = param->as_string;
                value->as_string.len = strlen(param->as_string);
                break;
                
                case TRAMA_TEMPLATE_PARAMTYPE_USER:
                *type = TINYTEMPLATE_TYPE_DICT;
                value->as_dict.data = param;
                value->as_dict.get  = get_property;
                break;
                
                case TRAMA_TEMPLATE_PARAMTYPE_POST:
                *type = TINYTEMPLATE_TYPE_DICT;
                value->as_dict.data = param;
                value->as_dict.get  = get_property;
                break;
                
                case TRAMA_TEMPLATE_PARAMTYPE_GROUP:
                *type = TINYTEMPLATE_TYPE_DICT;
                value->as_dict.data = param;
                value->as_dict.get  = get_property;
                break;

                case TRAMA_TEMPLATE_PARAMTYPE_USERITER:
                *type = TINYTEMPLATE_TYPE_ARRAY;
                value->as_array.data = param;
                value->as_array.next = next_item;
                break;

                case TRAMA_TEMPLATE_PARAMTYPE_POSTITER:
                *type = TINYTEMPLATE_TYPE_ARRAY;
                value->as_array.data = param;
                value->as_array.next = next_item;
                break;
                
                case TRAMA_TEMPLATE_PARAMTYPE_GROUPITER:
                *type = TINYTEMPLATE_TYPE_ARRAY;
                value->as_array.data = param;
                value->as_array.next = next_item;
                break;
            }
            return true;
        }
    }
    return false;
}

int trama_template_eval(trama_template_t *template, 
                        trama_template_params_t *params, 
                        char *dst, size_t max,
                        char *errmsg, size_t errmax)
{
    if (params->failed) {
        snprintf(errmsg, errmax, "Too many parameters");
        return false;
    }

    eval_context_t context = {.dst=dst, .max=max, .out=0, .params=params};
    
    tinytemplate_status_t status;
    status = tinytemplate_eval(template->data, template->program, &context, 
                             get_param, append_output, errmsg, errmax);
    if (status != TINYTEMPLATE_STATUS_DONE)
        return -1;
    return (int) context.out;
}