#include "tunnelSock2sock.h"

socksPool socks_Pool[MAX_POOL];
int tunnel_live_num = 0;
int can_write_pool = True;

// init socks pool for diff requests
int socks_Pool_init() {
    int id;
    for (id = 0; id < MAX_POOL; id++) {
        socks_Pool[id].ref_sock = -1;
        socks_Pool[id].dest_sock = -1;
        socks_Pool[id].flag = FALSE; // 
        socks_Pool[id].rwstate = NULL_SOCK;
    }
    can_write_pool = TRUE;
    return 1;
}

int tunnel_sock_to_sock(int ref_sock, int dest_sock) {
    int tunnel_id = tunnel_set_ref_sock(ref_sock);
    if (tunnel_id != -1) {
        if (tunnel_set_dest_sock_and_run_it(tunnel_id, dest_sock) != -1) {
            return 1;
        }
    }
    return -1;
}

int tunnel_set_ref_sock(int ref_sock) {
    int tunnel_id;
    tunnel_id = tunnel_get_enable_id();
    if (tunnel_id != -1) {
        socks_Pool[tunnel_id].flag = True;
        socks_Pool[tunnel_id].ref_sock = ref_sock;
        socks_Pool[tunnel_id].rwstate = REF_SOCK_OK;
    }
    else {
        tunnel_id = -1;
    }
    // unlock write pool now
    can_write_pool = True;
    return tunnel_id;
}

int tunnel_get_enable_id() {
    int i = 0;
    while (tunnel_live_num >= MAX_POOL - 1 || !can_write_pool) {
        // wait Pool ready
        Sleep(1);
    }
    // lock pool write
    can_write_pool = False;
    // find a Null Pool
    for (i = 0; i < MAX_POOL; i++) {
        if (socks_Pool[i].flag == False) {
            tunnel_live_num++;
            socks_Pool[i].flag = True;
            //printf("[+] <-- %3d --> (Tunnel open) %d/%d\n", i, tunnel_live_num, MAX_POOL - tunnel_live_num);
            return i;
        }
    }
    return -1;
}

int tunnel_run_with_id(int tunnel_id_param) {
    //int tunnel_id = tunnel_id_param;
    int tunnel_id = *(int*)tunnel_id_param;
    int read_size, write_size;
    char buffer[10000];
    int ref_sock, dest_sock;
    ref_sock = socks_Pool[tunnel_id].ref_sock;
    dest_sock = socks_Pool[tunnel_id].dest_sock;

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
                         //socks_Pool[tunnel_id].ref_sock = -1;
                        break;
                    }
                    continue;
                }
            }

            if (FD_ISSET(dest_sock, &fds)) {
                if ((read_size = socket_recv(dest_sock, buffer, 10000)) > 0) {
                    write_size = socket_send(ref_sock, buffer, read_size);
                    if (write_size < 0 || write_size != read_size) {
                        //socks_Pool[tunnel_id].dest_sock = -1;
                        break;
                    }
                    continue;
                }
            }
            break; // for socket close itself connect
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
    if (socks_Pool[tunnel_id].ref_sock > 0) {
        closesocket(socks_Pool[tunnel_id].ref_sock);
        socks_Pool[tunnel_id].ref_sock = -1;
        flag = 1;
    }
    if (socks_Pool[tunnel_id].dest_sock > 0) {
        closesocket(socks_Pool[tunnel_id].dest_sock);
        socks_Pool[tunnel_id].dest_sock = -1;
        flag = 1;
    }
    socks_Pool[tunnel_id].rwstate = NULL_SOCK;
    socks_Pool[tunnel_id].flag = False;
    if (flag) tunnel_live_num--;
    //printf("[*] --> %3d <-- (Tunnel close)  %d/%d\n", tunnel_id, tunnel_live_num, MAX_POOL - tunnel_live_num);
    return 1;
}

int tunnel_set_dest_sock_and_run_it(int tunnel_id, int destsock) {
    int* poolnum = (int*)malloc(sizeof(int));
    *poolnum = tunnel_id;

    socks_Pool[tunnel_id].dest_sock = destsock;
    socks_Pool[tunnel_id].rwstate = DEST_SOCK_OK;

    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tunnel_run_with_id, (void*)poolnum, 0, NULL) < 0) {
        puts("[-] Can not get Destination socket");
        tunnel_close(tunnel_id);
        return -1;
    }
    return 1;
}