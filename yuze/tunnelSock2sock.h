#ifndef __tunnelSock2sock_H__
#define __tunnelSock2sock_H__

#include "public.h"

#define NULL_SOCK         0
#define REF_SOCK_OK         1
#define DEST_SOCK_OK         2

typedef struct tunnelPool {
    int ref_sock;
    int dest_sock;
    int flag;
    int rwstate;
} tunnelPool;

// func declare
int tunnel_Pool_init();
int tunnel_get_enable_id();
int tunnel_set_refSocket_and_get_enable_id(int ref_sock);
int tunnel_set_destSocket_and_run_it(int tunnel_id, int dest_sock);
int tunnel_sock_to_sock(int ref_sock, int dest_sock);
int tunnel_run_with_id(int tunnel_id_param);
int tunnel_close(int tunnel_id);

#define TUNNEL_POOLS_INIT_SUCCESS 1
#define TUNNEL_POOLS_INIT_FAIL 1

#endif // !_TUNNELSOCK2SOCK_H_