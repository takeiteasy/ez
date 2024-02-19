#define EZHTTP_IMPLEMENTATION
#include "ezhttp.h"

static ezResponse TestCallback(void) {
    return (ezResponse){ "<html>hello, world!</html>" };
}

static ezResponse TestCallbackPath(void) {
    return (ezResponse){ "<html><b>wow!</b></html>" };
}

int main(int argc, const char *argv[]) {
    ezRoute routes[] = {
        { "/", TestCallback },
        { "/test", TestCallbackPath },
        { NULL, NULL }
    };
    return ezHTTPServerBegin(8080, routes, NULL);
}
