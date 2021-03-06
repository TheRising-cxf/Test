
#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <queue>
#include <mutex>
#include <SDL.h>
#include <SDL_thread.h>

extern "C"{

#include <libavcodec\avcodec.h>

}

struct PacketQueue
{
	std::queue<AVPacket> queue;

	Uint32    nb_packets;
	Uint32    size;
	std::mutex g_Vmutex;
	PacketQueue();
	bool enQueue(const AVPacket *packet);
	bool deQueue(AVPacket *packet, bool block);
};

#endif