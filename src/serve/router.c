#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "router.h"

static chttp_string_t 
get_cookie_from_header(const char *name,
                       chttp_string_t src)
{
    // Cookie: items=16; headercolor=blue; screenmode=dark; job=111
    size_t i = 0;
    while (i < src.len) {
        
        while (i < src.len && isspace(src.str[i]))
            i++;

        chttp_string_t cookie_name;
        {
            size_t off = i;
            while (i < src.len && isalpha(src.str[i]))
                i++;
            size_t len = i - off;
            cookie_name = CHTTP_STRING_NEW(src.str + off, len);
        }

        chttp_string_t cookie_value;
        {
            if (i == src.len && src.str[i] != '=')
                cookie_value = CHTTP_STRING_FROM_LITERAL("");
            else {
                i++;
                size_t off = i;
                while (i < src.len && isalpha(src.str[i]))
                    i++;
                size_t len = i - off;
                cookie_value = CHTTP_STRING_NEW(src.str + off, len);
            }
        }

        while (i < src.len && isspace(src.str[i]))
            i++;

        if (i < src.len && src.str[i] == ';')
            i++;
/*
        fprintf(stderr, "cookie (%.*s, %.*s)\n",
                (int) cookie_name.len, cookie_name.str,
                (int) cookie_value.len, cookie_value.str);
*/
        if (chttp_string_match(cookie_name, name))
            return cookie_value;
    }

    return CHTTP_STRING_FROM_LITERAL("");
}

static chttp_string_t 
get_param_from_urlencoded_string(const char *param, 
                                 chttp_string_t src)
{
    size_t i = 0;
    while (i < src.len) {
        
        chttp_string_t current_param;
        {
            // Get to the next "=" or end of the source
            size_t off = i;
            while (i < src.len && src.str[i] != '=' && src.str[i] != '&')
                i++;
            size_t len = i - off;
            current_param = CHTTP_STRING_NEW(src.str + off, len);
        }

        chttp_string_t current_value;
        if (i == src.len || src.str[i] != '=') 
            current_value = CHTTP_STRING_FROM_LITERAL("");
        else {

            i++;

            // Get to the next "&" or end of the string
            size_t off = i;
            while (i < src.len && src.str[i] != '&')
                i++;
            size_t len = i - off;
            current_value = CHTTP_STRING_NEW(src.str + off, len);
        }

        if (i < src.len) {
            assert(src.str[i] == '&');
            i++;
        }

        if (chttp_string_match(current_param, param))
            return current_value;
    }
    return CHTTP_STRING_FROM_LITERAL("");
}

static int 
route_notfound(trama_t *trama,
               trama_template_cache_t *cache,
               const char *logged, 
               char *pool, size_t pool_size)
{
    char message[256];

    trama_template_t *template = trama_template_load(cache, "templates/notfound.tt", message, sizeof(message));
    if (template == NULL) {
        trama_log(trama, "Failed to load template (%s)", message);
        return -1;
    }

    trama_template_params_t params;
    trama_template_params_init(&params);
    trama_template_params_setInteger(&params, "logged", logged != NULL);
    if (logged)
        trama_template_params_setString(&params, "username", logged);
    int num = trama_template_eval(template, &params, pool, pool_size, message, sizeof(message));
    if (num < 0)
        trama_log(trama, "Failed to evaluate template (%s)", message);
    trama_template_params_free(&params);
    return num;
}

