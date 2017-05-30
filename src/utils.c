/* fsusb2i   (c) 2015-2016 trinity19683
  util functions for recfsusb2i (Linux OSes)
  utils.c
  2016-01-05
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

#include "utils.h"
#include "message.h"

static void usage(const char *argv0)
{
	msg("Usage: %s", argv0);
	msg(" [-v] CHANNEL RECSEC DESTFILE\n");
	exit(1);
}

void parseOption(int argc, char * const argv[], struct Args* args)
{
	int c;
	char *ptr;
	memset(args, 0, sizeof(struct Args));
	for(;;) {
		int option_index = 0;
		static struct option long_options[] = {
			{"dev", required_argument, NULL, 200},
			{"nd", no_argument, NULL, 500},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, "v", long_options, &option_index);
		if(0 > c) break;
		switch( c ) {
		case 'v':   //# verbose
			args->flags |= 0x1;
			break;
		case 200:   //# specify devfile (usbdevfs)
			if( optarg )
				args->devfile = strdup(optarg);
			break;
		case 500:   //# disable descrambling (STD-B25)
			args->flags |= 0x1000;
			break;
		}
	}

	if(argc - optind != 3) {
		usage(argv[0]);
	}
	ptr = argv[optind++];
	c = atoi(ptr);
	if(c < 13) {
		warn_msg(0,"invalid channel '%s'", ptr);
		exit(1);
	}else if(c <= 62) {
		//# UHF 13-62ch
		args->freq = (c - 13) * 6000 + 473143;
	}else if(c < 1000) {
		//# MHz
		args->freq = c * 1000 + 143;
	}else{
		//# kHz
		args->freq = c;
	}
	ptr = argv[optind++];
	if('-' == ptr[0] || (c = atoi(ptr)) <= 0) {
		args->recsec = 0;
	}else{
		args->recsec = c;
	}
	ptr = argv[optind++];
	if(strcmp("-", ptr) != 0) {
		args->destfile = ptr;
	}
}

/* change signal handler (SIGINT, SIGTERM) */
void setSignalHandler(const int mode, void (*func_handler)(int))
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	if(0 == mode) {
		sa.sa_handler = SIG_DFL;
	}else{
		sa.sa_handler = func_handler;
		sa.sa_flags = SA_RESTART;
	}
	sigaction(SIGINT , &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

void u_difftime(struct timespec* ta, struct timespec* tb, int* tsec, int* tmsec)
{
	int vsec, vusec;
	vsec = (int)(tb->tv_sec - ta->tv_sec);
	if(tb->tv_nsec < ta->tv_nsec) {
		vsec--;
		vusec = (1000000000 + tb->tv_nsec - ta->tv_nsec) / 1000000;
	}else{
		vusec = (tb->tv_nsec - ta->tv_nsec) / 1000000;
	}
	if(NULL == tsec) {
		if(NULL != tmsec) *tmsec = vsec * 1000 + vusec;
	}else{
		*tsec = vsec;
		if(NULL != tmsec) *tmsec = vusec;
	}
}

/* *  OutputBuffer  * */

int OutputBuffer_release(struct OutputBuffer* const  pThis)
{
	if( pThis->pOutput ) {
		OutputBuffer_release(pThis->pOutput);
	}
	if( pThis->release ) {
		pThis->release( pThis );
	}
	free( pThis->buffer );
	return 0;
}

int OutputBuffer_put(struct OutputBuffer* const  pThis, void *buf, unsigned length)
{
	int r;
	if(pThis->length + length > pThis->bufSize) {
		r = pThis->process(pThis, NULL);
		if(0 > r)  return r;
		pThis->length = 0;
	}else{
		goto copyToBuffer;
	}
	while(length >= pThis->bufSize) {
		r = pThis->process(pThis, buf);
		if(0 > r)  return r;
		buf += pThis->bufSize;
		length -= pThis->bufSize;
	}
copyToBuffer:
	if(length > 0) {
		memcpy(pThis->buffer + pThis->length, buf, length);
		pThis->length += length;
	}
	return 0;
}

int OutputBuffer_flush(struct OutputBuffer* const  pThis)
{
	int r;
	if(0 < pThis->length) {
		r = pThis->process(pThis, NULL);
		if(0 > r)  return r;
		pThis->length = 0;
	}
	if( pThis->pOutput ) {
		r = OutputBuffer_flush(pThis->pOutput);
		if(0 > r)  return r;
	}
	return 0;
}

struct FileBufferedWriter_Data {
	FILE *fd;
};

static int FileBufferedWriter_release(struct OutputBuffer* const  pThis)
{
	struct FileBufferedWriter_Data* const  prv = (struct FileBufferedWriter_Data*)(pThis + 1);
	if(prv->fd != NULL) {
		fflush( prv->fd );
		if( stdout != prv->fd && fclose( prv->fd ) )
			warn_msg(errno,"failed to close output_file");
	}
	return 0;
}

static int FileBufferedWriter_process(struct OutputBuffer* const  pThis, void* const buf)
{
	struct FileBufferedWriter_Data* const  prv = (struct FileBufferedWriter_Data*)(pThis + 1);
	int r;
	void* ptr = (buf)? buf : pThis->buffer;
	int length = (buf)? pThis->bufSize : pThis->length;

	while(length > 0) {
		r = fwrite(ptr, 1, length, prv->fd);
		if(0 > r)  return r;
		else if(0 < r) {
			length -= r;
		}
	}
	return 0;
}

struct OutputBuffer* create_FileBufferedWriter(unsigned  bufSize, const char* const  filename)
{
	bufSize = (bufSize + 0xF) & ~0xFU;

	void* const  pBuffer = malloc(bufSize + sizeof(struct OutputBuffer) + sizeof(struct FileBufferedWriter_Data) );
	struct OutputBuffer* const  pThis = (pBuffer + bufSize);
	struct FileBufferedWriter_Data* const  prv = (struct FileBufferedWriter_Data*)(pThis + 1);
	if(! pBuffer) {
		return NULL;
	}

	//# open output file
	FILE *fd;
	if( filename ) {
		if(!( fd = fopen(filename, "a") )) {
			warn_msg(errno,"failed to open output_file '%s'", filename);
			return NULL;
		}
	}else{
		fd = stdout;
	}

	pThis->buffer = pBuffer;
	pThis->pOutput = NULL;
	pThis->bufSize = bufSize;
	pThis->length = 0;
	pThis->process = FileBufferedWriter_process;
	pThis->release = FileBufferedWriter_release;
	prv->fd = fd;
	return pThis;
}

#ifdef STD_B25
#include <arib25/arib_std_b25.h>
#include <arib25/b_cas_card.h>
#endif

#define TS_PACKET_SIZE  188

struct TSParser_Data {
	unsigned  synced_packet_count;
#ifdef STD_B25
	ARIB_STD_B25 *b25;
	B_CAS_CARD *b25cas;
#endif
};

static int TSParser_release(struct OutputBuffer* const  pThis)
{
	struct TSParser_Data* const  prv = (struct TSParser_Data*)(pThis + 1);
#ifdef STD_B25
	if( prv->b25 )
		prv->b25->release( prv->b25 );
	if( prv->b25cas )
		prv->b25cas->release( prv->b25cas );
#endif
	return 0;
}

static int TSParser_process(struct OutputBuffer* const  pThis, void* const buf)
{
	struct TSParser_Data* const  prv = (struct TSParser_Data*)(pThis + 1);
	int r;
	void* ptr = (buf)? buf : pThis->buffer;
	unsigned length = (buf)? pThis->bufSize : pThis->length;
	if(! length)  return 0;

	if( prv->synced_packet_count & 0xFFFF ) {  //# finding TS packet SYNC
		unsigned idx = prv->synced_packet_count >> 16, pkt_count = prv->synced_packet_count & 0xFFFF;
		char* const  p = ptr;
		//dmsgn("idx=%u, ",idx);
		for(; idx < length; idx += TS_PACKET_SIZE) {
			if((p[idx] & 0xFF) == 0x47) {
				pkt_count ++;
			}else{
				pkt_count = 1;
				for(; idx < length; idx ++) {
					if((p[idx] & 0xFF) == 0x47)  break;
				}
				//dmsgn("resync=%u, ",idx);
			}
		}
		//dmsg("end=%u",idx - length);
		if(800 > pkt_count || TS_PACKET_SIZE * 10 > length) {
			prv->synced_packet_count = (idx - length) << 16 | (pkt_count & 0xFFFF);
			return 0;
		}else{
			idx -= TS_PACKET_SIZE * 10;
			ptr += idx;
			length -= idx;
			//dmsg("last=%u",idx);
			prv->synced_packet_count = 0;
		}
	}
	if(pThis->pOutput) {
#ifdef STD_B25
		if( prv->b25 ) {
			ARIB_STD_B25_BUFFER b25_sbuf;
			ARIB_STD_B25_BUFFER b25_dbuf;
			b25_sbuf.data = ptr;
			b25_sbuf.size = length;
			if((r = prv->b25->put(prv->b25, &b25_sbuf) ) < 0) {
				warn_msg(r,"StdB25.put failed");
			}
			if((r = prv->b25->get(prv->b25, &b25_dbuf) ) < 0) {
				warn_msg(r,"StdB25.get failed");
			}else{
				ptr = b25_dbuf.data;
				length = b25_dbuf.size;
			}
		}
#endif
		r = OutputBuffer_put(pThis->pOutput, ptr, length);
		if(0 > r)  return r;
	}
	return 0;
}

#ifdef STD_B25
static void init_STD_B25(ARIB_STD_B25 ** const  pB25, B_CAS_CARD ** const  pB25cas)
{
	int r;
	const unsigned  MULTI2_round = 4;

	ARIB_STD_B25* const  b25 = create_arib_std_b25();
	if(b25) {
		msg("# StdB25 interface : ");
	}else{
		warn_msg(0,"Create StdB25 interface failed");
		goto skip_init1;
	}
	r = b25->set_multi2_round(b25, MULTI2_round );
	if(0 > r) {
		warn_msg(r,"StdB25.MULTI2 round=%u failed", MULTI2_round);
		goto skip_init2;
	}
	b25->set_strip(b25, 1 );
	b25->set_emm_proc(b25, 0 );

	B_CAS_CARD* const  bc = create_b_cas_card();
	if(! bc) {
		warn_msg(0,"Create CAS interface failed");
		goto skip_init2;
	}
	r = bc->init(bc);
	if(0 > r) {
		warn_msg(r,"Init CAS failed");
		goto skip_init3;
	}
	r = b25->set_b_cas_card(b25, bc);
	if(0 > r) {
		warn_msg(r,"StdB25.SetCAS failed");
		goto skip_init3;
	}
	*pB25 = b25;
	*pB25cas = bc;

	B_CAS_ID casID;
	r = bc->get_id(bc, &casID);
	if(r < 0) {
		warn_msg(r,"Get CAS ID failed");
	}else{
		for(r = 0; r < casID.count; r++) {
			const uint64_t  val = casID.data[r];
			dmsgn("%u%014llu, ", (val >> 45) & 0x7,val & ((1ULL << 45) - 1));
		}
	}
	msg(" done.\n");
	return;

skip_init3:
	bc->release(bc);
skip_init2:
	b25->release(b25);
skip_init1:
	*pB25 = NULL;
	*pB25cas = NULL;
}
#endif

struct OutputBuffer* create_TSParser(unsigned  bufSize, struct OutputBuffer* const  pOutput, const unsigned  mode)
{
	const unsigned min_bufSize =  TS_PACKET_SIZE * 512;
	bufSize = (bufSize > min_bufSize)? (bufSize + 0xF) & ~0xFU  : min_bufSize;

	void* const  pBuffer = malloc(bufSize + sizeof(struct OutputBuffer) + sizeof(struct TSParser_Data) );
	struct OutputBuffer* const  pThis = (pBuffer + bufSize);
	struct TSParser_Data* const  prv = (struct TSParser_Data*)(pThis + 1);
	if(! pBuffer) {
		return NULL;
	}

	pThis->buffer = pBuffer;
	pThis->pOutput = pOutput;
	pThis->bufSize = bufSize;
	pThis->length = 0;
	pThis->process = TSParser_process;
	pThis->release = TSParser_release;
	prv->synced_packet_count = 1;
#ifdef STD_B25
	if(mode & 0x1) {
		init_STD_B25(&( prv->b25 ), &( prv->b25cas ));
	}else{
		prv->b25 = NULL;
		prv->b25cas = NULL;
	}
#endif
	return pThis;
}

/*EOF*/