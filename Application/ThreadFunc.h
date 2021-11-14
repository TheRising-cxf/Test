#pragma once
#include"Video.h"
int DecodeVideo(void *arg); // 将packet解码，并将解码后的Frame放入FrameQueue队列中
int GetAVStream(void *data);
/**
 * 向设备发送audio数据的回调函数
 */
void audio_callback(void* userdata, Uint8 *stream, int len);

/**
 * 解码Avpacket中的数据填充到缓冲空间
 */
int audio_decode_frame(AudioState *audio_state, uint8_t *audio_buf, int buf_size);