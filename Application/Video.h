
#ifndef VIDEO_H
#define VIDEO_H

#include "PacketQueue.h"
#include "FrameQueue.h"
#include "Media.h"

struct MediaState;
/**
 * 播放音频所需的数据封装
 */
struct VideoState
{
	PacketQueue videoq;        // 保存的video packet的队列缓存

	int stream_index;           // index of video stream
	AVCodecContext *video_ctx;  // have already be opened by avcodec_open2
	AVStream *stream;           // video stream

	FrameQueue frameq;          // 保存解码后的原始帧数据
	AVFrame *frame;
	AVFrame *displayFrame;

	double frame_timer;         // Sync fields
	double frame_last_pts;
	double frame_last_delay;
	double video_clock;

	void videoPlayInit(MediaState *media);

	double synchronize(AVFrame *srcFrame, double pts);
	
	VideoState();

	~VideoState();
};

#endif