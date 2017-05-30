/* fsusb2i   (c) 2015-2016 trinity19683
  util functions for recfsusb2i (Linux OSes)
  utils.h
  2016-01-05
*/
#pragma once

#include <time.h>

/* options, parameters */
struct Args {
	unsigned int flags;
	unsigned int freq;
	unsigned int recsec;
	char* devfile;
	char* destfile;
};

void parseOption(int argc, char * const argv[], struct Args *);
void setSignalHandler(const int mode, void (*func_handler)(int));

#define u_gettime(tp)  clock_gettime(CLOCK_MONOTONIC, tp)
void u_difftime(struct timespec *, struct timespec *, int *, int *);

struct OutputBuffer {
	void *buffer;
	struct OutputBuffer* pOutput;
	int bufSize;
	int length;
	int (* process)(struct OutputBuffer* const, void* const buf);
	int (* release)(struct OutputBuffer* const);
};

int OutputBuffer_release(struct OutputBuffer* const);
int OutputBuffer_put(struct OutputBuffer* const, void *buf, unsigned length);
int OutputBuffer_flush(struct OutputBuffer* const);
struct OutputBuffer* create_FileBufferedWriter(unsigned  bufSize, const char* const);
struct OutputBuffer* create_TSParser(unsigned  bufSize, struct OutputBuffer* const  pOutput, const unsigned  mode);


/*EOF*/