static int 
route_groups(trama_t *trama,
             trama_template_cache_t *cache,
             const char *logged, 
             char *pool, size_t pool_size)
{
    char message[256];

    trama_template_t *template = trama_template_load(cache, "templates/groups.tt", message, sizeof(message));
    if (template == NULL) {
        trama_log(trama, "Failed to load template (%s)", message);
        return -1;
    }

    trama_groupiter_t iter;
    trama_status_t status = trama_listGroups(trama, &iter);
    if (status != TRAMA_STATUS_OK)
        return -1;

    trama_template_params_t params;
    trama_template_params_init(&params);
    trama_template_params_setInteger(&params, "logged", logged != NULL);
    if (logged)
        trama_template_params_setString(&params, "username", logged);
    trama_template_params_setGroupIter(&params, "groups", &iter);
    int num = trama_template_eval(template, &params, pool, pool_size, message, sizeof(message));
    if (num < 0)
        trama_log(trama, "Failed to evaluate template (%s)", message);
    trama_template_params_free(&params);
    trama_closeGroupIter(&iter);
    return num;
}

static int 
route_post(trama_t *trama,
           trama_template_cache_t *cache,
           const char *logged, 
           int post_id, 
           char *pool, 
           size_t pool_size)
{
    char message[256];

    trama_template_t *template;
    template = trama_template_load(cache, "templates/post.tt", message, sizeof(message));
    if (template == NULL) {
        trama_log(trama, "Failed to load template (%s)", message);
        return -1;
    }

    trama_post_t post;
    if (trama_getPost(trama, post_id, &post) != TRAMA_STATUS_OK)
        return -1;

    trama_template_params_t params;
    trama_template_params_init(&params);
    trama_template_params_setPost(&params, "post", &post);
    trama_template_params_setInteger(&params, "logged", logged != NULL);
    if (logged)
        trama_template_params_setString(&params, "username", logged);
    int num = trama_template_eval(template, &params, pool, pool_size, message, sizeof(message));
    if (num < 0)
        trama_log(trama, "Failed to evaluate template (%s)", message);
    trama_template_params_free(&params);
    return num;
}

static int 
route_group(trama_t *trama,
            trama_template_cache_t *cache,
            const char *logged, 
            const char *group_name, 
            char *pool, size_t pool_size)
{
    assert(group_name != NULL);

    char message[256];

    trama_template_t *template;
    template = trama_template_load(cache, "templates/group.tt", message, sizeof(message));
    if (template == NULL) {
        trama_log(trama, "Failed to load template (%s)", message);
        return -1;
    }

    trama_group_t group;
    if (trama_getGroup(trama, group_name, &group) != TRAMA_STATUS_OK)
        return -1;

    trama_postiter_t iter;
    if (trama_listGroupPosts(trama, group_name, &iter) != TRAMA_STATUS_OK)
        return -1;

    trama_template_params_t params;
    trama_template_params_init(&params);
    trama_template_params_setGroup(&params, "group", &group);
    trama_template_params_setPostIter(&params, "posts", &iter);
    trama_template_params_setInteger(&params, "logged", logged != NULL);
    if (logged)
        trama_template_params_setString(&params, "username", logged);
    int num = trama_template_eval(template, &params, pool, pool_size, message, sizeof(message));
    if (num < 0)
        trama_log(trama, "Failed to evaluate template (%s)", message);
    trama_template_params_free(&params);
    trama_closePostIter(&iter);
    return num;
}

static int 
route_user(trama_t *trama,
           trama_template_cache_t *cache,
           const char *logged,
           const char *user_name, 
           char *pool, size_t pool_size)
{
    assert(user_name != NULL);

    char message[256];

    trama_template_t *template;
    template = trama_template_load(cache, "templates/user.tt", message, sizeof(message));
    if (template == NULL) {
        trama_log(trama, "Failed to load template (%s)", message);
        return -1;
    }

    trama_user_t user;
    if (trama_getUser(trama, user_name, &user) != TRAMA_STATUS_OK)
        return -1;
    
    trama_template_params_t params;
    trama_template_params_init(&params);
    trama_template_params_setUser(&params, "user", &user);
    trama_template_params_setInteger(&params, "logged", logged != NULL);
    if (logged)
        trama_template_params_setString(&params, "username", logged);
    int num = trama_template_eval(template, &params, pool, pool_size, message, sizeof(message));
    if (num < 0)
        trama_log(trama, "Failed to evaluate template (%s)", message);
    trama_template_params_free(&params);
    return num;
}

