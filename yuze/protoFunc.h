#ifndef __PROTOFUNC_H__
#define __PROTOFUNC_H__

#include "public.h"

#define MAXTUNNELNUM 1000

int extract_and_tunnel(int* clnt_sockParam);
int check_proto_version(int clnt_socket);
int extractRequestHeader(int clnt_socket);

int FillinSocketbuff(char* s);

#endif // _PROTOFUNC_H_
