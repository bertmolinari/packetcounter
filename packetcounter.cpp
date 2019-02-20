#include <pthread.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curl/curl.h>

#include <was/storage_account.h>
#include <was/table.h>

int setupLogger();
int setupPacketListener();
int setupWebListener();
int setupCurl();

int teardownLogger();
int teardownPacketListener();
int teardownWebListener();
int teardownCurl();

int writeCounterDataToStorage();

void* packetListener(void* param);
void logError(const char* message, int error);

pthread_t g_packetListenerThread;
pthread_t g_webListenerThread;
CURL* g_curlObjectPtr = NULL;
int g_connectionCounter = 0;

// Define the Storage connection string with your values.
utility::string_t storage_connection_string(U("DefaultEndpointsProtocol=https;AccountName=connectioninfo;AccountKey="));


FILE* g_logFile = NULL;
std::string g_clientConnectionInfoStorageAccountName = "connectioninfo.table.core.windows.net";
std::string g_clientConnectionInfoTableName = "clientConnectionInfo";
std::string g_entityFormatString = "{ \"PartitionKey\":%s, \"RowKey\":%s, \"Count\":%d }";
std::string g_clientConnectionPartitionKey = "packetcounter";

int main(int argc, char** argv)
{
   int result = 0;

    if(argc != 2)
    {
        ::printf("Usage: packetcounter <azure-storage-account-key>\n");
        return 0;
    }

    storage_connection_string += argv[1];

    std::string s = "foo";
    result = setupLogger();
    if(result != 0)
    {
        printf("encountered error while setting up logger: %d\n", result);
    }

    // setup the listener for packets
    result = setupPacketListener();
    if(result != 0)
    {
        logError("encountered error while setting up packert listener: %d\n", result);
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

    g_logFile = fopen("logfile.txt", "w+");
    if(g_logFile == NULL)
    {
        return -1;
    }

    // we want each file in the log to have a date/time stamp so create that starting with getting the current time
    time_t curtime;
    time(&curtime);

    // put the string version of this into a buffer, and remove the line feed that ctime() adds so we have a nicely formatted log line in the file
    char timedateStamp[128];
    sprintf( timedateStamp, "%s", ctime(&curtime));
    int timedateStampLength = strlen(timedateStamp);
    timedateStamp[ timedateStampLength -1 ] = '\0';

    // compose the line that indicicates log file initialization
    char buffer[512];
    sprintf( buffer, "%s ---logger initialized ---\n", timedateStamp);

    // write the initialized line to disk
    int bytesWritten = fwrite(buffer, 1, strlen(buffer),  g_logFile);
    if(bytesWritten != strlen(buffer))
    {
        printf("Error while writing to log file: %d, bytesWritten: %d\n", ferror( g_logFile ), bytesWritten);
    }
    fflush( g_logFile );

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

void logError(const char* message, int error)
{
    if( g_logFile != NULL )
    {
        time_t curtime;
        time(&curtime);
        char timedateStamp[128];
        sprintf( timedateStamp, "%s", ctime(&curtime));
        int timedateStampLength = strlen(timedateStamp);
        timedateStamp[ timedateStampLength -1 ] = '\0';

        char buffer[512];
        sprintf(buffer, "%s %s %d\n", timedateStamp, message, error);
        fwrite(buffer, strlen(buffer), sizeof(char), g_logFile);
        fflush(g_logFile);
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
    char buffer[2048];
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    int portNumber = 7891;

    /*---- Create the socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
    welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

    /* get the IP addresses associatedw ith this machine
    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    serverAddr.sin_family = AF_INET;
    /* Set port number, using htons function to use proper byte order */
    serverAddr.sin_port = htons(portNumber);
    /* Set IP address to localhost */
    // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    /* Set all bits of the padding field to 0 */
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*---- Bind the address struct to the socket ----*/
    bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*---- Listen on the socket, with 5 max connection requests queued ----*/
    int result;
    result = listen( welcomeSocket, 5);
    if(listen(welcomeSocket,5)==0)
    {
        logError("Listening with no error", 0);
        printf("Listening on port: %d\n", portNumber);
    }
    else
    {
        logError("Listen failed with error", result);
    }

    /* get the name of the machine we're running on in order to output it back to clients */
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);

    while(1)
    {
        /*---- Accept call creates a new socket for the incoming connection ----*/
        addr_size = sizeof serverStorage;
        newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

        /*---- Send message to the socket of the incoming connection ----*/
        sprintf( buffer, "Hello from packet counter, running on %s!\n", hostname);
        send(newSocket,buffer,strlen(buffer),0);

        g_connectionCounter++;

        writeCounterDataToStorage();

        logError("Connection count: ", g_connectionCounter);

        sleep(1);
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


int setupCurl()
{

  CURLcode res;

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  g_curlObjectPtr = curl_easy_init();
  if(g_curlObjectPtr) {
    // /* First set the URL that is about to receive our POST. This URL can
    //    just as well be a https:// URL if that is what should receive the
    //    data. */
    // curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");
    // /* Now specify the POST data */
    // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

    // /* Perform the request, res will get the return code */
    // res = curl_easy_perform(curl);
    // /* Check for errors */
    // if(res != CURLE_OK)
    //   fprintf(stderr, "curl_easy_perform() failed: %s\n",
    //           curl_easy_strerror(res));

  }
  return 0;
}

int writeCounterDataToStorage()
{
    int res = 0;

    try
    {
        // Retrieve the storage account from the connection string.
        azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);

        // Create the table client.
        azure::storage::cloud_table_client table_client = storage_account.create_cloud_table_client();

        // Retrieve a reference to a table.
        azure::storage::cloud_table table = table_client.get_table_reference(U("clientConnectionInfo"));

        // Create the table if it doesn't exist.
        table.create_if_not_exists();

        // std::string hostname;
        char hostname[1024];
        hostname[1023] = '\0';
        gethostname(hostname, 1023);

        time_t curtime;
        time(&curtime);
        char timedateStamp[128];
        sprintf( timedateStamp, "%s", ctime(&curtime));
        int timedateStampLength = strlen(timedateStamp);
        timedateStamp[ timedateStampLength -1 ] = '\0';

        // Create a new  entity using the hostname of the machine we're running on as the partitionkey and the date/time as the rowkey
        azure::storage::table_entity entry(hostname, timedateStamp);

        azure::storage::table_entity::properties_type& properties = entry.properties();
        properties.reserve(1);
        properties[U("Count")] = azure::storage::entity_property(g_connectionCounter);

        // Create the table operation that inserts the customer entity.
        azure::storage::table_operation insert_operation = azure::storage::table_operation::insert_entity(entry);

        // Execute the insert operation.
        azure::storage::table_result insert_result = table.execute(insert_operation);
    }
    catch( azure::storage::storage_exception e)
    {
        logError(e.what(), 0);
    }

    return res;

    // if( g_curlObjectPtr )
    // {
    //     // std::string hostname;
    //     char hostname[1024];
    //     hostname[1023] = '\0';
    //     gethostname(hostname, 1023);

    //     char url[2048];
    //     sprintf( url, "https://%s/%s(PartitionKey=\'%s\',RowKey=\'%s\'",    g_clientConnectionInfoStorageAccountName.c_str(), 
    //                                                                         g_clientConnectionInfoTableName.c_str(),
    //                                                                         g_clientConnectionPartitionKey.c_str(),
    //                                                                         hostname);
    //     char buffer [ 2048 ];
    //     sprintf( buffer, g_entityFormatString.c_str(), g_clientConnectionPartitionKey.c_str(), hostname, g_connectionCounter);

    //     curl_easy_setopt(g_curlObjectPtr, CURLOPT_URL, url);
    //     curl_easy_setopt(g_curlObjectPtr, CURLOPT_PUT, 1L);
    //     curl_easy_setopt(g_curlObjectPtr, CURLOPT_READDATA, buffer);

    //     struct curl_slist *list = NULL;
        
    //     /* the following HTTP headers are needed for authorization with the Azure Storage service (see https://docs.microsoft.com/en-us/rest/api/storageservices/authorize-with-shared-key for reference) */
    //     /*   - x-ms-date
    //          - Authorization
    //     */

    //     /* create and add the x-ms-date header */
    //     char dateHeader[1024];

    //     time_t curtime;
    //     time(&curtime);
    //     char timedateStamp[128];
    //     sprintf( timedateStamp, "%s", ctime(&curtime));
    //     int timedateStampLength = strlen(timedateStamp);
    //     timedateStamp[ timedateStampLength -1 ] = '\0';

    //     sprintf(dateHeader, "x-ms-date: %s", timedateStamp);
    //     list = curl_slist_append(list, dateHeader);


    //     // /* Now specify the POST data */
    //     // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

    //     /* Perform the request, res will get the return code */
    //     res = curl_easy_perform(g_curlObjectPtr);
    //     /* Check for errors */
    //     if(res != CURLE_OK)
    //     {
    //         logError("curl_easy_perform() failed:", res);
    //     }
    // }

    // return res;
}

int teardownCurl()
{
    if( g_curlObjectPtr != NULL)
    {
        /* always cleanup */
        curl_easy_cleanup(g_curlObjectPtr);

        curl_global_cleanup();
    }
}