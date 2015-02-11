#include "stdafx.h"
#include "VideoManager.h"
#include <chrono>
#include <thread>
#include <iostream>

void VideoManager::registerall()
{
    // Register all formats and codecs
    av_register_all();
}

VideoManager::VideoManager(const std::string &filename)
    : ctex(0)
{

    // Open video file
    if (avformat_open_input(&pFormatCtx, filename.c_str(), NULL, NULL) != 0)
        throw std::string("Nu pot deschide fisierul"); // Couldn't open file

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
       throw std::string("Stream invalid"); // Couldn't find stream information

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, filename.c_str(), 0);

    // Find the first video stream
    videoStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }

    if (videoStream == -1)
        throw std::string("Nu exista niciun stream video in fisier"); // Didn't find a video stream
        
    // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        throw std::string("Codecul nu este suportat");// Codec not found
    }

    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        throw std::string("Nu se poate deschide codecul"); // Could not open codec

    // Allocate video frame
    pFrame = av_frame_alloc();

    pSwsCtx = sws_getCachedContext(pSwsCtx, pCodecCtx->width, pCodecCtx->height,
        pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
        PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
    if (pSwsCtx == NULL)
        throw std::string("Cannot initialize the sws context\n");


    //Initializam bufferele interne
    buffers[0] = new unsigned char[pCodecCtx->width * pCodecCtx->height * 3];
    buffers[1] = new unsigned char[pCodecCtx->width * pCodecCtx->height * 3];

    gen_textures(pCodecCtx->width, pCodecCtx->height);
}


VideoManager::~VideoManager(void)
{
    // Free the YUV frame
    av_free(pFrame);
    // Close the codec
    avcodec_close(pCodecCtx);
    // Close the video file
    avformat_close_input(&pFormatCtx);

    delete[] buffers[0];
    delete[] buffers[1];
}


void VideoManager::gen_textures(int width, int height)
{
    for (int i = 0; i < 2; ++i) {
        glGenTextures(1, &textures[i]);
		glBindTexture(GL_TEXTURE_2D, textures[i]);

		//filtrare
		//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
            GL_LINEAR);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
            GL_LINEAR);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, pCodecCtx->width, pCodecCtx->height,
            0, GL_RGB, GL_UNSIGNED_BYTE, buffers[i]);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void VideoManager::fill_texture(int index)
{
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    glTexSubImage2D(GL_TEXTURE_2D, 0,0, 0, pCodecCtx->width, pCodecCtx->height,
        GL_RGB, GL_UNSIGNED_BYTE, buffers[index]);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint VideoManager::get_next_frame_tex()
{
    static AVPacket packet, tmppack;
    static bool read_frame = true;

    if (read_frame){
        if (av_read_frame(pFormatCtx, &packet) < 0) {
            av_seek_frame(pFormatCtx, videoStream, 0, AVSEEK_FLAG_FRAME);
            return get_next_frame_tex();
        }
        tmppack.data = packet.data;
        tmppack.size = packet.size;
    }

    if (packet.stream_index != videoStream) {
        return get_next_frame_tex();
    }

    // Decode video frame
    read_frame = false;

    int frame_finished;
    int size;
    if (packet.size <= 0 || (size = avcodec_decode_video2(pCodecCtx, pFrame, &frame_finished, &packet)) <= 0) {
        packet.size = tmppack.size;
        packet.data = tmppack.data;
        av_free_packet(&packet);
        read_frame = true;
        return get_next_frame_tex();
    }

    // Did we get a video frame?
    if (!frame_finished) {
        return  get_next_frame_tex();
    }

    static auto last_frame_t = std::chrono::system_clock::now();

    auto now = std::chrono::system_clock::now();
    int dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_frame_t).count();
    int msecs = 1000 * pCodecCtx->framerate.den / pCodecCtx->framerate.num;

    if (dur < msecs)
        std::this_thread::sleep_for(std::chrono::milliseconds(msecs - dur));

    last_frame_t = std::chrono::system_clock::now();

    packet.data += size;
    packet.size -= size;
    AVPicture pict;
    ctex ^= 1;
    pict.data[0] = buffers[ctex];
    pict.linesize[0] = 3 * pCodecCtx->width;
    // Convert the image into YUV format that SDL uses
    sws_scale(pSwsCtx, (const uint8_t * const *) pFrame->data,
        pFrame->linesize, 0, pCodecCtx->height, pict.data,
        pict.linesize);
    
    fill_texture(ctex);

    return textures[ctex];
}