#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8001"

struct threadinfo {
    SOCKET ClientSocket;
    int id;
};

DWORD WINAPI handleClient(LPVOID IpParameter) {
    struct threadinfo* temp = (struct threadinfo*)IpParameter;
    int iSendResult;
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    SOCKET ClientSocket = temp->ClientSocket;
    int id = temp->id;

    int random;
    char response[DEFAULT_BUFLEN] = "!quit!"; // Lets client know echo is finished

    printf("Connecting Client %d.\n", id);
    char welcomeMessage[DEFAULT_BUFLEN];
    sprintf_s(welcomeMessage, sizeof(welcomeMessage), "Client %d, Thank you for connecting", id);

    iSendResult = send(ClientSocket, welcomeMessage, strlen(welcomeMessage), 0);
    if (iSendResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    do {
        memset(recvbuf, 0, recvbuflen); //needed to fix buffer content
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {

            printf("Recieved: %s\n", recvbuf);

            if (strcmp(recvbuf, "quit") == 0) {
                printf("Disconnecting Client %d", id);
                break;
            }

            random = 2 + (rand() % 9); // Generates number between 2 and 10 inclusive
            char randomstr[3];
            sprintf_s(randomstr, 3, "%d", random);
            printf("random %s\n", randomstr);
            Sleep(100);
            iResult = send(ClientSocket, randomstr, strlen(randomstr), 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            for (int i = 0; i < random; i++) {
                if (strcmp(recvbuf, "test\n") == 0) {
                    printf("Yes");
                }
                Sleep(100);
                iSendResult = send(ClientSocket, recvbuf, strlen(recvbuf), 0);

                if (iSendResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }
            }
        }
        else if (iResult == 0) {
            printf("Connection closed\n");
        }
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    //WSACleanup();

    return 0;
}

int main(void)
{

    int id = 0;
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];

    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    //ZeroMemory(&hints, sizeof(hints)); //equivalent to
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("server is listening\n");

    while (1)
    {    // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        // No longer need server socket
        //closesocket(ListenSocket);

        HANDLE handle;
        struct threadinfo temp;

        temp.ClientSocket = ClientSocket;
        temp.id = id;


        handle = CreateThread(NULL, 0, handleClient, &temp, 0, NULL);
        if (handle == NULL) {
            printf("Thread was unable to be made\n");
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        id += 1;
    }
}
