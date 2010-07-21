#include "glue.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

void _fdglue_t_setHandler(fdglue_t* this, int fd, fdglue_handle_type_t const type, fdglue_handler_t const hnd, fdglue_handler_replace_t const action) {
    assert(this);
    if (action != FDGHR_APPEND) {
        fdglue_handlerlist_t dummy;
        dummy.fd = -1;
        dummy.next = this->handlers;
        ////   dummy  ->  handlers  ->  ...  ->  NULL
        struct fdglue_handlerlist_t* it,* last = &dummy;
        for (it = dummy.next; it; it = it->next) {
            if (it->fd == fd && it->type == type) {
                switch (action) {
                case FDGHR_REPLACE:
                    it->hnd = hnd;
                    break;
                case FDGHR_REMOVE:
                    last->next = it->next;
                    free(it);
                    it = last;
                    break;
                default: {}
                }
            }
            last = it;
        }
        it = last;
        assert(it);
        this->handlers = dummy.next;
    } else {
        fdglue_handlerlist_t* p;
        assert(DYNAMIC_MEMORY);
        p = malloc(sizeof(fdglue_handlerlist_t));
        p->fd = fd;
        p->type = type;
        p->hnd = hnd;
        p->next = this->handlers;
        this->handlers = p;
        if (fd > this->nfds)
            this->nfds = fd;
    }
}

void _fdglue_t_listen(fdglue_t* this, unsigned timeout) {
    assert(this);
    static fd_set rd, wr, er;
    static fd_set* fdmap[FDGHT_SIZE];
    fdmap[FDGHT_READ] = &rd;
    fdmap[FDGHT_WRITE] = &wr;
    fdmap[FDGHT_ERROR] = &er;
    FD_ZERO(&rd);
    FD_ZERO(&wr);
    FD_ZERO(&er);
    struct fdglue_handlerlist_t* it;
    for (it = this->handlers; it; it = it->next) {
        FD_SET(it->fd,fdmap[it->type]);
    }
    struct timeval tv = {.tv_sec = timeout, .tv_usec = 0};
    if (-1 != select(this->nfds+1,&rd,&wr,&er,&tv)) {
        for (it = this->handlers; it; it = it->next) {
            if (FD_ISSET(it->fd,fdmap[it->type])) {
                it->hnd.handle(&(it->hnd));
            }
        }
    }
}

fdglue_t* fdglue(fdglue_t* this) {
    CTOR(this);
    this->handlers = NULL;
    this->nfds = -1;
    this->setHandler = _fdglue_t_setHandler;
    this->listen = _fdglue_t_listen;
    return this;
}
