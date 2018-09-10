#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

int setupLogger();
int setupPacketListener();
int setupWebListener();

int teardownLogger();
int teardownPacketListener();
int teardownWebListener();

void* packetListener(void* param);

void logError();

pthread_t g_packetListenerThread;
pthread_t g_webListenerThread;

FILE* g_logFile = NULL;



int main(int argc, char** argv)
{
   int errno = 0;

    errno = setupLogger();
    if(errno != 0)
    {
        printf("encountered error while setting up logger: %d\n", errno);
    }

    // setup the listener for packets
    errno = setupPacketListener();
    if(errno != 0)
    {
        logError("encountered error while setting up packert listener: %d\n", errno);
    }

    // setup the listener for HTTP requests to check status
    setupWebListener();

    while(1)
    {
        sleep(1);
    }

    teardownWebListener();
    teardownPacketListener();
    teardownLogger();

    return 0; 
}

/* log related functions */
int setupLogger()
{
    if(g_logFile != NULL )
    {
        return -2;
    }

    g_logFile = fopen("logfile2.txt", "w+");
    if(g_logFile == NULL)
    {
        return -1;
    }

   time_t curtime;

   time(&curtime);

    char buffer[512];
    sprintf( buffer, "%s ---logger initialized ---\n", ctime(&curtime));

    printf("%s", buffer);

    int bytesWritten = fwrite(buffer, strlen(buffer), 1, g_logFile);
    if(bytesWritten != strlen(buffer))
    {
        printf("Error while writing to log file: %d, bytesWritten: %d\n", ferror( g_logFile ), bytesWritten);
    }

    printf("log file should be created\n");
    return 0;
}

int teardownLogger()
{
    if(g_logFile != NULL)
    {
        fclose(g_logFile);
    }

    return 0;
}

void logError(char* message, int error)
{
    if( g_logFile != NULL )
    {
        time_t curtime;
       time(&curtime);

        char buffer[512];
        sprintf( buffer, "%s", ctime(&curtime));
        fwrite(buffer, strlen(buffer), sizeof(char), g_logFile);

        sprintf(buffer, message, error);
        fwrite(buffer, strlen(buffer), sizeof(char), g_logFile);

        printf(message, error);
    }
}

/* packet related functions */
int setupPacketListener()
{
    int result = 0;
    pthread_attr_t threadAttributes;

    result = pthread_create( &g_packetListenerThread, NULL, packetListener, NULL );
 
    return result;
}

void* packetListener(void* param)
{
    int welcomeSocket, newSocket;
    char buffer[1024];
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(7891);
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*---- Bind the address struct to the socket ----*/
  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  /*---- Listen on the socket, with 5 max connection requests queued ----*/
  int result;
  result = listen( welcomeSocket, 5);
  if(listen(welcomeSocket,5)==0)
    logError("Listening with no error: %d\n", 0);
  else
    logError("Listen failed with error: %d\n", result);

  /*---- Accept call creates a new socket for the incoming connection ----*/
  addr_size = sizeof serverStorage;
  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

  /*---- Send message to the socket of the incoming connection ----*/
  strcpy(buffer,"Hello World\n");
  send(newSocket,buffer,13,0);

    while(1)
    {
//        logError("from thread -- there was no error %d\n", 0);
    }

    return NULL;
}
int teardownPacketListener()
{
    return 0;
}

/* web related functions */
int setupWebListener()
{
    return 0;
}

int teardownWebListener()
{
    return 0;
}
