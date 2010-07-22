
#ifndef __TUNNEL_H
#define __TUNNEL_H

#include <arpa/inet.h>

#define MAX_QUEUED 10

// implementation of a FIFO queue as a circular array
typedef struct write_queue {
    char *messages[MAX_QUEUED];
    int first;
    int last;
} write_queue;

// structure of a tun device
typedef struct tundev {
    char *ifname;
    int fd;
    int client; // client we're serving
    write_queue queue;
} tundev;


/*************************/
/* Function declarations */
/*************************/

/** 
 * Creates a new tun/tap device, or connects to a already existent one depending
 * on the arguments.
 * 
 * @param dev The name of the device to connect to or '\0' when a new device should be
 *            should be created.
 * 
 * @return Error-code.
 */
int tunOpen(int client_no, char *dev);

/** 
 * Reads data from the tunnel and exits if a error occurred.
 * 
 * @param client_no
 * @param buf This is where the read data are written.
 * @param length maximum number of bytes to read.
 * 
 * @return number of bytes read.
 */
int tunRead(int client_no, char *buf, int n);

/** 
 * Add one message to the write queue
 * and then try to send them all out
 * 
 * @param client_no client connected
 * @param buf buffer to send
 * @param len length of the buffer
 */
void addToWriteQueue(int client_no, char *buf, int len);

/** 
 * Setup the tunnel module
 * 
 * @param tun_flags IFF_TUN or IFF_TAP basically for tun/tap devices
 */
void tunSetup(int tun_flags);

#endif
