/**
 * file: chat.h
 *
 * Implementation of a chat client
 */

#ifndef ALLISONK_CHAT_H
#define ALLISONK_CHAT_H

#include "server.h"

/**
 * Peforms all global need to init the chat server
 */
void chat_global_init(void);

/**
 * Entry point for a chat client handler thread
 */
void chat_run(struct client_info *ci);

/**
 * Adds a new user to the chat session 
 *
 * \param sock      Socket to communicate on
 * \return          0 on success, -1 on failure
 */
int chat_register_user(int sock);

/**
 * Parses a message received from a client
 * 
 * \param from_sock Socket we received message on
 * \param buffer    Buffer recieved
 * \param sz        Size of buffer
 */
void parse_message(int from_sock, char buffer[], size_t sz);

/**
 * Sends a message to a user
 *
 * \param psock     User to send to
 * \param pmsg      Message to send
 */
void chat_send_to_user(void *psock, void *pmsg);

/**
 * Broadcasts a message to all users
 *
 * \param from_sock     Which user this came from
 * \param msg           Message to send
 */
void chat_broadcast(int from_sock, const char *msg);

/**
 * Global cleanup for the chat client
 */
void chat_global_destroy(void);

#endif
