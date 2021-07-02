
#include "VideoDisplay.h"
#include <iostream>

extern "C"{

#include <libswscale\swscale.h>
#include <libavutil\time.h>

}
#include <QImage>

static const double SYNC_THRESHOLD = 0.01;
static const double NOSYNC_THRESHOLD = 10.0;

// �ӳ�delay ms��ˢ��video֡
void schedule_refresh(MediaState *media, int delay)
{
	SDL_AddTimer(delay, sdl_refresh_timer_cb, media);
}

uint32_t sdl_refresh_timer_cb(uint32_t interval, void *opaque)
{
	SDL_Event event;
	event.type = FF_REFRESH_EVENT;
	event.user.data1 = opaque;
	SDL_PushEvent(&event);
	return 0; /* 0 means stop timer */
}

bool video_refresh_timer(void *userdata)
{
	MediaState *media = (MediaState*)userdata;
	VideoState *video = media->video;

	if (video->stream_index >= 0)
	{
		if (video->videoq->queue.empty())
		{
			schedule_refresh(media, 1);
			return false;
		}

		else
		{
			video->frameq.deQueue(&video->frame);

			// ����Ƶͬ������Ƶ�ϣ�������һ֡���ӳ�ʱ��
			double current_pts = *(double*)video->frame->opaque;
			double delay = current_pts - video->frame_last_pts;
			if (delay <= 0 || delay >= 1.0)
				delay = video->frame_last_delay;

			video->frame_last_delay = delay;
			video->frame_last_pts = current_pts;

			// ��ǰ��ʾ֡��PTS��������ʾ��һ֡���ӳ�
			double ref_clock = media->audio->get_audio_clock();

			double diff = current_pts - ref_clock;// diff < 0 => video slow,diff > 0 => video quick

			double threshold = (delay > SYNC_THRESHOLD) ? delay : SYNC_THRESHOLD;

			if (fabs(diff) < NOSYNC_THRESHOLD) // ��ͬ��
			{
				if (diff <= -threshold) // ���ˣ�delay��Ϊ0
					delay = 0;
				else if (diff >= threshold) // ���ˣ��ӱ�delay
					delay *= 2;
			}
			video->frame_timer += delay;
			double actual_delay = video->frame_timer - static_cast<double>(av_gettime()) / 1000000.0;
			if (actual_delay <= 0.010)
				actual_delay = 0.010; 

			schedule_refresh(media, static_cast<int>(actual_delay * 1000 + 0.5));

			SwsContext *sws_ctx = sws_getContext(video->video_ctx->width, video->video_ctx->height, video->video_ctx->pix_fmt,
			video->displayFrame->width,video->displayFrame->height,(AVPixelFormat)video->displayFrame->format, SWS_BICUBIC, nullptr, nullptr, nullptr);

			sws_scale(sws_ctx, (uint8_t const * const *)video->frame->data, video->frame->linesize, 0, 
				video->video_ctx->height, video->displayFrame->data, video->displayFrame->linesize);

			sws_freeContext(sws_ctx);
			return true;
		}
	}
	else
	{
		schedule_refresh(media, 100);
		return false;
	}
}

bool video_refresh_timer_test(void * userdata)
{
	MediaState *media = (MediaState*)userdata;

	AVPacket *packet = av_packet_alloc();

	if (av_read_frame(media->pFormatCtx, packet) >= 0) {
		if (packet->stream_index == media->video->stream_index) {
			int ret = avcodec_send_packet(media->video->video_ctx, packet);
			if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
			{
				av_packet_unref(packet);
				return false;
			}

			ret = avcodec_receive_frame(media->video->video_ctx, media->video->frame);
			if (ret < 0 && ret != AVERROR_EOF)
			{
				av_packet_unref(packet);
				return false;
			}
			SwsContext *sws_ctx = sws_getContext(media->video->video_ctx->width, media->video->video_ctx->height, media->video->video_ctx->pix_fmt,
				media->video->displayFrame->width, media->video->displayFrame->height, (AVPixelFormat)media->video->displayFrame->format, SWS_BICUBIC, nullptr, nullptr, nullptr);

			sws_scale(sws_ctx, (uint8_t const * const *)media->video->frame->data, media->video->frame->linesize, 0,
				media->video->video_ctx->height, media->video->displayFrame->data, media->video->displayFrame->linesize);

			sws_freeContext(sws_ctx);
			av_packet_unref(packet);
		}
		else {
			av_packet_unref(packet);
		}
		av_packet_free(&packet);
		return true;
	}
	else {
		av_packet_free(&packet);
		return false;
	}
	return true;
}
