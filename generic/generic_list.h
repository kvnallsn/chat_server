/**
 * file: chat_list.h
 *
 * Represents a list of users in the chat
 */

#ifndef ALLISONK_GENERIC_LIST_H
#define ALLISONK_GENERIC_LIST_H

/**
 * Represents a generic list
 */
struct list;

/**
 * Initializes a list. Will allocate a list
 * and store it in the dereferened parameter
 *
 * \param lst           List to initialize
 * \param deallocate    Deallocate function
 * \param compare       Comparison function
 */
void list_init(struct list **lst, void (*deallocate)(void *p),
        int (*compare)(void *lhs, void *rhs));

/**
 * Adds an item to the chat list
 *
 * \param lst       List to insert value
 * \param data      Data to insert
 */
void list_add(struct list *lst, void *data);

/**
 * Returns a user from the list
 *
 * \param lst       List to search
 * \param idx       Index of desired item
 */
void* list_get(struct list *lst, int idx);

/**
 * Searches the list for a specific item
 *
 * \param lst       List to search
 * \param item      What to search for
 */
void *list_find(struct list *lst, void *item);

/**
 * Deletes a user from the list
 * 
 * \param lst       List to remove from
 * \param data      Data to delete
 */
void list_remove(struct list *lst, void *data);

/**
 * Runs a function for each user in the list
 *
 * \param lst       List to iterate
 * \param param     Extra parameter to pass to function
 * \param fn        Function to run
 */
void list_for_each(struct list *lst,
        void *param,
        void (*fn)(void *data, void *param));

/**
 * Destroy a list, deallocated all it's contents.
 * Sets dereference parameter to NULL
 *
 * \param lst       List to deallocate
 */
void list_destroy(struct list **lst);

#endif
