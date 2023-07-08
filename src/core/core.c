#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"

static const char schema[] =
    "CREATE TABLE Users(\n"
    "\tname TEXT PRIMARY KEY,\n"
    "\tpass TEXT NOT NULL\n"
    ");\n"
    "\n"
    "CREATE TABLE Groups(\n"
    "\tname TEXT PRIMARY KEY,\n"
    "\tdscr TEXT\n"
    ");\n"
    "\n"
    "CREATE TABLE Posts(\n"
    "\tid       INTEGER PRIMARY KEY AUTOINCREMENT,\n"
    "\tgroup_   TEXT    NOT NULL,\n"
    "\tauthor   TEXT    NOT NULL,\n"
    "\ttitle    TEXT    NOT NULL,\n"
    "\tbody     TEXT    NOT NULL,\n"
    "\tupvote   INTEGER NOT NULL,\n"
    "\tdownvote INTEGER NOT NULL,\n"
    "\tFOREIGN KEY (group_) REFERENCES Groups(name),\n"
    "\tFOREIGN KEY (author) REFERENCES Users(name)\n"
    ");\n"
    "\n"
    "CREATE TABLE Comments(\n"
    "\tid             INTEGER PRIMARY KEY AUTOINCREMENT,\n"
    "\tparent_post    INTEGER NOT NULL,\n"
    "\tparent_comment INTEGER,\n"
    "\tauthor         TEXT    NOT NULL,\n"
    "\tbody           TEXT    NOT NULL,\n"
    "\tupvote         INTEGER NOT NULL,\n"
    "\tdownvote       INTEGER NOT NULL,\n"
    "\tFOREIGN KEY (author) REFERENCES Users(name),\n"
    "\tFOREIGN KEY (parent_post)    REFERENCES Posts(id),\n"
    "\tFOREIGN KEY (parent_comment) REFERENCES Comments(id)\n"
    ");\n"
    "\n"
    "CREATE TABLE UserGroups(\n"
    "\tuser   TEXT NOT NULL,\n"
    "\tgroup_ TEXT NOT NULL,\n"
    "\tPRIMARY KEY (user, group_),\n"
    "\tFOREIGN KEY (user)   REFERENCES Users(name),\n"
    "\tFOREIGN KEY (group_) REFERENCES Groups(name)\n"
    ");\n"
    "\n"
    "PRAGMA foreign_keys = ON;\n";

void trama_setLogFunc(trama_t *trama, void *ptr, void (*fn)(void*, const char*))
{
    trama->logfn  = fn;
    trama->logptr = ptr;
}

void trama_log(trama_t *trama, const char *fmt, ...)
{
    if (trama->logfn) {

        char buffer[256];

        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
    
        trama->logfn(trama->logptr, buffer);
    }
}

trama_status_t 
trama_getUser(trama_t *trama, 
              const char *name, 
              trama_user_t *user)
{
    int code;
    sqlite3_stmt *stmt;
    static const char text[] = "SELECT pass FROM Users WHERE name=?";

    code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }
    if ((code = sqlite3_bind_text(stmt, 1, name, -1, NULL)) != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    code = sqlite3_step(stmt);
    if (code == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_NOTFOUND;
    }
    if (code != SQLITE_ROW) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }
    
    const char *pass = (char*) sqlite3_column_text(stmt, 0);
    assert(pass);

    strncpy(user->name, name, TRAMA_MAXUNAME);
    strncpy(user->pass, pass, TRAMA_MAXPASSW);

    sqlite3_finalize(stmt);
    return TRAMA_STATUS_OK;
}

trama_status_t 
trama_createPost(trama_t *trama, 
                 const trama_post_t *post)
{
    sqlite3_stmt *stmt;
    static const char text[] = "INSERT INTO Posts(group_, author, title, body, upvote, downvote) VALUES (?, ?, ?, ?, 0, 0)";
    int code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }
    if ((code = sqlite3_bind_text(stmt, 1, post->group,  -1, NULL)) != SQLITE_OK ||
        (code = sqlite3_bind_text(stmt, 2, post->author, -1, NULL)) != SQLITE_OK ||
        (code = sqlite3_bind_text(stmt, 3, post->title,  -1, NULL)) != SQLITE_OK ||
        (code = sqlite3_bind_text(stmt, 4, post->body, post->size, NULL)) != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_step(stmt)) != SQLITE_DONE) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    trama_log(trama, "Created post [%s] by [%s]", post->title, post->author);
    sqlite3_finalize(stmt);
    return TRAMA_STATUS_OK;
}

