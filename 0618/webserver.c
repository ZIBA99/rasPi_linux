#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

static void *clnt_connection(void *arg);
int sendData(FILE* fp, char *ct, char *filename);
void sendOk(FILE* fp);
void sendError(FILE* fp);

int main(int argc, char **argv)
{
    int ssock;
    pthread_t thread;
    struct sockaddr_in servaddr, cliaddr;
    unsigned int len;

    if(argc != 2){
        printf("usage: %s <port>\n", argv[0]);
        return -1;
    }

    ssock = socket(AF_INET, SOCK_STREAM, 0);
    if(ssock == -1){
        perror("socket()");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if(bind(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        perror("bind()");
        return -1;
    }

    if(listen(ssock, 10) == -1){
        perror("listen()");
        return -1;
    }

    while(1){
        char mesg[BUFSIZ];
        int csock;

        len = sizeof(cliaddr);
        csock = accept(ssock, (struct sockaddr*)&cliaddr, &len);
        if(csock == -1){
            perror("accept()");
            continue;
        }

        inet_ntop(AF_INET, &cliaddr.sin_addr, mesg, BUFSIZ);
        printf("client IP: %s:%d\n", mesg, ntohs(cliaddr.sin_port));

        // 스레드에 소켓 디스크립터 복사하여 전달
        int *pclient = malloc(sizeof(int));
        if(pclient == NULL){
            perror("malloc()");
            close(csock);
            continue;
        }
        *pclient = csock;

        pthread_create(&thread, NULL, clnt_connection, pclient);
        pthread_detach(thread); // 스레드 자원 자동 해제
    }
    return 0;
}

static void *clnt_connection(void *arg)
{
    int csock = *((int*)arg);
    free(arg);

    FILE *clnt_read, *clnt_write;
    char reg_line[BUFSIZ], reg_buf[BUFSIZ];
    char method[BUFSIZ], type[BUFSIZ];
    char filename[BUFSIZ], *ret;

    clnt_read = fdopen(csock, "r");
    clnt_write = fdopen(dup(csock), "w");

    if(clnt_read == NULL || clnt_write == NULL){
        perror("fdopen()");
        close(csock);
        return NULL;
    }

    if(fgets(reg_line, BUFSIZ, clnt_read) == NULL){
        fclose(clnt_read);
        fclose(clnt_write);
        return NULL;
    }
    fputs(reg_line, stdout);

    ret = strtok(reg_line, "/ ");
    strcpy(method, (ret != NULL) ? ret : "");

    if(strcmp(method, "POST") == 0) {
        sendOk(clnt_write);
        goto END;
    } else if(strcmp(method, "GET") != 0){
        sendError(clnt_write);
        goto END;
    }

    ret = strtok(NULL, " ");
    strcpy(filename, (ret != NULL) ? ret : "");

    if(filename[0] == '/'){
        // '/' 제거
        memmove(filename, filename + 1, strlen(filename));
    }

    // 기본 Content-Type 설정 (예: text/html)
    strcpy(type, "text/html");

    // 요청 헤더 읽기 (간단히 무시)
    do {
        if(fgets(reg_line, BUFSIZ, clnt_read) == NULL)
            break;
        fputs(reg_line, stdout);
        strcpy(reg_buf, reg_line);
        char *atr = strchr(reg_buf, ':');
        // 필요시 헤더 처리 가능
    } while(strncmp(reg_line, "\r\n", 2) != 0);

    sendData(clnt_write, type, filename);

END:
    fclose(clnt_read);
    fclose(clnt_write);

    pthread_exit(NULL);
    return NULL;
}

int sendData(FILE* fp, char *ct, char *filename)
{
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server: Netscape-Enterprise/6.0\r\n";
    char cnt_type[BUFSIZ];
    char end[] = "\r\n";
    char buf[BUFSIZ];
    int fd, len;

    snprintf(cnt_type, sizeof(cnt_type), "Content-Type:%s\r\n", ct);

    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_type, fp);
    fputs(end, fp);

    fd = open(filename, O_RDONLY);
    if(fd < 0){
        perror("open()");
        sendError(fp);
        return -1;
    }

    do {
        len = read(fd, buf, BUFSIZ);
        if(len > 0){
            fwrite(buf, 1, len, fp);
        }
    } while(len == BUFSIZ);

    close(fd);
    fflush(fp);

    return 0;
}

void sendOk(FILE* fp)
{
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server: Netscape-Enterprise/6.0\r\n\r\n";

    fputs(protocol, fp);
    fputs(server, fp);
    fflush(fp);
}

void sendError(FILE* fp)
{
    char protocol[] = "HTTP/1.1 400 Bad Request\r\n";
    char server[] = "Server: Netscape-Enterprise/6.0\r\n";
    char cnt_len[] = "Content-Length:1024\r\n";
    char cnt_type[] = "Content-Type:text/html\r\n\r\n";

    char content1[] = "<html><head><title>BAD Connection</title></head>";
    char content2[] = "<body><font size=+5>Bad Request</font></body></html>";

    printf("send_error\n");

    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len, fp);
    fputs(cnt_type, fp);
    fputs(content1, fp);
    fputs(content2, fp);
    fflush(fp);
}

