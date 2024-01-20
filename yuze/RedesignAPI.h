#ifndef __RedesignAPI_H__
#define __RedesignAPI_H__

#include "public.h"

// func for init windows platform socket
int socket_api_init();

// My redesign Socket api
int socket_initListenServer(int port, int backlog);
SOCKET socket_acceptClient(SOCKET server_socket);
SOCKET socket_connect(char* serverName, int port);
int socket_recv(SOCKET s, char* buf, int len);
int socket_send(SOCKET s, char* buf, int len);

int Reply_Build_Target_OK(SOCKET s);
int Reply_Cannot_Build_Target_Now(SOCKET s, char cmd);
int Reply_Cannot_Build_Target_Reason(SOCKET s);


#endif // !_REDESIGNAPI_H_