trama_status_t
trama_createUser(trama_t *trama, const trama_user_t *user)
{
    sqlite3_stmt *stmt;
    static const char text[] = "INSERT INTO Users(name, pass) VALUES (?, ?)";
    int code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }
    if ((code = sqlite3_bind_text(stmt, 1, user->name, -1, NULL)) != SQLITE_OK ||
        (code = sqlite3_bind_text(stmt, 2, user->pass, -1, NULL)) != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_step(stmt)) != SQLITE_DONE) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    trama_log(trama, "Created user (%s, %s)", user->name, user->pass);
    sqlite3_finalize(stmt);
    return TRAMA_STATUS_OK;
}

trama_status_t 
trama_createGroup(trama_t *trama, 
                  trama_group_t *group)
{
    sqlite3_stmt *stmt;
    static const char text[] = "INSERT INTO Groups(name, dscr) VALUES (?, ?)";
    int code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }
    if ((code = sqlite3_bind_text(stmt, 1, group->name, -1, NULL)) != SQLITE_OK ||
        (code = sqlite3_bind_text(stmt, 2, group->desc, -1, NULL)) != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_step(stmt)) != SQLITE_DONE) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    sqlite3_finalize(stmt);
    return TRAMA_STATUS_OK;
}

trama_status_t
trama_listGroupPosts(trama_t *trama, 
                     const char *group_name,
                     trama_postiter_t *iter)
{
    sqlite3_stmt *stmt;
    static const char text[] = "SELECT id, author, title, body, upvote, downvote FROM Posts WHERE group_=?";
    int code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }
    if ((code = sqlite3_bind_text(stmt, 1, group_name, -1, NULL))) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }
    iter->trama = trama;
    iter->stmt = stmt;
    return TRAMA_STATUS_OK;
}

trama_status_t 
trama_nextPost(trama_postiter_t *iter, 
               trama_post_t *post)
{
    int code;
    if ((code = sqlite3_step(iter->stmt)) != SQLITE_ROW) {
        if (code != SQLITE_DONE)
            trama_log(iter->trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_FINISH;
    }

    post->id = sqlite3_column_int(iter->stmt, 0);
    strncpy(post->author, (char*) sqlite3_column_text(iter->stmt, 1), TRAMA_MAXUNAME);
    strncpy(post->title,  (char*) sqlite3_column_text(iter->stmt, 2), TRAMA_MAXTITLE);

    char *body = strdup((char*) sqlite3_column_text(iter->stmt, 3));
    post->body = body;
    post->size = sqlite3_column_bytes(iter->stmt, 3);

    post->upvote   = sqlite3_column_int(iter->stmt, 4);
    post->downvote = sqlite3_column_int(iter->stmt, 5);

    return TRAMA_STATUS_OK;
}

void trama_closePostIter(trama_postiter_t *iter)
{
    sqlite3_finalize(iter->stmt);
}

trama_status_t 
trama_listGroups(trama_t *trama, 
                 trama_groupiter_t *iter)
{
    sqlite3_stmt *stmt;
    static const char text[] = "SELECT name, dscr FROM Groups";
    int code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }
    iter->trama = trama;
    iter->stmt = stmt;
    return TRAMA_STATUS_OK;
}

trama_status_t 
trama_nextGroup(trama_groupiter_t *iter, 
                trama_group_t *group)
{
    int code;
    if ((code = sqlite3_step(iter->stmt)) != SQLITE_ROW) {
        if (code != SQLITE_DONE)
            trama_log(iter->trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_FINISH;
    }
    strncpy(group->name, (char*) sqlite3_column_text(iter->stmt, 0), TRAMA_MAXUNAME);
    strncpy(group->desc, (char*) sqlite3_column_text(iter->stmt, 1), TRAMA_MAXDESCR);
    return TRAMA_STATUS_OK;
}

void trama_closeGroupIter(trama_groupiter_t *iter)
{
    sqlite3_finalize(iter->stmt);
}

trama_status_t 
trama_listUsers(trama_t *trama, 
                trama_useriter_t *iter)
{
    sqlite3_stmt *stmt;
    static const char text[] = "SELECT name, pass FROM Users";
    int code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }
    iter->trama = trama;
    iter->stmt = stmt;
    return TRAMA_STATUS_OK;
}

