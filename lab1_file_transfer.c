#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

int protocol = -1; //如果protocol為0為TCP 為1為UDP
int port; //連接桿
char filename[256]; //存輸入的TXT檔案名字 以便之後找位置
char ip[256];//存IP

void error(const char *msg) //print error msg(總之函式裡面有)
{
    perror(msg);
    exit(0);
}


void server(){
	if(protocol == 0){
	     int sockfd, newsockfd, percent, data = 0, count = 1;
	     socklen_t clilen;
	     char buffer[256];
	     struct sockaddr_in serv_addr, cli_addr;
	     int n;
	     FILE *file;
	     //if (argc < 2) {
	     //    fprintf(stderr,"ERROR, no port provided\n");
	     //    exit(1);
	     //}
	     file = fopen(filename,"rb");//rb是開啟檔案給程式讀取wb下面有註解 一起使用的
	     sockfd = socket(AF_INET, SOCK_STREAM, 0);//建立一個 socket設定他的領域形式跟Protocal 通常是0 讓kernel默認
	     if (sockfd < 0){
	        error("ERROR opening socket");
	     }
	     bzero((char *) &serv_addr, sizeof(serv_addr)); //下面註解過了
	     serv_addr.sin_family = AF_INET;//ip.port
	     serv_addr.sin_addr.s_addr = INADDR_ANY;
	     serv_addr.sin_port = htons(port);//portno->port
	     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
	              error("ERROR on binding");
	     }//connect是去別人家用資料 bind是把地址綁在自己身上的感覺
	     listen(sockfd,5);
	     clilen = sizeof(cli_addr);
	     newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);//看不懂幹麻的可能是有關父子關係的東西總之不能accept就是這裡出問題
	     if (newsockfd < 0){
	          error("ERROR on accept");
	     }
	     bzero(buffer,256);

	     //n = read(newsockfd,buffer,255);
	     //if (n < 0) error("ERROR reading from socket");
	     //printf("Here is the message: %s\n",buffer);
	     //n = write(newsockfd,"I got your message",18);
	     //if (n < 0) error("ERROR writing to socket"
             fseek(file,0,SEEK_END);
             int size = ftell(file);
             fseek(file,0,SEEK_SET);//成功紀錄檔案大小
 	     
	     while(!feof(file)){
	     	percent = fread(buffer,sizeof(char),1,file);
		write(newsockfd,buffer,percent);
                //data = data + percent;
           	//printf("%d\n",data);
                data = data + percent;
	   	if(data*20 >= size*count){
                        printf("%d%% ",count*5);
                        time_t timep;
                        time(&timep);
                        printf("%s",ctime(&timep));
                        //printf("\n");
                        count++;

	     	}
	     }
	     fclose(file);
	     close(newsockfd);
	     close(sockfd);
	}else if(protocol == 1){

	    int sockfd, percent, n, data = 0, count = 1;
	    struct sockaddr_in servaddr;
            char buffer[256];
            struct sockaddr_in peeraddr;
            socklen_t peerlen = sizeof(peeraddr);
	    FILE *file;
	    //struct stat st;//我要用來計算檔案大小用的
	    //stat(filename, &st);
		 
	    file = fopen(filename, "rb"); //用來讀取資料
            if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
                error("socket error");
            }

	    memset(&servaddr, 0, sizeof(servaddr)); //port ip
	    servaddr.sin_family = AF_INET;
	    servaddr.sin_port = htons(port); //port
	    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
	        error("bind error");
	    }
	    //接收client傳來的訊息
	    recvfrom(sockfd, buffer, sizeof(buffer), 0,(struct sockaddr *)&peeraddr, &peerlen);
	   
	    fseek(file,0,SEEK_END);
	    int size = ftell(file);
	    fseek(file,0,SEEK_SET);//成功紀錄檔案大小
	    //printf("%d Bytes\n",size);//發現用這種方法好像會跟Fopen衝突 另闢新徑 
	    //int size;
	    //size = st.st_size;
	    while(!feof(file)){
		percent = fread(buffer, sizeof(char), 1, file);//一次讀1byte    
	    	percent = sendto(sockfd, buffer, percent, 0,(struct sockaddr *)&peeraddr, peerlen);
	    	data = data + percent;
	    	//printf("%d\n",data);
	    	if(data*20 >= size*count){
			printf("%d%% ",count*5);
			time_t timep;
			time(&timep);
			printf("%s",ctime(&timep));
			//printf("\n");
			count++;
		}
	    }
	    fclose(file);
   	    close(sockfd);
	}

}

