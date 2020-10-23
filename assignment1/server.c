// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080
int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1000] = {0};
    char *hello = "Hello from server";
    int pid = 0;

    printf("execve=0x%p\n", execve);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // do fork in parent only
    /*
        printf("the Number of args: %d \n", argc);
        for(int i = 0; i < argc; i++){
            printf("the arg #%d: %s \n",i,argv[i]);
        }
    */
    if(argc == 1){
        // Fork to create a new child
        pid = fork();
        if(pid == -1){
            printf("Failed to fork");
            return pid;
        }

        // If child process, then perform listen on the created socket
        else if(pid == 0) {
            //call exec here to get new address space for the child            
            char *args[] = {"./server", "child", NULL};
            execv(args[0], args);
        }

        //If parent process, wait for child server to finish and return
        else {
            printf("Parent process waiting\n");
            //this wait will pick up the child so there is no zombies.
            wait();
            printf("Child process returned\n");
        }
    // this should be in the child re-exec
    } else {

        printf("User ID before privilege drop %d\n",getuid());
        // if the setuid call fails, exit out of program with error.
        int returnVal = setuid(65534);
        if(returnVal) {
            printf("Failed to change user ID: %d \n", returnVal);
            return -1;
        }
        printf("User ID after privilege drop %d\n",getuid());
        printf("Child process running\n");
        //the child listens for connections
        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        // accept will return a new socket for the connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                    (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        valread = read( new_socket , buffer, 1024);
        printf("%s\n",buffer );
        send(new_socket , hello , strlen(hello) , 0 );
        printf("Hello message sent\n");
    }

    return 0;
}
