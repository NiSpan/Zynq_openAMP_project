#ifndef MEMORY_ACCESS_H
#define MEMORY_ACCESS_H

#define OCM_SIZE 	256*1024		/* Size of OCM, 256KB */
#define OCM_LOC  	0xFFFC0000		/* Start location of OCM */
	
#define FREE_DDR_SIZE	((1*1024*1024)+(512*1024))		/* Size of free DDR, 2MB */
#define FREE_DDR_BASE	0x3FE00000	/* Start location of free DDR */

#include <stdint.h>

#include <vector>
#include <math.h>
#include <cv.h>
#include "ipoint.h"


/*
 * Maps the free ddr space to application
 * virtual space and returns pointer to start
 */
void open_ddr();

/*
 * Maps the OCM space to application
 * virtual space and returns pointer to start
 */
void open_ocm();

/*
 * Functions to write from source to OCM and DDR
 * size bytes,unsigned int data
 */
void write_to_ddrU8(uint8_t *source, int size);
void write_to_ocmU8(uint8_t *source, int size);

/*
 * Functions to write from source to OCM and DDR
 * size bytes,integer 32 data
 */
void write_to_ddrINT32(int* source, int size);
void write_to_ocmINT32(int* source, int size);

/*
 * Functions to write from source to OCM and DDR
 * size bytes,float data
 */
void write_to_ddrF32(float *source, int size);
void write_to_ocmF32(float *source, int size);

/*
 * Functions to read from OCM and DDR
 * unsigned int data
 */
void read_from_ddrU8(uint8_t *dst, int size);
void read_from_ocmU8(uint8_t *dst, int size);

/*
 * Functions to read from OCM and DDR
 * integer 32 data
 */
void read_from_ddrINT32(int *dst, int size);
void read_from_ocmINT32(volatile int *dst, int size);

/*
 * Functions to read from OCM and DDR
 * float data
 */
void read_from_ddrF32(float *dst, int size);
void read_from_ocmF32(float *dst, int size);

/*
 * Write coordinates and scale of ipoints
 * to on chip memory
 */
void write_ipts_to_ocm(std::vector<Ipoint> &ipts);

/*
 * Read ipoints orientation and descriptor
 * from ddr memory
 */
void read_ipts_from_ddr(std::vector<Ipoint> &ipts, volatile int ipts_count);
/*
 * Un-maps the memory space for both
 * DDR and OCM, also closes /dev/mem
 */
void close_memory();

#endif
