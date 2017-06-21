#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <ctime>
#include <iostream>

#include "usb_camera.h"
#include "zynq_SURF.h"
#include "my_parameters.h"
/*
 *  What input do i have for surf?
 * -1- static image 
 * -2- capture from usb camera 8bit
 * -3- capture from usb camera 32bit float
 */
#define PROCEDURE	1

int staticImageSurf(void){
	IplImage *cv_img=cvLoadImage("imgs/2.jpg");
	float* int_img;	
	struct timeval begin,end;
	printf("\n********************\n");
	printf("CV ipl image parameters\n");
	printf("Width  : %d\n",cv_img->width);
	printf("Height : %d\n\n",cv_img->height);
	printf("Number of channels  : %d\n",cv_img->nChannels);
	printf("Image size in bytes : %d\n",cv_img->imageSize);
	printf("Image width step    : %d\n",cv_img->widthStep);
	printf("\n********************\n");
	
	
	IplImage *gray=getGray(cv_img);	
	unsigned char *my_img = (unsigned char *)gray->imageData;
	gettimeofday(&begin,NULL);
	int_img = computeIntegral_NEON(my_img);
	gettimeofday(&end,NULL);	
	printf("Integral image compute took : %4lf seconds\n ",((end.tv_sec-begin.tv_sec)+((end.tv_usec-begin.tv_usec)/1000000.0)));
	
	printf("%0.4f\n",(int_img[640])*(1.0/255.0));

	printf("\n********************\n");
	printf("CV ipl image parameters\n");
	printf("Width  : %d\n",gray->width);
	printf("Height : %d\n\n",gray->height);
	printf("Number of channels  : %d\n",gray->nChannels);
	printf("Image size in bytes : %d\n",gray->imageSize);
	printf("Image width step    : %d\n",gray->widthStep);
	printf("\n********************\n");

	open_window("Test");
	show_frame(my_img,"Test");	
	destroy_window("Test");

	cvReleaseImage(&cv_img);
	cvReleaseImage(&gray);

	return 0;
}

int video8bit(void){
	// init camera	
	int fd = init_device("/dev/video0");	
	// number of frames to capture	
	int count=FRAME_COUNT;			
	// frame store	
	uint8_t* image=(uint8_t*)calloc(WIDTH*HEIGHT,sizeof(uint8_t));				
	struct timeval begin,end;
	
	// mmaps buffers from device
	// init stream camera and open cv window		
	init_mmap();
	start_capture();
	open_window("Test");
	
	while (count-- > 0) {
		fd_set fds;
                struct timeval tv;
                int r;

                FD_ZERO(&fds);
                FD_SET(fd, &fds);

                /* Timeout. */
                tv.tv_sec = 2;
                tv.tv_usec = 0;

                r = select(fd + 1, &fds, NULL, NULL, &tv);
		gettimeofday(&begin,NULL);
		image = read_frame(SCALING);
		gettimeofday(&end,NULL);
		printf("Frame took : %4lf seconds \n ",((end.tv_sec-begin.tv_sec)+((end.tv_usec-begin.tv_usec)/1000000.0)));
		show_frame(image,"Test");
	}
	destroy_window("Test");
	clean_memory();
	free(image);
	close(fd);
	printf("The end.\n");
	return 0;
}

int video32bitFloat(void){
	// init camera
	int fd = init_device("/dev/video0");			
	// number of frames to capture	
	int count=FRAME_COUNT;					
	// array for camera frame	
	uint8_t* image     =(uint8_t*)calloc(WIDTH*HEIGHT,sizeof(uint8_t));				
	// array for new float frame	
	float*	 float_img =(float*)calloc(WIDTH*HEIGHT,sizeof(float));			
	
	struct timeval begin,end;

	// mmaps buffers from device
	// init stream camera and open cv window
	init_mmap();
	start_capture();
	open_window("Test");

	while (count-- > 0) {
		fd_set fds;
                struct timeval tv;
                int r;

                FD_ZERO(&fds);
                FD_SET(fd, &fds);

                /* Timeout. */
                tv.tv_sec = 2;
                tv.tv_usec = 0;

                r = select(fd + 1, &fds, NULL, NULL, &tv);
		gettimeofday(&begin,NULL);
		image = read_frame(SCALING);
		gettimeofday(&end,NULL);
		printf("Read frame took : %4lf seconds \n ",((end.tv_sec-begin.tv_sec)+((end.tv_usec-begin.tv_usec)/1000000.0)));
		
		gettimeofday(&begin,NULL);
		turn_to_gray_float(image, float_img, WIDTH, HEIGHT);
		gettimeofday(&end,NULL);
		printf("Turn to float took : %4lf seconds \n ",((end.tv_sec-begin.tv_sec)+((end.tv_usec-begin.tv_usec)/1000000.0)));
		show_frame_float(float_img,"Test");
	}
	destroy_window("Test");
	clean_memory();
	free(image);
	free(float_img);
	close(fd);
	printf("The end.\n");
}

int main(){
	if(PROCEDURE == 1) return staticImageSurf();
	if(PROCEDURE == 2) return video8bit();
	if(PROCEDURE == 3) return video32bitFloat();
	return 0;
}
