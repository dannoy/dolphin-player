#ifndef BROOV_VQUEUE_H
#define BROOV_VQUEUE_H

#include "SDL.h"
#include "SDL_thread.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavcodec/avfft.h"

typedef struct AVFrameList {
   AVFrame *pkt;
   double  pts;
   int64_t pos;   
   struct AVFrameList *next;
} AVFrameList;

typedef struct VPacketQueue 
{
  AVFrameList *first_pkt;
  AVFrameList *last_pkt;

  int           nb_packets;
  int           abort_request;

  SDL_mutex    *mutex;
  SDL_cond     *cond;
} VPacketQueue;

void vpacket_queue_init(VPacketQueue *q);
int vpacket_queue_put(VPacketQueue *q, AVFrame *pkt, double *pts, int64_t *pos) ;
void vpacket_queue_abort(VPacketQueue *q);
int vpacket_queue_get(VPacketQueue *q, AVFrame **pkt, double *pts, int64_t *pos, int block);
void vpacket_queue_flush(VPacketQueue *q);
void vpacket_queue_end(VPacketQueue *q);

#endif /* #ifndef BROOV_VQUEUE_H */
