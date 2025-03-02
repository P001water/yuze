#ifndef __REDESIGNAPI_H__
#define __REDESIGNAPI_H__

#include "public.h"

// Socket API for TCP
int socket_api_init();
int create_tcp_listening_socket(int port, int backlog);
int socket_acceptClient(int server_socket);
int socket_connect(char* serverName, int port);
int socket_recv(int s, char* buf, int len);
int socket_send(int s, char* buf, int len);
int socket_close(int socket);
void set_socket_nonblocking(int socket_fd);

// Socket API for UDP
int socket_initUdpServer(int port);
int socket_recvfrom(int udp_sock, char* buf, int len, struct sockaddr_in* client_addr, socklen_t* client_addr_len);
int socket_sendto(int udp_sock, char* buf, int len, struct sockaddr_in* dest_addr, socklen_t dest_addr_len);

// Response helpers
int Reply_Build_Target_OK(int s);
int Reply_Cannot_Build_Target_Now(int s, char cmd);
int Reply_Cannot_Build_Target_Reason(int s);

#endif // __REDESIGNAPI_H__
