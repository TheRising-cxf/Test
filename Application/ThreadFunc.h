#pragma once
#include"Video.h"
int DecodeVideo(void *arg); // ��packet���룬����������Frame����FrameQueue������
int GetAVStream(void *data);
/**
 * ���豸����audio���ݵĻص�����
 */
void audio_callback(void* userdata, Uint8 *stream, int len);

/**
 * ����Avpacket�е�������䵽����ռ�
 */
int audio_decode_frame(AudioState *audio_state, uint8_t *audio_buf, int buf_size);