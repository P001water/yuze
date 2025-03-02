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
int start_reverse_socksPort(int socksPort);
int start_control_socket(int controlPort);

int reverseProxyClient(const char* target_server, const char* user, const char* password);
int reverseClient_build_tunnel(rsocksStructalias* rserverConfig);
int connect2controlSocket(char* revserProxyServer, int crontrolPort);