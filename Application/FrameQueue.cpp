
#include "FrameQueue.h"

FrameQueue::FrameQueue()
{
	nb_frames = 0;

}

bool FrameQueue::enQueue(const AVFrame* frame)
{
	AVFrame* p = av_frame_alloc();

	int ret = av_frame_ref(p, frame);
	if (ret < 0)
		return false;

	p->opaque = (void *)new double(*(double*)p->opaque); //上一个指向的是一个局部的变量，这里重新分配pts空间

	g_mutex.lock();
	queue.push(p);

	nb_frames++;
	
	g_mutex.unlock();
	
	return true;
}

bool FrameQueue::deQueue(AVFrame **frame)
{
	bool ret = true;

	g_mutex.lock();
	while (true)
	{
		if (!queue.empty())
		{
			if (av_frame_ref(*frame, queue.front()) < 0)
			{
				ret = false;
				break;
			}

			auto tmp = queue.front();
			queue.pop();

			av_frame_free(&tmp);

			nb_frames--;

			ret = true;
			break;
		}
		else
		{
			ret = false;
			break;
			//SDL_CondWait(cond, mutex);
		}
	}

	g_mutex.unlock();
	return ret;
}