static int 
route_users(trama_t *trama,
            trama_template_cache_t *cache,
            const char *logged,
            char *pool, size_t pool_size)
{
    char message[256];

    trama_template_t *template = trama_template_load(cache, "templates/users.tt", message, sizeof(message));
    if (template == NULL) {
        trama_log(trama, "Failed to load template (%s)", message);
        return -1;
    }

    trama_useriter_t iter;
    trama_status_t status = trama_listUsers(trama, &iter);
    if (status != TRAMA_STATUS_OK)
        return -1;

    trama_template_params_t params;
    trama_template_params_init(&params);
    trama_template_params_setInteger(&params, "logged", logged != NULL);
    if (logged)
        trama_template_params_setString(&params, "username", logged);
    trama_template_params_setUserIter(&params, "users", &iter);
    int num = trama_template_eval(template, &params, pool, pool_size, message, sizeof(message));
    if (num < 0)
        trama_log(trama, "Failed to evaluate template (%s)", message);
    trama_template_params_free(&params);
    trama_closeUserIter(&iter);
    return num;
}

static int 
route_create_post(trama_t *trama,
                  trama_template_cache_t *cache,
                  const char *logged,
                  const char *group_name,
                  chttp_request_t *req, 
                  chttp_string_t req_body,
                  chttp_response_t *res,
                  char *pool,
                  size_t pool_size)
{
    bool redirect = false;
    const char *error = NULL;
    if (req->method == CHTTP_METHOD_POST) {
        chttp_string_t title = get_param_from_urlencoded_string("title", req_body);
        chttp_string_t body  = get_param_from_urlencoded_string("body", req_body);
        if (title.len == 0 || body.len == 0)
            error = "Empty fields";
        else if (title.len > TRAMA_MAXTITLE)
            error = "Title is too long";
        else if (body.len > TRAMA_MAXPOSTBODY)
            error = "Body is too long";
        else if (logged == NULL)
            error = "You're not logged in";
        else {
            trama_post_t post;

            strncpy(post.title, title.str, TRAMA_MAXTITLE);
            post.title[title.len] = '\0';
            
            size_t logged_len = strlen(logged);
            assert(logged_len <= TRAMA_MAXUNAME);
            strncpy(post.group,  group_name, 1+TRAMA_MAXGNAME);
            strncpy(post.author, logged,     1+TRAMA_MAXUNAME);

            char *body2 = malloc(body.len+1);
            if (body2 == NULL)
                error = "Internal error";
            else {
                memcpy(body2, body.str, body.len);
                body2[body.len] = '\0';
                post.body = body2;
                post.size = body.len;
                if (trama_createPost(trama, &post) != TRAMA_STATUS_OK)
                    error = "Internal error";
                else
                    redirect = true;
            }
        }
    }

    int num;
    {
        assert(group_name != NULL);
        char message[256];

        trama_template_t *template;
        template = trama_template_load(cache, "templates/create_post.tt", message, sizeof(message));
        if (template == NULL) {
            trama_log(trama, "Failed to load template (%s)", message);
            return -1;
        }

        trama_group_t group;
        if (trama_getGroup(trama, group_name, &group) != TRAMA_STATUS_OK)
            return -1;
        
        trama_template_params_t params;
        trama_template_params_init(&params);
        trama_template_params_setGroup(&params, "group", &group);
        trama_template_params_setInteger(&params, "error",  error != NULL);
        trama_template_params_setInteger(&params, "logged", logged != NULL);
        if (logged) trama_template_params_setString(&params, "username", logged);
        if (error)  trama_template_params_setString(&params, "message", error);
        num = trama_template_eval(template, &params, pool, pool_size, message, sizeof(message));
        if (num < 0)
            trama_log(trama, "Failed to evaluate template (%s)", message);
        trama_template_params_free(&params);
    }

    if (num < 0)
        return -1;
    
    if (redirect) {
        res->status = 303;
        chttp_append_response_header_format(res, "Location", " /groups/%s", group_name);
    } else
        res->status = 200;
    return num;
}