trama_status_t 
trama_nextUser(trama_useriter_t *iter, 
               trama_user_t *user)
{
    int code;
    if ((code = sqlite3_step(iter->stmt)) != SQLITE_ROW) {
        if (code != SQLITE_DONE)
            trama_log(iter->trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_FINISH;
    }
    strncpy(user->name, (char*) sqlite3_column_text(iter->stmt, 0), TRAMA_MAXUNAME);
    strncpy(user->pass, (char*) sqlite3_column_text(iter->stmt, 1), TRAMA_MAXPASSW);
    return TRAMA_STATUS_OK;
}

void trama_closeUserIter(trama_useriter_t *iter)
{
    sqlite3_finalize(iter->stmt);
}

trama_status_t 
trama_open(trama_t *trama, 
           const char *file)
{
    int code;

    code = sqlite3_open(file, &trama->database);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }
    
    char *msg;
    code = sqlite3_exec(trama->database, schema, NULL, NULL, &msg);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_free(msg);
        sqlite3_close(trama->database);
        return TRAMA_STATUS_ERROR;
    }

    return TRAMA_STATUS_OK;
}

void trama_close(trama_t *trama)
{
    sqlite3_close(trama->database);
}

trama_status_t 
trama_getPost(trama_t *trama, int id, 
              trama_post_t *post)
{
    sqlite3_stmt *stmt;
    static const char text[] = "SELECT id, group_, author, title, body, upvote, downvote FROM Posts WHERE id=?";
    int code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_bind_int(stmt, 1, id)) != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_step(stmt)) != SQLITE_ROW) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    post->id = sqlite3_column_int(stmt, 0);
    strncpy(post->group,  (char*) sqlite3_column_text(stmt, 1), TRAMA_MAXGNAME+1);
    strncpy(post->author, (char*) sqlite3_column_text(stmt, 2), TRAMA_MAXUNAME+1);
    strncpy(post->title,  (char*) sqlite3_column_text(stmt, 3), TRAMA_MAXTITLE+1);

    char *body = strdup((char*) sqlite3_column_text(stmt, 4));
    post->body = body;
    post->size = sqlite3_column_bytes(stmt, 4);

    post->upvote   = sqlite3_column_int(stmt, 5);
    post->downvote = sqlite3_column_int(stmt, 6);

    sqlite3_finalize(stmt);
    return TRAMA_STATUS_OK;
}

trama_status_t 
trama_getGroup(trama_t *trama, 
               const char *name, 
               trama_group_t *group)
{
    sqlite3_stmt *stmt;
    static const char text[] = "SELECT dscr FROM Groups WHERE name=?";
    int code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_bind_text(stmt, 1, name, -1, NULL)) != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_step(stmt)) != SQLITE_ROW) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    const char *desc = (char*) sqlite3_column_text(stmt, 0);
    if (desc == NULL)
        desc = "";
    strncpy(group->name, name, TRAMA_MAXUNAME);
    strncpy(group->desc, desc, TRAMA_MAXDESCR);

    sqlite3_finalize(stmt);
    return TRAMA_STATUS_OK;
}

trama_status_t 
trama_getComment(trama_t *trama, int id, 
                 trama_comment_t *comment)
{
    int code;
    sqlite3_stmt *stmt;

    static const char text[] = "SELECT id, parent_post, parent_comment, author, body, upvote, downvote FROM Posts WHERE id=?";
    code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_bind_int(stmt, 1, id)) != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_step(stmt)) != SQLITE_ROW) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    comment->id = sqlite3_column_int(stmt, 0);
    comment->parent_post = sqlite3_column_int(stmt, 1);
    comment->parent_comment = sqlite3_column_int(stmt, 2);
    strncpy(comment->author, (char*) sqlite3_column_text(stmt, 3), TRAMA_MAXUNAME);

    char *body = strdup((char*) sqlite3_column_text(stmt, 4));
    comment->body = body;
    comment->size = sqlite3_column_bytes(stmt, 4);

    comment->upvote   = sqlite3_column_int(stmt, 5);
    comment->downvote = sqlite3_column_int(stmt, 6);

    comment->parent = NULL;
    comment->next = NULL;

    sqlite3_finalize(stmt);
    return TRAMA_STATUS_OK;
}

void trama_freePost(trama_post_t *post)
{
    free(post->body);
}

void trama_freeComment(trama_comment_t *comment)
{
    free(comment->body);
}

