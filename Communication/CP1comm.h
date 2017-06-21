#ifndef CP1COMM_H
#define CP1COMM_H

#define SHUTDOWN_MSG	0xEF56A55A
#define SIGNAL_START	0x00000001

#define RPMSG_GET_KFIFO_SIZE 1
#define RPMSG_GET_AVAIL_DATA_SIZE 2
#define RPMSG_GET_FREE_SPACE 3

/*
 * Opens rpmsg device file for
 * writing signals to cpu1
 */
void open_comm();

/*
 * Writes command signal to rpmsg
 * channel for cpu1 to read, only signal start
 * for now
 */
void write_to_cpu1();

/*
 * Reads from rpmsg channel
 * Does not return until it gets a
 * response
 */
void read_from_cpu1();

/*
 * Sends shutdown signal to cpu1 and
 * closes rpmsg device file
 */
void close_comm();

#endif
