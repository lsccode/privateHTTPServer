#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include "nvp_types.h"

#define debug(format,...) \
    do{ \
    fprintf(stderr,"file( %s ), fun( %s ),line( %d ), "format, __FILE__,__func__,__LINE__, ##__VA_ARGS__); \
} \
while(0)

    typedef struct tagLocalMsg
    {
        NVP_U32 ulMsgType;
        NVP_U32 ulMsgLen;
        NVP_CHAR data[0];
    }tLocalMsg;

typedef struct tagImgInfo
{
    NVP_CHAR imageName[256];
}tImgInfo;

typedef struct tagLiveImgs
{
    NVP_U32 ulCap;
    NVP_U32 ulSize;
    tImgInfo *pstImg;
}tLiveImg;

#define M_MSG_END "LocalCommunicationCurrentEnd"
#define M_MSG_TYPE_END (0XFFFFFFFF)

NVP_S32 createUnixStreamServer()
{
    int server_sockfd;
    int server_len;
    struct sockaddr_in server_address;

    /*  Remove any old socket and create an unnamed socket for the server.  */

    
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /*  Name the socket.  */

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
    if(bind(server_sockfd, (struct sockaddr *)&server_address, server_len) < 0)
    {
        debug("open /tmp/server_socket ,%d %s failed\n",errno,strerror(errno));
    }

    /*  Create a connection queue and wait for clients.  */

    listen(server_sockfd, 5);
    return server_sockfd;
}

int readFileList(char *basePath,tLiveImg *pstLiveImg)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];

    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8)    ///file
        {
            if(pstLiveImg->ulSize < pstLiveImg->ulCap)
            {
                strcpy(pstLiveImg->pstImg[pstLiveImg->ulSize].imageName,basePath);
                strcat(pstLiveImg->pstImg[pstLiveImg->ulSize].imageName,"/");
                strcat(pstLiveImg->pstImg[pstLiveImg->ulSize].imageName,ptr->d_name);
                ++pstLiveImg->ulSize;
            }
        }

    }
    closedir(dir);
    return 1;
}

int printFile(tLiveImg *pstLiveImg)
{
    unsigned int i = 0;
    for(i = 0; i < pstLiveImg->ulSize;++i)
    {
        printf("image: %s\n",pstLiveImg->pstImg[i].imageName);
    }

}

int main(int argc, char *argv[])
{
    NVP_U32 ulServerCount = 0;
    NVP_S32 server_sockfd = createUnixStreamServer();
    NVP_S32 siIndex = 0;
    tLiveImg stLiveImag = {0};
    stLiveImag.ulSize = 0;
    stLiveImag.ulCap = 256;
    stLiveImag.pstImg = (tImgInfo *)malloc(256*sizeof(tImgInfo));

    readFileList("./images",&stLiveImag);
    printFile(&stLiveImag);

    if(server_sockfd < 0)
    {
        debug("create createUnixStreamServer server error!\n");
        return -1;
    }

    while(1) {
        tLocalMsg stMst = {0};
        tLocalMsg *pstMsg = NULL;
        NVP_CHAR szBuf[1024*4] = {0};
        struct sockaddr_un client_address;
        printf("server waiting\n");

        /*  Accept a connection.  */
        int client_len = sizeof(client_address);
        int client_sockfd = accept(server_sockfd, 
                                   (struct sockaddr *)&client_address, &client_len);
        
        if(client_sockfd < 0)
        {
            debug("accept error %d-%s\n",errno,strerror(errno));
        }

        /*  We can now read/write to client on client_sockfd.  */
        while(1)
        {
            if(read(client_sockfd, &stMst,sizeof(stMst)) < 0)
            {
                debug("read client_sockfd  error %d-%s\n",errno,strerror(errno));
            }
            //debug("\n");
            if(siIndex == stLiveImag.ulSize)
            {
                siIndex = 0;
            }
            //debug("\n");
            int imagfd = open(stLiveImag.pstImg[siIndex].imageName,O_RDONLY);
            if(imagfd < 0)
            {
                debug("open imagfd failed\n",pstMsg->ulMsgLen);
            }
            
            debug("open %s\n",stLiveImag.pstImg[siIndex].imageName);
            while(1)
            {
                pstMsg = (tLocalMsg *)szBuf;			
                ++pstMsg->ulMsgType;
                pstMsg->ulMsgLen = read(imagfd,pstMsg->data,2*1024);
                if(pstMsg->ulMsgLen < 0)
                {
                    debug("read file error!\n");
                }
                //debug("write %d!\n",sizeof(tLocalMsg) + pstMsg->ulMsgLen);
                send(client_sockfd, szBuf, sizeof(tLocalMsg) + pstMsg->ulMsgLen,0);
                if(pstMsg->ulMsgLen < 2*1024)
                {
                    debug("mesg len = %u,close\n",pstMsg->ulMsgLen + 8);
                    break;  
                }
            }       
            //debug("\n");
            ++siIndex;
            pstMsg = (tLocalMsg *)szBuf;
            pstMsg->ulMsgType = M_MSG_TYPE_END;
            pstMsg->ulMsgLen  = strlen(M_MSG_END);
            memcpy(pstMsg->data,M_MSG_END,pstMsg->ulMsgLen);
            
            if(send(client_sockfd, szBuf, sizeof(tLocalMsg) + pstMsg->ulMsgLen,0) < 0)
            {
                debug("send error!\n");
                continue;
            }
            close(imagfd);
            debug("server for user %u\n",++ulServerCount);
        }
        close(client_sockfd);
        
        debug("server for user %u\n",++ulServerCount);
    }

    return 0;
}

