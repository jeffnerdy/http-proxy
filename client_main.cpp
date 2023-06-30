#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    // variable declaration
    WSADATA wsaData;

    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serv_addr;

    const char* http_request = "34.235.32.249@80@GET /ip HTTP/1.1\r\nHost: www.httpbin.org\r\n\r\n";
    const char* server_ip = argv[1];
    char buffer[1024] = { 0 };
    u_short server_port = atoi(argv[2]);
    int iResult;

    // set up
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    // connect to proxy
    iResult = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "connect() failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "update: connected to proxy" << std::endl;

    // send packet
    iResult = send(sock, http_request, strlen(http_request), 0);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "update: HTTP request sent" << std::endl;

    // receive packet
    iResult = recv(sock, buffer, 1024, 0);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "recv() failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "update: HTTP response received" << std::endl;

    // print received packet
    std::cout << buffer << std::endl;

    closesocket(sock);
    WSACleanup();

    std::cin.get();

    return 0;
}

