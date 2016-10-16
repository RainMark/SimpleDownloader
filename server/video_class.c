#include "video_class.h"

int inline die(const char *msg)
{
        printf("ERROR: %s\n", msg);
        exit(-1);
        return 0;
}

struct video_class *video_class_new(struct video_class_ops *ops, struct video_class_hanlder *hanlder)
{
        struct video_class *svr = malloc(sizeof (struct video_class));
        svr->ops = ops;
        svr->hanlder = hanlder;
        svr->ipaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        return svr;
}

void inline video_class_destory(struct video_class *svr)
{
        close(svr->skt_desc);
        free(svr);
}

int __bind(struct video_class *svr, const char *addr, int port)
{
        svr->skt_desc = socket(AF_INET, SOCK_STREAM, 0);
        svr->ipaddr.sin_family = AF_INET;
        svr->ipaddr.sin_port = htons(port);
        
        if (addr)
                !inet_pton(AF_INET, addr, &(svr->ipaddr.sin_addr)) || die("inet_pton() failed!");
                
#ifdef DEBUG
        printf("skt_desc : %d \naddr : %s\nport: %d\n", svr->skt_desc, addr, port);
#endif 

        return bind(svr->skt_desc, (struct sockaddr *)&(svr->ipaddr), sizeof svr->ipaddr);
}

int __run(struct video_class *svr, int queue_cnt)
{
        int clt_desc;
        
        if (listen(svr->skt_desc, queue_cnt))
                die("listen() failed!");
                
#ifdef DEBUG
        printf("Runing...\n");
#endif 
                
        while (1) {
                int nread, nwrite;
                clt_desc = accept(svr->skt_desc, (struct sockaddr *)NULL, NULL);
                if (clt_desc < 0)
                        continue;
                
                if (!fork()) {
                        nread = read(svr->skt_desc, svr->buf, BUF_SIZE);
                        svr->buf[nread] = '\0';
#ifdef DEBUG
        printf("Reiceve :%s\n", svr->buf);
#endif 
                        svr->ops->response(svr, clt_desc);
                        close(clt_desc);
                        exit(0);
                }
                
                close(clt_desc);
        }
        
        return 0;
}

int __response(struct video_class *svr, int desc)
{
        int retvar = 0;
        char req[BUF_SIZE];
        
        sscanf(svr->buf, "%s", req);
        
#ifdef DEBUG
        printf("Request :%s\n", req);
#endif 

        if (!strcmp("FILE_FETCH", req)) {
                char file[BUF_SIZE];
                
                sscanf(svr->buf + strlen(req), "%s", file);
                retvar = svr->hanlder->send_file(svr, desc, file);
                
        } else if (!strcmp("GREET", req)) {
                retvar = svr->hanlder->say_hello(svr, desc, "Video server v0.1");

        } else {
                retvar = svr->hanlder->say_hello(svr, desc, "Request error!");
                
        }
        
        return retvar;
}

struct video_class_ops svr_ops = {
        .bind     = __bind,
        .run      = __run,
        .response = __response,
};

int __say_hello(struct video_class *svr, int client, const char *msg)
{
        int nwrite;
        char buf[BUF_SIZE];
        
        sprintf(buf, "%s\r\n%s", "GREET", msg);
        nwrite = write(client, buf, strlen(buf));
        
        return nwrite == strlen(buf);
}

int __send_file(struct video_class *svr, int client, const char *file)
{
        int nread, nwrite;
        char buf[BUF_SIZE];
        struct stat st;
        int retvar = 0;
        int fd = open(file, O_RDONLY);

        if (fd < 0) {
                svr->hanlder->say_hello(svr, client, "File not exist!");
                retvar = -1;
                goto no_file;
        }
                
        stat(file, &st);
        sprintf(buf, "%s\r\n%ld", "FILE_START", st.st_size);
        nwrite = write(client, buf, strlen(buf));
        while (!(nread = read(fd, buf, BUF_SIZE)))
                nwrite = write(client, buf, nread);
        close(fd);
                
no_file:
        return retvar;
}

struct video_class_hanlder svr_hanlder = {
        .say_hello      = __say_hello,
        .send_file      = __send_file,
};