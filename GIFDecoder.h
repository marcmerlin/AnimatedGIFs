#ifndef _GIFDECODER_H_
#define _GIFDECODER_H_

#include <stdint.h>

typedef void (*callback)(void);
typedef void (*pixel_callback)(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue);
typedef int (*get_bytes_callback)(void * buffer, int numberOfBytes);
typedef void* (*get_buffer_callback)(void);

typedef bool (*file_seek_callback)(unsigned long position);
typedef unsigned long (*file_position_callback)(void);
typedef int (*file_read_callback)(void);
typedef int (*file_read_block_callback)(void * buffer, int numberOfBytes);

typedef struct rgb_24 {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_24;

class GifDecoder {
public:
    int startDecoding(void);
    int decodeFrame(void);
    
    void setScreenClearCallback(callback f);
    void setUpdateScreenCallback(callback f);
    void setDrawPixelCallback(pixel_callback f);
    void setStartDrawingCallback(callback f);

    void setFileSeekCallback(file_seek_callback f);
    void setFilePositionCallback(file_position_callback f);
    void setFileReadCallback(file_read_callback f);
    void setFileReadBlockCallback(file_read_block_callback f);

private:
    void parseTableBasedImage(void);
    void decompressAndDisplayFrame(unsigned long filePositionAfter);
    int parseData(void);
    int parseGIFFileTerminator(void);
    void parseCommentExtension(void);
    void parseApplicationExtension(void);
    void parseGraphicControlExtension(void);
    void parsePlainTextExtension(void);
    void parseGlobalColorTable(void);
    void parseLogicalScreenDescriptor(void);
    bool parseGifHeader(void);
    void copyImageDataRect(uint8_t *dst, uint8_t *src, int x, int y, int width, int height);
    void fillImageData(uint8_t colorIndex);
    void fillImageDataRect(uint8_t colorIndex, int x, int y, int width, int height);
    static int readIntoBuffer(void *buffer, int numberOfBytes);
    int readWord(void);
    void backUpStream(int n);
    int readByte(void);

    void lzw_decode_init(int csize, get_bytes_callback f);
    int lzw_decode(uint8_t *buf, int len, uint8_t *bufend);
    void lzw_setTempBuffer(uint8_t * tempBuffer);
};

#endif
