#include "public.h"

// Simple port forwarder function
void port_forward(int listen_port, const char* forward_address) {
    char forward_server[256] = "127.0.0.1"; // Default IP address
    int forward_port;

    // Check if forward_address contains an IP:PORT or just a PORT
    if (strchr(forward_address, ':')) {
        sscanf(forward_address, "%[^:]:%d", forward_server, &forward_port);
    }
    else {
        forward_port = atoi(forward_address); // Treat input as port only
    }

    int listen_sock = create_tcp_listening_socket(listen_port, SOCKET_LISTEN_BACKLOG);
    if (listen_sock < 0) {
        printf("[-] Error: Unable to start port forward listener on port %d\n", listen_port);
        return;
    }

    printf("[+] Forwarding from 0.0.0.0:%d to %s:%d\n", listen_port, forward_server, forward_port);

    while (True) {  
        struct sockaddr_in sa;
        int slen = sizeof(sa);
        int client_sock = accept(listen_sock, (struct sockaddr*)&sa, &slen);
        if (client_sock < 0) {
            if (errno == EINTR)
                continue;
            perror("[-] accept() failed");
            break;
        }

        int forward_sock = socket_connect(forward_server, forward_port);
        if (forward_sock < 0) {
            puts("[-] Error: Unable to connect to forward target");
            close(client_sock);
            continue;
        }

        dbg_log("[+] Established forward tunnel between client %s:%d and target %s:%d\n",
            inet_ntoa(sa.sin_addr), ntohs(sa.sin_port), forward_server, forward_port);

        tunnel_sock_to_sock(client_sock, forward_sock);
    }
    close(listen_sock);
}
