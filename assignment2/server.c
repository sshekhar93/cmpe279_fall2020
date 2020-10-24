// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080
int main(int argc, char const *argv[]) {
    //pipe
    int fd1[2];
    int fd2[2];
    
    if(pipe(fd1) == -1){
        printf("Pipe 1 Failed");
        return -1;
    }

    if(pipe(fd2) == -1){
        printf("Pipe 2 Failed");
        return -1;
    }

    if(argc == 1){
        int server_fd, new_socket, valread;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);
        // char buffer[1000] = {0};
        // char *hello = "Hello from server";
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

        //PIPE CODE
        //close the reading ends of the pipes
            close(fd1[0]);
            close(fd2[0]);

            //write the address to pipe 1
            //write(fd[1], address, addrlen);

            write(fd1[1], address.sin_family, sizeof(AF_INET));
            write(fd1[1], address.sin_addr.s_addr, sizeof(INADDR_ANY));
            write(fd1[1], address.sin_port, sizeof(PORT));
            close(fd1[1]);
            
            //write the server_fd to pipe 2

            write(fd2[1], server_fd, sizeof(server_fd));
            close(fd2[1]);

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

        //If parent process, write to the pipes and then wait for child server to finish to return
        else {
            printf("Parent process waiting\n");
            wait();
            printf("Child process returned\n");
        }
    // this should be in the child re-exec
    } else {
        //initialize variables
        int new_socket, valread;
        char buffer[1000] = {0};
        char *hello = "Hello from server";

        //close the write end of the pipes
        close(fd1[1]);
        close(fd2[1]);

        //read the server_fd from pipe 2
        int server_fd;
        read(fd2[0], server_fd, sizeof(int));


        //read the address from pipe1
        struct sockaddr_in address;
        read(fd1[0], address.sin_family, sizeof(int));
        read(fd1[0], address.sin_addr.s_addr, sizeof(INADDR_ANY));
        read(fd1[0], address.sin_port, sizeof(PORT));
        int addrlen = sizeof(address);


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
