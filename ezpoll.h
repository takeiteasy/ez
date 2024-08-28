/* ezpoll.h - Wrapper over epoll, kqueue and winsock
   https://github.com/takeiteasy/ez
 
 Windows implementation of epoll comes from the cpoll project:
 https://sourceforge.net/projects/cpoll/
 The Windows relies on the C++ stl to save implementing a whole hash table
 for what is suppose to be a minimal wrapper. Linux/Mac/BSD are all C.

 The MIT License (MIT)
 
 Copyright (c) 2024 George Watson
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef EZPOLL_HEADER
#define EZPOLL_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#define EZPOLL_BACKEND_EPOLL  1
#define EZPOLL_BACKEND_KQUEUE 2

#ifndef EZPOLL_BACKEND
#if defined(__APPLE__) || defined(__MACH__) || defined(__unix__)
#define EZPOLL_BACKEND EZPOLL_BACKEND_KQUEUE
#else
#if defined(_WIN32) || defined(_WIN64)
#define EZPOLL_PLATFORM_WINDOWS
#endif
#define EZPOLL_BACKEND EZPOLL_BACKEND_EPOLL
#endif

int ezPollInit(void);
int ezPollWait(void);
int ezPollCreate(int fd);
int ezPollGet(int idx);
int ezPollClose(int fd);

#if defined(__cplusplus)
}
#endif
#endif // EZPOLL_HEADER

#if defined(EZPOLL_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
#ifdef EZPOLL_PLATFORM_WINDOWS
#include <Winsock2.h>
#include <map>
#include <vector>
#include <stdint.h>
#else
#include <sys/epoll.h>
#endif
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
#include <sys/event.h>
#else
#error "Unknown backend"
#endif

#define DEFAULT_BUFFER_SIZE 1024
#ifndef BUFFER_SIZE
#define BUFFER_SIZE DEFAULT_BUFFER_SIZE
#endif
#define DEFAULT_EVENT_SIZE 256
#ifndef EVENT_SIZE
#define EVENT_SIZE DEFAULT_EVENT_SIZE
#endif

#ifdef EZPOLL_PLATFORM_WINDOWS
enum EPOLL_EVENTS {
    EPOLLIN      = 0x0001,
    EPOLLOUT     = 0x0002,
    EPOLLRDHUP   = 0x0004,
    EPOLLPRI     = 0x0008,
    EPOLLERR     = 0x0010,
    EPOLLHUP     = 0x0020,
    EPOLLET      = 0x0040,
    EPOLLONESHOT = 0x0080
};

enum EPOLL_OPCODES {
    EPOLL_CTL_ADD,
    EPOLL_CTL_DEL,
    EPOLL_CTL_MOD
};

typedef union epoll_data {
    void* ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t events;
    epoll_data_t data;
};

struct fd_t {
    int fd;
    struct epoll_event event;

    fd_t() {}
    fd_t(int _fd, struct epoll_event _event) {
        fd = fd;
        event = _event;
    }
};

typedef std::vector<fd_t> cp_internal;

static long get_wsa_mask(uint32_t epoll_events)
{
    long mask = 0;
    if(epoll_events & EPOLLIN)
        mask |= FD_READ;
    if(epoll_events & EPOLLOUT)
        mask |= FD_WRITE;
    if(epoll_events & EPOLLRDHUP)
        mask |= 0; // ??
    if(epoll_events & EPOLLPRI)
        mask |= 0; // ??
    if(epoll_events & EPOLLERR)
        mask |= 0; // ??
    if(epoll_events & EPOLLHUP)
        mask |= FD_CLOSE;
    if(epoll_events & EPOLLET)
        mask |= 0; // ??
    return mask;
}

static uint32_t get_cp_mask(WSANETWORKEVENTS* wsa_events) {
    __uint32_t mask = 0;
    if(wsa_events->lNetworkEvents & FD_READ)
        mask |= EPOLLIN;
    if(wsa_events->lNetworkEvents & FD_WRITE)
        mask |= EPOLLOUT;
    if(wsa_events->lNetworkEvents & FD_CLOSE)
        mask |= EPOLLHUP;
    return mask;
}
#endif

static struct {
    int initialised;
    int queue;
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
#ifdef EZPOLL_PLATFORM_WINDOWS
    int cp_next_id;
    std::map<int, cp_internal> cp_data;
#else
    struct epoll_event events[EVENT_SIZE];
#endif
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    struct kqueue events[EVENT_SIZE];
#endif
} state = {
    .initialised = 0
};

#ifdef EZPOLL_PLATFORM_WINDOWS
static int epoll_create(int size) {
    // maintaining error condition for compatibility
    // however, this parameter is ignored.
    if(size < 0) {
        // set errno to EINVAL
        return -1;
    }

    ++state.cp_next_id;

    // ran out of ids!  wrapped around.
    if(state.cp_next_id > (state.cp_next_id + 1)) {
        state.cp_next_id = 0;
    }

    while(state.cp_next_id < (state.cp_next_id + 1)) {
        if(state.cp_data.find(state.cp_next_id) == state.cp_data.end())
            break;
        ++state.cp_next_id;
    }

    if(state.cp_next_id < 0) {
        // two billion fds, eh...
        // set errno to ENFILE
        return -1;
    }

    state.cp_data_array[state.cp_next_id] = cp_internal();

    return state.cp_next_id;
}

int epoll_ctl(int cpfd, int opcode, int fd, struct epoll_event* event) {
    if(cpfd < 0 || state.cp_data.find(cpfd) != state.cp_data.end())
        return -1; // EBADF

    // TODO: find out if it's possible to tell whether fd is a socket
    // descriptor.  If so, make sure it is; if not, set EPERM and return -1.

    cp_internal& cpi = state.cp_data[cpfd];

    if(opcode == EPOLL_CTL_ADD) {
        for(cp_internal::size_t i = 0; i < cpi.size(); ++i)
            if(cpi[i].fd == fd)
                return -1; // EEXIST

        fd_t f(fd, *event);
        f.event.events |= EPOLLHUP;
        f.event.events |= EPOLLERR;

        cpi.push_back(f);
    }
    else if(opcode == EPOLL_CTL_MOD) {
        for(cp_internal::size_t i = 0; i < cpi.size(); ++i) {
            if(cpi[i].fd == fd) {
                cpi[i].event = *event;
                cpi[i].event.events |= EPOLLHUP;
                cpi[i].event.events |= EPOLLERR;
                return 0;
            }
        }

        // ENOENT
        return -1;
    }
    else if(opcode == EPOLL_CTL_DEL) {
        for(cp_internal::size_t i = 0; i < cpi.size(); ++i) {
            if(cpi[i].fd == fd) {
                cpi.erase(i);
                return 0;
            }
        }

        // ENOENT
        return -1;
    }

    // EINVAL
    return -1;
}

int epoll_wait(int cpfd, struct epoll_event* events, int maxevents, int timeout) {
    if(cpfd < 0 || state.cp_data.find(cpfd) == state.cp_data.end() || maxevents < 1)
        return -1; // EINVAL

    cp_internal& cpi = state.cp_data[cpfd];

    WSAEVENT* wsa_events = new WSAEVENT[cpi.size()];

    for(cp_internal::size_t i = 0; i < cpi.size(); ++i) {
        wsa_events[i] = WSACreateEvent();

        WSAEventSelect(cpi[i].fd, wsa_events[i], get_wsa_mask(cpi[i].event.events));
    }


    DWORD wsa_result = WSAWaitForMultipleEvents(num_wait_fds, wsa_events, FALSE, timeout, FALSE);

    int num_ready = 0;

    if(wsa_result != WSA_WAIT_TIMEOUT) {
        int e = wsa_result - WSA_WAIT_EVENT_0;

        for(cp_internal::size_t i = e; i < cpi.size() && num_ready < maxevents; ++i) {
            WSANETWORKEVENTS ne;
            if(WSAEnumNetworkEvents(cpi[i].fd, 0, &ne) != 0)
                return -1;
            
            if(ne.lNetworkEvents != 0) {
                if(cpi[i].event.events & EPOLLONESHOT)
                    cpi[i].event.events = 0;

                events[num_ready].events = get_cp_mask(&ne);
                events[num_ready].data = cpi[i].fd;
                ++num_ready;
            }
        }
    }

    for(cp_internal::size_t i = 0; i < cpi.size(); ++i) {
        // now unset the event notifications
        WSAEventSelect(cpi[i].fd, 0, 0);
        // clean up event
        WSACloseEvent(wsa_events[i]);
    }

    delete [] wsa_events;

    if(num_ready == 0)
    {
        // EINTR
    }

    return num_ready;
}

int epoll_close(int cpfd) {
    if(cpfd < 1 || state.cp_data.find(cpfd) == state.cp_data.end())
        return -1;

    state.cp_data.erase(cpfd);
    return 0;
}

void epoll_cleanup() {
    WSACleanup();
    state.cp_data.clear();
}
#endif

int ezPollInit(void) {
    if (state.initialised)
        return 0;
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    state.queue = epoll_create(EVENT_SIZE);
    state.event_count = 0;
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    state.queue = kqueue();
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
    state.cp_next_id = 0;
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif
    return 1;
}

int ezPollCreate(int fd) {
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(state.queue, EPOLL_CTRL_ADD, fd, &event);
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    struct kqueue event;
    EV_SET(&event, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(state.queue, &event, 1, NULL, 0, NULL);
#endif
    return 1;
}

int ezPollWait(void) {
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    return epoll_wait(state.queue, state.events, EVENT_SIZE, -1);
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    return kevent(state.queue, NULL, 0, state.events, EVENT_SIZE, -1);
#endif
}

int ezPollGet(int idx) {
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    return state.events[idx].data.fd;
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    return (int)state.events[idx].ident;
#endif
}
    
int ezPollClose(int fd) {
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    epoll_ctrl(state.queue, EPOLL_CTL_DEL, state.events[fd].data.fd, NULL);
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    (void)fd;
#endif
}
#endif
