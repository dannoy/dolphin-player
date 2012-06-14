/* Run the system dependent event loops */
void SDL_PumpEvents(void)
{
    if (!SDL_EventThread) {
        SDL_VideoDevice *_this = SDL_GetVideoDevice();

        /* Get events from the video subsystem */
        if (_this) {
            _this->PumpEvents(_this);
        }
    }
}
int
SDL_WaitEventTimeout(SDL_Event * event, int timeout)
{
    Uint32 expiration = 0;

    for (;;) {
        SDL_PumpEvents();
        switch (SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
        case -1:
            return 0;
        case 1:
            return 1;
        case 0:
            if (timeout == 0) {
                /* Polling and no events, just return */
                return 0;
            }
            if (timeout > 0 && ((int) (SDL_GetTicks() - expiration) >= 0)) {
                /* Timeout expired and no events */
                return 0;
            }
            SDL_Delay(10);
            break;
        }
    }
}

int
SDL_PushEvent(SDL_Event * event)
{
    if (SDL_EventOK && !SDL_EventOK(SDL_EventOKParam, event)) {
        return 0;
    }
    if (SDL_PeepEvents(event, 1, SDL_ADDEVENT, 0, 0) <= 0) {
        return -1;
    }

    SDL_GestureProcessEvent(event);
    

    return 1;
}

/* Lock the event queue, take a peep at it, and unlock it */
int
SDL_PeepEvents(SDL_Event * events, int numevents, SDL_eventaction action,
               Uint32 minType, Uint32 maxType)
{
    int i, used;

    /* Don't look after we've quit */
    if (!SDL_EventQ.active) {
        return (-1);
    }
    /* Lock the event queue */
    used = 0;
    if (SDL_mutexP(SDL_EventQ.lock) == 0) {
        if (action == SDL_ADDEVENT) {
            for (i = 0; i < numevents; ++i) {
                used += SDL_AddEvent(&events[i]);
            }
        } else {
            SDL_Event tmpevent;
            int spot;

            /* If 'events' is NULL, just see if they exist */
            if (events == NULL) {
                action = SDL_PEEKEVENT;
                numevents = 1;
                events = &tmpevent;
            }
            spot = SDL_EventQ.head;
            while ((used < numevents) && (spot != SDL_EventQ.tail)) {
                Uint32 type = SDL_EventQ.event[spot].type;
                if (minType <= type && type <= maxType) {
                    events[used++] = SDL_EventQ.event[spot];
                    if (action == SDL_GETEVENT) {
                        spot = SDL_CutEvent(spot);
                    } else {
                        spot = (spot + 1) % MAXEVENTS;
                    }
                } else {
                    spot = (spot + 1) % MAXEVENTS;
                }
            }
        }
        SDL_mutexV(SDL_EventQ.lock);
    } else {
        SDL_SetError("Couldn't lock event queue");
        used = -1;
    }
    return (used);
}

