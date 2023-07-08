#ifndef TRAMA_CORE_H
#define TRAMA_CORE_H

#include "sqlite3.h"

#define TRAMA_MAXTITLE 255
#define TRAMA_MAXUNAME 63
#define TRAMA_MAXGNAME 127
#define TRAMA_MAXPASSW 127
#define TRAMA_MAXDESCR 511
#define TRAMA_MAXPOSTBODY 4096

typedef struct {
    sqlite3 *database;
    void  *logptr;
    void (*logfn)(void*, const char*);
} trama_t;

typedef struct {
    char name[TRAMA_MAXGNAME+1];
    char desc[TRAMA_MAXDESCR+1];
} trama_group_t;

typedef struct {
    char name[TRAMA_MAXUNAME+1];
    char pass[TRAMA_MAXPASSW+1];
} trama_user_t;

typedef struct {
    int id;
    char  group[TRAMA_MAXGNAME+1];
    char  title[TRAMA_MAXTITLE+1];
    char author[TRAMA_MAXUNAME+1];
    char  *body;
    size_t size;
    int   upvote;
    int downvote;
} trama_post_t;

typedef struct trama_comment_t trama_comment_t;
struct trama_comment_t {
    int id;
    int parent_post;
    int parent_comment;
    char author[TRAMA_MAXUNAME+1];
    char  *body;
    size_t size;
    int   upvote;
    int downvote;
    trama_comment_t *parent;
    trama_comment_t *child;
    trama_comment_t *next;
};

typedef enum {
    TRAMA_STATUS_OK,
    TRAMA_STATUS_NOTFOUND,
    TRAMA_STATUS_FINISH,
    TRAMA_STATUS_ERROR,
} trama_status_t;

#define TRAMA_COMMENTSPERGROUP 32

typedef struct trama_commentgroup_t trama_commentgroup_t;
struct trama_commentgroup_t {
    trama_commentgroup_t *next;
    trama_comment_t slots[TRAMA_COMMENTSPERGROUP];
};

typedef struct {
    trama_comment_t *free_list;
    trama_commentgroup_t *tail;
    trama_commentgroup_t  head;
} trama_commentpool_t;

void             trama_commentpool_init(trama_commentpool_t *pool);
void             trama_commentpool_free(trama_commentpool_t *pool);
trama_comment_t *trama_commentpool_alloc(trama_commentpool_t *pool);

typedef struct {
    trama_t *trama;
    sqlite3_stmt *stmt;
} trama_groupiter_t;

trama_status_t trama_listGroups(trama_t *trama, trama_groupiter_t *iter);
trama_status_t trama_nextGroup(trama_groupiter_t *iter, trama_group_t *group);
void           trama_closeGroupIter(trama_groupiter_t *iter);

typedef struct {
    trama_t *trama;
    sqlite3_stmt *stmt;
} trama_useriter_t;

trama_status_t trama_listUsers(trama_t *trama, trama_useriter_t *iter);
trama_status_t trama_nextUser(trama_useriter_t *iter, trama_user_t *user);
void           trama_closeUserIter(trama_useriter_t *iter);

typedef struct {
    trama_t *trama;
    sqlite3_stmt *stmt;
} trama_postiter_t;

trama_status_t trama_listGroupPosts(trama_t *trama, const char *group_name, trama_postiter_t *iter);
trama_status_t trama_nextPost(trama_postiter_t *iter, trama_post_t *post);
void           trama_closePostIter(trama_postiter_t *iter);

void trama_setLogFunc(trama_t *trama, void *ptr, void (*fn)(void*, const char*));
void trama_log(trama_t *trama, const char *fmt, ...);

trama_status_t trama_open (trama_t *trama, const char *file);
void           trama_close(trama_t *trama);

trama_status_t trama_getPost(trama_t *trama, int id,           trama_post_t  *post);
trama_status_t trama_getUser(trama_t *trama, const char *name, trama_user_t  *user);
trama_status_t trama_getGroup(trama_t *trama, const char *name, trama_group_t *group);
trama_status_t trama_getComment(trama_t *trama, int id, trama_comment_t *comment);

void           trama_freePost(trama_post_t *post);
void           trama_freeComment(trama_comment_t *comment);

trama_status_t 
trama_getPostComments(trama_t *trama, int post_id,
                      trama_commentpool_t *pool,
                      trama_comment_t **root);

trama_status_t trama_createPost(trama_t *trama, const trama_post_t *post);
trama_status_t trama_createUser(trama_t *trama, const trama_user_t *user);
trama_status_t trama_createGroup(trama_t *trama, trama_group_t *group);

#endif