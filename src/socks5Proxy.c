#include "socks5Proxy.h"
#include "socks5Proto.h"

// start socks5 proxy server
int start_proxy(int listenPort, const char* user, const char* password) {
    int serv_sock = create_tcp_listening_socket(listenPort, SOCKET_LISTEN_BACKLOG);
    if (serv_sock < 0) {
        printf("[-] Error --> Unable to start server on port %d.\n", listenPort);
        exit(1);
    }
    printf("[+] SOCKS5 Proxy server listening on 0.0.0.0:%d\n", listenPort);

    while (1) {
        struct sockaddr_in sa;
        socklen_t slen = sizeof(sa);
        int client_sock = accept(serv_sock, (struct sockaddr*)&sa, &slen);
        if (client_sock == -1) {
            if (errno == EINTR) continue;
            else break;
        }

        ThreadArgs* args = (ThreadArgs*)malloc(sizeof(ThreadArgs));
        if (!args) {
            printf("[-] Failed to allocate memory for thread arguments\n");
            socket_close(client_sock);
            continue;
        }
        args->client_sock = client_sock;
        args->user = user;
        args->password = password;

        // 创建线程处理客户端请求
#ifndef _WIN32
        pthread_t thread_id;
#else
        HANDLE thread_id;
#endif
        creatThread_multi_Platform(thread_id, Socks5RroxyCMD_and_tunnel, args);
    }

    socket_close(serv_sock);
    return 0;
}


