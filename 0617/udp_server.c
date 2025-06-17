#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>   // <netinet.h> → <netinet/in.h> 로 수정
#include <sys/socket.h>

#define UDP_PORT 5100

int main(int argc, char **argv)
{
    int sockfd, n;  // socktfd → sockfd로 수정
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char mesg[1000];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));  // memset(&servaddr. 0, ...) → 콤마(,)로 수정
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // sin_sddr.s → sin_addr.s_addr로 수정
    servaddr.sin_port = htons(UDP_PORT);

    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    do {
        len = sizeof(cliaddr);
        n = recvfrom(sockfd, mesg, 1000, 0, (struct sockaddr *)&cliaddr, &len);
        sendto(sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

        mesg[n] = '\0';  // meg[n] → mesg[n]으로 수정
        printf("Received data : %s\n", mesg);
    } while(strncmp(mesg, "q", 1));  // mseg → mesg로 수정

    close(sockfd);

    return 0;
}

