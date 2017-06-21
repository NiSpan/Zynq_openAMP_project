#ifndef USB_CAMERA_H
#define USB_CAMERA_H
#include <sys/types.h>

/*
 * Struct for storing frames
 */
struct buffer {
        void   *start;
        size_t  length;
};

/*
 * Prints error message
 */
void errno_exit(const char *s);

/*
 * Releases memory space used by the buffers of
 * v4l2 application
 */
void clean_memory(void);

/*
 * Creates a CV window with the given name
 */
void open_window(const char *window_name);

/*
 * Shows on screen one frame
 */
void show_frame(uint8_t *image, const char *window_name);

/*
 * Shows on screen one frame of float precision
 */ 
void show_frame_float(float *image, const char *window_name);

/*
 * Destroys a CV window with the given name
 */
void destroy_window(const char *window_name);

/*
 * Querys the capabilities of the device file and
 * tries to set the format given.
 */
int init_device(const char *device);

/*
 * Requests buffers from the device memory space
 * and maps them to the application's address
 * space
 */
void init_mmap(void);

/*
 * Queues buffers and starts streaming from
 * capture device
 */
void start_capture(void);

/*
 * Dequeues a buffer, reads a frame and returns
 * a pointer to the start of the frame, but
 * in single channel 8bit with neon
 * If SCALE ,returns 512x384
 */
uint8_t* read_frame(bool SCALE);
#endif
