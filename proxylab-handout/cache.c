// simple lru-cache without hashtable
#include "csapp.h"
#include "cache.h"

typedef struct Node{
    char* host;
    char* path;
    char* buf;
    int port;
    int size;
    struct Node* next;
    struct Node *pre;
}Node;
static int readcnt;
static sem_t rmutex,wmutex;
static Node *head;
static int size;
static int capacity;


//FILE* testof;
void init_cache(int cap){

    sem_init(&rmutex,0,1);
    sem_init(&wmutex,0,1);
    head = (Node*) malloc(sizeof(Node));
    head->next = head;
    head->pre = head;
    capacity = cap;
//    testof = fopen("test.txt","w+");
};


Node * createNode(char *host, int port, char *path, char* buff,int bufsize){
    Node* newNode = (Node*) malloc(sizeof(Node));
    int hostlen = strlen(host);
    newNode->host = (char*)malloc(hostlen+1);
    strcpy(newNode->host,host);
    newNode->port = port;
    int pathlen = strlen(path);
    newNode->path = (char*)malloc(pathlen+1);
    strcpy(newNode->path,path);
    newNode->buf = (char*)malloc(bufsize);
    newNode->size = bufsize;
    memcpy(newNode->buf,buff,bufsize);
    return newNode;
}

void removelast(){
    Node* p = head->pre;
    p->next ->pre = p->pre;
    p->pre->next = p->next;

    size -= p->size;
    free(p->path);
    free(p->host);
    free(p->buf);
    free(p);
}
void put_cache(char *host, int port, char *path, char* buff, int bufsize){
//    fprintf(testof,"put host:%s path:%s\n",host,path);
//    fflush(testof);
//    rio_writen(fileno(testof),buff,bufsize);
//    fprintf(testof,"\n\n");
//    fflush(testof);
    Node *nnode = createNode(host,port,path,buff,bufsize);
    P(&wmutex);
    while(size+bufsize > capacity){
           removelast();
    }
    size+=bufsize;
    nnode ->next = head->next;
    nnode->pre = head;
    nnode->next->pre = nnode;
    nnode->pre->next = nnode;

    V(&wmutex);
}


/*
 * if cache found, return 1;
 * otherwise, return 0;
 * */
int get_cache(char*host, int port, char*path, void **bufptr, int * size){
    P(&rmutex);
    if(++readcnt==1){
        P(&wmutex);
    }
    V(&rmutex);
    Node*p = head->next;
    while(p!=head){
        if(port==p->port && !strcmp(host,p->host) && !strcmp(path,p->path)){
            break;
        }
        p = p->next;
    }
    if(p!=head){
        *bufptr = malloc(p->size);
        memcpy(*bufptr,p->buf,p->size);
        *size = p->size;
    }
    P(&rmutex);
    if(--readcnt==0){
        V(&wmutex);
    }
    V(&rmutex);
    if(p==head)return 0;

    // move node to head
    P(&wmutex);
    p->pre->next = p->next;
    p->next->pre = p->pre;
    p->pre = head;
    p->next = head->next;
    p->pre->next = p;
    p->next->pre = p;
    V(&wmutex);

//    fprintf(testof,"get host:%s path:%s, size:%d\n",host,path,*size);
//    fflush(testof);
//    rio_writen(fileno(testof),*bufptr,*size);
//    fflush(testof);


    return 1;
}





