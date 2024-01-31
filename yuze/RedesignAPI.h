#ifndef __RedesignAPI_H__
#define __RedesignAPI_H__

#include "public.h"

// func for init windows platform socket
int socket_api_init();

// My redesign Socket api
int socket_initListenServer(int port, int backlog);
int socket_acceptClient(int server_socket);
int socket_connect(char* serverName, int port);
int socket_recv(int s, char* buf, int len);
int socket_send(int s, char* buf, int len);
int socket_close(int socket);

int Reply_Build_Target_OK(int s);
int Reply_Cannot_Build_Target_Now(int s, char cmd);
int Reply_Cannot_Build_Target_Reason(int s);


#endif // !_REDESIGNAPI_H_