#ifndef PROXYLAB_HANDOUT_CACHE_H
#define PROXYLAB_HANDOUT_CACHE_H

void init_cache(int cap);
void put_cache(char *host, int port, char *path, char* buff, int bufsize);
int get_cache(char*host, int port, char*path, void **bufptr, int * size);


#endif //PROXYLAB_HANDOUT_CACHE_H
