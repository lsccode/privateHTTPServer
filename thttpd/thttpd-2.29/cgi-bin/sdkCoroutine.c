#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "nvp_types.h"

#define debug(format,...) \
    do{ \
    fprintf(stderr,"file( %s ), fun( %s ),line( %d ), "format, __FILE__,__func__,__LINE__, ##__VA_ARGS__); \
} \
while(0)

NVP_S32 createUdpServer()
{
    NVP_S32 sockfd;
    struct sockaddr_in addr;
    
    sockfd = socket (AF_INET,SOCK_DGRAM,0);
    if(sockfd <0)
    {
        debug("socket");
        exit(1);
    }
    
    bzero ( &addr, sizeof(addr) );
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6870);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))<0)
    {
        debug("bind");
        exit(1);
    }
    
    return sockfd;
}

typedef struct tagLocalMsg
{
    NVP_U32 ulMsgType;
    NVP_U32 ulMsgLen;
    NVP_CHAR data[0];
}tLocalMsg;
// "Connection: keep-alive\r\n" \

#define M_HTML_MSG \
"HTTP/1.0 200 OK\r\n" \
"Connection: keep-alive\r\n" \
"Content-type:text/html\r\n\r\n"\
"<html>\n"\
"<head><title>An html page from a cgi</title></head>\n"\
"<body>\n"\
"<h1>Board.Info</h1>\n"\
"</body>\n"\
"</html>"

#define M_MSG_END "LocalCommunicationCurrentEnd"
#define M_MSG_TYPE_END (0XFFFFFFFF)

int main(int argc, char *argv[])
{
    NVP_S32 slSocket = createUdpServer();
    if(slSocket < 0)
    {
        debug("create udp server error!\n");
        return -1;
    }
    NVP_U32 ulServerCount = 0;
    while(1)
    {
        tLocalMsg stMst = {0};
        tLocalMsg *pstMsg = NULL;
        NVP_CHAR szBuf[1024] = {0};
        
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        debug("\nserver runing ...\n");
        if(recvfrom(slSocket,&stMst,sizeof(stMst),0,(struct sockaddr *)&clientAddr,&clientLen) < 0)
        {
            debug("recv error,!");
            continue;
        }
        
        debug("recv (%s:%d),mestype = %u!\n",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port),stMst.ulMsgType);
        
        pstMsg = (tLocalMsg *)szBuf;
        pstMsg->ulMsgType = 1;
        pstMsg->ulMsgLen  = strlen(M_HTML_MSG);
        memcpy(pstMsg->data,M_HTML_MSG,pstMsg->ulMsgLen );
        
        if(sendto(slSocket,szBuf,sizeof(tLocalMsg) + pstMsg->ulMsgLen,0,(struct sockaddr *)&clientAddr,clientLen) < 0)
        {
            debug("send error error(%d),strerr(%s)!\n",errno,strerror(errno));
            continue;
        }
        
        pstMsg = (tLocalMsg *)szBuf;
        pstMsg->ulMsgType = M_MSG_TYPE_END;
        pstMsg->ulMsgLen  = strlen(M_HTML_MSG);
        memcpy(pstMsg->data,M_MSG_END,pstMsg->ulMsgLen);
        if(sendto(slSocket,szBuf,sizeof(tLocalMsg) + pstMsg->ulMsgLen,0,(struct sockaddr *)&clientAddr,clientLen) < 0)
        {
            debug("send error!\n");
            continue;
        }
        debug("server for user %u\n",++ulServerCount);
    }
    
    return 0;
}
#if 0
int main(int argc, char *argv[])
{
    printf("Content-type:text/html\n\n");
    printf("<html>\n");
    printf("<head><title>An html page from a cgi</title></head>\n");
    printf("<body>\n");
    printf("<h1>Board.Info</h1>\n");
    printf("</body>\n");
    printf("</html>\n");
    fflush(stdout);
    return 0;
}
#endif
