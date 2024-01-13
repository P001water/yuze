#include "socksServer.h"

int create_socks_server(int listenPort) {
    SOCKET serv_sock;

    serv_sock = socket_initServer(listenPort, 500);
    if (serv_sock == INVALID_SOCKET)
    {
        printf("[-] Error: --> Unable to start server on port %d.\n", listenPort);
        exit(1);
    }
    printf("[+] Create_socks_server 0.0.0.0:%d <--[yuze]--> socks server\n", listenPort);

    while (True) {
        struct sockaddr_in sa;
        int slen = sizeof(sa);
        SOCKET s = accept(serv_sock, (struct sockaddr*)&sa, &slen);
        if (s == -1) {
            if (errno == EINTR)
                continue; /* Try again. */
            else
                break;  
        }
        SOCKET param = s;
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)extract_and_tunnel, &param, 0, 0);
        s = INVALID_SOCKET;
        Sleep(1);
    }
}