static int route_login(trama_t *trama,
                       trama_template_cache_t *cache,
                       const char *logged,
                       chttp_request_t *req, 
                       chttp_string_t req_body,
                       chttp_response_t *res,
                       char *pool,
                       size_t pool_size)
{
    bool redirect = false;
    const char *error = NULL;
    if (req->method == CHTTP_METHOD_POST) {
        chttp_string_t name = get_param_from_urlencoded_string("name", req_body);
        chttp_string_t pass = get_param_from_urlencoded_string("pass", req_body);
        
        if (name.len == 0 || pass.len == 0)
            error = "Empty name of pass";
        else if (name.len > TRAMA_MAXUNAME || pass.len > TRAMA_MAXPASSW)
            error = "Invalid name or pass";
        else {
            char name2[TRAMA_MAXUNAME+1];
            char pass2[TRAMA_MAXPASSW+1];
            memcpy(name2, name.str, name.len);
            memcpy(pass2, pass.str, pass.len);
            name2[name.len] = '\0';
            pass2[pass.len] = '\0';

            trama_user_t user;
            trama_status_t status = trama_getUser(trama, name2, &user);

            if (status == TRAMA_STATUS_NOTFOUND)
                error = "Invalid credentials"; // Invalid name
            else if (status != TRAMA_STATUS_OK)
                error = "Internal error";
            else if (strncmp(pass2, user.pass, TRAMA_MAXPASSW))
                error = "Invalid credentials"; // Invalid pass
            else {
                redirect = true;
                chttp_append_response_header_format(res, "Set-Cookie", " name=%s", name2);
            }
        }
    }

    char message[256];

    trama_template_t *template;
    template = trama_template_load(cache, "templates/login.tt", message, sizeof(message));
    if (template == NULL) {
        trama_log(trama, "Failed to load template (%s)", message);
        return -1;
    }

    trama_template_params_t params;
    trama_template_params_init(&params);
    trama_template_params_setInteger(&params, "error",  error != NULL);
    trama_template_params_setInteger(&params, "logged", logged != NULL);
    if (error)  trama_template_params_setString(&params, "message", error);
    if (logged) trama_template_params_setString(&params, "username", logged);
    int num = trama_template_eval(template, &params, pool, pool_size, message, sizeof(message));
    if (num < 0)
        trama_log(trama, "Failed to evaluate template (%s)", message);
    trama_template_params_free(&params);

    if (num < 0)
        return num;

    if (redirect) {
        res->status = 303;
        chttp_append_response_header(res, "Location", " /groups");
    } else
        res->status = 200;
    return num;
}

trama_status_t 
trama_router_init(trama_router_t *router, trama_t *trama)
{
    router->trama = trama;
    trama_template_cache_init(&router->cache);
    return TRAMA_STATUS_OK;
}

void trama_router_free(trama_router_t *router)
{
    trama_template_cache_free(&router->cache);
}

