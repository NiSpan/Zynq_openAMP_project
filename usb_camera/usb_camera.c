#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <sys/mman.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "usb_camera.h"
#include "my_parameters.h"
#include "neonFunctions.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer       	*buffers;
static unsigned int   	n_buffers;
int                  	fd = -1;
uint8_t			bigbuffer[(WIDTH*HEIGHT)];
uint8_t			smallbuffer[(512*384)]; // buffer if i want scale of output, comment out if not

void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

void clean_memory(void){
	unsigned int i;
	for (i = 0; i < n_buffers; ++i)
        	if (-1 == munmap(buffers[i].start, buffers[i].length))
                	errno_exit("munmap");
	free(buffers);
}

void open_window(const char *window_name){
	cvNamedWindow(window_name,CV_WINDOW_AUTOSIZE);
}

void show_frame(uint8_t *image, const char *window){
	IplImage* cv_image = cvCreateImageHeader(cvSize(WIDTH,HEIGHT), IPL_DEPTH_8U, 1);
	if (!cv_image){
		printf("Error\n");
    		exit(EXIT_FAILURE);
	}
	cvSetData(cv_image, image, cv_image->widthStep);
	cvShowImage(window, cv_image);
	cvWaitKey();
	cvReleaseImageHeader(&cv_image);
}

void show_frame_float(float *image, const char *window){
	IplImage* cv_image = cvCreateImageHeader(cvSize(WIDTH,HEIGHT), IPL_DEPTH_32F, 1);
	if (!cv_image){
		printf("Error\n");
    		exit(EXIT_FAILURE);
	}
	cvSetData(cv_image, image, cv_image->widthStep);
	cvShowImage(window, cv_image);
	cvWaitKey();
	cvReleaseImageHeader(&cv_image);
}

void destroy_window(const char *window_name){
	cvDestroyWindow(window_name);
}

void init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_TYPE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        printf("Device does not support memory mapping\n");
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                printf("Insufficient buffer memory on device\n");
                exit(EXIT_FAILURE);
        }

        buffers = (buffer*)calloc(req.count, sizeof(*buffers));

        if (!buffers) {
                printf("Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type        = V4L2_TYPE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");
		
                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit("mmap");
        }
}

int init_device(const char *device){
	struct v4l2_capability video_cap;
	struct v4l2_format format;
	struct v4l2_fmtdesc descriptor={0};
	struct v4l2_frmsizeenum framesize;
	descriptor.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	framesize.index = 0;

	if((fd = v4l2_open(device, O_RDWR | O_NONBLOCK, 0)) == -1){
	        perror("cam_info: Can't open device");
	}

	if(v4l2_ioctl(fd, VIDIOC_QUERYCAP, &video_cap) == -1)
	        perror("cam_info: Can't get capabilities");
	else {
	        printf("Driver:\t\t '%s'\n", video_cap.driver);
	}

	if(!(video_cap.capabilities & V4L2_CAP_STREAMING)){
		perror("This device does not handle streaming i/o method \n");
	}

		if(v4l2_ioctl(fd, VIDIOC_ENUM_FMT, &descriptor) == -1){
		perror("cam_info: Can't get format descriptor");
	}

	printf("Format: %s\n", descriptor.description);

	framesize.pixel_format = descriptor.pixelformat;

	if(v4l2_ioctl(fd , VIDIOC_ENUM_FRAMESIZES, &framesize) == -1 ){
		perror("cam_info: Can't get frame sizes");
	}

	if (framesize.type == V4L2_FRMSIZE_TYPE_DISCRETE){
		printf("Original height: %d\n", framesize.discrete.height);
		printf("Original width: %d\n", framesize.discrete.width);
	}

	format.type = V4L2_TYPE;
	format.fmt.pix.pixelformat = FORMAT;
	format.fmt.pix.width = WIDTH;
	format.fmt.pix.height = HEIGHT;


	if(v4l2_ioctl(fd, VIDIOC_S_FMT, &format) < 0){
		perror("VIDIOC_S_FMT");
	}
	return fd;
}

void start_capture(void){
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_TYPE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;

                        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == ioctl(fd, VIDIOC_STREAMON, &type)){
		clean_memory();
        	errno_exit("VIDIOC_STREAMON");
	}
}

uint8_t* read_frame(bool SCALE){
	struct v4l2_buffer buf;
	uint8_t *fin_img;
	CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf))
        	errno_exit("VIDIOC_DQBUF");

        assert(buf.index < n_buffers);
	
	YUVtoGRAY_neon(buffers[buf.index].start, bigbuffer,buf.bytesused);
	fin_img = bigbuffer;
	if(SCALE){
		scale_GRAY_to_512x384_neon(bigbuffer, smallbuffer,WIDTH);
		fin_img = smallbuffer;
	}
        
	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        	errno_exit("VIDIOC_QBUF");
	return fin_img;
}
