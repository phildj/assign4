#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>
#include <sqlite3.h> 
#include <stdlib.h>
#include <string.h>


/*
    CONCURRENT SERVER: THREAD EXAMPLE
    Must be linked with the "pthread" library also, e.g.:
       cc -o example example.c -lnsl -lsocket -lpthread 

    This program creates a connection socket, binds a name to it, then
    listens for connections to the sockect.  When a connection is made,
    it accepts messages from the socket until eof, and then waits for
    another connection...

    This is an example of a CONCURRENT server -- by creating threads several
    clients can be served at the same time...

    This program has to be killed to terminate, or alternately it will abort in
    120 seconds on an alarm...
*/


struct serverParm {
           int connectionDesc;
       };
       
void *serverThread(void *parmPtr) {

#define PARMPTR ((struct serverParm *) parmPtr)
    int recievedMsgLen;
    char messageBuf[1025];
   	char *zErrMsg = 0;
	  int rc;
    sqlite3 *db;
    sqlite3_stmt *statement;   
    FILE *f;
    time_t t;
    time(&t);
	  /* Open database */
  	rc = sqlite3_open("book.db", &db);
  	if( rc )
  	{
	  	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	  	exit(0);
  	}
	  else
	  {
  		fprintf(stderr, "Opened database successfully\n");
	  }

    /* Server thread code to deal with message processing */
    printf("DEBUG: connection made, connectionDesc=%d\n",
            PARMPTR->connectionDesc);
    if (PARMPTR->connectionDesc < 0) {
        printf("Accept failed\n");
        return(0);    /* Exit thread */
    }
    
    /* Receive messages from sender... */
    while ((recievedMsgLen= read(PARMPTR->connectionDesc,messageBuf,sizeof(messageBuf)-1)) > 0) 
    {
        //recievedMsgLen[messageBuf] = '\0';
        messageBuf[recievedMsgLen] = '\0';
        char result[1025] = "";
    if ( sqlite3_prepare(db, messageBuf, -1, &statement, 0 ) == SQLITE_OK ) 
	  {
		  int ctotal = sqlite3_column_count(statement);
		  int res = 0;
      f = fopen("a4p2ServerLog.txt", "a");
      printf("Process ID : %d, Thread ID: %d\n", getpid(), pthread_self());
      printf("Processing: %s\n", messageBuf);   
      fprintf(f, "Process ID : %d, Thread ID: %d\n", getpid(), pthread_self());
      fprintf(f, "Processing: %s\n", messageBuf);
      fclose(f);
		  while ( 1 )         
		  {      
			  res = sqlite3_step(statement);
			  if ( res == SQLITE_ROW ) 
			  {
           int i ;
  	      for(i = 0; i < ctotal; i++)
          {
              if(i == ctotal - 1)
              {
                strcat(result, (char*)sqlite3_column_text(statement, i));
                strcat(result, "\n");
              }
              else
              {
                strcat(result, (char*)sqlite3_column_text(statement, i)); 
                strcat(result, " | ");
              }
          }
             
			  }										
			  if ( res == SQLITE_DONE || res==SQLITE_ERROR)                                                                              
			  {
			  	break;                                                                                                                     
			  }   
	  	}																	
	  }
        if (write(PARMPTR->connectionDesc,result,1025) < 0) {
               perror("Server: write error");
               return(0);
           }
    }
    sqlite3_close(db);
    close(PARMPTR->connectionDesc);  /* Avoid descriptor leaks */
    free(PARMPTR);                   /* And memory leaks */
    return(0);                       /* Exit thread */
}

int main (int argc, char **argv) {
    int listenDesc;
    struct sockaddr_in myAddr;
    struct serverParm *parmPtr;
    int connectionDesc;
    pthread_t threadID;
    FILE *f;
    f = fopen("a4p2ServerLog.txt", "w");
    fclose(f);
    /* For testing purposes, make sure process will terminate eventually */
    alarm(120);  /* Terminate in 120 seconds */

    /* Create socket from which to read */
    if ((listenDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("open error on socket");
        exit(1);
    }

    /* Create "name" of socket */
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_port = htons((int) strtol(argv[1], (char **)NULL, 10));
    //myAddr.sin_port = htons(PORTNUMBER);
        
    if (bind(listenDesc, (struct sockaddr *) &myAddr, sizeof(myAddr)) < 0) {
        perror("bind error");
        exit(1);
    }

    /* Start accepting connections.... */
    /* Up to 5 requests for connections can be queued... */
    listen(listenDesc,5);
    while (1) /* Do forever */ {
        /* Wait for a client connection */
        connectionDesc = accept(listenDesc, NULL, NULL);

        /* Create a thread to actually handle this client */
        parmPtr = (struct serverParm *)malloc(sizeof(struct serverParm));
        parmPtr->connectionDesc = connectionDesc;
        if (pthread_create(&threadID, NULL, serverThread, (void *)parmPtr) 
              != 0) {
            perror("Thread create error");
            close(connectionDesc);
            close(listenDesc);
            exit(1);
        }
        printf("Parent ready for another connection\n"); 
    }

}
