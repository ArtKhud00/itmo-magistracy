#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>

#define MESS_SIZE 10000

int server = -1;
int fd = -1;
pthread_mutex_t st_mutex;

void interr_handle(int signum){
    (void)signum;
    if(fd>0){
        close(fd);
    }
    if(server>0){
        close(server);
    }
    printf("\n-------------------------------\n");
    printf("Server was manually interrupted\n");
    printf("-------------------------------\n");
    exit(-1);
}

int Socket (int domain, int type, int protocol){
    int res = socket(domain, type, protocol);
    if (res == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Bind (int sockfd, const struct sockaddr *addr, socklen_t addrlen ){
    int res = bind(sockfd, addr, addrlen);
    if (res == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog){
    int res = listen(sockfd, backlog);
    if (res == -1){
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    int res = accept(sockfd, addr, addrlen);
    if (res == -1){
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

void removeChar(char *str, char removable) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != removable) dst++;
    }
    *dst = '\0';
}

char* DirCont(char *path){
    char* dirFilesData = (char*)malloc(MESS_SIZE);
    sprintf(dirFilesData, "\n[dir] %s: \n", path);
    DIR *dir;
    struct dirent *entry;
    dir = opendir (path);
    if (!dir){
        perror("opendir error");
        close(server);
        close(fd);
        exit(EXIT_FAILURE);
    }
    while ( (entry = readdir(dir)) != NULL) {
        char buffer[MESS_SIZE];
        sprintf(buffer,"%s\n",entry->d_name);
        strncat(dirFilesData, buffer, strlen(buffer));
    }
    closedir(dir);
    return dirFilesData;
}


void* thread_routine(void *arg){
    int fd = *((int*)arg);
    char buf[MESS_SIZE];
    (void)pthread_mutex_lock(&st_mutex);
    while(read(fd, buf, MESS_SIZE) > 0){
        removeChar(buf, '\r');
        removeChar(buf, '\n');
        char* reply;
        // if we have more than 1 path
        if(strchr(buf, ' '))
        {
            char* dirs = strtok(buf, " ");
            while (dirs != NULL){
                char* tempreply = DirCont(dirs);
                strncat(reply, tempreply, strlen(tempreply));
                dirs = strtok(NULL, " "); 
            }
        }
        else{
            reply = DirCont(buf);
        }
        write(fd,reply,strlen(reply));
        memset(buf, 0, sizeof(buf));
    }

    (void)pthread_mutex_unlock(&st_mutex);
    sleep(2);
    printf("th%ld\n\n", pthread_self());        
    return NULL;    
}


int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    signal(SIGINT, interr_handle);
    pthread_t th;
    pthread_attr_t ta;
    (void) pthread_attr_init(&ta);
    (void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
    (void) pthread_mutex_init(&st_mutex, 0);
    server = Socket(AF_INET, SOCK_STREAM, 0);
    printf("  Server was created  \n");
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(34543);
    Bind(server, (struct sockaddr *) &addr, sizeof addr);
    printf("Bind to port %d\n", addr.sin_port);
    // 5 is number of clients to be listened
    Listen(server, 5);
    socklen_t addrlen = sizeof addr;
    // client file descriptor to read/write data 
    // socket to work 
    while((fd = Accept(server, (struct sockaddr *) &addr, &addrlen))){
        int *cfd = malloc(1);
        cfd = &fd;
        //creating thread for every connection
        if(pthread_create(&th, &ta, thread_routine, cfd)!=0){
            perror("create thread error");
            close(server);
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
