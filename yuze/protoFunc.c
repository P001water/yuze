#include "protoFunc.h"


int check_proto_version(SOCKET clnt_socket)
{
    SOCKET clnt_sock = clnt_socket;
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
                printf("[-] ERROR: --> SOCKSv4 Not Supported now!\n");
            }
        }
    }
    socket_send(clnt_sock, response_v5_fail, reply_length); // resopnse that socekt4 proto is not supported
    return False;
}

// parse socksv5 protocol to get destination socket
SOCKET extractRequestHeader(SOCKET clnt_socket)
{
    char reqheader[4];
    char domainlen[200];
    int domainLength;
    char in[4];
    struct in_addr ipv4Address;
    char destination[300];
    unsigned short port;
    unsigned char port_bytes[2];
    char addrtype; // to note the socksv5 proto value

    SOCKET dest_sock;
    int receivedBytes;

    receivedBytes = socket_recv(clnt_socket, reqheader, 4);
    if (receivedBytes != 4) {
        puts("[-] errors in From_clnt_req_Get_dest_Socket! ");
        Reply_Cannot_Build_Target_Now(clnt_socket, 255);
        return -1;
    }
    addrtype = reqheader[3];

    switch (addrtype)
    {
    case 1: // IPv4
        receivedBytes = socket_recv(clnt_socket, in, 4); // Receive the ip port
        ipv4Address = *(struct in_addr*)in;
        strcpy(destination, inet_ntoa(*(struct in_addr*)in));
        //printf("[*] Connect to %s\n [ip]", destination);

        receivedBytes = socket_recv(clnt_socket, (char*)port_bytes, 2); // Receive the port bytes
        if (receivedBytes != sizeof(port_bytes)) {
            Reply_Cannot_Build_Target_Now(clnt_socket, 0x08);
            return -1;
        }
        
        port = (port_bytes[0] << 8) | port_bytes[1];  // 从两个字节（通常是网络字节顺序，即大端序）中构建一个16位的端口号。
        printf("[+] TCP ---> %s:%d\n", destination, port); 
        dest_sock = socket_connect(destination, port);
        if (dest_sock == -1) {
            Reply_Cannot_Build_Target_Reason(clnt_socket);
            return -1;
        }
        else {
            Reply_Build_Target_OK(clnt_socket);
            return dest_sock;
        }

    case 3: // Domain name
        receivedBytes = socket_recv(clnt_socket, domainlen, 1);
        if (receivedBytes != 1) 
            return -1;

        domainLength = domainlen[0];  // 客户端请求的第一个字节为域名的长度
        if (domainLength <= 0) 
            return -1;

        receivedBytes = socket_recv(clnt_socket, destination, domainLength);
        if (receivedBytes != domainLength) {
            puts("Something error on read URL");
            printf("the read url is %s \n", destination);
            return -1;
        }
        destination[receivedBytes] = 0;
        // Receive the port bytes
        receivedBytes = socket_recv(clnt_socket, (char*)port_bytes, 2);
        if (receivedBytes != sizeof(port_bytes)) {
            Reply_Cannot_Build_Target_Now(clnt_socket, 0x08);
            return -1;
        }

        // 从两个字节（通常是网络字节顺序，即大端序）中构建一个16位的端口号。
        port = (port_bytes[0] << 8) | port_bytes[1];

        printf("[+] TCP ---> %s:%d\n", destination, port);
        dest_sock = socket_connect(destination, port);
        if (dest_sock == -1) {
            Reply_Cannot_Build_Target_Reason(clnt_socket);
            return -1;
        }
        else {
            Reply_Build_Target_OK(clnt_socket);
            return dest_sock;
        }
    case 4: // IPv6 (not supported)
        Reply_Cannot_Build_Target_Now(clnt_socket, 0x08);
        puts("[-] Not support IPv6");
        return -1;

    default: 
        Reply_Cannot_Build_Target_Now(clnt_socket, 0x08);
        puts("[-] NOT IPv4, IPv6, or URL?");
        return -1;
    }
    return -1;
}


int extract_and_tunnel(SOCKET* clnt_sockParam)
{
    //puts("[*] come to extract_and_tunnel func");
    SOCKET clnt_sock = *clnt_sockParam, dest_sock;

    if (check_proto_version(clnt_sock) == True) { // Check the sockets proto version
        Sleep(1);
        dest_sock = extractRequestHeader(clnt_sock); // from socksv5 proto parse the destination url
        if (dest_sock == -1) {
            closesocket(clnt_sock);
            return -1;
        }
        else
        {
            tunnel_sock_to_sock(clnt_sock, dest_sock);
        }
    }
}
