#ifndef __VIDEO_CLASS_H
#define __VIDEO_CLASS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEBUG

#ifndef BUF_SIZE
#define BUF_SIZE (1024)
#endif /*BUF_SIZE*/

struct video_class;
struct video_class_hanlder;
struct video_class_ops;

struct video_class_hanlder {
        int (*say_hello)(struct video_class *, int, const char *);
        int (*send_file)(struct video_class *, int, const char *);
};

struct video_class_ops {
        int (*bind)(struct video_class *, const char *, int);
        int (*run)(struct video_class *, int);
        int (*response)(struct video_class *, int);
};

struct video_class {
        struct sockaddr_in ipaddr;
        int skt_desc;
        char buf[BUF_SIZE];
        
        struct video_class_ops *ops;
        struct video_class_hanlder *hanlder;
};

extern int die(const char *);
extern struct video_class *video_class_new(struct video_class_ops *, struct video_class_hanlder *);
extern void video_class_destory(struct video_class *);

#endif /* __VIDEO_CLASS_H  */