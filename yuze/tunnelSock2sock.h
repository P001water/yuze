#ifndef __tunnelSock2sock_H__
#define __tunnelSock2sock_H__

#include "public.h"

#define NULL_SOCK         0
#define REF_SOCK_OK         1
#define DEST_SOCK_OK         2

typedef struct socksPool {
    int ref_sock;
    int dest_sock;
    int flag;
    int rwstate;
} socksPool;

// func declare
int socks_Pool_init();
int tunnel_get_enable_id();
int tunnel_set_ref_sock(int ref_sock);
int tunnel_set_dest_sock_and_run_it(int tunnel_id,int dest_sock);
int tunnel_sock_to_sock(int ref_sock, int dest_sock);
int tunnel_run_with_id(int tunnel_id_param);
int tunnel_close(int tunnel_id);

int create_clnt_port_server(int* lstPort);
int create_rsocks_port_server(int* p_rlstPort);
int walk_socksPool_run();
int rtunnel_get_enable_id(int flag);

#endif // !_TUNNELSOCK2SOCK_H_