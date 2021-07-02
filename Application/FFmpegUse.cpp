#include <SDL.h>
#include <SDL_thread.h>

#include "PacketQueue.h"
#include "Audio.h"
#include "Media.h"
#include "VideoDisplay.h"
#include "FFmpegUse.h"
extern bool Quit;
ReadAV::~ReadAV()
{
}

void ReadAV::setFilePath(string s)
{
	filePath = s;
}

void ReadAV::run()
{
	av_register_all();

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

	char filename[100];
	strcpy(filename, filePath.c_str());
	MediaState media(filename);

	if (media.openInputAV())
		SDL_CreateThread(decode_thread, "", &media); // 创建解码线程，读取packet到队列中缓存
	else {
		return;
	}

	media.audio->audio_play(); // create audio thread

	media.video->video_play(&media); // create video thread

	AVStream *audio_stream = media.pFormatCtx->streams[media.audio->stream_index];
	AVStream *video_stream = media.pFormatCtx->streams[media.video->stream_index];

	double audio_duration = audio_stream->duration * av_q2d(audio_stream->time_base);
	double video_duration = video_stream->duration * av_q2d(video_stream->time_base);

	cout << "audio时长：" << audio_duration << endl;
	cout << "video时长：" << video_duration << endl;

	SDL_Event event;
	while (true) // SDL event loop
	{
		SDL_WaitEvent(&event);
		switch (event.type)
		{
		case FF_QUIT_EVENT:
		case SDL_QUIT:
			Quit = 1;
			SDL_Quit();

			return;
			break;

		case FF_REFRESH_EVENT:
		{
			if (video_refresh_timer(&media)) {
				QImage  Qimg((uchar*)media.video->displayFrame->data[0], media.video->video_ctx->width, media.video->video_ctx->height, QImage::Format_RGB32);
				emit return_QImage(Qimg);
				// Display the image to screen
				/*SDL_UpdateTexture(media.video->bmp, &(media.video->rect), media.video->displayFrame->data[0], media.video->displayFrame->linesize[0]);
				SDL_RenderClear(media.video->renderer);
				SDL_RenderCopy(media.video->renderer, media.video->bmp, &media.video->rect, &media.video->rect);
				SDL_RenderPresent(media.video->renderer);*/
				av_frame_unref(media.video->frame);
			}
		}
		break;

		default:
			break;
		}
	}
}

ReadCap::~ReadCap()
{
}

void ReadCap::setCapName(string s)
{
	capName = s;
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
void ReadCap::run()
{
	avdevice_register_all();

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	char filename[100];
	strcpy(filename, capName.c_str());
	MediaState media(filename);

	if (!media.openInputcap()) {
		return;
	}
	int width = media.frameWidth;
	int height = media.frameHeight;

	media.video->frame = av_frame_alloc();
	media.video->displayFrame = av_frame_alloc();

	media.video->displayFrame->format = AV_PIX_FMT_RGB32;
	media.video->displayFrame->width = width;
	media.video->displayFrame->height = height;

	int numBytes = avpicture_get_size((AVPixelFormat)media.video->displayFrame->format, media.video->displayFrame->width, media.video->displayFrame->height);
	uint8_t *buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

	avpicture_fill((AVPicture*)media.video->displayFrame, buffer, (AVPixelFormat)media.video->displayFrame->format, media.video->displayFrame->width, media.video->displayFrame->height);

	while (true) {
		if (video_refresh_timer_test(&media)) {
			QImage  Qimg((uchar*)media.video->displayFrame->data[0], media.video->video_ctx->width, media.video->video_ctx->height, QImage::Format_RGB32);
			Qimg = Qimg.mirrored(true, false);
			emit return_QImage(Qimg);
			av_frame_unref(media.video->frame);
		}
	}

}
ReadAudio::~ReadAudio()
{
}

void ReadAudio::setAudioName(string s)
{
	audioName = s;
}

void ReadAudio::run()
{
	avdevice_register_all();

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	char filename[100];
	strcpy(filename, audioName.c_str());
	MediaState media(filename);

	if (!media.openInputAudio()) {
		return;
	}

}