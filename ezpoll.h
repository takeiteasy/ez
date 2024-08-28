/* ezpoll.h - Wrapper over epoll, kqueue and iocp
   https://github.com/takeiteasy/ez

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

#define EZPOLL_BACKEND_EPOLL  0
#define EZPOLL_BACKEND_KQUEUE 1
#define EZPOLL_BACKEND_IOCP   2

#ifndef EZPOLL_BACKEND
#if defined(__APPLE__) || defined(__MACH__)
#define EZPOLL_BACKEND EZPOLL_BACKEND_KQUEUE
#elif defined(_WIN32) || defined(_WIN64)
#define EZPOLL_BACKEND EZPOLL_BACKEND_IOCP
#else
#define EZPOLL_BACKEND EZPOLL_BACKEND_EPOLL
#endif
#endif

int ezPollInit(void);
int ezPollWait(void);
#if EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
int ezPollCreate(HANDLE fd);
HANDLE ezPollGet(void);
int ezPollClose(HANDLE fd);
#else
int ezPollCreate(int fd);
int ezPollGet(int idx);
int ezPollClose(int fd);
#endif

#if defined(__cplusplus)
}
#endif
#endif // EZPOLL_HEADER

#if defined(EZPOLL_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
#include <sys/epoll.h>
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
#include <sys/event.h>
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
// #include <windows.h>
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

static struct {
    int initialised;
#if EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
    HANDLE queue;
#else
    int queue;
#endif
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    struct epoll_event events[EVENT_SIZE];
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    struct kqueue events[EVENT_SIZE];
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
    // ...
#endif
} state = {
    .initialised = 0
};

int ezPollInit(void) {
    if (state.initialised)
        return 0;
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    state.queue = epoll_create(EVENT_SIZE);
    state.event_count = 0;
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    state.queue = kqueue();
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
    state.queue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
#endif
    return 1;
}

#if EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
int ezPollCreate(HANDLE fd) {
#else
int ezPollCreate(int fd) {
#endif
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(state.queue, EPOLL_CTRL_ADD, fd, &event);
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    struct kqueue event;
    EV_SET(&event, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(state.queue, &event, 1, NULL, 0, NULL);
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
    HANDLE iop = CreateIoCompletionPort(fd, completionPortHandle, (DWORD)fd, 0);
#endif
    return 1;
}

int ezPollWait(void) {
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    return epoll_wait(state.queue, state.events, EVENT_SIZE, -1);
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    return kevent(state.queue, NULL, 0, state.events, EVENT_SIZE, -1);
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
    WaitForSingleObjectEx(state.queue, INFINITE, FALSE);
    DWORD n = 1;
    LPOVERLAPPED overlapped = NULL;
    return GetQueuedCompletionStatus(state.queue, NULL, &overlapped, &n, INFINITE);
#endif
}

#if EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
HANDLE ezPollGet(int idx) {
#else
int ezPollGet(int idx) {
#endif
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    return state.events[idx].data.fd;
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    return (int)state.events[idx].ident;
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
    // ...
#endif
}
    
#if EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
int ezPollClose(HANDLE fd) {
#else
int ezPollClose(int fd) {
#endif
#if EZPOLL_BACKEND == EZPOLL_BACKEND_EPOLL
    epoll_ctrl(state.queue, EPOLL_CTL_DEL, state.events[fd].data.fd, NULL);
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_KQUEUE
    (void)fd;
#elif EZPOLL_BACKEND == EZPOLL_BACKEND_IOCP
    CloseHandle(state);
#endif
}
#endif