void client(){
	if(protocol == 0){
	    int sockfd, n, percent;//percent 紀錄傳送百分比
	    struct sockaddr_in serv_addr;//已經被定義好了這樣就可以使用connect()
	    struct hostent *server;
	    char buffer[256];
	    FILE *file;//準備要來覆寫txt檔案 //'wb'
	    char filename[256] = "file_recv";//因為還沒有這檔案 fopen會幫我自動建立這檔案
	    //if (argc < 3) {
	    //   fprintf(stderr,"usage %s hostname port\n", argv[0]);
	    //   exit(0);
	    //}
	    //portno = atoi(argv[2]);
	    file = fopen(filename,"wb");
	    if(file == NULL){
		error("ERROR Fopen!");
	    }
	    sockfd = socket(AF_INET, SOCK_STREAM, 0);//建立一個 socket設定他的領域形式跟Protocal 通常是0 讓kernel默認
	    if (sockfd < 0){
	            error("ERROR opening socket");//代表創socket失敗
	    }
	    server = gethostbyname(ip);//argv[1]->ip Readme 說的
	    if (server == NULL) {
	        //fprintf(stderr,"ERROR, no such host\n");
	        //exit(0);
		    error("no such host!");
	    }
	    bzero((char *) &serv_addr, sizeof(serv_addr));//設定ip.port
	    serv_addr.sin_family = AF_INET;
	    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	    serv_addr.sin_port = htons(port);//portno->port

	    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		    error("ERROR connecting");//connect 讓我們去server那取資料
	    }
	    //printf("Please enter the message: ");
	    bzero(buffer,256);
	    //fgets(buffer,255,stdin);
	    //percent = write(sockfd,buffer,strlen(buffer));
	    //if (n < 0)
	    //     error("ERROR writing to socket");
	    //bzero(buffer,256)
	    while(1){
	    percent = read(sockfd,buffer,sizeof(char));
	    if(percent == 0){
	    	printf("Finished!\n");
			break;
	    }else if(percent < 0){
		error("ERROR transmission!");
	    }
	    percent = fwrite(buffer,sizeof(char),1,file);
	    }
	    //if (n < 0)
	    //     error("ERROR reading from socket");
	    //printf("%s\n",buffer);

	    fclose(file);
	    close(sockfd);
	}else if(protocol == 1){
	    int sockfd,percent;
   	    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
            	error("socket");
	    }
	    struct sockaddr_in servaddr;
	    char buffer[256];
	    char filename[256] = "file_recv";//用來接收檔案用的
	    FILE *file;
	    socklen_t server_len = sizeof(servaddr);

	    file = fopen(filename,"wb");//自動建一個檔
	    if(file == NULL){
		error("ERROR fopen");
	    }

	    memset(&servaddr, 0, sizeof(servaddr));//ip.port
	    servaddr.sin_family = AF_INET;
	    servaddr.sin_port = htons(port);//use port
	    servaddr.sin_addr.s_addr = inet_addr(ip);//use ip
	
	    //int ret;
	    //char sendbuf[1024] = {0};
	    //char recvbuf[1024] = {0};
	    //while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	    //{
	    //int servadde_len = sizeof(servaddr);
	        sendto(sockfd, buffer, 255, 0, (struct sockaddr *)&servaddr, server_len);
	        
		while(1){//跟TCP類似的東西
			percent = recvfrom(sockfd, buffer, 255, 0, (struct sockaddr *)&servaddr, &server_len);
		        if(percent == 0){//UDP 用的就是sendto.recvfrom
                		printf("Finished!\n");
                        	break;
            		}else if(percent < 0){
                		error("ERROR transmission!");
            		}
            		percent = fwrite(buffer,sizeof(char),1,file);
	        }
	        //fputs(recvbuf, stdout);
	        //memset(sendbuf, 0, sizeof(sendbuf));
	        //memset(recvbuf, 0, sizeof(recvbuf));
	    //}
	    fclose(file);
	    close(sockfd);
	}
}
int main(int argc, char *argv[]){

        if(strcmp(argv[1], "tcp") == 0){     //設定我的input
                protocol = 0;
        }else if(strcmp(argv[1], "udp") == 0){
                protocol = 1;
        }else{
                printf("Error protcol input!\n");
        }

        strcpy(ip, argv[3]);
        port = atoi(argv[4]);

        if(strcmp(argv[2], "send") == 0){
                strcpy(filename, argv[5]);
                server();
        }else if(strcmp(argv[2], "recv") == 0){
                client();
        }else{
                printf("Error client / server input!");
        }

        //strcpy(ip, argv[3]);
        //port = atoi(argv[4]);
        //printf("%d\n",port);

        return 0;
}

