#ifndef __PROTOFUNC_H__
#define __PROTOFUNC_H__

#include "public.h"

#define MAXTUNNELNUM 1000


int socks5_authenticate(int client_sock, const char* user, const char* password);

// Existing function declarations
#ifdef _WIN32
DWORD WINAPI Socks5RroxyCMD_and_tunnel(LPVOID arg);
#else
void* SocksCMD_and_tunnel(void* arg);
#endif



int check_proto_version(int clnt_socket);
int ParseSocksCMD(int clnt_socket);

int FillinSocketbuff(char* s);


#endif // __PROTOFUNC_H__
