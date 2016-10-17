#include "video_class.h"

extern struct video_class_handler svr_handler;
extern struct video_class_ops svr_ops;

struct video_class *server;

void int_handler(int signum)
{
        #ifdef DEBUG
                printf("Exiting...\n");
        #endif

        video_class_destory(server);
        exit(0);
}

int main(int argc, char *argv[])
{
        int port;

        if (3 != argc)
                die("./command ip port");

        signal(SIGCHLD, SIG_IGN);
        signal(SIGINT, int_handler);

        sscanf(argv[2], "%d", &port);

        server = video_class_new(&svr_ops, &svr_handler);
        server->ops->bind(server, NULL, port);
        server->ops->run(server, 10);

        return 0;
}
