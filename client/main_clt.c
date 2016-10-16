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
#include <error.h>

#define DEBUG
#undef DEBUG

#define BUF_SIZE (1024)

#define die(msg) \
        do { \
                perror(msg); \
                exit(-1); \
        } while (0)

int sd;

void int_hanlder(int signum)
{
        #ifdef DEBUG
                printf("Exiting...\n");
        #endif

        close(sd);
        exit(0);
}

int main(int argc, char *argv[])
{
        struct sockaddr_in svr_addr;
        char buf[BUF_SIZE];
        char res[BUF_SIZE];
        char file[BUF_SIZE];
        int fd;
        unsigned long file_size;
        char file_size_str[BUF_SIZE];
        int nread;

        if (4 != argc)
                die("Usge: ./command ip port file");

        signal(SIGINT, int_hanlder);

        sd = socket(AF_INET, SOCK_STREAM, 0);

        svr_addr.sin_family = AF_INET;
        svr_addr.sin_port   = htons(strtol(argv[2], NULL, 10));
        inet_pton(AF_INET, argv[1], &svr_addr.sin_addr);

        #ifdef DEBUG
                printf("Address: %s\nPort: %d\n", argv[1], ntohs(svr_addr.sin_port));
        #endif

        if (connect(sd, (struct sockaddr *)&svr_addr, sizeof svr_addr))
                die("connect()");

        /* 创建空文件 */
        sprintf(file, "./%s", argv[3]);
        fd = open(file, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);

        /* 构造请求 */
        sprintf(buf, "%s\r\n%s", "GET", argv[3]);
        write(sd, buf, strlen(buf));

        /* 接收回复 */
        nread = read(sd, buf, BUF_SIZE);
        buf[nread] = '\0';
        sscanf(buf, "%s", res);

        if (strcmp("START", res)) {
                printf("%s\n", buf);
                sprintf(buf, "rm -rf %s", file);
                system(buf);
                exit(0);
        }

        /* 计算文件大小 */
        sscanf(buf + strlen(res), "%s", file_size_str);
        file_size = strtol(file_size_str, NULL, 10);

        /* 传出文件 */
        while(file_size) {
                nread = read(sd, buf, BUF_SIZE);
                write(fd, buf, nread);

                #ifdef DEBUG
                        printf("file_size: %d\nnread: %d\n", file_size, nread);
                        getchar();
                #endif

                file_size -= nread;
        }
        close(fd);
        close(sd);

        sprintf(buf, "vlc %s", file);

        #ifdef DEBUG
                printf("%s\n", buf);
        #else
                system(buf);
        #endif

        return 0;
}
