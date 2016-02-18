#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <stdint.h>

/* struct forward declarations */
struct protocol_state;
struct pbuf;

/** this sets up a newly received connection
 *
 * This function sets up everything the protocol needs if it accepts the
 * new connection. Otherwise it signals it back to the ethernet driver
 * which closes the connection.
 *
 * @return a pointer to use as arg, NULL if rejected
 */
struct protocol_state* protocol_accept_connection(void);

/* this function is called then the TCP connection got reset.
 * The protocol may no longer use the connection and should clean up all
 * the used resources */
void protocol_remove_connection(struct protocol_state*);

/** this function is the entry point for the protocol parser
 *
 * @param state the protcol state which was returned by accept_connection
 * @param p pointer to the pbuf pointer. This is a **, because it changes
 * the pbuf-pointer once it has handled the packet
 * @return the amount of bytes processed
 */
uint16_t protocol_handle_packet(struct protocol_state* state,  struct pbuf** p);

#endif /* _PROTOCOL_H */
