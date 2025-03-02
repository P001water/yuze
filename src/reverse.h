#pragma once

typedef struct rsocksStruct {
    char Host[300];
    int port;
    const char* user;
    const char* password;
    int tunnel_id;
} rsocksStructalias;


int reverseProxy(int controlPort, int socksPort, const char* connect_server, const char* user, const char* password);

int reverseProxyServer(int controlPort, int socksPort);
void* start_reverse_socksPort(void* arg);
void* start_control_socket(void* arg);

int reverseProxyClient(const char* target_server, const char* user, const char* password);
void* reverseClient_build_tunnel(void* arg);
int connect2controlSocket(char* revserProxyServer, int crontrolPort);