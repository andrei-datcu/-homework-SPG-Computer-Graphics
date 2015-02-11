#pragma once

#define UNDEFINE_STDC_CONSTANT_MACROS
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <glew/glew.h>

#include <string>

class VideoManager
{
public:
    VideoManager(const std::string &filename);
    ~VideoManager(void);

    static void registerall();

    GLuint get_next_frame_tex();

private:

    void gen_textures(int width, int height);
    void fill_texture(int index);

    unsigned char* buffers[2];

    GLuint textures[2];
    int ctex, videoStream;
    AVCodecContext *pCodecCtx;
    AVFormatContext *pFormatCtx;
    AVCodec *pCodec;
    AVFrame *pFrame;
    struct SwsContext * pSwsCtx;
};

