#ifndef __PROTOFUNC_H__
#define __PROTOFUNC_H__

#include "public.h"

#define MAXTUNNELNUM 1000

int extract_and_tunnel(SOCKET* clnt_sockParam);
int check_proto_version(SOCKET clnt_socket);
SOCKET extractRequestHeader(SOCKET clnt_socket);

int FillinSocketbuff(char* s);

#endif // _PROTOFUNC_H_
