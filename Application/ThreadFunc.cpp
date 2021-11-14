#include"ThreadFunc.h"
extern "C" {
#include <libswresample\swresample.h>
}
#include <mutex>
int  g_stopAudio = 0;
int  g_isFinish = 0;//正常播放完退出
bool g_isStop = false;//暂停播放
bool g_isExitThread = false;//直接退出
std::mutex g_mutex;
int DecodeVideo(void *arg)
{
	VideoState *video = (VideoState*)arg;

	AVFrame *frame = av_frame_alloc();

	AVPacket packet;
	double pts;

	while (true)
	{
		if (video->frameq.nb_frames >= FrameQueue::capacity)
		{
			SDL_Delay(500 * 2);
			continue;
		}
		bool flag = video->videoq.deQueue(&packet, false);
		g_mutex.lock();
		if (g_isExitThread) {
			g_mutex.unlock();
			av_free_packet(&packet);
			break;
		}
		if (!flag && g_isFinish == 1) {
			g_isFinish = 2;
			g_mutex.unlock();
			av_free_packet(&packet);
			break;
		}
		g_mutex.unlock();
		int ret = avcodec_send_packet(video->video_ctx, &packet);
		av_free_packet(&packet);
		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
			continue;

		ret = avcodec_receive_frame(video->video_ctx, frame);
		if (ret < 0 && ret != AVERROR_EOF)
			continue;

		if ((pts = av_frame_get_best_effort_timestamp(frame)) == AV_NOPTS_VALUE)
			pts = 0;

		pts *= av_q2d(video->stream->time_base);

		pts = video->synchronize(frame, pts);

		frame->opaque = &pts;


		video->frameq.enQueue(frame);

		av_frame_unref(frame);
	}
	av_frame_free(&frame);

	return 0;
}
int GetAVStream(void *data)
{
	g_mutex.lock();
	g_isFinish = 0;
	g_mutex.unlock();
	MediaState *media = (MediaState*)data;

	AVPacket *packet = av_packet_alloc();

	while (true)
	{
		int ret = av_read_frame(media->pFormatCtx, packet);
		if (ret < 0)
		{
			if (ret == AVERROR_EOF)
				break;
			if (media->pFormatCtx->pb->error == 0) // No error,wait for user input
			{
				SDL_Delay(10);
				continue;
			}
			else
				break;

		}
		g_mutex.lock();
			if (g_isExitThread) {
				g_mutex.unlock();
				break;
			}
		g_mutex.unlock();
		if (packet->stream_index == media->audio->stream_index) // audio stream
		{
			media->audio->audioq.enQueue(packet);
			av_packet_unref(packet);
		}

		else if (packet->stream_index == media->video->stream_index) // video stream
		{
			media->video->videoq.enQueue(packet);
			av_packet_unref(packet);
		}
		else
			av_packet_unref(packet);
	}

	av_packet_free(&packet);
	g_mutex.lock();
		g_isFinish = 1;
	g_mutex.unlock();
	return 0;
}
/**
* 向设备发送audio数据的回调函数
*/
void audio_callback(void* userdata, Uint8 *stream, int len)
{
	g_mutex.lock();
	if (g_stopAudio > 0 || g_isExitThread) {
		g_stopAudio = 2;
		g_mutex.unlock();
		return;
	}
	g_mutex.unlock();
	AudioState *audio_state = (AudioState*)userdata;

	SDL_memset(stream, 0, len);

	int audio_size = 0;
	int len1 = 0;
	while (len > 0)// 向设备发送长度为len的数据
	{
		g_mutex.lock();
		if (g_isExitThread) {
			g_stopAudio = 2;
			g_mutex.unlock();
			return;
		}
		g_mutex.unlock();
		if (audio_state->audio_buff_index >= audio_state->audio_buff_size) // 缓冲区中无数据
		{
			// 从packet中解码数据
			audio_size = audio_decode_frame(audio_state, audio_state->audio_buff, sizeof(audio_state->audio_buff));	

			if (audio_size < 0) // 没有解码到数据或出错，填充0
			{
				return;
			}
			else
				audio_state->audio_buff_size = audio_size;

			audio_state->audio_buff_index = 0;
		}
		len1 = audio_state->audio_buff_size - audio_state->audio_buff_index; // 缓冲区中剩下的数据长度
		if (len1 > len) // 向设备发送的数据长度为len
			len1 = len;

		SDL_MixAudio(stream, audio_state->audio_buff + audio_state->audio_buff_index, len, SDL_MIX_MAXVOLUME);
		len -= len1;
		stream += len1;
		audio_state->audio_buff_index += len1;
	}
}

int audio_decode_frame(AudioState *audio_state, uint8_t *audio_buf, int buf_size)
{
	AVFrame *frame = av_frame_alloc();
	int data_size = 0;
	AVPacket pkt;
	SwrContext *swr_ctx = nullptr;
	static double clock = 0;
	if (!audio_state->audioq.deQueue(&pkt, false))
		return -1;

	if (pkt.pts != AV_NOPTS_VALUE)
	{
		audio_state->audio_clock = av_q2d(audio_state->stream->time_base) * pkt.pts;
	}
	int ret = avcodec_send_packet(audio_state->audio_ctx, &pkt);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		return -1;

	ret = avcodec_receive_frame(audio_state->audio_ctx, frame);
	if (ret < 0 && ret != AVERROR_EOF)
		return -1;

	// 设置通道数或channel_layout
	if (frame->channels > 0 && frame->channel_layout == 0)
		frame->channel_layout = av_get_default_channel_layout(frame->channels);
	else if (frame->channels == 0 && frame->channel_layout > 0)
		frame->channels = av_get_channel_layout_nb_channels(frame->channel_layout);

	AVSampleFormat dst_format = AV_SAMPLE_FMT_S16;//av_get_packed_sample_fmt((AVSampleFormat)frame->format);
	Uint64 dst_layout = av_get_default_channel_layout(frame->channels);
	// 设置转换参数
	swr_ctx = swr_alloc_set_opts(nullptr, dst_layout, dst_format, frame->sample_rate,
		frame->channel_layout, (AVSampleFormat)frame->format, frame->sample_rate, 0, nullptr);
	if (!swr_ctx || swr_init(swr_ctx) < 0)
		return -1;

	// 计算转换后的sample个数 a * b / c
	uint64_t dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples, frame->sample_rate, frame->sample_rate, AVRounding(1));
	// 转换，返回值为转换后的sample个数
	int nb = swr_convert(swr_ctx, &audio_buf, static_cast<int>(dst_nb_samples), (const uint8_t**)frame->data, frame->nb_samples);
	data_size = frame->channels * nb * av_get_bytes_per_sample(dst_format);

	// 每秒钟音频播放的字节数 sample_rate * channels * sample_format(一个sample占用的字节数)
	audio_state->audio_clock += static_cast<double>(data_size) / (2 * audio_state->stream->codec->channels * audio_state->stream->codec->sample_rate);


	av_frame_free(&frame);
	swr_free(&swr_ctx);

	return data_size;
}