#include "tunnelSock2sock.h"

tunnelPool tunnelPools[MAX_POOL];
int tunnel_live_num = 0;
int tunnels_ready = True;

// init socks pool for diff requests
int tunnel_Pool_init() {
    int id;
    for (id = 0; id < MAX_POOL; id++) {
        tunnelPools[id].ref_sock = -1;
        tunnelPools[id].dest_sock = -1;
        tunnelPools[id].flag = False; // flag to main tunnel is used or not 
    }
    tunnels_ready = True;
    return TUNNEL_POOLS_INIT_SUCCESS;
}

int tunnel_sock_to_sock(int ref_sock, int dest_sock) {
    int tunnel_id = tunnel_set_refSocket_and_get_enable_id(ref_sock);
    if (tunnel_id != -1) {
        if (tunnel_set_destSocket_and_run_it(tunnel_id, dest_sock) != -1) {
            return True;
        }
    }
    return -1;
}

int tunnel_set_refSocket_and_get_enable_id(int ref_sock) {
    int tunnel_id;
    tunnel_id = tunnel_get_enable_id();
    if (tunnel_id != -1) {
        tunnelPools[tunnel_id].flag = True;
        tunnelPools[tunnel_id].ref_sock = ref_sock;
    }
    else {
        tunnel_id = -1;
    }
    tunnels_ready = True;
    return tunnel_id;
}

int tunnel_get_enable_id() {
    int i = 0;
    while (tunnel_live_num >= MAX_POOL - 1 || !tunnels_ready) {
        Sleep(1);
    }
    tunnels_ready = False;
    // find a Null tunnel from tunnel_pool
    for (i = 0; i < MAX_POOL; i++) {
        if (tunnelPools[i].flag == False) {
            tunnel_live_num++;
            tunnelPools[i].flag = True;
            return i;
        }
    }
    return -1;
}

int tunnel_run_with_id(int tunnel_id_param) {
    int tunnel_id = *(int*)tunnel_id_param;
    int read_size, write_size;
    char buffer[10000];
    int ref_sock, dest_sock;
    ref_sock = tunnelPools[tunnel_id].ref_sock;
    dest_sock = tunnelPools[tunnel_id].dest_sock;

    while (True) {
        fd_set fds;
        struct timeval tv = { 1,0 };

        FD_ZERO(&fds);
        FD_SET(ref_sock, &fds);
        FD_SET(dest_sock, &fds);

        int maxfd = max(ref_sock, dest_sock) + 1;
        int iResult = select(maxfd, &fds, NULL, NULL, &tv);

        if (iResult < 0) {
            //puts("[-] tunnel_sock_to_sock -> Select error.");
            break;
        }
        else if (iResult == 0) {
            //puts("[-] tunnel_sock_to_sock -> Socket time out.");
            break;
        }
        else
        {
            if (FD_ISSET(ref_sock, &fds)) {
                if ((read_size = socket_recv(ref_sock, buffer, 10000)) > 0) {
                    write_size = socket_send(dest_sock, buffer, read_size);
                    if (write_size < 0 || write_size != read_size) {
                        //tunnelPools[tunnel_id].ref_sock = -1;
                        break;
                    }
                    continue;
                }
            }

            if (FD_ISSET(dest_sock, &fds)) {
                if ((read_size = socket_recv(dest_sock, buffer, 10000)) > 0) {
                    write_size = socket_send(ref_sock, buffer, read_size);
                    if (write_size < 0 || write_size != read_size) {
                        //tunnelPools[tunnel_id].dest_sock = -1;
                        break;
                    }
                    continue;
                }
            }
            break; // for socket close itself connect, Important
        }
        memset(buffer, 0, 10000);
        read_size = write_size = 0;
    }

    //printf("[*] %d tunnel close\n", tunnel_id);
    tunnel_close(tunnel_id);
    return 0;
}

int tunnel_close(int tunnel_id) {
    int flag = 0;
    if (tunnelPools[tunnel_id].ref_sock > 0) {
        closesocket(tunnelPools[tunnel_id].ref_sock);
        tunnelPools[tunnel_id].ref_sock = -1;
        flag = 1;
    }
    if (tunnelPools[tunnel_id].dest_sock > 0) {
        closesocket(tunnelPools[tunnel_id].dest_sock);
        tunnelPools[tunnel_id].dest_sock = -1;
        flag = 1;
    }
    tunnelPools[tunnel_id].flag = False;
    if (flag) {
        tunnel_live_num--;
    }
    return 1;
}

int tunnel_set_destSocket_and_run_it(int tunnel_id, int destsock) {
    int* ptunnel_id = (int*)malloc(sizeof(int));
    *ptunnel_id = tunnel_id;

    tunnelPools[tunnel_id].dest_sock = destsock;

    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tunnel_run_with_id, ptunnel_id, 0, NULL) < 0) {
        puts("[-] Can not set Destination socket");
        tunnel_close(tunnel_id);
        return -1;
    }
    return 1;
}

