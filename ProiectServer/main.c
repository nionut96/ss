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
    struct addrinfo hints = {0};

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    rescode = getaddrinfo(NULL, "27001", &hints, &res);
    if (rescode != 0) {
        printf("ERR: Failed to connect\n");
        WSACleanup();
        return 1;
    }

    SOCKET sockListen = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockListen == INVALID_SOCKET) {
        printf("ERR: Failed to init socket\n");
        WSACleanup();
        return 1;
    }

    rescode = bind(sockListen, res->ai_addr, (int) res->ai_addrlen);
    if (rescode != 0) {
        printf("ERR: Failed to bind socket\n");
        return 1;
    }

    if (listen(sockListen, SOMAXCONN) == SOCKET_ERROR) {
        printf("ERR: At listening\n");
        closesocket(sockListen);
        return 1;
    }

    SOCKET sockClient = accept(sockListen, NULL, NULL);
    if (sockClient == INVALID_SOCKET) {
        printf("ERR: At accepting a connection\n");
        closesocket(sockListen);
        return 1;
    }
    printf("Got connection\n");

    char pathDest[1000] = {0};
    rescode = recv(sockClient, pathDest, sizeof(pathDest), 0);
    if (rescode <= 0) {
        printf("ERR: At receiving data\n");
        return 1;
    }

    for (size_t i = 0; i < strlen(pathDest); i++) {
        if (i > 0 && pathDest[i - 1] == '.' && pathDest[i] == '.') {
            pathDest[i] = '/';
        }
        if (pathDest[i] == ':') {
            pathDest[i] = '/';
        }
    }

    char aux[1050];

    strcpy(aux, "saved_files/");
    strcat(aux, pathDest);
    strcpy(pathDest, aux);

    FILE *file = fopen(pathDest, "wb");
    if (file == NULL) {
        printf("ERR: At opening destination file (%s)\n", pathDest);
        char result[2] = "0";
        rescode = send(sockClient, result, 1, 0);
        if (rescode == SOCKET_ERROR) {
            printf("ERR: At sending the result\n");
            return 1;
        }
        return 1;
    } else {
        char result[2] = "1";
        rescode = send(sockClient, result, 1, 0);
        if (rescode == SOCKET_ERROR) {
            printf("ERR: At sending the result\n");
            return 1;
        }
    }

    unsigned int fileSize;
    rescode = recv(sockClient, (char *) &fileSize, sizeof(int), 0);
    printf("Received file size: %d\n", fileSize);
    if (rescode <= 0) {
        printf("ERR: At receiving file size\n");
        return 1;
    }

    char buf[4097];

    do {
        rescode = recv(sockClient, buf, 4096, 0);
        if (rescode < 0) {
            printf("ERR: At receiving data\n");
            return 1;
        }
        fileSize -= rescode;
        fwrite(buf, 1, (size_t) rescode, file);
        buf[rescode] = 0;
    } while (fileSize > 0);

    printf("Finished reading\n");

    char result[2] = "1";
    rescode = send(sockClient, result, 1, 0);
    if (rescode == SOCKET_ERROR) {
        printf("ERR: At sending the result\n");
        return 1;
    }

    freeaddrinfo(res);

    return 0;
}