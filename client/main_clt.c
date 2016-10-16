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

#define BUF_SIZE (1024)

#define die(msg) \
        do { \
                printf("ERROR: %s\n", (msg)); \
                exit(-1); \
        } while (0)

int main(int argc, char *argv[])
{
        int sd;
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
                
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd < 0)
                die("socket()");
        
        svr_addr.sin_family = AF_INET;
        svr_addr.sin_port   = htons(strtol(argv[2], NULL, 10));
        inet_pton(AF_INET, argv[1], &svr_addr.sin_addr);
        
#ifdef DEBUG
        printf("Address: %s\nPort: %d\n", argv[1], ntohs(svr_addr.sin_port));
#endif 

        if (connect(sd, (struct sockaddr *)&svr_addr, sizeof svr_addr))
                die("connect()");
                
        sprintf(buf, "%s\r\n%s", "FILE_FETCH", argv[3]);

#ifdef DEBUG
        printf("Buf: %s\n", buf);
#endif 

        write(sd, buf, strlen(buf));
        nread = read(sd, buf, BUF_SIZE);
        buf[nread] = '\0';
        sscanf(buf, "%s", res);
        if (strcmp("FILE_START", res))
                die(buf);
        
        sscanf(buf + strlen(res) + 2, "%s", file_size_str);
        file_size = strtol(file_size_str, NULL, 10);
        
        sprintf(file, "/tmp/%s", argv[3]);
        fd = open(file, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
        while(file_size) {
                nread = read(sd, buf, BUF_SIZE);
                write(fd, buf, nread);
                file_size -= nread;
        }
        
        sprintf(buf, "vlc %s", file);
#ifdef DEBUG
        printf("%s\n", buf);
#else
        system(buf);
#endif
        return 0;
}