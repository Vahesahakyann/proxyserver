#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<poll.h>
#include<sys/socket.h>
#include<netdb.h>
#include<sys/mman.h>
#include<arpa/inet.h>
#include<string.h>
#define MAX_LISTEN 10 
#define MAX_CLIENT 10 
//clientnum//messagesize//message !!!this is message construction
//aktivacnel bolori hamar nonblock rejim vor kardalis chblokavorven ete mejy ban chlini
int main()
{
    int state=1;
    struct sockaddr_in proxyaddress;//address to create socket to listen clients 
    proxyaddress.sin_family=AF_INET;
    proxyaddress.sin_port=htons(7777);
    inet_pton(AF_INET,"127.0.0.1",&proxyaddress.sin_addr.s_addr);
    struct pollfd proxyclient[MAX_CLIENT];
    for(int i=0;i<MAX_CLIENT;i++)
    {
        proxyclient[i].events=POLLIN|POLLOUT|POLLHUP;
        proxyclient[i].fd=-1;
    }
    struct pollfd con_prox_poll;//poll struct to find out whether i can read without block
    con_prox_poll.events=POLLIN;
     con_prox_poll.fd=socket(AF_INET,SOCK_STREAM,0);
    int checkbind=bind(con_prox_poll.fd,(struct sockaddr*)&proxyaddress,sizeof(proxyaddress));
    if(checkbind==-1)
    {
        perror("error;binding con_prox_fd\n");
        exit(1);
    }
    if(listen(con_prox_poll.fd,MAX_LISTEN)==-1)
    {
        perror("error:listening");
        exit(1);
    }
    int sizeofsockaddrin=sizeof(proxyaddress);
    char *buffer=malloc(4096);
    char *bufferforward=malloc(4096);
    if(buffer==NULL|bufferforward==NULL)
    {
        perror("error:can't get dynamic memory,buffer or bufferforward\n");
        exit(EXIT_FAILURE);
    }
    /////
    struct sockaddr_in backend_address;//address structure for proxy server for connec() to backend server
    backend_address.sin_family=AF_INET;
    backend_address.sin_port=htons(5555);
    inet_pton(AF_INET,"127.0.0.1",&backend_address.sin_addr.s_addr);
    /////
    struct pollfd forwardmessages;//poll to findout if we can write or read without block
    forwardmessages.fd=socket(AF_INET,SOCK_STREAM,0);//creating socket to connect to backend server
    forwardmessages.events=POLLIN|POLLOUT|POLLHUP;
    /////
    int connecttobackend=connect(forwardmessages.fd,(struct sockaddr*)&backend_address,sizeof(backend_address));
    if(connecttobackend==-1)
    {
        perror("error:can't establish connection with backend server\n");
        exit(EXIT_FAILURE);
    }
    int numbertoread;//number to understand how many bytes proxy to read from proxyclients
    int clientnum;//
    int findoutclient;//memory to receive from backend server whom to send 
    int findoutsize;//memory to understand how many bytes backend server response message is long
    while(state)
    {
        if(poll(&con_prox_poll,1,0)>0)
        {
            if(con_prox_poll.revents&POLLIN)
            {
                for(int i=0;i<MAX_CLIENT;i++)
                {
                    if(proxyclient[i].fd==-1)
                    {
                        proxyclient[i].fd=accept(con_prox_poll.fd,(struct sockaddr*)&proxyaddress,&sizeofsockaddrin);
                        break;
                    }
                }
                printf("all connection are busy, or poll end with error\n");
                /// stex  kareli e pakel
            }
            
        }  
        int is_unblockread=poll(proxyclient,MAX_CLIENT,2000);
        int is_unblockwrite=poll(&forwardmessages,1,1000);
        if(is_unblockread>0)
        {
            for(int i=0;i<MAX_CLIENT;i++)
            {
                if(proxyclient[i].revents&POLLHUP)//if socket is closed on the other side we proxy also closes
                {
                    close(proxyclient[i].fd);
                    proxyclient[i].fd=-1;
                }
                if((proxyclient[i].revents&POLLIN)&(forwardmessages.revents&POLLOUT)) 
                {
                    if(read(proxyclient[i].fd,&numbertoread,sizeof(int))<=0)//reading message length
                    {
                        perror("error:cant read how many bytes to read\n");
                    }
                    int readbytes=read(proxyclient[i].fd,buffer,numbertoread);//reading message from client
                    if(readbytes==0 |readbytes==-1)
                    {
                        continue;
                    }
                    else
                    {//if proxy can read from clients without being blocked and can forward to backend server without being blocked
                       //clientnum/messagesize/message   !!!!!!!!! this is message format
                        send(forwardmessages.fd,&i,sizeof(int),MSG_MORE);//writing client number
                        send(forwardmessages.fd,&numbertoread,sizeof(int),MSG_MORE);//writing message length
                        send(forwardmessages.fd,buffer,numbertoread,0);//writing message 
                    }
                }
            }
        }
        int is_unblock_read_back=poll(&forwardmessages,1,0);//checking if we can read without being blocked
        if(forwardmessages.revents&POLLIN)
        { 
            
            while(read(forwardmessages.fd,&findoutclient,sizeof(int))>0)  //to get clientnumber
            {
             read(forwardmessages.fd,&findoutsize,sizeof(int));//to get message length
             read(forwardmessages.fd,bufferforward,findoutsize);//to get message
             write(proxyclient[findoutclient].fd,bufferforward,findoutsize);//send message to client
             }; 

        }
        printf("checking:new cycle\n");
    }
    free(buffer);
    free(bufferforward);
     return 0;   
}

    












