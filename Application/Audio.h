
#ifndef AUDIO_H
#define AUDIO_H

#include "PacketQueue.h"
extern "C"{

#include <libavformat/avformat.h>

}


/**
 * 播放audio时所需要的数据封装
 */
struct AudioState
{
	const uint32_t BUFFER_SIZE;// 缓冲区的大小

	PacketQueue audioq;

	double audio_clock; // audio clock
	AVStream *stream; // audio stream

	uint8_t *audio_buff;       // 解码后数据的缓冲空间
	uint32_t audio_buff_size;  // buffer中的字节数
	uint32_t audio_buff_index; // buffer中未发送数据的index
	
	int stream_index;          // audio流index
	AVCodecContext *audio_ctx; // 已经调用avcodec_open2打开

	AudioState();              //默认构造函数
	AudioState(AVCodecContext *audio_ctx, int audio_stream);
	
	~AudioState();

	/**
	* audio play
	*/
	bool audio_play();

	void audio_close();

	// get audio clock
	double get_audio_clock();
};
#endif