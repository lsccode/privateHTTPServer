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
    NVP_S32 slSocket = createUdpServer();
	NVP_S32 siIndex = 0;
	tLiveImg stLiveImag = {0};
	stLiveImag.ulSize = 0;
	stLiveImag.ulCap = 256;
	stLiveImag.pstImg = (tImgInfo *)malloc(256*sizeof(tImgInfo));
	
	readFileList("./images",&stLiveImag);
	printFile(&stLiveImag);
	
    if(slSocket < 0)
    {
        debug("create udp server error!\n");
        return -1;
    }
    
    while(1)
    {
        tLocalMsg stMst = {0};
        tLocalMsg *pstMsg = NULL;
        NVP_CHAR szBuf[1024*4] = {0};
        
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        debug("\nserver runing ...\n");
        if(recvfrom(slSocket,&stMst,sizeof(stMst),0,(struct sockaddr *)&clientAddr,&clientLen) < 0)
        {
            debug("recv error,!");
            continue;
        }
        
        debug("recv (%s:%d),mestype = %u!\n",inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port),stMst.ulMsgType);
       
		
		if(siIndex == stLiveImag.ulSize)
		{
			siIndex = 0;
		}
        
		int imagfd = open(stLiveImag.pstImg[siIndex].imageName,O_RDONLY);
        if(imagfd < 0)
        {
            debug("open imagfd failed\n",pstMsg->ulMsgLen);
        }
		while(1)
		{
			pstMsg = (tLocalMsg *)szBuf;			
			pstMsg->ulMsgType = 1;
			pstMsg->ulMsgLen = read(imagfd,pstMsg->data,3*1024);
			
            debug("send mesg len = %u\n",pstMsg->ulMsgLen + 8);
			if(sendto(slSocket,szBuf,sizeof(tLocalMsg) + pstMsg->ulMsgLen,0,(struct sockaddr *)&clientAddr,clientLen) < 0)
			{
				debug("send error error(%d),strerr(%s)!\n",errno,strerror(errno));
				continue;
			}
			if(pstMsg->ulMsgLen < 3*1024)
            {
                debug("mesg len = %u,close\n",pstMsg->ulMsgLen + 8);
                break;  
            }
	
		}
        close(imagfd);
		++siIndex;
				
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
            printf("d_name:%s/%s\n",basePath,ptr->d_name);
        else if(ptr->d_type == 10)    ///link file
            printf("d_name:%s/%s\n",basePath,ptr->d_name);
        else if(ptr->d_type == 4)    ///dir
        {
            memset(base,'\0',sizeof(base));
            strcpy(base,basePath);
            strcat(base,"/");
            strcat(base,ptr->d_name);
            readFileList(base);
        }
    }
    closedir(dir);
    return 1;
}

int main(void)
{
    DIR *dir;
    char basePath[256] = {0};

    ///get the current absoulte path
    memset(basePath,'\0',sizeof(basePath));
    getcwd(basePath, 999);
    printf("the current dir is : %s\n",basePath);
	strcat(basePath,"images");

    readFileList(basePath);
    return 0;
}
#endif
