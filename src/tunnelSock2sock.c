#include "tunnelSock2sock.h"

#define MAX_POOL       1000

tunnel tunnelPools[MAX_POOL];
int tunnel_live_num = 0;
int tunnels_ready = True;

// init socks pool for diff requests
int tunnel_Pool_init() {
    int id;
    for (id = 0; id < MAX_POOL; id++) {
        tunnelPools[id].ref_sock = -1;
        tunnelPools[id].dest_sock = -1;
        tunnelPools[id].flag = False; // flag to mean tunnel is used or not 
    }
    tunnels_ready = True;
    return 1;
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
        tunnelPools[tunnel_id].ref_sock = ref_sock;
        tunnelPools[tunnel_id].flag = True;
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
        sleep_multi_Platform(1);
    }
    tunnels_ready = False;
    for (i = 0; i < MAX_POOL; i++) {  // find a Null tunnel from tunnel_pool
        if (tunnelPools[i].flag == False) {
            tunnel_live_num++;
            tunnelPools[i].flag = True;
            //printf("[-] Tunnel_id <-- %3d --> Open/use\n", i);
            return i;
        }
    }
    return -1;
}
int tunnel_set_destSocket_and_run_it(int tunnel_id, int destsock) {
    int* ptunnel_id = (int*)malloc(sizeof(int)); // Allocate memory for tunnel_id
    if (!ptunnel_id) {
        puts("[-] Memory allocation failed");
        return -1;
    }
    *ptunnel_id = tunnel_id; // Store tunnel_id in allocated memory
    tunnelPools[tunnel_id].dest_sock = destsock;

#ifndef WIN32
    pthread_t thread_id;
#else
    HANDLE thread_id;
#endif
    // Pass the allocated memory as the thread parameter
    int iResult = creatThread_multi_Platform(thread_id, tunnel_run_with_id, ptunnel_id);
    if (iResult < 0) {
        puts("[-] Can not set Destination socket");
        free(ptunnel_id); // Free allocated memory on failure
        tunnel_close(tunnel_id);
        return -1;
    }
    return 1;
}

#define BUFFER_SIZE 8192

// Optimized tunnel_run_with_id for efficient and robust data transfer
#ifdef _WIN32
DWORD WINAPI tunnel_run_with_id(LPVOID arg)
#else
void* tunnel_run_with_id(void* arg)
#endif
{
    int* ptunnel_id = (int*)arg; // Cast the argument back to int pointer
    int tunnel_id = *ptunnel_id; // Dereference to get the tunnel_id
    free(ptunnel_id); // Free the allocated memory

    int ref_sock = tunnelPools[tunnel_id].ref_sock;
    int dest_sock = tunnelPools[tunnel_id].dest_sock;

    char buffer[BUFFER_SIZE];
    fd_set fds;
    int maxfd = (ref_sock > dest_sock) ? ref_sock : dest_sock;

    int idle_count = 0;

    while (1) {
        FD_ZERO(&fds);
        FD_SET(ref_sock, &fds);
        FD_SET(dest_sock, &fds);

        struct timeval timeout = { 5, 0 };
        int activity = select(maxfd + 1, &fds, NULL, NULL, &timeout);
        if (activity < 0) {
            perror("[-] select() failed");
            break;
        }
        else if (activity == 0) {
            idle_count++;
            if (idle_count % 12 == 0) { // Log only every minute of inactivity
                dbg_log("[DEBUG] Tunnel %d has been idle for %d minutes", tunnel_id, idle_count / 12);
            }
            continue;
        }

        idle_count = 0; // Reset idle count on activity

        if (FD_ISSET(ref_sock, &fds)) {
            int bytes_read = recv(ref_sock, buffer, sizeof(buffer), 0);
            if (bytes_read < 0) {
                //perror("[-] recv() failed on ref_sock");
                break;
            }
            else if (bytes_read == 0) {
                //printf("[DEBUG] ref_sock closed in tunnel %d\n", tunnel_id);
                shutdown(dest_sock, 1);
                FD_CLR(ref_sock, &fds);
            }
            else {
                int bytes_sent = send(dest_sock, buffer, bytes_read, 0);
                if (bytes_sent < 0) {
                    //perror("[-] send() failed to dest_sock");
                    break;
                }
            }
        }

        if (FD_ISSET(dest_sock, &fds)) {
            int bytes_read = recv(dest_sock, buffer, sizeof(buffer), 0);
            if (bytes_read < 0) {
                //perror("[-] recv() failed on dest_sock");
                break;
            }
            else if (bytes_read == 0) {
                //printf("[DEBUG] dest_sock closed in tunnel %d\n", tunnel_id);
                shutdown(ref_sock, 1);
                FD_CLR(dest_sock, &fds);
            }
            else {
                int bytes_sent = send(ref_sock, buffer, bytes_read, 0);
                if (bytes_sent < 0) {
                    //perror("[-] send() failed to ref_sock");
                    break;
                }
            }
        }

        if (!FD_ISSET(ref_sock, &fds) && !FD_ISSET(dest_sock, &fds)) {
            //printf("[DEBUG] Both sockets closed for tunnel %d\n", tunnel_id);
            break;
        }
    }

    //printf("[DEBUG] Tunnel %d closing\n", tunnel_id);
    tunnel_close(tunnel_id);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}


int tunnel_close(int tunnel_id) {
    int flag = 0;
    if (tunnelPools[tunnel_id].ref_sock > 0) {
        socket_close(tunnelPools[tunnel_id].ref_sock);
        tunnelPools[tunnel_id].ref_sock = -1;
        flag = 1;
    }
    if (tunnelPools[tunnel_id].dest_sock > 0) {
        socket_close(tunnelPools[tunnel_id].dest_sock);
        tunnelPools[tunnel_id].dest_sock = -1;
        flag = 1;
    }
    //printf("[-] Tunnel_id <-- %3d --> Close/useless\n", tunnel_id);
    tunnelPools[tunnel_id].flag = False;
    if (flag) {
        tunnel_live_num--;
    }
    return 1;
}



