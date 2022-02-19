#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "cachelab.h"

int Verbose;
int SetIndexBits;
unsigned SetIndexMask;
int Associativity;
int BlockBits;
unsigned TagShift;
unsigned long TagMask;
char TraceFileName[256];


int Miss = 0, Hit = 0, Eviction = 0;

void parseArgs(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'v':
                Verbose = 1;
                break;
            case 's':
                SetIndexBits = atoi(optarg);
                break;
            case 'E':
                Associativity = atoi(optarg);
                break;
            case 'b':
                BlockBits = atoi(optarg);
                break;
            case 't':
                strcpy(TraceFileName, optarg);
                break;
            default:
                break;
                // print help; ;
        }
    }
}


typedef struct CacheLine {
    char valid;
    unsigned lru;
    unsigned long tag;
} CacheLine;

CacheLine *cache;
unsigned CacheSetSize;

void initCache() {
    SetIndexMask = (1 << SetIndexBits) - 1;
    TagShift = SetIndexBits + BlockBits;
    TagMask = (1L << (64 - TagShift)) - 1;
    CacheSetSize = Associativity * sizeof(CacheLine);
    int CacheSize = (1 << SetIndexBits) * CacheSetSize;
    cache = (CacheLine *) malloc(CacheSize);
    memset(cache, 0, CacheSize);
}

int accessCache(unsigned long address) {
    unsigned sindex = (address >> BlockBits) & SetIndexMask;
    unsigned long tag = (address >> TagShift) & TagMask;
    CacheLine *cacheBase = cache + Associativity * sindex;
//    printf("address:%lx\tcache set index:%d\tcache tag:%lx\n", address, sindex, tag);
    for (int i = 0; i < Associativity; ++i) {
        if ((cacheBase + i)->tag == tag && (cacheBase + i)->valid) {
            // cache hit
            ++Hit;
            unsigned curLru = (cacheBase + i)->lru;
            for (int j = 0; j < Associativity; ++j) {
                if ((cacheBase + j)->valid &&(cacheBase + j)->lru < curLru) {
                    ++(cacheBase + j)->lru;
                }
            }
            (cacheBase + i)->lru = 0;
            return 1;
        }
    }
    // cache miss
    ++Miss;
    int ret = 0;
    ret = 0x4;
    for(int i=0;i<Associativity;++i){
        if((cacheBase+i)->valid){
            (cacheBase+i)->lru = (1+(cacheBase+i)->lru) % Associativity;
        }
    }
    for (int i = 0; i < Associativity; ++i) {
        if((cacheBase+i)->lru==0){
            if((cacheBase+i)->valid){
                ++Eviction;
                ret |= 0x2;
            }else{
                (cacheBase+i)->valid = 1;
            }
            (cacheBase+i)->tag = tag;
            break;
        }
    }
    return ret;
}


void simulate(FILE *fp) {
    const int buffSize = 1024;
    char line[buffSize];
    char ins;
    unsigned long address;
    unsigned size;
    while (fgets(line, buffSize, fp)) {
        if (line[0] != ' ')continue;
        sscanf(line, " %c %lx,%d", &ins, &address, &size);
        //printf("%c,%x,%d\n",ins,add,size);
        int ret = 0;
        switch (ins) {
            case 'M':
                ++Hit ;
            case 'L':
            case 'S':
                ret = accessCache(address);
                break;
        }
        //unsigned sindex = (address >> BlockBits) & SetIndexMask;
        if(Verbose){
            printf("%c %lx,%x ",ins,address,size);
            if(ret &0x4)printf("miss ");
            if(ret &0x2)printf("eviction ");
            if(ret & 0x1)printf("hit ");
            if(ins=='M')printf("hit ");
            printf("\n");
        }
    }

}


int main(int argc, char *argv[]) {
    parseArgs(argc, argv);
    FILE *fp = fopen(TraceFileName, "r");
    if (fp == NULL) {
        printf("file %s not found!\n", TraceFileName);
        exit(0);
    }
    initCache();
    simulate(fp);

    //printf("hits:%d misses:%d evictions:%d\n", Hit, Miss, Eviction);
    printSummary(Hit, Miss, Eviction);
    fclose(fp);
    free(cache);
    return 0;
}
