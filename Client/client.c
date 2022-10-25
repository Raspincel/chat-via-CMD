#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
//-lws2_32 -DWINVER=0x501 at the end of the line when compilling with gcc

int initializeWS2(WSADATA* wsaData);
int initializeHints(struct addrinfo* hints);
int getInfos(const char* argv, struct addrinfo* hints, struct addrinfo* result);
void setConnect(SOCKET* ConnectSocket, struct addrinfo* result);
int sendAndReceiveData(SOCKET* ConnectSocket);

#define DEFAULT_PORT "27015"

int main(int argc, char* argv[]) 
{
    WSADATA wsaData;
    if (initializeWS2(&wsaData))
        return 1;

    struct addrinfo* result = NULL;
    // struct addrinfo* result = NULL;
    struct addrinfo hints;
    
    initializeHints(&hints);

    int temp = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (temp != 0)
    {
        printf("getaddrinfo failed: %d\n", temp);
        WSACleanup();
        return 1;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;
    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    
    setConnect(&ConnectSocket, result);

    // information no longer needed
    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    if(sendAndReceiveData(&ConnectSocket) != 0)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();    

    return 0;
}

int initializeWS2(WSADATA* wsaData)
{
    // if successful, returns 0
    if (WSAStartup(MAKEWORD(2, 2), wsaData))
        return 1;

    return 0;
}

int initializeHints(struct addrinfo* hints)
{
    ZeroMemory(hints, sizeof(*hints));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_protocol = IPPROTO_TCP;
    hints->ai_flags = 0;
}

void setConnect(SOCKET* ConnectSocket, struct addrinfo* result)
{
    if(connect(*ConnectSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
    {
        closesocket(*ConnectSocket);
        *ConnectSocket = INVALID_SOCKET;
    }
}

int sendAndReceiveData(SOCKET* ConnectSocket)
{
    #define DEFAULT_BUFLEN 512

    int recvbuflen = DEFAULT_BUFLEN;

    char recvbuf[DEFAULT_BUFLEN];

    int iSendResult = 1;
    int iReceiveResult = 1;

    // Receive and send data
    do
    {
        // Receiving
        
        if (iReceiveResult > 0 && iSendResult > 0)
        {
            memset(recvbuf, 0, sizeof(recvbuf));
            iReceiveResult = recv(*ConnectSocket, recvbuf, recvbuflen, 0);
            printf("User 1: %sMessage's size: %d\n\n", recvbuf, iReceiveResult);
        }

        recvbuf[iReceiveResult - 1] = '\0';
        if (!strcmp(recvbuf, "close connection"))
            iReceiveResult = 0;

        // Sending
        if (iSendResult > 0 && iReceiveResult > 0)
        {

            // send new message
            memset(recvbuf, 0, sizeof(recvbuf));
            printf("Send message: ");
            fgets(recvbuf, recvbuflen, stdin);

            iSendResult = send(*ConnectSocket, recvbuf, (int) strlen(recvbuf), 0);
            printf("Message's size: %d\n\n", iSendResult);

            if (iSendResult == SOCKET_ERROR)
            {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(*ConnectSocket);
                WSACleanup();
                return 1;
            }

            recvbuf[iSendResult - 1] = '\0';

            if (!strcmp(recvbuf, "close connection"))
                iSendResult = 0;
            

        }
        else if (iSendResult == 0 || iReceiveResult == 0)
        {
            printf("Connection closing...\n");
            return shutdown(*ConnectSocket, SD_BOTH);
        }
        else
        {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(*ConnectSocket);
            WSACleanup();
            return 1;
        }

    } while (iSendResult > 0 || iReceiveResult > 0);

    return 0;
}