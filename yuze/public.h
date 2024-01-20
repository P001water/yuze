#pragma warning(disable:4996);
#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#define _CRT_SECURE_NO_WARNINGS
#define PTW32_STATIC_LIB
#define WIN32_LEAN_AND_MEAN 

#include<stdio.h>
#include<string.h>    
#include<stdlib.h>  

#ifdef _WIN32
#include<WinSock2.h>
#include<ws2tcpip.h>
#include<stdlib.h>
//#include<unistd.h>
#include<Windows.h>
#pragma comment(lib,"Ws2_32.lib")
#else
#include<pthread.h>
#include<netdb.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<signal.h>
#endif


// self-write C function module 
#include "protoFunc.h"
#include "RedesignAPI.h"
#include "tunnelSock2sock.h"
#include "socksServer.h"
#include "portTransmit.h"

#define MAX_POOL       1000
#define True              1
#define False             0

#define SOCKET_CONNECT_ERROR -1
#define SOCKET_LISTEN_BACKLOG 1000


//design for rsocks server
#define CONTROL_SOCKET 2
#define CONFIRM_CONTROL_SOCKET 1
#define NEW_PROXY_SOCKET 4
#define RSOCKET_SERVER_NOTICE_LEN 3


//design for main.c -> parse CMD Command
int flag;
int action;
int listenPort;
char refHost[100];
int refPort;
char destHost[100];
int destPort;


#endif // !_PUBLIC_H_