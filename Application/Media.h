
#ifndef MEDIA_H
#define MEDIA_H

#include <string>
#include "Audio.h"
#include "Video.h"

extern "C" {

#include <libavformat\avformat.h>
#include <libavdevice\avdevice.h>

}
class WavHeader {
private:
	unsigned char ChunkID[4];  // "RIFF"
	unsigned int  ChunkSize; // 36 + SubChunk2Size
	unsigned char Format[4]; // "WAVE"

	unsigned char  Subchunk1ID[4]; // "fmt"
	unsigned int   Subchunk1Size; // 16 for pcm
	unsigned short AudioFormat; //  PCM=1
	unsigned short NumChannels;
	unsigned int   SampleRate;
	unsigned int   ByteRate; // SampleRate * NumChannels * BitsPerSample/8
	unsigned short BlockAlign; // NumChannels * BitsPerSample/8
	unsigned short BitsPerSample; //8 bits = 8, 16 bits = 16

	unsigned char   Subchunk2ID[4]; // "data"
	unsigned int    Subchunk2Size; // NumSamples * NumChannels * BitsPerSample/8

public:
	WavHeader(int rate, int bits, int channels, int sec)
	{
		strcpy((char *)ChunkID, "RIFF");
		Subchunk2Size = (sec * rate * channels * bits) >> 3;
		ChunkSize = 36 + Subchunk2Size;
		strcpy((char *)Format, "WAVE");

		strcpy((char *)Subchunk1ID, "fmt ");
		Subchunk1Size = 16;
		AudioFormat = 1;
		NumChannels = channels;
		SampleRate = rate;
		ByteRate = (rate * channels * bits) >> 3;
		BlockAlign = (channels * bits) >> 3;
		BitsPerSample = bits;

		strcpy((char *)Subchunk2ID, "data");
		Subchunk2Size = (sec * rate * channels * bits) >> 3;
	}
};
struct VideoState;

struct MediaState
{
	AudioState *audio;
	VideoState *video;
	int  frameWidth =  0;
	int  frameHeight = 0;
	AVFormatContext *pFormatCtx;


	char* filename;
	//bool quit;

	MediaState(char *filename);
	~MediaState();

	bool openInputAV();
	bool openInputcap();
	bool openInputAudio();
};

int decode_thread(void *data);
int decode_thread_Cap(void *data);

#endif