#include "socksAPI.h"

int socket_api_init()
{
#ifdef _WIN32  // init socket lib for win
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        puts("[-] Windows WSAStartup failed");
        return 1;
    }
#else
    signal(SIGPIPE, SIG_IGN); // ignore signal to avoid
#endif
    return 1;
}

// Initialize and start a TCP listening server
int create_tcp_listening_socket(int port, int backlog)
{
    int server_socket, opt = 1;
    struct sockaddr_in server_addr;

    // Create TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        perror("Failed to create socket");
        return -1;
    }

    // Allow address reuse
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Failed to set socket options");
        close(server_socket);
        return -1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Bind socket to the specified address and port
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        close(server_socket);
        return -1;
    }

    // Start listening for incoming connections
    if (listen(server_socket, backlog) < 0) {
        perror("Failed to listen on socket");
        close(server_socket);
        return -1;
    }

    return server_socket;
}


/*
 * If the listening socket signaled there is a new connection ready to
 * be accepted, we accept it and return -1 on error or the new client
 * socket on success.
 */
int socket_acceptClient(int server_socket) {
    int s;

    while (True) {
        struct sockaddr_in sa;
        socklen_t slen = sizeof(sa);
        s = accept(server_socket, (struct sockaddr*)&sa, &slen);
        if (s == -1) {
            if (errno == EINTR)
                continue; /* Try again. */
            else
                return -1;
        }
        break;
    }
    return s;
}


// Updated version of socket_connect using getaddrinfo()
int socket_connect(char* serverName, int port)
{
    int ConnectSocket;
    struct addrinfo hints, * result, * rp;
    int s;

    // Prepare the hints structure for getaddrinfo
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // Force IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    s = getaddrinfo(serverName, NULL, &hints, &result);
    if (s != 0) {
        //printf("[-] getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    // Loop through the results and connect to the first valid address
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        ConnectSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (ConnectSocket == -1) {
            continue; // Try next address
        }

        ((struct sockaddr_in*)rp->ai_addr)->sin_port = htons(port);
        if (connect(ConnectSocket, rp->ai_addr, rp->ai_addrlen) == 0) {
            break; // Successful connection
        }
    }
    

    freeaddrinfo(result); // Free the linked list

    if (rp == NULL) { // No address succeeded
        dbg_log("[-] Could not connect to server %s", serverName);
        return -1;
    }

    return ConnectSocket; // Successful connection
}

void set_socket_nonblocking(int socket_fd) {
#ifndef _WIN32
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
#else
    u_long mode = 1;
    ioctlsocket(socket_fd, FIONBIO, &mode);
#endif
}

// socket_recv(int£¬buff£¬length)
int socket_recv(int s, char* buf, int len)
{
    return recv(s, buf, len, 0);
}

// socket_send(int£¬buff£¬length)
int socket_send(int s, char* buf, int len)
{
    return send(s, buf, len, 0);
}

int socket_close(int socket) {
    if (socket > 0) {
#ifdef WIN32
        closesocket(socket);
#else
        close(socket);
#endif
    }
    return 1;
}

int Reply_Cannot_Build_Target_Now(int s, char cmd)
{
    char replaybuf[4]; // Buffer that will be sent
    int msglen = 4; // Represents length of the message

    // Preparing the message
    replaybuf[0] = 5;
    replaybuf[1] = cmd;
    replaybuf[2] = 0;
    replaybuf[3] = 1;

    socket_send(s, replaybuf, msglen);
    return 1;
}

int Reply_Cannot_Build_Target_Reason(int s) {
#ifdef WIN32
    int Error = WSAGetLastError();
    if (Error == 10060)
    {
        Reply_Cannot_Build_Target_Now(s, 6);
    }
    else if (Error > 10060)
    {
        if (Error == 10061)
        {
            Reply_Cannot_Build_Target_Now(s, 5);
        }
        else if (Error == 10065)
        {
            Reply_Cannot_Build_Target_Now(s, 4);
        }
    }
    else if (Error == 10051)
    {
        Reply_Cannot_Build_Target_Now(s, 3);
    }
#else
    switch (errno) {
    case ENETUNREACH:
        Reply_Cannot_Build_Target_Now(s, 0x03);
        break;
    case EHOSTUNREACH:
        Reply_Cannot_Build_Target_Now(s, 0x04);
        break;
    case ECONNREFUSED:
        Reply_Cannot_Build_Target_Now(s, 0x05);
        break;
    case ETIMEDOUT:
        Reply_Cannot_Build_Target_Now(s, 0x06);
        break;
    }
#endif
    return 1;
}


int Reply_Build_Target_OK(int s)
{
    char replaybuf[10]; // Buffer that will be sent
    int msglen = 10; // Represents length of the message

    // Preparing the message
    replaybuf[0] = 5;
    replaybuf[1] = 0;
    replaybuf[2] = 0;
    replaybuf[3] = 1;

    memset(&replaybuf[4], 65, 6);

    socket_send(s, replaybuf, msglen);
    return 1;
}

// Initialize UDP server socket
int socket_initUdpServer(int port) {
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("[-] Failed to create UDP socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(udp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[-] Failed to bind UDP socket");
        close(udp_sock);
        return -1;
    }

    printf("[+] UDP server initialized on port %d\n", port);
    return udp_sock;
}

// Receive data from UDP client
int socket_recvfrom(int udp_sock, char* buf, int len, struct sockaddr_in* client_addr, socklen_t* client_addr_len) {
    int recv_len = recvfrom(udp_sock, buf, len, 0, (struct sockaddr*)client_addr, client_addr_len);
    if (recv_len < 0) {
        perror("[-] Failed to receive UDP data");
    }
    return recv_len;
}

// Send data to UDP client
int socket_sendto(int udp_sock, char* buf, int len, struct sockaddr_in* dest_addr, socklen_t dest_addr_len) {
    int sent_len = sendto(udp_sock, buf, len, 0, (struct sockaddr*)dest_addr, dest_addr_len);
    if (sent_len < 0) {
        perror("[-] Failed to send UDP data");
    }
    return sent_len;
}
