// see http://www.zlib.net/manual.html for more info

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "structs.h"
#include <zlib.h>

#include "util.h"
#include "compress.h"

#define LEVEL Z_BEST_COMPRESSION
#define COMPRESS 0
#define DECOMPRESS 1

// static streams used for compression
static z_stream strm_compress;
static z_stream strm_decompress;

/** 
 * Initialize the stream to default values
 * Used in same way by compression and decompression
 * 
 * @param strm zstream to initialize
 */
void _reset_zstream(z_stream *strm) {
    strm->zalloc = Z_NULL;
    strm->zfree = Z_NULL;
    strm->opaque = Z_NULL;
}

void init_compression(void) {
    _reset_zstream(&strm_compress);
    _reset_zstream(&strm_decompress);
    deflateInit(&strm_compress, LEVEL);
    inflateInit(&strm_decompress);
}

void close_compression(void) {
    deflateEnd(&strm_compress);
    inflateEnd(&strm_decompress);
}

/** 
 * Setup the stream for compression and decompression
 * 
 * @param strm The stream to set up.
 * @param data The stream's input.
 * @param result Address, where the result will be written.
 */
void _setup_zstream(z_stream *strm, const payload_t *data, payload_t *result) {
    // setup the correct values in the stream
    strm->avail_in = data->len;
    strm->avail_out = result->len;
    strm->next_in = (unsigned char *) data->stream;
    strm->next_out = (unsigned char *) result->stream;
}

void print_gained(streamlen_t before, streamlen_t after) {
    LOG_INFO("Compressed data is %.5f%% of original size.", ((float) after / before) * 100);
}

/** 
 * Performs compression or decompression on a data stream.
 * 
 * @param mode Either COMPRESS or DECOMPRESS.
 * @param data The stream's input.
 * @param result Address, where the result will be written.
 * 
 * @return Z_OK
 */
int _zlib_manage(int mode, const payload_t data, payload_t *result) {
    int ret;
    z_stream *strm;

    switch (mode) {
    case COMPRESS:
        strm = &strm_compress;
        _setup_zstream(strm, &data, result);
        ret = deflate(strm, Z_FINISH);
        deflateReset(&strm_compress);
        break;

    case DECOMPRESS:
        strm = &strm_decompress;
        _setup_zstream(strm, &data, result);
        ret = inflate(strm, Z_FINISH);
        inflateReset(&strm_decompress);
        break;
    }

    assert(ret == Z_STREAM_END);
    result->len = (result->len - strm->avail_out);
    return Z_OK;
}

int payload_compress(const payload_t data, payload_t *result) {
    return _zlib_manage(COMPRESS, data, result);
}

int payload_decompress(const payload_t data, payload_t *result) {
    return _zlib_manage(DECOMPRESS, data, result);
}
