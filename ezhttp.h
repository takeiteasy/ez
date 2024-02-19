/* ezhttp.h -- Simple embedable HTTP server
 https://github.com/takeiteasy/

 NOTE: Windows is not implemented yet, POSIX only

 TODO:
   - [ ] Windows support
   - [ ] Improve route match check (callback or regex)
   - [ ] Add (optional) server logging

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

#ifndef EZHTTP_HEADER
#define EZHTTP_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>

#if !defined(EZHTTP_BUFFER_SIZE)
#define EZHTTP_BUFFER_SIZE 1024
#endif

#if !defined(EZHTTP_MAX_ROUTES)
#define EZHTTP_MAX_ROUTES 8
#endif

// TODO: EZHTTP_ENABLE_LOG does nothing
#if !defined(EZHTTP_ENABLE_LOG)
#define EZHTTP_ENABLE_LOG 0
#endif

typedef struct ezResponse {
    const char *body;
    const char *mime;
} ezResponse;

typedef struct ezRoute {
    const char *route;
    ezResponse(*callback)(void);
} ezRoute;

typedef struct ezHTTPServerDesc {
    int port;
    ezRoute routes[EZHTTP_MAX_ROUTES];
    int *quit;
} ezHTTPServerDesc;

int ezHTTPServerBegin(int port, ezRoute *routes, int *quit);
void ezHTTPServerBeginThread(void *args);

#if defined(__cplusplus)
}
#endif
#endif // EZHTTP_HEADER

#if defined(EZHTTP_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
int ezHTTPServerBegin(int port, ezRoute *routes, int *quit) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        return 0;
    struct sockaddr_in host = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    struct sockaddr_in client;
    size_t hostLength = sizeof(host);
    size_t clientLength = sizeof(client);
    if (bind(sock, (struct sockaddr*)&host, hostLength))
        return 0;
    if (listen(sock, SOMAXCONN))
        return 0;

    char buffer[EZHTTP_BUFFER_SIZE];
    for (;;) {
        if (quit)
            if (!*quit)
                return 1;
        int clientSock = accept(sock, (struct sockaddr*)&host, (socklen_t*)&hostLength);
        if (clientSock < 0)
            continue;
        int addr = getsockname(clientSock, (struct sockaddr*)&client, (socklen_t*)&clientLength);
        if (addr < 0)
            goto BAIL;
        if (read(clientSock, buffer, EZHTTP_BUFFER_SIZE) < 0)
            goto BAIL;

        char method[EZHTTP_BUFFER_SIZE],
             uri[EZHTTP_BUFFER_SIZE],
             version[EZHTTP_BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
        if (strncmp("GET", method, 3))
            goto BAIL;
        size_t uriLength = strlen(uri);

        for (int i = 0; i < EZHTTP_MAX_ROUTES; i++) {
            ezRoute *route = &routes[i];
            if (!route || (!route->route && !route->callback))
                break;
            // TODO: Replace this strncmp call with either a callback from user or use regex
            if (!strncmp(route->route, uri, uriLength)) {
                ezResponse response = route->callback();
                if (!response.body)
                    goto BAIL;
                if (!response.mime)
                    response.mime = "text/html";
                static char header[] = "HTTP/1.0 200 OK\r\nServer: ezHTTPServer\r\nContent-type: ";
                size_t finalLength = strlen(header) + strlen(response.mime) + strlen(response.body) + 7;
                char *final = malloc(finalLength);
                if (!final)
                    goto BAIL;
                sprintf(final, "%s%s\r\n\r\n%s\r\n", header, response.mime, response.body);
                final[finalLength-1] = '\0';
                write(clientSock, final, finalLength);
                free(final);
                goto BAIL;
            }
        }

    BAIL:
        close(clientSock);
    }
}

void ezHTTPServerBeginThread(void* _args) {
    ezHTTPServerDesc *args = (ezHTTPServerDesc*)_args;
    assert(args);
    if (!ezHTTPServerBegin(args->port, args->routes, args->quit))
        perror("ezHTTPServer");
}
#endif // EZHTTP_IMPLEMENTATION
