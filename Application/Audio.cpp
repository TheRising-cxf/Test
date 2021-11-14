
#include "Audio.h"
#include "ThreadFunc.h"
#include <iostream>
#include <fstream>
#include<mutex>
extern "C" {

#include <libswresample\swresample.h>

}
#include<Windows.h>
extern int  g_stopAudio;
extern bool g_isExitThread;
extern std::mutex g_mutex;
AudioState::AudioState()
	:BUFFER_SIZE(192000)
{
	audio_ctx = nullptr;
	stream_index = -1;
	stream = nullptr;
	audio_clock = 0;

	audio_buff = new uint8_t[BUFFER_SIZE];
	audio_buff_size = 0;
	audio_buff_index = 0;
}

AudioState::AudioState(AVCodecContext *audioCtx, int index)
	:BUFFER_SIZE(192000)
{
	audio_ctx = audioCtx;
	stream_index = index;
	

	audio_buff = new uint8_t[BUFFER_SIZE];
	audio_buff_size = 0;
	audio_buff_index = 0;
}

AudioState::~AudioState()
{
	avcodec_close(audio_ctx);
	if (audio_buff)
		delete[] audio_buff;
	while (!audioq.queue.empty()) {
		AVPacket pkt = audioq.queue.front();
		audioq.queue.pop();
		av_free_packet(&pkt);
	}
}

bool AudioState::audio_play()
{
	SDL_AudioSpec desired;
	desired.freq = audio_ctx->sample_rate;
	desired.channels = audio_ctx->channels;
	desired.format = AUDIO_S16SYS;
	desired.samples = 1024;
	desired.silence = 0;
	desired.userdata = this;
	desired.callback = audio_callback;

	if (SDL_OpenAudio(&desired, nullptr) < 0)
	{
		return false;
	}
	g_stopAudio = 0;
	return true;
}
void AudioState::audio_close() {
	while (true) {
		g_mutex.lock();
		if (g_stopAudio == 2) {
			g_mutex.unlock();
			break;
		}
		g_mutex.unlock();
		Sleep(10);
	}
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	return;
}

double AudioState::get_audio_clock()
{
	int hw_buf_size = audio_buff_size - audio_buff_index;
	int bytes_per_sec = stream->codec->sample_rate * audio_ctx->channels * 2;

	double pts = audio_clock - static_cast<double>(hw_buf_size) / bytes_per_sec;

	
	return pts;
}


