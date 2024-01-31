#ifndef _PORTTRANSMIT_H_
#define _PORTTRANSMIT_H_
#include "public.h"

typedef struct slaveStruct {
    char refHost[300];
    int refport;
    char destHost[300];
    int destport;
    int tunnel_id;
}slaveStructalias;

int yuze_slave(char* ref_host, int ref_port, char* dest_host, int dest_port);
int yuze_tran(int refPort, char* destHost, int destPort);
int yuze_listen(int refPort, int destPort);

int yuzelisten_create_clntPortServer(int* lstPort);
int yuzelisten_create_rsocksPortServer(int* p_rlstPort);
int yuzeslave_build_tunnel(slaveStructalias* slavesConfig);

#endif // !_PORTTRANSMIT_H_