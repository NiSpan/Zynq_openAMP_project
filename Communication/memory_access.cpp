#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "memory_access.h"

#include <vector>
#include <math.h>
#include <cv.h>
#include "ipoint.h"

int memf = open("/dev/mem", O_RDWR);
volatile void* ddr=NULL;
volatile void* ocm=NULL;

void open_ddr(){
	
	ddr = mmap(NULL, FREE_DDR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memf, FREE_DDR_BASE);
	if(ddr==MAP_FAILED){
		printf("ERROR: Failed to mmap ddr.\n");
		close_memory();
		exit(0);
	}
	memset((void*)(ddr), 0, FREE_DDR_SIZE);
	printf("Mapped DDR: 2 MB @ 0x%x\n", FREE_DDR_BASE);
}

void open_ocm(){
	
	ocm = mmap(NULL, OCM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memf, OCM_LOC);
	if(ocm==MAP_FAILED){
		printf("ERROR: Failed to mmap ocm.\n");
		close_memory();
		exit(0);
	}
	memset((void*)(ocm), 0, OCM_SIZE);
	printf("Mapped ocm: 256 KiB @ 0x%x\n", OCM_LOC);
}

void write_to_ddrU8(uint8_t *source, int size){
	int i;
	uint8_t* tmp = (uint8_t*)ddr;
	for(i=0;i<size;i++){
		tmp[i] = source[i];
	}
}
void write_to_ocmU8(uint8_t *source, int size){
	int i;
	uint8_t* tmp = (uint8_t*)ocm;
	for(i=0;i<size;i++){
		tmp[i] = source[i];
	}
}
void write_to_ddrINT32(int* source, int size){
	int i;
	int* tmp = (int*)ddr;
	for(i=0;i<size/4;i++){
		tmp[i] = source[i];
	}
}
void write_to_ocmINT32(int* source, int size){
	int i;
	int* tmp = (int*)ocm;
	for(i=0;i<size/4;i++){
		tmp[i] = source[i];
	}
}
void write_to_ddrF32(float *source, int size){
	int i;
	float* tmp = (float*)ddr;
	for(i=0;i<size/4;i++){
		tmp[i] = source[i];
	}
}
void write_to_ocmF32(float *source, int size){
	int i;
	float* tmp = (float*)ocm;
	for(i=0;i<size/4;i++){
		tmp[i] = source[i];
	}
}

void read_from_ddrU8(uint8_t *dst, int size){
	int i;
	uint8_t *tmp = (uint8_t *)ddr;
	for(i=0;i<size;i++){
		dst[i] = tmp[i];	
	}
}
void read_from_ocmU8(uint8_t *dst, int size){
	int i;
	uint8_t *tmp = (uint8_t *)ocm;
	for(i=0;i<size;i++){
		dst[i] = tmp[i];	
	}
}
void read_from_ddrINT32(int *dst, int size){
	int i;
	int *tmp = (int *)ddr;
	for(i=0;i<size/4;i++){
		dst[i] = tmp[i];	
	}
}
void read_from_ocmINT32(volatile int *dst, int size){
	int i;
	int *tmp = (int *)ocm;
	for(i=0;i<size/4;i++){
		dst[i] = tmp[i];	
	}
}
void read_from_ddrF32(float *dst, int size){
	int i;
	float *tmp = (float *)ddr;
	for(i=0;i<size/4;i++){
		dst[i] = tmp[i];	
	}
}
void read_from_ocmF32(float *dst, int size){
	int i;
	float *tmp = (float *)ocm;
	for(i=0;i<size/4;i++){
		dst[i] = tmp[i];	
	}
}

void write_ipts_to_ocm(std::vector<Ipoint> &ipts){
	int i;
	*(int*)(ocm) = ipts.size();
	float* tmp = (float*)(ocm);
	for(i=0;i<ipts.size();i++){
		*(tmp+(i*3)+1) = ipts[i].x;
		*(tmp+(i*3)+2) = ipts[i].y;
		*(tmp+(i*3)+3) = ipts[i].scale;	
	}
}

void read_ipts_from_ddr(std::vector<Ipoint> &ipts, volatile int ipts_count){
	int i,j;
	float* tmp = (float*)ddr;
	for(i=0;i<ipts_count;i++){
		Ipoint ipt;
		ipt.x 		= *(tmp+(i*69));
		ipt.y 		= *(tmp+(i*69) + 1);
		ipt.scale	= *(tmp+(i*69) + 2);
		ipt.orientation = *(tmp+(i*69) + 3);
		ipt.laplacian   = (int)(*(tmp+(i*69) + 4));
		for(j=0;j<64;j++){
			ipt.descriptor[j] = *(tmp+((i*69)+5+j));		
		}
		ipts.push_back(ipt);	
	}
}

void close_memory(){
	if(ocm)
		munmap((void*)ocm,OCM_SIZE);	
	if(ddr)
		munmap((void*)ddr,FREE_DDR_SIZE);
	if(memf > 0) 
		close(memf);
}
