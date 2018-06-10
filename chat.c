/**
 * file: chat.c
 *
 * Implementation of a chat client
 */

#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "chat.h"
#include "generic/generic_list.h"

#define SERVER_NAME         "neptune"

#define TAG_PADDING         5
#define MAX_HANDLE_LEN      28
#define MAX_BUFFER          256
#define MAX_CMD_LEN         10
#define MAX_MSG_LEN         MAX_HANDLE_LEN + MAX_BUFFER + TAG_PADDING
#define FMT_REGISTER_DONE   "'%s' has joined the chat!"
#define FMT_MESSAGE         "[%s] %s\n"

#define TAG_INFO            "info"
#define TAG_ADMIN           "admin"
#define TAG_MOD             "mod"

/** Declarations **/

struct user {
    int sock;
    char handle[MAX_HANDLE_LEN];
};

void user_deallocate(void *p);
int user_compare(void *puser, void *psock);
void trim_ending(char *line);

/** Definitions **/

static char str_register_user[] = "enter handle: ";
static char str_handle_too_long[] = "handle too long\n";

static struct list *users;
static pthread_mutex_t mtx_user_list;

/** User list functions **/

void user_deallocate(void *p)
{
    struct user *u;
    u = (struct user*)p;
    free(u);
}

int user_compare(void *puser, void *psock)
{
    int sock;
    struct user *u;

    u = (struct user*)puser;
    sock = (int)psock;

    return (u->sock < sock ? -1 :
            (u->sock == sock ? 0 : 1));
}

/** Chat functions **/

void chat_global_init(void)
{
    pthread_mutex_init(&mtx_user_list, NULL);

    pthread_mutex_lock(&mtx_user_list);
    /* Initialize the chat user list */
    list_init(&users, user_deallocate, user_compare);
    pthread_mutex_unlock(&mtx_user_list);
}

void chat_global_destroy(void)
{
    pthread_mutex_lock(&mtx_user_list);
    list_destroy(&users);
    pthread_mutex_unlock(&mtx_user_list);

    pthread_mutex_destroy(&mtx_user_list);
}

void trim_ending(char *line)
{
    size_t len, i;

    len = strlen(line);
    for (i = 0; i < len; i++) {
        if (line[i] == '\r' || line[i] == '\r' || line[i] < 0x20 || line[i] == 0x7f)
            line[i] = ' ';
    } 
}

void handle_cmd(char *cmd)
{
    char *c, *save;
    size_t cmdlen;

    if (cmd == NULL)
        return;

    c = strtok_r(cmd, " ", &save);
    if (c == NULL)
        return;

    cmd = c;
    cmdlen = strlen(cmd);
    if (strncmp(cmd, "who", MAX_CMD_LEN) == 0) {

    } else if (strcmp(cmd, "server") == 0) {
        chat_broadcast(-1, SERVER_NAME);
    } else {
        printf("%s", cmd);
    }
}

void parse_message(int from_sock, char buffer[], size_t sz)
{
    if (sz == 0 || strlen(buffer) == 0)
        return;

    trim_ending(buffer);
    if (buffer[0] == '/') {
        handle_cmd(buffer + 1);
    } else {
        chat_broadcast(from_sock, buffer);
    }
}

void chat_run(struct client_info *ci)
{
    struct pollfd fds[1];
    int client_stop = 0, err;
    ssize_t nset, nrecv;
    char buffer[MAX_BUFFER] = { 0 };
    struct user *u;

    err = chat_register_user(ci->sock);
    if (err == -1)
        goto chat_exit;

    fds[0].fd = ci->sock;
    fds[0].events = POLLIN | POLLHUP;

    do {
        nset = poll(fds, ARR_SIZE(fds), -1);
        if (nset >= 1) {
            if (fds[0].revents & POLLIN) {
                /* Read input */
                nrecv = recv(fds[0].fd, buffer, ARR_SIZE(buffer) - 1, 0);
                if (nrecv > 0) {
                    buffer[nrecv] = '\0';
                    parse_message(fds[0].fd, buffer, nrecv);
                } else if (nrecv == 0) {
                    client_stop = 1;
                }
            }

            if (fds[0].revents & POLLHUP) {
                /* Client hung up! */
                client_stop = 1;
            }
        }
    } while (!client_stop);

    pthread_mutex_lock(&mtx_user_list);
    u = list_find(users, (void*)(long)ci->sock);
    if (u)
        list_remove(users, u);
    pthread_mutex_unlock(&mtx_user_list);

chat_exit:
    return;
}

/* Adds a new user to the chat session */
int chat_register_user(int sock)
{
    size_t nwritten;
    ssize_t nsent, nrecv;
    struct user *u;
    char buffer[MAX_BUFFER] = { 0 };
    char reg_str[MAX_HANDLE_LEN + ARR_SIZE(FMT_REGISTER_DONE)] = { 0 };

send_register_msg:
    /* first we need to register a user */
    nsent = send(sock, str_register_user, ARR_SIZE(str_register_user), 0);
    if (nsent == -1) {
        perror("[chat:register:send]");
        return -1;
    }

    nrecv = recv(sock, buffer, ARR_SIZE(buffer), 0);
    if (nrecv == -1) {
        perror("[chat:register:recv]");
        return -1;
    } else if (nrecv == 0) {
        return -1;
    } else if (nrecv > MAX_HANDLE_LEN) {
        send(sock, str_handle_too_long, ARR_SIZE(str_handle_too_long), 0);
        goto send_register_msg;
    } 

    /* Strip newline, carriage return */
    buffer[nrecv - 2] = '\0';    
    nwritten = (size_t)snprintf(reg_str, ARR_SIZE(reg_str), FMT_REGISTER_DONE, buffer);

    /* Add user to list */
    u = calloc(1, sizeof(*u));
    u->sock = sock;
    strncpy(u->handle, buffer, MAX_HANDLE_LEN);

    pthread_mutex_lock(&mtx_user_list);
    list_add(users, u);
    pthread_mutex_unlock(&mtx_user_list);

    chat_broadcast(-1, reg_str);

    return 0;
}

/* Sends a message to a user */
void chat_send_to_user(void *puser, void *pmsg)
{
    ssize_t nsent;
    struct user *u;
    const char *msg;

    u = (struct user*)puser;
    msg = (const char*)pmsg;

    nsent = send(u->sock, msg, strlen(msg), 0);
    if (nsent == -1) {
        perror("[chat:send_to_user]");
        return;
    }
}

/* Broadcasts a message to all users */
void chat_broadcast(int from_sock, const char *msg)
{
    struct user *user;
    const char *handle;
    char formatted[MAX_MSG_LEN];


    if (from_sock == -1) {
        handle = TAG_INFO;
    } else {
        pthread_mutex_lock(&mtx_user_list);
        user = list_find(users, (void*)(long)from_sock);
        pthread_mutex_unlock(&mtx_user_list);
        handle = user->handle;
    }

    snprintf(formatted, MAX_MSG_LEN, FMT_MESSAGE, handle, msg);

    /* format message */
    pthread_mutex_lock(&mtx_user_list);
    list_for_each(users, formatted, chat_send_to_user);
    pthread_mutex_unlock(&mtx_user_list);
}
