
#include "Media.h"
#include <iostream>

extern "C"{
#include <libavutil/time.h>
}
MediaState::MediaState()
{
	pFormatCtx = nullptr;
	audio = new AudioState();

	video = new VideoState();
	//quit = false;
}


MediaState::~MediaState()
{
	avformat_close_input(&pFormatCtx);
	if(audio)
		delete audio;

	if (video)
		delete video;
}

bool MediaState::openInputAV()
{
	// Open input file
	if (avformat_open_input(&pFormatCtx, filename, nullptr, nullptr) < 0)
		return false;

	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
		return false;

	// Output the stream info to standard 
	av_dump_format(pFormatCtx, 0, filename, 0);

	for (uint32_t i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audio->stream_index < 0)
			audio->stream_index = i;

		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && video->stream_index < 0)
			video->stream_index = i;
	}

	if (audio->stream_index < 0 || video->stream_index < 0)
		return false;

	// Fill audio state
	AVCodec *pCodec = avcodec_find_decoder(pFormatCtx->streams[audio->stream_index]->codec->codec_id);
	if (!pCodec)
		return false;

	audio->stream = pFormatCtx->streams[audio->stream_index];

	audio->audio_ctx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(audio->audio_ctx, pFormatCtx->streams[audio->stream_index]->codec) != 0)
		return false;

	avcodec_open2(audio->audio_ctx, pCodec, nullptr);

	// Fill video state
	AVCodec *pVCodec = avcodec_find_decoder(pFormatCtx->streams[video->stream_index]->codec->codec_id);
	if (!pVCodec)
		return false;

	video->stream = pFormatCtx->streams[video->stream_index];

	video->video_ctx = avcodec_alloc_context3(pVCodec);
	if (avcodec_copy_context(video->video_ctx, pFormatCtx->streams[video->stream_index]->codec) != 0)
		return false;

	avcodec_open2(video->video_ctx, pVCodec, nullptr);

	video->frame_timer = static_cast<double>(av_gettime()) / 1000000.0;
	video->frame_last_delay = 40e-3;

	frameWidth = video->stream->codec->width;
	frameHeight = video->stream->codec->height;
	av_free(pVCodec);
	av_free(pCodec);

	return true;
}

bool MediaState::openInputcap()
{
	AVInputFormat *ifmt = av_find_input_format("dshow");
	//Set own video device's name
	if (avformat_open_input(&pFormatCtx, filename, ifmt, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return false;
	}
	int video_index = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (video_index == -1) {
		printf("Could not find a video stream");
		avformat_close_input(&pFormatCtx);
		return false;
	}
	// ��ȡ������
	AVCodecParameters *params = pFormatCtx->streams[video_index]->codecpar;

	// ��ȡ������
	AVCodec *codec = avcodec_find_decoder(params->codec_id);

	video->stream = pFormatCtx->streams[video->stream_index];
	video->video_ctx = avcodec_alloc_context3(codec);
	if (avcodec_copy_context(video->video_ctx, pFormatCtx->streams[video_index]->codec) != 0)
		return false;

	avcodec_open2(video->video_ctx, codec, nullptr);

	video->frame_timer = static_cast<double>(av_gettime()) / 1000000.0;
	video->frame_last_delay = 40e-3;

	frameWidth = video->video_ctx->width;
	frameHeight = video->video_ctx->height;
	video->stream_index = video_index;
	return true;
}

bool MediaState::openInputAudio()
{
	AVInputFormat *ifmt = av_find_input_format("dshow"); 
	AVFormatContext *fmtContxt = avformat_alloc_context();
	AVDictionary *options = NULL;

	av_dict_set(&options, "sample_rate", "44100", 0);
	av_dict_set(&options, "channels", "1", 0);          

	//4. ʹ��ָ���Ĳ���������ָ�����豸
		if (avformat_open_input(&fmtContxt, filename, ifmt, &options) < 0)
		{
			perror("avformat open");
			return false;
		}

	//5. ���豸�󣬡��ɼ���������ݸ�ʽ.
		AVCodecID id = fmtContxt->streams[0]->codec->codec_id; //��ȡ���ݵĸ�ʽID
	//qDebug() << id << "   " << avcodec_get_name(id);
	// ��ʾ��:pcm_s16le, Ҳ����pcm���ݣ�����������ΪС��16λ



	//6. ׼��һ֡���ݵĿռ䣬����ȡ����:
	AVPacket * packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	av_read_frame(fmtContxt, packet); //��ȡһ֡����
	// packet->dataָ��ָ���������ݻ��������׵�ַ
	// packet->size�ǵ�ǰ֡���ݵ���ʵ��С


	//7. ��װ��һ��wav�ļ�.wav�ļ���ʽ�ο�http://soundfile.sapp.org/doc/WaveFormat/
	//�����˵wav��ʵ��һ���ļ�ͷ + pcm�������.
	//¼��60���wav�ļ�

	WavHeader header(44100, 16, 1, 60);
	FILE *fl = fopen("my.wav", "w");
	fwrite(&header, sizeof(header), 1, fl);   // д���ļ�ͷ

	int len = 44100 * 1 * 2 * 60;  //60������ݳ���
	for (int n = 0; n < len;)
	{
		if (av_read_frame(fmtContxt, packet) < 0)
		{
			perror("read frame failed");
			return 1;
		}
		//qDebug() << packet->size;

		n += packet->size;
		fwrite(packet->data, packet->size, 1, fl);  //д��pcm����
	}
	fclose(fl);

	return true;
}

int decode_thread_Cap(void *data)
{
	MediaState *media = (MediaState*)data;

	AVPacket *packet = av_packet_alloc();

	int video_index = av_find_best_stream(media->pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (video_index == -1) {
		printf("Could not find a video stream");
		return 0;
	}

	// ��ȡ������
	AVCodecParameters *params = media->pFormatCtx->streams[video_index]->codecpar;

	// ��ȡ������
	AVCodec *codec = avcodec_find_decoder(params->codec_id);

	// ��ʼ��һ����������������
	AVCodecContext *codecContext = avcodec_alloc_context3(codec);
	if (avcodec_parameters_to_context(codecContext, params) != 0) {
		printf("Could not copy codec context");
		avformat_close_input(&media->pFormatCtx);
		return 0;
	}

	avcodec_open2(codecContext, codec, nullptr);

	while (av_read_frame(media->pFormatCtx, packet) >= 0) {
		if (packet->stream_index == video_index) {
			media->video->videoq.enQueue(packet);
			av_packet_unref(packet);
		}
		else {
			av_packet_unref(packet);
		}

	}

	av_packet_free(&packet);

	return 0;
}