void trama_router_respond(trama_router_t *router, 
                          chttp_request_t *req, 
                          chttp_string_t req_body, 
                          chttp_response_t *res, 
                          chttp_responsebody_t *res_body)
{
    trama_t *trama = router->trama;
    
    char  logged_maybe[TRAMA_MAXUNAME+1];
    char *logged;
    {
        chttp_string_t name_cookie;
        chttp_header_t *header = chttp_get_request_header(*req, "Cookie");
        if (header)
            name_cookie = get_cookie_from_header("name", header->body);
        else
            name_cookie = CHTTP_STRING_FROM_LITERAL("");
        
        if (name_cookie.len == 0 || name_cookie.len > TRAMA_MAXUNAME)
            logged = NULL;
        else {
            logged = logged_maybe;
            memcpy(logged, name_cookie.str, name_cookie.len);
            logged[name_cookie.len] = '\0';
        }
    }
    
    char group_name[TRAMA_MAXGNAME+1];
    char  user_name[TRAMA_MAXUNAME+1];
    int post_id;

    if (!chttp_pathcmp(req->url.path, "/groups")) {
        if (req->method != CHTTP_METHOD_GET) {
            res->status = 400;
        } else {
            int n = route_groups(trama, &router->cache, logged, router->pool, sizeof(router->pool));
            if (n < 0)
                res->status = 500;
            else {
                res->status = 200;
                res_body->data = router->pool;
                res_body->size = (size_t) n;
            }
        }
    } else if (!chttp_pathcmp(req->url.path, "/login")) {
        
        if (req->method != CHTTP_METHOD_GET && req->method != CHTTP_METHOD_POST) {
            res->status = 400;
        } else {

            int n = route_login(trama, &router->cache, logged, req, req_body, res, router->pool, sizeof(router->pool));
            if (n < 0)
                res->status = 500;
            else {
                res_body->data = router->pool;
                res_body->size = (size_t) n;
            }
        }

    } else if (!chttp_pathcmp(req->url.path, "/posts/:d", &post_id)) {
        if (req->method != CHTTP_METHOD_GET) {
            res->status = 400;
        } else {
            int n = route_post(trama, &router->cache, logged, post_id, router->pool, sizeof(router->pool));
            if (n < 0)
                res->status = 500;
            else {
                res->status = 200;
                res_body->data = router->pool;
                res_body->size = (size_t) n;
            }
        }
    } else if (!chttp_pathcmp(req->url.path, "/groups/:s", TRAMA_MAXGNAME, group_name)) {
        if (req->method != CHTTP_METHOD_GET) {
            res->status = 400;
        } else {
            int n = route_group(trama, &router->cache, logged, group_name, router->pool, sizeof(router->pool));
            if (n < 0)
                res->status = 500;
            else {
                res->status = 200;
                res_body->data = router->pool;
                res_body->size = (size_t) n;
            }
        }
    } else if (!chttp_pathcmp(req->url.path, "/users")) {
        if (req->method != CHTTP_METHOD_GET) {
            res->status = 400;
        } else {
            int n = route_users(trama, &router->cache, logged, router->pool, sizeof(router->pool));
            if (n < 0)
                res->status = 500;
            else {
                res->status = 200;
                res_body->data = router->pool;
                res_body->size = (size_t) n;
            }
        }
    } else if (!chttp_pathcmp(req->url.path, "/users/:s", TRAMA_MAXUNAME, user_name)) {
        if (req->method != CHTTP_METHOD_GET) {
            res->status = 400;
        } else {
            int n = route_user(trama, &router->cache, logged, user_name, router->pool, sizeof(router->pool));
            if (n < 0)
                res->status = 500;
            else {
                res->status = 200;
                res_body->data = router->pool;
                res_body->size = (size_t) n;
            }
        }
    } else if (!chttp_pathcmp(req->url.path, "/groups/:s/create_post", TRAMA_MAXGNAME, group_name)) {
        if (req->method != CHTTP_METHOD_GET && 
            req->method != CHTTP_METHOD_POST) {
            res->status = 400;
        } else {
            int n = route_create_post(trama, &router->cache, logged, group_name, req, req_body, res, router->pool, sizeof(router->pool));
            if (n < 0)
                res->status = 500;
            else {
                res_body->data = router->pool;
                res_body->size = (size_t) n;
            }
        }
    } else {
        if (req->method != CHTTP_METHOD_GET) {
            res->status = 400;
        } else {
            int n = route_notfound(trama, &router->cache, logged, router->pool, sizeof(router->pool));
            if (n < 0)
                res->status = 500;
            else {
                res->status = 404;
                res_body->data = router->pool;
                res_body->size = (size_t) n;
            }
        }
    }
}