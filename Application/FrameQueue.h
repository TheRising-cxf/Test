
#ifndef FRAME_QUEUE_H
#define FRAME_QUEUE_H

#include <SDL_thread.h>
#include<mutex>
#include <queue>

extern "C"{

#include <libavcodec\avcodec.h>

}

struct FrameQueue
{
	static const int capacity = 30;

	std::queue<AVFrame*> queue;

	uint32_t nb_frames;
	std::mutex g_mutex;
	FrameQueue();
	bool enQueue(const AVFrame* frame);
	bool deQueue(AVFrame **frame);
};



#endif