#include <stdio.h>
#include <string.h>
#include <time.h>

int setupLogger();
int setupPacketListener();
int setupWebListener();

int teardownLogger();
int teardownPacketListener();
int teardownWebListener();

void logError();

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
        logError("encountered error while setting up packert listener: $d\n", errno);
    }

    // setup the listener for HTTP requests to check status
    setupWebListener();

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

    g_logFile = fopen("logfile.txt", "w+");
    if(g_logFile == NULL)
    {
        return -1;
    }

   time_t curtime;

   time(&curtime);

    char buffer[512];
    sprintf( buffer, "%s ---logger initialized ---\n", ctime(&curtime));

    fwrite(buffer, strlen(buffer), 1, g_logFile);

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
    }
}

/* packet related functions */
int setupPacketListener()
{
    return 0;
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
