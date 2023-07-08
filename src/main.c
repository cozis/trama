#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "core/core.h"
#include "serve/serve.h"

static trama_serve_handle_t handle;

static void handle_termination_signal(int signum)
{
    (void) signum;
    trama_serve_stop(&handle);
}

static void log_func(void *data, const char *str)
{
    FILE *stream = data;
    fprintf(stream, "Log: %s\n", str);
}

int main(void)
{
    trama_t trama;
    if (trama_open(&trama, ":memory:") != TRAMA_STATUS_OK) {
        fprintf(stderr, "Error: Failed to set-up core\n");
        return -1;
    }
    trama_setLogFunc(&trama, stderr, log_func);

    trama_group_t group;
    strncpy(group.name, "Matematica", TRAMA_MAXGNAME);
    strncpy(group.desc, "Qui roba di matematica", TRAMA_MAXDESCR);
    if (trama_createGroup(&trama, &group) != TRAMA_STATUS_OK) {
        fprintf(stderr, "Error: Failed to create group\n");
        return -1;
    }

    strncpy(group.name, "Youtubers", TRAMA_MAXGNAME);
    strncpy(group.desc, "Comunità di youtuber", TRAMA_MAXDESCR);
    if (trama_createGroup(&trama, &group) != TRAMA_STATUS_OK) {
        fprintf(stderr, "Error: Failed to create group\n");
        return -1;
    }

    trama_user_t user;
    strncpy(user.name, "cozis", TRAMA_MAXUNAME);
    strncpy(user.pass, "hoyo",  TRAMA_MAXPASSW);
    if (trama_createUser(&trama, &user) != TRAMA_STATUS_OK) {
        fprintf(stderr, "Error: Failed to create user\n");
        return -1;
    }

    strncpy(user.name, "cozis2", TRAMA_MAXUNAME);
    strncpy(user.pass, "hoyo",   TRAMA_MAXPASSW);
    if (trama_createUser(&trama, &user) != TRAMA_STATUS_OK) {
        fprintf(stderr, "Error: Failed to create user\n");
        return -1;
    }
    
    signal(SIGTERM, handle_termination_signal);
    trama_serve(&trama, NULL, 8080, &handle);
    trama_close(&trama);
    return 0;
}