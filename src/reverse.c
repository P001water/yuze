#include "public.h" 

//design for reverse connect service
int control_socket;

//design for reverse proxy server
#define CONTROL_SOCKET 2
#define CONFIRM_CONTROL_SOCKET 1
#define NEW_PROXY_SOCKET 4
#define REVERSEPROXY_NOTICE_LEN 3 

int reverseProxy(int controlPort, int socksPort, const char* connect_server, const char* user, const char* password) {
    if (connect_server) {
        printf("[+] Starting reverse Proxy client...\n");
        return reverseProxyClient(connect_server, user, password);
    }
    else {
        printf("[+] Starting reverse Proxy server...\n");
        return reverseProxyServer(controlPort, socksPort);
    }
}

int reverseProxyServer(int controlPort, int socksPort) {
    printf("[+] socks5 ports:%d, control ports %d\n", socksPort, controlPort);

#ifndef WIN32
    pthread_t socks_thread, control_thread;
#else
    HANDLE socks_thread, control_thread;
#endif

    // Start reverse socks port listener thread
    int result = creatThread_multi_Platform(socks_thread, start_reverse_socksPort, socksPort);
    if (result < 0) {
        fprintf(stderr, "[-] Error: Failed to start reverse socks server on port %d\n", socksPort);
        return -1;
    }

    // Start control socket listener thread
    result = creatThread_multi_Platform(control_thread, start_control_socket, controlPort);
    if (result < 0) {
        fprintf(stderr, "[-] Error: Failed to start control server on port %d\n", controlPort);
        return -1;
    }


    //to prevent the Host process exit
    for (;;) {
        sleep_multi_Platform(10000);
    }
    return 1;
}

// for listen socket connect from client 
void* start_reverse_socksPort(void* arg) {
    int socksPort = *((int*)arg);
    char sendbuf[REVERSEPROXY_NOTICE_LEN], recvbuf[REVERSEPROXY_NOTICE_LEN];
    int sendlen, recvlen;

    int serv_sock = create_tcp_listening_socket(socksPort, 500);
    if (serv_sock < 0)
    {
        printf("[-] Error: --> Unable to start server on port %d.\n", socksPort);
        exit(1);
    }

    while (True) {
        struct sockaddr_in sa;
        socklen_t slen = sizeof(sa);
        int s = accept(serv_sock, (struct sockaddr*)&sa, &slen);
        if (s < 0) {
            if (errno == EINTR)
                continue; /* Try again. */
            else
                break;
        }
        FillinSocketbuff(recvbuf);
        int tunnel_id = tunnel_set_refSocket_and_get_enable_id(s);
        sendbuf[0] = True;
        sendbuf[1] = NEW_PROXY_SOCKET;
        sendbuf[2] = tunnel_id;
        sendlen = socket_send(control_socket, sendbuf, REVERSEPROXY_NOTICE_LEN);
        if (sendlen != REVERSEPROXY_NOTICE_LEN) {
            puts("[-] No Control_socket");
            return NULL;
        }
    }
}

// Control socket listener for reverse connection host
void* start_control_socket(void* arg) {
    int controlPort = *((int*)arg);
    char sendbuf[REVERSEPROXY_NOTICE_LEN], recvbuf[REVERSEPROXY_NOTICE_LEN];
    int sendlen, recvlen;

    int serv_sock = create_tcp_listening_socket(controlPort, SOCKET_LISTEN_BACKLOG);
    if (serv_sock < 0)
    {
        dbg_log("[-] Error: Unable to start control server on port %d. Exiting.", controlPort);
        exit(1);
    }


    while (True) {
        struct sockaddr_in sa;
        socklen_t slen = sizeof(sa);
        int s = accept(serv_sock, (struct sockaddr*)&sa, &slen);
        if (s == -1) {
            if (errno == EINTR)
                continue; /* Interrupted by signal, retry. */
            else {
                perror("[-] accept() failed");
                break;
            }
        }

        dbg_log("[+] New incoming connection from %s:%d", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));

        int recvlen = socket_recv(s, recvbuf, REVERSEPROXY_NOTICE_LEN);
        if (recvlen != REVERSEPROXY_NOTICE_LEN) {
            puts("[-] Error: Invalid data received from client");
            close(s);
            continue;
        }

        if (recvbuf[1] == CONTROL_SOCKET) {
            printf("[+] Control socket established\n");
            sendbuf[1] = CONFIRM_CONTROL_SOCKET;
            sendlen = socket_send(s, sendbuf, REVERSEPROXY_NOTICE_LEN);
            if (sendlen != REVERSEPROXY_NOTICE_LEN) {
                puts("[-] Error: Failed to confirm control socket");
                close(s);
                continue;
            }
            control_socket = s;
        }
        else if (recvbuf[1] == NEW_PROXY_SOCKET) {
            dbg_log("[+] New proxy socket request received");
            int tunnel_id = recvbuf[2] - '0' + 48;
            if (tunnel_set_destSocket_and_run_it(tunnel_id, s) < 0) {
                puts("[-] Error: Failed to set up new proxy socket");
                close(s);
            }
        }
        else {
            dbg_log("[-] Error: Unknown socket type received");
            close(s);
        }
    }
    return NULL;
}




