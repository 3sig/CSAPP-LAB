#include <stdio.h>
#include "csapp.h"
#include "cache.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define SMALL_SIZE 512
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *connection_hdr = "Connection: close\r\n";
static const char *proxy_header = "Proxy-Connection: close\r\n";

void *proxy_thread(void *vargp);
void doproxy(int fd);
int connect_host(char *host, char* port);
int skip_header(char *header);

int main(int argc, char * argv[])
{
    int listenfd, *connfdp;

    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    pthread_t tid;

    if(argc != 2){
        fprintf(stderr,"usage: %s <port>\n", argv[0]);
        exit(0);
    }
    init_cache(MAX_CACHE_SIZE);
    listenfd = Open_listenfd(argv[1]);

    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd,(SA*) &clientaddr, &clientlen);
        Pthread_create(&tid,NULL,proxy_thread,connfdp);
    }
    return 0;
}

void *proxy_thread(void * vargp){
    int connfd = *((int*)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);
    doproxy(connfd);
    Close(connfd);
    return NULL;
}
void doproxy(int fd){

    char buf[MAXLINE], method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    rio_t rio;
    // init rio and read first line, i.e. GET XXX HTTP/1.1
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio,buf,MAXLINE);

    // parse the first line
    sscanf(buf,"%s %s %s",method, uri, version);

    // parse URI, extract protocol, host, port and path
    char protocol[SMALL_SIZE],host[SMALL_SIZE],port[SMALL_SIZE],path[SMALL_SIZE];
    sscanf(uri,"%[^://]://%[^:]:%[^/]%s",protocol,host,port,path);
    int portnum = atoi(port);
    // check cache
    int cachesz;
    void * cachebuf = NULL;
    if(get_cache(host,portnum,path,&cachebuf,&cachesz)){
        Rio_writen(fd,cachebuf,cachesz);
        free(cachebuf);
        return;
    }


    int hostfd = connect_host(host, port);
    if(hostfd<0){
        fprintf(stderr,"CONNECT HOST %s:%s FAILED!\n",host,port);
        return;
    }
    int nsize = sprintf(buf,"GET %s HTTP/1.0\r\n",path);
    rio_writen(hostfd,(void*)buf,nsize);
    rio_writen(hostfd,(void*)user_agent_hdr,strlen(user_agent_hdr));
    rio_writen(hostfd,(void*)connection_hdr,strlen(connection_hdr));
    rio_writen(hostfd,(void*)proxy_header,strlen(proxy_header));

    // read remaining bytes
    char header[SMALL_SIZE];
    while((nsize = Rio_readlineb(&rio,buf,MAXLINE)) > 0){

        sscanf(buf,"%[^:]:",header);
        if(skip_header(header)){
            continue;
        }
        rio_writen(hostfd,(void*)buf,nsize);
        if(!strcmp(buf,"\r\n"))break;
    }
    rio_t hostrio;
    Rio_readinitb(&hostrio,hostfd);

    void* cbuf = malloc(0);
    int cbufSize = 0;
    while((nsize = Rio_readnb(&hostrio,buf,MAXLINE))>0){
        Rio_writen(fd,(void*)buf,nsize);
        cbuf = realloc(cbuf,cbufSize+nsize);
        memcpy(cbuf+cbufSize,buf,nsize);
        cbufSize+=nsize;
    }
    if(cbufSize<MAX_OBJECT_SIZE) {
        put_cache(host, portnum, path, cbuf, cbufSize);
    }else{
        free(cbuf);
    }
    close(hostfd);
}


int connect_host(char *host, char* port){
    int fd = 0;
    fd = open_clientfd(host,port);
    return fd;
}

int skip_header(char *header){
    if(!strcmp(header,"Host")){
        return 1;
    }
    if(!strcmp(header,"GET")){
        return 1;
    }
    if(!strcmp(header,"User-Agent")){
        return 1;
    }
    if(!strcmp(header,"Connection")){
        return 1;
    }
    if(!strcmp(header,"Proxy-Connection")){
        return 1;
    }
    return 0;
}
