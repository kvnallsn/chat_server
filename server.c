/**
 * file: server.c
 *
 * A Chat Server
 */

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <pthread.h>

#include "server.h"
#include "chat.h"
#include "generic/generic_list.h"

#define DEFAULT_PORT    9004
#define BACKLOG         5
#define BASE_10         10

/* Declarations */
void usage(char *prog_name);
void sig_handler(int signo);
void* handle_client(void *arg);
void server_loop(int sock);

/* Definitions */
static int stop = 0;
static pthread_mutex_t mtx_thread_list;
static struct list *threads;

/**
 * Prints a usage statement for the server
 */
void usage(char *prog_name)
{
    fprintf(stderr, "usage: %s [port]\n", prog_name);
}

/**
 * Handles any signals received
 */
void sig_handler(int signo)
{
    switch (signo) {
        case SIGINT:
            stop = 1;
            break;
        default:
            break;
    }
}

/**
 * Handle the client
 */
void* handle_client(void *arg)
{
    struct client_info *ci;
    pthread_t thread_id;

    ci = (struct client_info*)arg;

    print_connection("connected", &(ci->caddr));
    chat_run(ci);
    print_connection("disconnected", &(ci->caddr));

    /* remove from list */
    pthread_mutex_lock(&mtx_thread_list);
    thread_id = pthread_self();
    list_remove(threads, (void*)thread_id);
    pthread_mutex_unlock(&mtx_thread_list);

    free(ci);

    return NULL;
}

/**
 * Main event loop of our server
 *
 * \param sock      Server's listening socket
 */
void server_loop(int sock)
{
    int nset;
    struct pollfd pfds[1];

    pfds[0].fd = sock;
    pfds[0].events = POLLIN;

    pthread_mutex_init(&mtx_thread_list, NULL);

    list_init(&threads, NULL, NULL);
    chat_global_init();

    /**
     * Accepts connections from clients and starts a new
     * thread for each successful conenction.  Additionaly,
     * we track each thread identifier in a list to cleanup
     * if the server exits before all connections are closed */
    do {
        nset = poll(pfds, ARR_SIZE(pfds), -1);
        if (nset >= 1) { 
            if (pfds[0].revents & POLLIN) {

                int err;
                pthread_t thread;
                struct client_info *ci;
                socklen_t sz;


                ci = malloc(sizeof(struct client_info));
                sz = sizeof(ci->caddr);
               
                ci->sock = accept(sock, (struct sockaddr*)&(ci->caddr), &sz);
                if (ci->sock== -1) {
                    perror("[server:accept]");
                    continue;
                }

                err = pthread_create(&thread, NULL, handle_client, ci);
                if (err != 0) {
                   fprintf(stderr, "[server:pthread_create]: failed to create thread\n");
                } else {
                    pthread_mutex_lock(&mtx_thread_list);
                    list_add(threads, (void*)thread);                    
                    pthread_mutex_unlock(&mtx_thread_list);
                } 
            }
        }

    } while (!stop);

    printf("Destroying threads\n");

    /* signal all threads to disconnect and shutdown */
    list_destroy(&threads);
    chat_global_destroy();

    pthread_mutex_destroy(&mtx_thread_list);
}

/**
 * Print an IP port connection string
 *
 * \param addr    Address to print
 */
void print_connection(const char *msg, struct sockaddr_in *addr)
{
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, addr, ip_str, sizeof(*addr));
    printf("[server] connection %s:%u %s\n", ip_str, ntohs(addr->sin_port), msg);
}

/**
 * Entry point for our main program
 */
int main(int argc, char *argv[])
{
    int sock, err, ret, sockopt;
    unsigned short port = DEFAULT_PORT;
    struct sockaddr_in saddr;

    if (argc == 2) {
        char *canary = NULL;
        long lport;
       
        lport  = strtol(argv[1], &canary, BASE_10);
        if (canary == argv[1]  || lport > 65535) {
            fprintf(stderr, "[server] port field invalid.  Must be below 65535\n");
            return EXIT_FAILURE;
        }

        port = (unsigned short)lport;
    } else if (argc > 2) {
        usage(argv[0]);
    }

    ret = EXIT_SUCCESS;

    printf("[server] listening on 0.0.0.0:%u\n", port);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("[server:socket]");
        return EXIT_FAILURE;
    }

    sockopt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = INADDR_ANY;
    err = bind(sock, (const struct sockaddr*)&saddr, sizeof(saddr));
    if (err == -1) {
        perror("[server:bind]");
        ret = EXIT_FAILURE;
        goto shutdown_socket;
    }

    err = listen(sock, BACKLOG);
    if (err == -1) {
        perror("[server:listen]");
        ret = EXIT_FAILURE;
        goto shutdown_socket;
    }

    signal(SIGINT, sig_handler);

    server_loop(sock);

shutdown_socket:
    close(sock);

    return ret;
}
