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
int main()
{   struct pollfd proxyclient[10];
    for(int i=0;i<10;i++)
    {
        proxyclient[i].events=POLLIN|POLLOUT|POLLHUP;
        proxyclient[i].fd=-1;
    }
    int state=1;
    struct sockaddr_in myaddressinfo;
    myaddressinfo.sin_family=AF_INET;
    myaddressinfo.sin_port=htons(7777);
    int proxyip;
    inet_pton(AF_INET,"127.0.0.1",&proxyip);
    myaddressinfo.sin_addr.s_addr=proxyip;
    int con_prox_fd=socket(AF_INET,SOCK_STREAM,0);
    struct pollfd forwardsocket[10];
    for(int i=0;i<10;i++)
    {
        forwardsocket[i].events=POLLIN|POLLOUT|POLLHUP;
        forwardsocket[i].fd=-1;
    }

    int checkbind=bind(con_prox_fd,(struct sockaddr*)&myaddressinfo,sizeof(myaddressinfo));
    if(checkbind==-1)
    {
        perror("error;binding con_prox_fd\n");
        exit(1);
    }
    if(listen(con_prox_fd,MAX_LISTEN)==-1)
    {
        perror("error:listening");
        exit(1);
    }
    struct pollfd serverpoll;
    serverpoll.events=POLLIN;
    serverpoll.fd=con_prox_fd;
    int sizeofsockaddrin=sizeof(myaddressinfo);
    char *buffertoread=malloc(4096);
    char *buffertowrite=malloc(4096);
    struct sockaddr_in destaddress;
    destaddress.sin_family=AF_INET;
    destaddress.sin_port=htons(5555);
    inet_pton(AF_INET,"192.168.10.155",&destaddress.sin_addr.s_addr);
    

    while(state)
    {
        int unblockconnect=poll(&serverpoll,1,0);
        
        if(serverpoll.revents&POLLIN)
        {
            for(int j=0;j<10;j++)
            {
                if(proxyclient[j].fd==-1)
                {
                    proxyclient[j].fd=accept(con_prox_fd,NULL,NULL);//i am not interested who it is
                    forwardsocket[j].fd=socket(AF_INET,SOCK_STREAM,0);
                    if(forwardsocket[j].fd==-1)
                    {
                        perror("error:proxy cannot create forwarding socket\n");
                        //
                    }
                    if(connect(forwardsocket[j].fd,(struct sockaddr*)&destaddress,sizeof(destaddress))==-1)
                    {
                        send(proxyclient[j].fd,"the server is unable to take requests\n",strlen("the server is unable to take requests\n"),0);
                        close(forwardsocket[j].fd);
                        forwardsocket[j].fd=-1;
                        
                    }
                    

                }
            }
        }

        int unblockread=poll(proxyclient,10,2000);
        for(int i=0;i<10;i++)
        {
            if(proxyclient[i].revents&POLLHUP)
            {
                proxyclient[i].fd=-1;
            }
            else if(proxyclient[i].revents&POLLIN)
            {
             int readcount=read(proxyclient[i].fd,buffertoread,4095);
             buffertoread[strlen(buffertoread)]='\0';
             // what if nothing is read??? consider variants
            }
             
             
        }


            

    }

    
        
         

  
    

}   