/*
typedef struct trama_commentgroup_t trama_commentgroup_t;
struct trama_commentgroup_t {
    trama_commentgroup_t *next;
    trama_comment_t slots[TRAMA_COMMENTSPERGROUP];
};

typedef struct {
    trama_commentgroup_t *tail;
    trama_commentgroup_t  head;
} trama_commentpool_t;
*/

static void 
commentgroup_init(trama_commentgroup_t *group)
{
    group->next = NULL;
    for (int i = 0; i < TRAMA_COMMENTSPERGROUP-1; i++)
        group->slots[i].next = &group->slots[i+1];
    group->slots[TRAMA_COMMENTSPERGROUP-1].next = NULL;
}

void trama_commentpool_init(trama_commentpool_t *pool)
{
    pool->tail = &pool->head;
    commentgroup_init(&pool->head);
}

void trama_commentpool_free(trama_commentpool_t *pool)
{
    trama_commentgroup_t *curs = pool->head.next;
    while (curs) {
        trama_commentgroup_t *next = curs->next;
        free(curs);
        curs = next;
    }
}

trama_comment_t*
trama_commentpool_alloc(trama_commentpool_t *pool)
{
    if (pool->free_list == NULL) {
        trama_commentgroup_t *group = malloc(sizeof(trama_commentgroup_t));
        if (group == NULL)
            return NULL;
        commentgroup_init(group);
        pool->free_list = group->slots;
        pool->tail->next = group;
        pool->tail = group;
    }

    trama_comment_t *slot = pool->free_list;
    pool->free_list = slot->next;

    return slot;
}

trama_status_t 
trama_getPostComments(trama_t *trama, int post_id,
                      trama_commentpool_t *pool,
                      trama_comment_t **root)
{
    int code;
    sqlite3_stmt *stmt;

    static const char text[] = "SELECT id, parent_comment, author, body, upvote, downvote FROM Comments WHERE parent_post=?";
    code = sqlite3_prepare_v2(trama->database, text, sizeof(text)-1, &stmt, NULL);
    if (code != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        return TRAMA_STATUS_ERROR;
    }

    if ((code = sqlite3_bind_int(stmt, 1, post_id)) != SQLITE_OK) {
        trama_log(trama, "SQLITE: %s", sqlite3_errstr(code));
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }

    const size_t list_start = 8;
    trama_comment_t **list = malloc(list_start * sizeof(trama_comment_t*));
    if (list == NULL) {
        sqlite3_finalize(stmt);
        return TRAMA_STATUS_ERROR;
    }
    size_t list_size = list_start;
    size_t list_used = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        
        trama_comment_t *comment = trama_commentpool_alloc(pool);
        if (comment == NULL) {
            free(list);
            sqlite3_finalize(stmt);
            return TRAMA_STATUS_ERROR;
        }

        int parent_comment = -1;
        if (sqlite3_column_type(stmt, 1) != SQLITE_NULL)
            parent_comment = sqlite3_column_int(stmt, 1);

        comment->id = sqlite3_column_int(stmt, 0);
        comment->parent_post = post_id;
        comment->parent_comment = parent_comment;
        strncpy(comment->author, (char*) sqlite3_column_text(stmt, 2), TRAMA_MAXUNAME);

        char *body = strdup((char*) sqlite3_column_text(stmt, 3));
        comment->body = body;
        comment->size = sqlite3_column_bytes(stmt, 3);

        comment->upvote = sqlite3_column_int(stmt, 4);
        comment->downvote = sqlite3_column_int(stmt, 5);

        comment->parent = NULL;
        comment->next = NULL;

        if (list_size == list_used) {
            size_t new_size = 2 * list_size;
            void *temp = realloc(list, new_size * sizeof(trama_comment_t));
            if (temp == NULL) {
                free(list);
                sqlite3_finalize(stmt);
                return TRAMA_STATUS_ERROR;
            }
            list_size = new_size;
            list = temp;
        }
        list[list_used++] = comment;
    }

    trama_comment_t  *head = NULL;
    trama_comment_t **tail = &head;

    for (size_t i = 0; i < list_used; i++) {
        trama_comment_t *child = list[i];
        if (child->parent_comment == -1) {
            *tail = child;
            tail = &child->next;
        } else
            for (size_t j = 0; j < list_used; j++) {
                trama_comment_t *parent = list[j];
                if (child->parent_comment == parent->id) {
                    child->parent = parent;
                    child->next = parent->child;
                    parent->child = child;
                }
            }
    }

    *root = head;
    free(list);
    sqlite3_finalize(stmt);
    return TRAMA_STATUS_OK;
}