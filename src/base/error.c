#include <stdlib.h>
#include <stdio.h>

typedef void (*sighandler)(int);

enum queryDebugger {
    waiting = 0,
    detected,
    not_attached,
};

static enum queryDebugger status = waiting;

static void handler(int signum) {
    status = not_attached;
}

void stop_gdb() {
    if (status == waiting) {
        sighandler prev;

        /* XXX: Set a handler that GDB filters, ensuring status stays as
         * 'detected' when running on the debugger. */
        status = detected;
        prev = signal(SIGTRAP, handler);
        raise(SIGTRAP);
        if (status == detected)
            signal(SIGTRAP, prev);
        else
            signal(SIGTRAP, SIG_IGN);
    }
    else if (status == detected)
        raise(SIGTRAP);
}
