#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define MAXLINE 4096 /*max text line length*/
//#define SERV_PORT 10010 /*port*/

int
main(int argc, char **argv) 
{
 int sockfd;
 struct sockaddr_in servaddr;
 char sendline[MAXLINE] = "";
 char recvline[MAXLINE] = "";
  FILE *f;
  f = fopen("a4Client3Log.txt", "w");
 // alarm(300);  // to terminate after 300 seconds
	
 //basic check of the arguments
 //additional checks can be inserted
 if (argc !=3) {
  perror("Usage: TCPClient <Server IP> <Server Port>"); 
  exit(1);
 }
	
 //Create a socket for the client
 //If sockfd<0 there was an error in the creation of the socket
 if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
  perror("Problem in creating the socket");
  exit(2);
 }
	
 //Creation of the socket
 memset(&servaddr, 0, sizeof(servaddr));
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr= inet_addr(argv[1]);
 servaddr.sin_port =  htons((int) strtol(argv[2], (char **)NULL, 10)); 
 //servaddr.sin_port =  htons(SERV_PORT); //convert to big-endian order
	
 //Connection of the client to the socket 
 if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
  perror("Problem in connecting to the server");
  exit(3);
 }
 f = fopen("a4Client3Log.txt", "a");
 while (1) {
 strcat(sendline, "delete from classics where author = 'Charles Dickens'");
  send(sockfd, sendline, strlen(sendline), 0);		
  if (recv(sockfd, recvline, MAXLINE,0) == 0){
   //error: server terminated prematurely
   perror("The server terminated prematurely"); 
   exit(4);
  }
  strcpy(sendline,"");
  strcat(sendline, "select * from classics");
  send(sockfd, sendline, strlen(sendline), 0);	
  if (recv(sockfd, recvline, MAXLINE,0) == 0){
   //error: server terminated prematurely
   perror("The server terminated prematurely"); 
   exit(4);
  }
  fprintf(f, "%s\n", recvline);
  fputs(recvline, stdout);
  sleep(2);
  printf("\n");
  strcpy(sendline,"");
  strcat(sendline, "INSERT INTO classics (`author`, `title`, `category`, `year`, `isbn`) VALUES ('Charles Dickens', 'The Old Curiosity Shop', 'Fiction', '1841', '9780099533474')");
  send(sockfd, sendline, strlen(sendline), 0);
  if (recv(sockfd, recvline, MAXLINE,0) == 0){
   //error: server terminated prematurely
   perror("The server terminated prematurely"); 
   exit(4);
  }
  strcpy(sendline,"");
  strcat(sendline, "select * from classics");
  send(sockfd, sendline, strlen(sendline), 0);		
  if (recv(sockfd, recvline, MAXLINE,0) == 0){
   //error: server terminated prematurely
   perror("The server terminated prematurely"); 
   exit(4);
  }
  fprintf(f, "%s\n", recvline);
  fputs(recvline, stdout);
  printf("\n");
  sleep(2);
  strcpy(sendline,"");
  fclose(f);
  f = fopen("a4Client3Log.txt", "a");
 }
 fclose(f);
 exit(0);
}
