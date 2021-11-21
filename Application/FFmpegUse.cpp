#include <SDL.h>
#include <SDL_thread.h>
#include <opencv.hpp>
#include "PacketQueue.h"
#include "Audio.h"
#include "Media.h"
#include "FFmpegUse.h"
#include "thread"
#include <Windows.h>
#include"ThreadFunc.h"
using namespace cv;
extern int  g_stopAudio;
extern int  g_isFinish;//正常播放完退出
extern bool g_isStop;//暂停播放
extern bool g_isExitThread;//直接退出
extern std::mutex g_mutex;
bool g_exitReadAV;
bool video_refresh_timer(void *userdata)
{
	MediaState *media = (MediaState*)userdata;
	VideoState *video = media->video;

	if (video->stream_index >= 0)
	{
			bool flag = video->frameq.deQueue(&video->frame);
			if (!flag) {
				return false;
			}
			// 将视频同步到音频上，计算下一帧的延迟时间
			double current_pts = *(double*)video->frame->opaque;
			double delay = current_pts - video->frame_last_pts;
			if (delay <= 0 || delay >= 1.0)
				delay = video->frame_last_delay;
			printf("%f\n", video->frame_last_pts);
			video->frame_last_delay = delay;
			video->frame_last_pts = current_pts;

			// 当前显示帧的PTS来计算显示下一帧的延迟
			double ref_clock = media->audio->get_audio_clock();
			double diff = current_pts - ref_clock;// diff < 0 => video slow,diff > 0 => video quick
			double threshold = (delay < SYNC_THRESHOLD) ? delay : SYNC_THRESHOLD;
			printf("%lf %lf %lf\n", ref_clock, current_pts, diff);
			if (fabs(diff) < NOSYNC_THRESHOLD) // 不同步
			{
				if (diff <= -threshold) // 慢了，delay设为0
					delay = 0;
				else if (diff >= threshold) // 快了，加倍delay
					delay *= 2;
			}
			video->frame_timer += delay;
			double actual_delay = video->frame_timer - static_cast<double>(av_gettime()) / 1000000.0;
			if (actual_delay <= 0.010)
				actual_delay = 0.010;
			Sleep(static_cast<int>(actual_delay * 1000 + 0.5));
			SwsContext *sws_ctx = sws_getContext(video->video_ctx->width, video->video_ctx->height, video->video_ctx->pix_fmt,
				video->displayFrame->width, video->displayFrame->height, (AVPixelFormat)video->displayFrame->format, SWS_BICUBIC, nullptr, nullptr, nullptr);

			sws_scale(sws_ctx, (uint8_t const * const *)video->frame->data, video->frame->linesize, 0,
				video->video_ctx->height, video->displayFrame->data, video->displayFrame->linesize);

			sws_freeContext(sws_ctx);
			return true;
	}
	else
	{
		Sleep(100);
		return false;
	}
}
cv::Mat AVFrameToCVMat(const AVFrame* frame) {
	int width = frame->width;
	int height = frame->height;
	cv::Mat image(height, width, CV_8UC3);
	int cv_lines_sizes[1];
	cv_lines_sizes[0] = image.step1();
	SwsContext* conversion = sws_getContext(width, height, (AVPixelFormat)frame->format,
		width, height, AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR,
		nullptr, nullptr, nullptr);
	sws_scale(conversion, frame->data, frame->linesize, 0, height, &image.data, cv_lines_sizes);
	sws_freeContext(conversion);
	return image;
}
ReadAV::~ReadAV()
{
}

void ReadAV::setFilePath(string s)
{
	filePath = s;
}

void ReadAV::run()
{	av_register_all();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	while (true) {
		g_mutex.lock();
		if (g_exitReadAV) {
			g_mutex.unlock();
			break;
		}
		if(g_isStop)
		{
			g_mutex.unlock();
			Sleep(10);
			continue;
		}
		g_mutex.unlock();


		char filename[100];
		strcpy(filename, filePath.c_str());
		MediaState media;
		media.filename = filename;
		std::thread  m_getStreamThread, m_decodeThread; //线程句柄
		if (media.openInputAV()) {
			m_getStreamThread = std::thread(GetAVStream, (void*)&media);
		}
		else {
			return;
		}

		media.audio->audio_play(); // create audio thread
		media.video->videoPlayInit(&media);
		m_decodeThread = std::thread(DecodeVideo, media.video);
		AVStream *audio_stream = media.pFormatCtx->streams[media.audio->stream_index];
		AVStream *video_stream = media.pFormatCtx->streams[media.video->stream_index];

		double audio_duration = audio_stream->duration * av_q2d(audio_stream->time_base);
		double video_duration = video_stream->duration * av_q2d(video_stream->time_base);
		emit return_videoTime(video_duration);
		double lastAudioClock = 0;
		bool isLastStop = true;
		bool isFirst = true;
		while (true) {
			g_mutex.lock();
			if (g_isFinish == 2|| g_isExitThread) {
				g_stopAudio = 1;
				g_mutex.unlock();
				break;
			}
			else if (g_isStop) {
				if (g_isStop != isLastStop)
				{
					isLastStop = g_isStop;
					SDL_PauseAudio(1);
				}
				g_mutex.unlock();
				continue;
			}
			else if (!g_isStop) {
				if (g_isStop != isLastStop)
				{
					isLastStop = g_isStop;
					SDL_PauseAudio(0);
				}
				media.video->frame_timer = static_cast<double>(av_gettime()) / 1000000.0;
			}
			g_mutex.unlock();
			if (video_refresh_timer(&media)) {
				QImage  Qimg((uchar*)media.video->displayFrame->data[0], media.video->video_ctx->width, media.video->video_ctx->height, QImage::Format_RGB32);
				emit return_QImage(Qimg.copy(), media.video->frame_last_pts);
				av_frame_unref(media.video->frame);
				g_mutex.lock();
				if (isFirst) {
					isFirst = false;
					g_isStop = true;
				}
				g_mutex.unlock();
			}
			else {
				Sleep(10);
			}
		}
		m_getStreamThread.join();
		m_decodeThread.join();
		media.audio->audio_close();
		g_mutex.lock();
			g_isFinish = 0;
			g_stopAudio = 0;
			g_isExitThread = false;
		g_mutex.unlock();
		emit return_Finish();
	}
	return;
}
