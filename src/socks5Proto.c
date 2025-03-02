#include "socks5Proto.h"

#define MAX_METHODS 255

// 修改后的线程函数，处理认证及后续逻辑
#ifdef _WIN32
DWORD WINAPI Socks5RroxyCMD_and_tunnel(LPVOID arg)
#else
void* SocksCMD_and_tunnel(void* arg)
#endif
{
    ThreadArgs* args = (ThreadArgs*)arg;
    int clnt_sock = args->client_sock;
    const char* user = args->user;
    const char* password = args->password;

    // 执行SOCKS5认证
    if (socks5_authenticate(clnt_sock, user, password) != 0) {
        dbg_log("[-] Authentication failed with client");
        socket_close(clnt_sock);
        free(args);
#ifdef _WIN32
        return 0;
#else
        return NULL;
#endif
    }

    int dest_sock = ParseSocksCMD(clnt_sock);
    if (dest_sock == -1) {
        socket_close(clnt_sock);
        free(args);
#ifdef _WIN32
        return 0;
#else
        return NULL;
#endif
    }

    // 建立数据传输隧道
    if (tunnel_sock_to_sock(clnt_sock, dest_sock) == -1) {
        printf("[-] Tunnel error occurred\n");
    }

    free(args);

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int socks5_authenticate(int client_sock, const char* user, const char* password) {
    unsigned char ver, num_methods;
    int recv_len;

    // Step 1: Receive SOCKS version
    if (receive_socks_version(client_sock) < 0) {
        return -1;
    }

    // Step 2: Receive authentication methods
    unsigned char methods[MAX_METHODS];
    if (receive_auth_methods(client_sock, methods, &num_methods) < 0) {
        return -1;
    }

    // Step 3: Decide authentication method based on server configuration
    unsigned char selected_method = 0x00; // NO AUTH by default
    if (user != NULL && password != NULL) {
        selected_method = 0x02; // USERNAME/PASSWORD AUTH
    }

    // Check if client supports the selected method
    int method_supported = 0;
    for (int i = 0; i < num_methods; i++) {
        if (methods[i] == selected_method) {
            method_supported = 1;
            break;
        }
    }

    if (!method_supported) {
        // Client does not support the required auth method
        send_auth_method_response(client_sock, 0xFF); // NO ACCEPTABLE METHODS
        return -1;
    }

    // Step 4: Send authentication method response
    if (send_auth_method_response(client_sock, selected_method) < 0) {
        return -1;
    }

    // Step 5: Perform username/password authentication if required
    if (selected_method == 0x02) {
        if (perform_username_password_authentication(client_sock, user, password) < 0) {
            return -1;
        }
    }

    return 0;  // Authentication successful
}


int receive_socks_version(int client_sock) {
    unsigned char ver;
    int recv_len = socket_recv(client_sock, (char*)&ver, 1);

    if (ver != 5) {
        dbg_log("[-] Unsupported SOCKS version: %d", ver);
        return -1;
    }

    return 0;  // Version is valid (SOCKS5)
}


int receive_auth_methods(int client_sock, unsigned char* methods, unsigned char* num_methods) {
    int recv_len = socket_recv(client_sock, (char*)num_methods, 1);
    if (*num_methods == 0 || *num_methods > MAX_METHODS) {
        dbg_log("[-] Invalid number of methods: %d", *num_methods);
        return -1;
    }

    recv_len = socket_recv(client_sock, (char*)methods, *num_methods);
    return 0;
}


unsigned char select_auth_method(unsigned char* methods, unsigned char num_methods) {
    // 默认选择不需要认证的方式 (0x00)
    unsigned char selected_method = 0xFF;

    for (int i = 0; i < num_methods; i++) {
        if (methods[i] == 0x00) {  // No authentication required
            selected_method = 0x00;
            break;
        }
        else if (methods[i] == 0x02) {  // Username/password authentication
            selected_method = 0x02;
        }
    }

    return selected_method;
}


int send_auth_method_response(int client_sock, unsigned char selected_method) {
    unsigned char response[2] = { 5, selected_method };
    return socket_send(client_sock, (char*)response, 2);
}


int perform_username_password_authentication(int client_sock, const char* user, const char* password) {
    unsigned char auth_ver, username_len, password_len;

    // 接收认证版本
    int recv_len = socket_recv(client_sock, (char*)&auth_ver, 1);
    if (auth_ver != 1) {
        dbg_log("[-] Unsupported authentication version: %d\n", auth_ver);
        return -1;
    }

    // 接收用户名长度和用户名
    recv_len = socket_recv(client_sock, (char*)&username_len, 1);
    char username[256];
    recv_len = socket_recv(client_sock, username, username_len);
    username[username_len] = '\0';

    // 接收密码长度和密码
    recv_len = socket_recv(client_sock, (char*)&password_len, 1);
    char password_received[256];
    recv_len = socket_recv(client_sock, password_received, password_len);
    password_received[password_len] = '\0';

    // 验证用户名和密码
    if (strcmp(username, user) == 0 && strcmp(password_received, password) == 0) {
        unsigned char auth_response[2] = { 1, 0 }; // Success
        socket_send(client_sock, (char*)auth_response, 2);
        return 0;  // Authentication success
    }
    else {
        unsigned char auth_response[2] = { 1, 1 }; // Failure
        socket_send(client_sock, (char*)auth_response, 2);
        return -1;  // Authentication failure
    }
}


int check_proto_version(int clnt_socket)
{
    int clnt_sock = clnt_socket;
    char buffer[500];
    char response_v5_ok[2] = { 5, 0 };   // Response for SOCKS5
    char response_v5_fail[2] = { 5, 255 }; // Response indicating failure
    int reply_length = 2;        // Length to use for replies
    int bytes_received;
    int bytes_sent;

    if (clnt_sock > 0)
    {
        bytes_received = socket_recv(clnt_sock, buffer, 257);

        if (bytes_received <= 257 && bytes_received > 0)
        {
            // If the first byte of the received data indicates SOCKSv5... the value symbols that socket proto version
            if (buffer[0] == 5)
            {

                bytes_sent = socket_send(clnt_sock, response_v5_ok, reply_length); // Send a response indicating SOCKSv5 is supported.
                //printf("Success: --> SOCKSv5 proto connect now!\n");
                if (bytes_sent == reply_length)
                {
                    return True;
                }
            }
            else
            {
                dbg_log("[-] ERROR: --> SOCKSv4 Not Supported now!\n");
            }
        }
    }
    socket_send(clnt_sock, response_v5_fail, reply_length); // resopnse that socekt4 proto is not supported
    return False;
}

void Reply_With_Error(int clnt_socket, unsigned char rep_code) {
    unsigned char response[10] = { 5, rep_code, 0, 1, 0, 0, 0, 0, 0, 0 };
    socket_send(clnt_socket, (char*)response, sizeof(response));
}

void HandleSocks5Bind(int clnt_socket) {
    dbg_log("[-] BIND command not supported.\n");
    Reply_With_Error(clnt_socket, 0x07);
}

int HandleSocks5Connect(int clnt_socket, unsigned char addrtype) {
    char destination[300];
    unsigned char port_bytes[2];
    unsigned short port;
    struct in_addr ipv4Address;
    struct in6_addr ipv6Address;
    char domainlen[1];

    if (addrtype == SOCKS5_ATYP_IPV4) {
        int receivedBytes = socket_recv(clnt_socket, (char*)&ipv4Address, sizeof(ipv4Address));
        if (receivedBytes != sizeof(ipv4Address)) {
            printf("[-] Error reading IPv4 address.\n");
            Reply_With_Error(clnt_socket, 0x03);
            return -1;
        }
        strcpy(destination, inet_ntoa(ipv4Address));
    }
    else if (addrtype == SOCKS5_ATYP_DOMAIN) {
        int receivedBytes = socket_recv(clnt_socket, domainlen, sizeof(domainlen));
        if (receivedBytes != sizeof(domainlen)) {
            printf("[-] Error reading domain length.\n");
            Reply_With_Error(clnt_socket, 0x01);
            return -1;
        }

        int domainLength = domainlen[0];
        if (domainLength <= 0) {
            printf("[-] Invalid domain length.\n");
            Reply_With_Error(clnt_socket, 0x01);
            return -1;
        }

        receivedBytes = socket_recv(clnt_socket, destination, domainLength);
        if (receivedBytes != domainLength) {
            printf("[-] Error reading domain name.\n");
            Reply_With_Error(clnt_socket, 0x03);
            return -1;
        }
        destination[receivedBytes] = '\0';
    }
    else if (addrtype == SOCKS5_ATYP_IPV6) {
        int receivedBytes = socket_recv(clnt_socket, (char*)&ipv6Address, sizeof(ipv6Address));
        if (receivedBytes != sizeof(ipv6Address)) {
            printf("[-] Error reading IPv6 address.\n");
            Reply_With_Error(clnt_socket, 0x03);
            return -1;
        }
        inet_ntop(AF_INET6, &ipv6Address, destination, sizeof(destination));
    }
    else {
        printf("[-] Unsupported address type: %d\n", addrtype);
        Reply_With_Error(clnt_socket, 0x06);
        return -1;
    }

    int receivedBytes = socket_recv(clnt_socket, (char*)port_bytes, sizeof(port_bytes));
    if (receivedBytes != sizeof(port_bytes)) {
        printf("[-] Error reading port.\n");
        Reply_With_Error(clnt_socket, 0x04);
        return -1;
    }

    port = (port_bytes[0] << 8) | port_bytes[1];
    dbg_log("TCP ---> %s:%d", destination, port);

    int dest_sock = socket_connect(destination, port);
    if (dest_sock == -1) {
        Reply_With_Error(clnt_socket, 0x05);
        return -1;
    }
    Reply_Build_Target_OK(clnt_socket);
    return dest_sock;
}


void HandleSocks5UdpAssociate(int clnt_socket) {
    struct sockaddr_in udp_bind_addr = { 0 };
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        printf("[-] Failed to create UDP socket.\n");
        Reply_With_Error(clnt_socket, 0x01);
        return;
    }

    udp_bind_addr.sin_family = AF_INET;
    udp_bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    udp_bind_addr.sin_port = 0;

    if (bind(udp_sock, (struct sockaddr*)&udp_bind_addr, sizeof(udp_bind_addr)) < 0) {
        printf("[-] Failed to bind UDP socket.\n");
        Reply_With_Error(clnt_socket, 0x01);
        close(udp_sock);
        return;
    }

    socklen_t addr_len = sizeof(udp_bind_addr);
    if (getsockname(udp_sock, (struct sockaddr*)&udp_bind_addr, &addr_len) < 0) {
        printf("[-] Failed to get UDP socket name.\n");
        Reply_With_Error(clnt_socket, 0x01);
        close(udp_sock);
        return;
    }

    Reply_Build_Target_OK(clnt_socket);
    printf("[+] UDP Relay bound at %s:%d\n", inet_ntoa(udp_bind_addr.sin_addr), ntohs(udp_bind_addr.sin_port));

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (1) {
        char buffer[65535];
        int recv_len = recvfrom(udp_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (recv_len < 0) {
            printf("[-] Failed to receive UDP data.\n");
            break;
        }

        printf("[UDP] Received %d bytes from client %s:%d\n", recv_len, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int sent_len = sendto(udp_sock, buffer, recv_len, 0, (struct sockaddr*)&client_addr, client_addr_len);
        if (sent_len != recv_len) {
            printf("[-] Failed to send UDP response to client.\n");
        }
    }

    close(udp_sock);
}

int ParseSocksCMD(int clnt_socket) {
    char reqheader[4];
    unsigned char cmd;
    unsigned char addrtype;

    int receivedBytes = socket_recv(clnt_socket, reqheader, sizeof(reqheader));
    if (receivedBytes != sizeof(reqheader)) {
        dbg_log("[-] Error reading request header. Received %d bytes, expected %d bytes.", receivedBytes, sizeof(reqheader));
        Reply_With_Error(clnt_socket, 0x01);
        return -1;
    }

    cmd = reqheader[1];
    addrtype = reqheader[3];

    switch (cmd) {
    case SOCKS5_CMD_CONNECT:
        return HandleSocks5Connect(clnt_socket, addrtype);

    case SOCKS5_CMD_BIND:
        HandleSocks5Bind(clnt_socket);
        return -1;

    case SOCKS5_CMD_UDP_ASSOCIATE:
        HandleSocks5UdpAssociate(clnt_socket);
        return 0;

    default:
        dbg_log("[-] Unsupported command: %d\n", cmd);
        Reply_With_Error(clnt_socket, 0x07);
        return -1;
    }

    return 0;
}



int FillinSocketbuff(char* s) {
    int  i;
    for (i = 0; i < 3; i++) {
        s[i] = '\0';
    }
    return True;
}

// Handle incoming UDP request
int handle_udp_request(int udp_sock, struct sockaddr_in* client_addr, socklen_t client_addr_len) {
    char buffer[65535];
    int recv_len = recvfrom(udp_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)client_addr, &client_addr_len);
    if (recv_len < 0) {
        perror("[-] Failed to receive UDP data");
        return -1;
    }

    printf("[DEBUG] Received UDP packet from %s:%d, size: %d\n",
        inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port), recv_len);

    // SOCKS5 UDP request structure: | RSV(2) | FRAG(1) | ATYP(1) | DST.ADDR | DST.PORT | DATA |
    if (recv_len < 10) {
        printf("[-] Malformed UDP request, too short\n");
        return -1;
    }

    unsigned char frag = buffer[2];
    if (frag != 0) {
        printf("[-] Fragmented UDP packets are not supported\n");
        return -1;
    }

    unsigned char atyp = buffer[3];
    char dest_addr[256];
    unsigned short dest_port;

    int addr_offset = 4;
    if (atyp == SOCKS5_ATYP_IPV4) { // IPv4
        struct in_addr ipv4_addr;
        memcpy(&ipv4_addr, &buffer[addr_offset], 4);
        inet_ntop(AF_INET, &ipv4_addr, dest_addr, sizeof(dest_addr));
        addr_offset += 4;
    }
    else if (atyp == SOCKS5_ATYP_DOMAIN) { // Domain name
        int domain_len = buffer[addr_offset];
        memcpy(dest_addr, &buffer[addr_offset + 1], domain_len);
        dest_addr[domain_len] = '\0';
        addr_offset += domain_len + 1;
    }
    else if (atyp == SOCKS5_ATYP_IPV6) { // IPv6
        struct in6_addr ipv6_addr;
        memcpy(&ipv6_addr, &buffer[addr_offset], 16);
        inet_ntop(AF_INET6, &ipv6_addr, dest_addr, sizeof(dest_addr));
        addr_offset += 16;
    }
    else {
        printf("[-] Unsupported address type: %d\n", atyp);
        return -1;
    }

    memcpy(&dest_port, &buffer[addr_offset], 2);
    dest_port = ntohs(dest_port);
    addr_offset += 2;

    printf("[DEBUG] UDP destination: %s:%d\n", dest_addr, dest_port);

    // Forward the payload data to destination
    int data_len = recv_len - addr_offset;
    int dest_sock = socket_connect(dest_addr, dest_port);
    if (dest_sock < 0) {
        printf("[-] Failed to connect to destination %s:%d\n", dest_addr, dest_port);
        return -1;
    }

    int sent_len = socket_send(dest_sock, buffer + addr_offset, data_len);
    if (sent_len != data_len) {
        printf("[-] Failed to send complete data to destination\n");
        socket_close(dest_sock);
        return -1;
    }

    socket_close(dest_sock);
    return 0;
}


