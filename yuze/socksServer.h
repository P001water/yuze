#ifndef _SOCKSSERVER_H_
#define _SOCKSSERVER_H_
#include "public.h"

typedef struct rsocksStruct {
    char Host[300];
    int port;
    int tunnel_id;
}rsocksStructalias;


//design for reverse connect service
SOCKET control_socket;

int create_socks_server(int listenPort);
int create_rsocks_client(char* refHost, int refPort);

int rsocksClient_build_tunnel(rsocksStructalias* rserverConfig);
int rsocksClient_init_ControlSocket(char* refHost, int refPort);

#endif // !_SOCKSSERVER_H_
