#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

int main() {
    int rescode;

    WSADATA wsa_data;
    rescode = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (rescode != 0) {
        printf("ERR: Failed to initialize winsock\n");
        return 0;
    }

    struct addrinfo *res = NULL;
    struct addrinfo *ptr= NULL;
    struct addrinfo hints = { 0 };

//    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    rescode = getaddrinfo("localhost", "27000", &hints, &res);
    if (rescode != 0) {
        printf("ERR: Failed to connect\n");
        WSACleanup();
        return 1;
    }

    ptr = res;

    SOCKET sockConnect = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (sockConnect == INVALID_SOCKET) {
        printf("ERR: Failed to init socket\n");
        WSACleanup();
        return 1;
    }

    rescode = connect(sockConnect, ptr->ai_addr, (int) ptr->ai_addrlen);
    if (rescode == SOCKET_ERROR) {
        printf("ERR: Failed to connect to socket\n");
        return 1;
    }

    char pathSource[1000] = { 0 };
    char pathDest[1000] = { 0 };

    printf("Please give a source path: ");
    if (fgets(pathSource, 999, stdin) == NULL) {
        printf("ERR: No input\n");
        return 1;
    }
    pathSource[strlen(pathSource) - 1] = 0;

    printf("Please give a destination path: ");
    if (fgets(pathDest, 999, stdin) == NULL) {
        printf("ERR: No input\n");
        return 1;
    }
    pathDest[strlen(pathDest) - 1] = 0;

    char buf[4097] = { 0 };
    FILE* file = fopen(pathSource, "r");
    if (file == NULL) {
        printf("ERR: Invalid file\n");
        return 1;
    }

    rescode = send(sockConnect, pathDest, (int) strlen(pathDest), 0);
    if (rescode == SOCKET_ERROR) {
        printf("ERR: At sending data\n");
        return 1;
    }

    size_t bytesRead = 1;
    while (bytesRead > 0) {
        printf("Reading file %s\n", pathSource);
        bytesRead = fread(buf, 1, LONG_LONG_MAX, file);
        rescode = send(sockConnect, buf, (int) bytesRead, 0);
        if (rescode == SOCKET_ERROR) {
            printf("ERR: At sending data\n");
            return 1;
        }
        buf[rescode] = 0;
    }


    freeaddrinfo(res);

    fclose(file);

    return 0;
}