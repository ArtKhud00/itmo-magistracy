#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>


#define BUFF_SIZE 10000

typedef struct HostParam{
    unsigned int port;
    char *host; 
} Host;

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

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    int res = connect(sockfd, addr, addrlen);
    if(res == -1){
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
}

void Inet_pton(int af, const char *src, void *dst){
    int res = inet_pton(af, src, dst);
    if(res == 0){
        printf("inet_pton failed: source does not contain a valid network address\n");
        exit(EXIT_FAILURE);
    }
    if(res == -1){
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }
}

void Write(int fd, const void *buf, size_t count){
    ssize_t nwritten = write(fd,buf,count);
    if(nwritten == -1){
        perror("write failed");
        exit(EXIT_FAILURE);
    }

}

// this function parses input string to extract host parameters
Host parseHost(char *hostpar){
    Host serverpar;
    char *hostPort = strtok(hostpar, ":");
    char *host = &hostPort[0];
    char *port = strtok(NULL, ":");
    serverpar.port = atoi(port);
    serverpar.host = host;
    return serverpar;
}

// this function sends directories names
// and recieves the contents of the sent catalogs
void SendRecvDirs(int sock, int ndir, char *dirs[] ){
    for (int i = 2; i < ndir; ++i){
        char buf[BUFF_SIZE];
        Write(sock, dirs[i], strlen(dirs[i]));
        ssize_t nread = read(sock,buf,BUFF_SIZE);
        printf("-----------------------\n");
        write(STDOUT_FILENO, buf, nread);
        printf("\n");
        printf("------------------------\n");
    }
    printf("_______________________\n");
}



int main(int argc, char *argv[]) {
    (void)argv;
    if (argc < 2){
        printf("Please type correct parameters:\n./client.exe [hostname]:[port] [dir] ...\n");
        exit(EXIT_FAILURE);
    } 

    int client = Socket(AF_INET, SOCK_STREAM, 0);
    Host serverpar = parseHost(argv[1]);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    // need to define server address and port
    addr.sin_port = htons(serverpar.port);
    Inet_pton(AF_INET, serverpar.host, &addr.sin_addr);
    //connect to server with parameters defined previously
    Connect(client, (struct sockaddr*) &addr, sizeof addr);
    printf("connected with server successfully\n");
    SendRecvDirs(client,argc,argv);
    close(client);
    return 0;
}
