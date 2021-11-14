
#include "Video.h"
#include "mutex"
#include <Windows.h>
extern "C"{

#include <libswscale\swscale.h>

}
VideoState::VideoState()
{
	video_ctx        = nullptr;
	stream_index     = -1;
	stream           = nullptr;

	frame            = nullptr;
	displayFrame     = nullptr;

	frame_timer      = 0.0;
	frame_last_delay = 0.0;
	frame_last_pts   = 0.0;
	video_clock      = 0.0;
}

VideoState::~VideoState()
{
	avcodec_close(video_ctx);
	if(frame!=NULL)
		av_frame_free(&frame);
	if (displayFrame != NULL)
	{
		if (displayFrame->data[0] != NULL)
			av_free(displayFrame->data[0]);
		av_frame_free(&displayFrame);
	}
	while (!videoq.queue.empty()) {
		AVPacket pkt = videoq.queue.front();
		videoq.queue.pop();
		av_free_packet(&pkt);
	}
	while (!frameq.queue.empty()) {
		AVFrame *frame = frameq.queue.front();
		frameq.queue.pop();
		av_frame_free(&frame);
	}
}

void VideoState::videoPlayInit(MediaState *media)
{
	int width = media->frameWidth;
	int height = media->frameHeight;

	frame = av_frame_alloc();
	displayFrame = av_frame_alloc();

	displayFrame->format = AV_PIX_FMT_RGB32;
	displayFrame->width = width;
	displayFrame->height = height;

	int numBytes = avpicture_get_size((AVPixelFormat)displayFrame->format,displayFrame->width, displayFrame->height);
	uint8_t *buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

	avpicture_fill((AVPicture*)displayFrame, buffer, (AVPixelFormat)displayFrame->format, displayFrame->width, displayFrame->height);
}

double VideoState::synchronize(AVFrame *srcFrame, double pts)
{
	double frame_delay;

	if (pts != 0)
		video_clock = pts; // Get pts,then set video clock to it
	else
		pts = video_clock; // Don't get pts,set it to video clock

	frame_delay = av_q2d(stream->codec->time_base);
	frame_delay += srcFrame->repeat_pict * (frame_delay * 0.5);

	video_clock += frame_delay;

	return pts;
}
