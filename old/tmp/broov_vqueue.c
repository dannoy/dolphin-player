#ifdef BROOV_PLAYER_VIDEO_THREAD
#include "broov_vqueue.h"
#include <android/log.h>

extern AVFrame flush_frame;
extern double  flush_pts;
extern int64_t flush_pos;

void vpacket_queue_init(VPacketQueue *q) 
{
  //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Inside vpacket_queue_init");
  memset(q, 0, sizeof(VPacketQueue));
  q->mutex = SDL_CreateMutex();
  q->cond = SDL_CreateCond();

  vpacket_queue_put(q, &flush_frame, &flush_pts, &flush_pos);
}

int vpacket_queue_put(VPacketQueue *q, AVFrame *pkt, double *pts, int64_t *pos) 
{
  AVFrameList *pkt1;

  /* duplicate the packet */
  //if(pkt != &flush_frame && av_dup_packet(pkt) < 0) {
  //  return -1;
  //}
  //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Inside vpacket_queue_put Frame:%p Pts:%lf Pos:%ld", pkt, *pts, *pos);

  pkt1 = av_malloc(sizeof(AVFrameList));
  if (!pkt1) {
    __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Unable to allocate memory: Failed");
    return -1;
  }
  pkt1->pkt = pkt;
  pkt1->pts = *pts;
  pkt1->pos = *pos;
  pkt1->next = NULL;

  //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Pkt1->pkt:%p", pkt1->pkt);
  
  SDL_LockMutex(q->mutex);

  if (!q->last_pkt)
    q->first_pkt = pkt1;
  else
    q->last_pkt->next = pkt1;
  q->last_pkt = pkt1;
  q->nb_packets++;

  SDL_CondSignal(q->cond);
  
  SDL_UnlockMutex(q->mutex);
  return 0;
}

void vpacket_queue_abort(VPacketQueue *q)
{
    __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Inside vpacket_queue_abort");
    SDL_LockMutex(q->mutex);

    q->abort_request = 1;

    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
int vpacket_queue_get(VPacketQueue *q, AVFrame **pkt, double *pts, int64_t *pos, int block)
{
  AVFrameList *pkt1;
  int ret;

  SDL_LockMutex(q->mutex);
  
  //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Inside vpacket_queue_get");
  for(;;) {
    
    if(q->abort_request) {
      __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Abort Request in video2q");
      ret = -1;
      break;
    }

    pkt1 = q->first_pkt;
    if (pkt1) {
      
      q->first_pkt = pkt1->next;
      if (!q->first_pkt)
	q->last_pkt = NULL;
      q->nb_packets--;

      *pkt = pkt1->pkt;
      *pos = pkt1->pos;
      *pts = pkt1->pts;

      //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "pkt:%p", pkt);
      //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Starpkt:%p", *pkt);
      av_free(pkt1);
      ret = 1;
      break;
    } else if (!block) {
      ret = 0;
      break;
    } else {
      SDL_CondWait(q->cond, q->mutex);
    }
  }
  SDL_UnlockMutex(q->mutex);
  return ret;
}

void vpacket_queue_flush(VPacketQueue *q) 
{
  AVFrameList *pkt, *pkt1;

  SDL_LockMutex(q->mutex);

  //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Inside vpacket_queue_flush");
  for (pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
    pkt1 = pkt->next;
   
    if (pkt->pkt == &flush_frame) {
       //Do not free the flush frame
       __android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Flush Frame in vpacket_queue_flush");

    } else {

       //__android_log_print(ANDROID_LOG_INFO, "BroovPlayer", "Non-Flush Frame in vpacket_queue_flush");
      //Free the av frame
      av_free(pkt->pkt);
    }

    //Free the av frame list allocated in put
    av_freep(&pkt);
  }
  q->last_pkt = NULL;
  q->first_pkt = NULL;
  q->nb_packets = 0;
  SDL_UnlockMutex(q->mutex);
}

void vpacket_queue_end(VPacketQueue *q)
{
    vpacket_queue_flush(q);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

#endif
