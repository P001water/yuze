#include "socksServer.h"

// Function that support the forward agent
int create_socks_server(int listenPort) {
    SOCKET serv_sock;

    serv_sock = socket_initListenServer(listenPort, SOCKET_LISTEN_BACKLOG);
    if (serv_sock == SOCKET_ERROR)
    {
        printf("[-] Error --> Unable to start server on port %d.\n", listenPort);
        exit(1);
    }
    printf("[+] Create_socketListenServer Success 0.0.0.0:%d <--[yuze]--> socksV5 server\n", listenPort);

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
        //Sleep(1);
    }
}

// Functions that support reverse proxy
int create_rsocks_client(char* refHost, int refPort) {
    rsocksStructalias rserverConfig;
    printf("[+] r_server %s:%d <--[yuze]--> rsocks server\n", refHost, refPort);

    SOCKET control_socket = rsocksClient_init_ControlSocket(refHost, refPort);
    if (control_socket == -1) {
        printf("[-] Can not get the Control_socket Connect\n");
    }

    for (;;) {
        fd_set readset;
        struct timeval tv = { 60,0 };  // set the long wait time for reverse proxy 

        char newSocketNotice[RSOCKET_SERVER_NOTICE_LEN];

        FD_ZERO(&readset);
        FD_SET(control_socket, &readset);

        int maxfd = control_socket + 1;
        int iResult = select(maxfd, &readset, NULL, NULL, &tv);
        if (iResult == -1) {
            puts("[-] create_rsocks_server func select() error\n");
            break;
        }
        else if (iResult == 0)
        {
            puts("[-] create_rsocks_server wait time-out\n");
        }
        else {
            if (FD_ISSET(control_socket, &readset)) {
                int readSize = socket_recv(control_socket, newSocketNotice, RSOCKET_SERVER_NOTICE_LEN);
                if (readSize == RSOCKET_SERVER_NOTICE_LEN && newSocketNotice[0] == True && newSocketNotice[1] == NEW_PROXY_SOCKET) {
                    strncpy(rserverConfig.Host, refHost, 300);
                    rserverConfig.port = refPort;
                    rserverConfig.tunnel_id = newSocketNotice[2] - '0' + 48; // convert int from char[]
                    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)rsocksClient_build_tunnel, &rserverConfig, 0, 0);
                }
                else
                {
                    printf("[-] Error Happended on %d\n", readSize);
                    create_rsocks_client(refHost, refPort);
                }
            }
        }
    }
}

int rsocksClient_init_ControlSocket(char* refHost, int refPort) {
    char NewSocketNoticebuff[RSOCKET_SERVER_NOTICE_LEN];

    SOCKET controlSocket = socket_connect(refHost, refPort);
    if (controlSocket == SOCKET_CONNECT_ERROR) {
        printf("[-] Reverse Proxy Error on connect %s:%d [can not connect]\n", refHost, refPort);
        return SOCKET_CONNECT_ERROR;
    }

    FillinSocketbuff(NewSocketNoticebuff);
    NewSocketNoticebuff[1] = CONTROL_SOCKET;
    int sendlen = socket_send(controlSocket, NewSocketNoticebuff, RSOCKET_SERVER_NOTICE_LEN);
    if (sendlen != RSOCKET_SERVER_NOTICE_LEN) {
        puts("[-] Error on Send");
        return -1;
    }
    int recvlen = socket_recv(controlSocket, NewSocketNoticebuff, RSOCKET_SERVER_NOTICE_LEN);
    if (recvlen != RSOCKET_SERVER_NOTICE_LEN) {
        puts("[-] Error on Send");
        return -1;
    }
    if (NewSocketNoticebuff[1] != CONFIRM_CONTROL_SOCKET) {
        puts("[-] Error happened on CONFIRM_CONTROL_SOCKET");
        return -1;
    }

    return controlSocket;
}

int rsocksClient_build_tunnel(rsocksStructalias* rserverConfig) {
    char rhost[300];
    char char_tunnel_id;
    char NewSocketNoticebuff[RSOCKET_SERVER_NOTICE_LEN];

    strncpy(rhost, rserverConfig->Host, 300);
    int rport = rserverConfig->port;
    char tunnel_id = rserverConfig->tunnel_id;

    SOCKET proxySocket = socket_connect(rhost, rport);

    FillinSocketbuff(NewSocketNoticebuff);
    NewSocketNoticebuff[0] = True;
    NewSocketNoticebuff[1] = NEW_PROXY_SOCKET;
    NewSocketNoticebuff[2] = tunnel_id;
    int sendlen = socket_send(proxySocket, NewSocketNoticebuff, RSOCKET_SERVER_NOTICE_LEN);
    if (sendlen != RSOCKET_SERVER_NOTICE_LEN) {
        puts("[-] Error on Send");
        return -1;
    }

    if (check_proto_version(proxySocket) == True) { // Check the sockets proto version
        Sleep(1);
        SOCKET dest_sock = extractRequestHeader(proxySocket); // from socksv5 proto parse the destination url
        if (dest_sock == -1) {
            closesocket(proxySocket);
            return -1;
        }
        else
        {
            tunnel_sock_to_sock(proxySocket, dest_sock);
        }
    }
}