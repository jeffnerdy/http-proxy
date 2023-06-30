#include "functions.h"

int main() {
    // variable declaration
    WSADATA wsaData;

    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;
    SOCKET serverSocket = INVALID_SOCKET;
    struct sockaddr_in prox_addr, client_addr, serv_addr;

    char buffer[1024] = { 0 };
    char client_ip[INET_ADDRSTRLEN];
    std::string http_response;
    std::string client_packet;
    std::string server_ip;
    u_short server_port;
    int len;
    int iResult;

    // set up
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    prox_addr.sin_family = AF_INET;
    prox_addr.sin_addr.s_addr = INADDR_ANY;
    prox_addr.sin_port = htons(12345);

    iResult = bind(listenSocket, (struct sockaddr*)&prox_addr, sizeof(prox_addr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // listening
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "listen() failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    while (true) {
        std::cout << "waiting for incoming connections..." << std::endl;

        // accept connection from client
        len = sizeof(client_addr);
        clientSocket = accept(listenSocket, (struct sockaddr*)&client_addr, &len);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "accept() failed: " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        // print client info
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "update: incoming connection accepted from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        // receive packet
        iResult = recv(clientSocket, buffer, 1024, 0);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "recv() failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "update: HTTP request received" << std::endl;

        // parse server info from packet
        client_packet = buffer;
        parsePacket(client_packet, server_ip, server_port);

        http_response = client_packet;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(server_port);
        inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr);
        
        // set up server socket
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }

        // connect to server
        iResult = connect(serverSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (iResult == SOCKET_ERROR) {
            std::cerr << "connect() failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "update: connected to server" << std::endl;

        // send packet to server
        iResult = send(serverSocket, http_response.c_str(), strlen(http_response.c_str()), 0);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "update: HTTP request sent" << std::endl;

        // receive packet from server
        memset(buffer, 0, 1024);
        iResult = recv(serverSocket, buffer, 1024, 0);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "update: HTTP response received" << std::endl;

        // send packet back to client
        http_response = buffer;
        iResult = send(clientSocket, http_response.c_str(), strlen(http_response.c_str()), 0);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "update: HTTP response sent to client" << std::endl;

        memset(buffer, 0, 1024);

        closesocket(serverSocket);
        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}