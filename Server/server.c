#include <stdbool.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
//-lws2_32 -DWINVER=0x501 at the end of the line when compilling with gcc

int initiateWS2(WSADATA* wsaData);
void setHints(struct addrinfo* hints);
int bindSocket(SOCKET* socket, struct addrinfo* result);
bool setListen(SOCKET* socket);
void setAccept(SOCKET* client, SOCKET* listen);
int sendAndReceiveData(SOCKET* ClientSocket);

#define DEFAULT_PORT "27015"

int main() 
{
    WSADATA wsaData;
    if (initiateWS2(&wsaData))
        return 1;

    struct addrinfo* result = NULL;
    struct addrinfo hints;
    
    setHints(&hints);
    
    int temp = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

    if(temp != 0)
    {
        printf("getaddrinfo failed: %d, %d\n", result, temp);
        WSACleanup();
        return 1;
    }
      
    struct hostent* localHost;
    char* localIP;

    localHost = gethostbyname("");
    localIP = inet_ntoa (*(struct in_addr*)*localHost->h_addr_list);
    printf("IP: %s\nName: %s\n\n", localIP, localHost->h_name);


    // listens for client connections
    SOCKET ListenSocket = INVALID_SOCKET;

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    temp = bindSocket(&ListenSocket, result);

    if (temp == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // address information is no longer needed
    freeaddrinfo(result);

    if (setListen(&ListenSocket))
    {
        printf("Listen failed with error %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    
    SOCKET ClientSocket = INVALID_SOCKET;


    setAccept(&ClientSocket, &ListenSocket);

    if (ClientSocket == INVALID_SOCKET)
    {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    if (sendAndReceiveData(&ClientSocket) != 0)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}

int initiateWS2(WSADATA* wsaData)
{
    // if successful, returns 0
    if (WSAStartup(MAKEWORD(2, 2), wsaData))
        return 1;

    return 0;
}

void setHints(struct addrinfo* hints)
{
    ZeroMemory(hints, sizeof (*hints));
    hints->ai_family = AF_INET;
    hints->ai_protocol = IPPROTO_TCP;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;
}

int bindSocket(SOCKET* socket, struct addrinfo* result)
{
    return  bind(*socket, result->ai_addr, (int)result->ai_addrlen);
}

bool setListen(SOCKET* socket)
{

    return listen(*socket, SOMAXCONN) == SOCKET_ERROR;
}

void setAccept(SOCKET* client, SOCKET* listen)
{
    *client = accept(*listen, NULL, NULL);
}

int sendAndReceiveData(SOCKET* ClientSocket)
{
    #define DEFAULT_BUFLEN 512

    char recvbuf[DEFAULT_BUFLEN];
    int iReceiveResult = 1;
    int iSendResult = 1;
    int recvbuflen = DEFAULT_BUFLEN;

    do
    {
        // Sending
        if (iSendResult > 0 && iReceiveResult > 0)
        {

            // send new message
            memset(recvbuf, 0, sizeof(recvbuf));
            printf("Send message: ");
            fgets(recvbuf, recvbuflen, stdin);
            iSendResult = send(*ClientSocket, recvbuf, (int) strlen(recvbuf), 0);
            printf("Message's size: %d\n\n", iSendResult);
            if (iSendResult == SOCKET_ERROR)
            {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(*ClientSocket);
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
            return shutdown(*ClientSocket, SD_BOTH);
        }
        else
        {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(*ClientSocket);
            WSACleanup();
            return 1;
        }

        // Receiving
        if (iReceiveResult > 0 && iSendResult > 0)
        {
            memset(recvbuf, 0, sizeof(recvbuf));
            iReceiveResult = recv(*ClientSocket, recvbuf, recvbuflen, 0);
            printf("User 2: %sMessage's size: %d\n\n", recvbuf, iReceiveResult);
        }

        recvbuf[iReceiveResult - 1] = '\0';
        if (!strcmp(recvbuf, "close connection"))
            iReceiveResult = 0;

    } while (iSendResult > 0 || iReceiveResult > 0);

    return 0;
}