
#include "PacketQueue.h"
#include <iostream>

PacketQueue::PacketQueue()
{
	nb_packets = 0;
	size       = 0;
}

bool PacketQueue::enQueue(const AVPacket *packet)
{
	AVPacket pkt;
	if (av_packet_ref(&pkt, packet) < 0)
		return false;

	g_Vmutex.lock();
	queue.push(pkt);

	size += pkt.size;
	nb_packets++;

	g_Vmutex.unlock();
	return true;
}

bool PacketQueue::deQueue(AVPacket *packet, bool block)
{
	bool ret = false;

	g_Vmutex.lock();
	while (true)
	{
		if (!queue.empty())
		{
			if (av_packet_ref(packet, &queue.front()) < 0)
			{
				ret = false;
				break;
			}
			AVPacket pkt = queue.front();

			queue.pop();
			av_packet_unref(&pkt);
			nb_packets--;
			size -= packet->size;

			ret = true;
			break;
		}
		else if (!block)
		{
			ret = false;
			break;
		}
		else
		{
			
		}
	}
	g_Vmutex.unlock();
	return ret;
}