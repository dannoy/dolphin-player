#ifndef BROOV_QUEUE_H
#define BROOV_QUEUE_H

#include "SDL.h"
#include "SDL_thread.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavcodec/avfft.h"

typedef struct PacketQueue 
{
  AVPacketList *first_pkt;
  AVPacketList *last_pkt;

  int           nb_packets;
  int           size;
  int           abort_request;

  SDL_mutex    *mutex;
  SDL_cond     *cond;
} PacketQueue;

int  packet_queue_put(PacketQueue *q, AVPacket *pkt);
void packet_queue_init(PacketQueue *q);
void packet_queue_abort(PacketQueue *q);
int  packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);
void packet_queue_flush(PacketQueue *q);
void packet_queue_end(PacketQueue *q);

#endif /* #ifndef BROOV_QUEUE_H */
