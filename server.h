/**
 * file: server.h
 *
 * Common functions and headers for the server
 */

#ifndef ALLISONK_SERVER_H
#define ALLISONK_SERVER_H

#include <netinet/in.h>

#define ARR_SIZE(a)     (sizeof(a) / sizeof(*a))

/**
 * Represents a client's connection information
 *
 * sock      Client's socket
 * caddr     Client's address information
 */
struct client_info {
    int sock;
    struct sockaddr_in caddr;
};

/**
 * Prints the connection information string
 *
 * \param msg       Message to print with connection string
 * \param addr      Address information
 */
void print_connection(const char *msg, struct sockaddr_in *addr);

#endif
