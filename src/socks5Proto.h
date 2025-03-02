#ifndef __PROTOFUNC_H__
#define __PROTOFUNC_H__

#include "public.h"

#define MAXTUNNELNUM 1000


int socks5_authenticate(int client_sock, const char* user, const char* password);

// Existing function declarations
#ifdef _WIN32
DWORD WINAPI Socks5RroxyCMD_and_tunnel(LPVOID arg);
#else
void* Socks5RroxyCMD_and_tunnel(void* arg);
#endif

int send_auth_method_response(int client_sock, unsigned char selected_method);
int receive_socks_version(int client_sock);
int receive_auth_methods(int client_sock, unsigned char* methods, unsigned char* num_methods);
int perform_username_password_authentication(int client_sock, const char* user, const char* password);

int check_proto_version(int clnt_socket);
int ParseSocksCMD(int clnt_socket);

int FillinSocketbuff(char* s);


#endif // __PROTOFUNC_H__