int reverseProxyClient(const char* target_server, const char* user, const char* password) {
    rsocksStructalias rserverConfig;

    char server_ip[128];
    int server_port;
    sscanf(target_server, "%[^:]:%d", server_ip, &server_port);
    printf("[+] Reverse Client %s:%d\n", server_ip, server_port);

    while (1) {
        int control_socket = connect2controlSocket(server_ip, server_port);
        if (control_socket == -1) {
            printf("[-] Failed to connect to control socket, retrying in 5 seconds...\n");
            sleep_multi_Platform(10000);
            continue;
        }

        // Ensure control socket is non-blocking
        set_socket_nonblocking(control_socket);

        fd_set readset, master_set;
        FD_ZERO(&master_set);
        FD_SET(control_socket, &master_set);

        int maxfd = control_socket;

        for (;;) {
            readset = master_set; // Use a copy to avoid modifying the original set
            struct timeval tv = { 5, 0 }; // Shorter timeout for better responsiveness

            int iResult = select(maxfd + 1, &readset, NULL, NULL, &tv);
            if (iResult == -1) {
                if (errno == EINTR) continue; // Interrupted, retry
                perror("[-] select() error");
                break;
            }
            else if (iResult == 0) {
                // Timeout: Could implement a keepalive or health check here
                continue;
            }

            if (FD_ISSET(control_socket, &readset)) {
                char newSocketNotice[REVERSEPROXY_NOTICE_LEN];
                int readSize = socket_recv(control_socket, newSocketNotice, REVERSEPROXY_NOTICE_LEN);
                if (readSize == REVERSEPROXY_NOTICE_LEN && newSocketNotice[0] == True && newSocketNotice[1] == NEW_PROXY_SOCKET) {
                    strncpy(rserverConfig.Host, server_ip, 300);
                    rserverConfig.port = server_port;
                    rserverConfig.tunnel_id = newSocketNotice[2];
                    rserverConfig.user = user;
                    rserverConfig.password = password;

#ifndef WIN32
                    pthread_t thread_id;
#else
                    HANDLE thread_id;
#endif
                    creatThread_multi_Platform(thread_id, reverseClient_build_tunnel, &rserverConfig);
                }
                else if (readSize == 0) {
                    puts("[-] Control socket closed by server, reconnecting...");
                    break;
                }
                else {
                    perror("[-] Error reading from control socket");
                    break;
                }
            }
        }

        socket_close(control_socket);
        printf("[-] Server Connection lost, retrying in 5 seconds...\n");
        sleep_multi_Platform(5);
    }

    return -1;
}

int connect2controlSocket(char* revserProxyServer, int crontrolPort) {
    char NewSocketNoticebuff[REVERSEPROXY_NOTICE_LEN];

    int controlSocket = socket_connect(revserProxyServer, crontrolPort);
    if (controlSocket == SOCKET_CONNECT_ERROR) {
        printf("[-] Reverse Proxy Error on connect %s:%d [can not connect]\n", revserProxyServer, crontrolPort);
        return SOCKET_CONNECT_ERROR;
    }

    FillinSocketbuff(NewSocketNoticebuff);
    NewSocketNoticebuff[1] = CONTROL_SOCKET;
    int sendlen = socket_send(controlSocket, NewSocketNoticebuff, REVERSEPROXY_NOTICE_LEN);
    if (sendlen != REVERSEPROXY_NOTICE_LEN) {
        puts("[-] Error on Send");
        return -1;
    }
    int recvlen = socket_recv(controlSocket, NewSocketNoticebuff, REVERSEPROXY_NOTICE_LEN);
    if (recvlen != REVERSEPROXY_NOTICE_LEN) {
        puts("[-] Error on Send");
        return -1;
    }
    if (NewSocketNoticebuff[1] != CONFIRM_CONTROL_SOCKET) {
        puts("[-] Error happened on CONFIRM_CONTROL_SOCKET");
        return -1;
    }

    return controlSocket;
}

void* reverseClient_build_tunnel(void* arg) {
    rsocksStructalias* rserverConfig = (rsocksStructalias*)arg; // Cast the argument
    char rhost[300];
    char NewSocketNoticebuff[REVERSEPROXY_NOTICE_LEN];

    strncpy(rhost, rserverConfig->Host, 300);
    int rport = rserverConfig->port;
    char tunnel_id = rserverConfig->tunnel_id;
    const char* user = rserverConfig->user;
    const char* password = rserverConfig->password;

    int proxySocket = socket_connect(rhost, rport);

    FillinSocketbuff(NewSocketNoticebuff);
    NewSocketNoticebuff[0] = True;
    NewSocketNoticebuff[1] = NEW_PROXY_SOCKET;
    NewSocketNoticebuff[2] = tunnel_id;
    int sendlen = socket_send(proxySocket, NewSocketNoticebuff, REVERSEPROXY_NOTICE_LEN);
    if (sendlen != REVERSEPROXY_NOTICE_LEN) {
        puts("[-] Error on Send");
        return NULL;
    }

    // 执行SOCKS5认证
    if (socks5_authenticate(proxySocket, user, password) != 0) {
        dbg_log("[-] Authentication failed with client\n");
        socket_close(proxySocket);
        return NULL;
    }

    int dest_sock = ParseSocksCMD(proxySocket);
    if (dest_sock == -1) {
        dbg_log("[-] Failed to resolve destination\n");
        socket_close(proxySocket);
        return NULL;
    }

    // 建立数据传输隧道
    if (tunnel_sock_to_sock(proxySocket, dest_sock) == -1) {
        printf("[-] Tunnel error occurred\n");
    }

}