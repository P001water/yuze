#pragma warning(disable:4996);
#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include<stdio.h>
#include<string.h>   
#include<stdlib.h> 

#ifdef WIN32
#include<WinSock2.h>
#include<ws2tcpip.h>
#include<stdlib.h>
#include<Windows.h>
#pragma comment(lib,"Ws2_32.lib")
#define creatThread_multi_Platform(thread_id, funcName, param) \
CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)funcName, &param, 0, NULL);
#define HANDLE_ID_multi_Platform HANDLE;
#define sleep_multi_Platform(param) \
Sleep(param);
#else
#include<errno.h>
#include<unistd.h>
#include<pthread.h>
#include<netdb.h>
#include<sys/select.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<signal.h>
#define creatThread_multi_Platform(thread_id, funcName, param) \
pthread_create(&thread_id, 0, funcName, &param);
#define HANDLE_ID_multi_Platform pthread_t;
#define sleep_multi_Platform(param) \
sleep(param);
#endif

// self-write C function module
#include "protoFunc.h"
#include "RedesignAPI.h"
#include "tunnelSock2sock.h"
#include "socksServer.h"
#include "portTransmit.h"

#define YUZE_VERSION 0.1.0
#define True              1
#define False             0
#define MAX_POOL       1000
#define SOCKET_CONNECT_ERROR -1
#define SOCKET_LISTEN_BACKLOG 1000

//design for rsocks server
#define CONTROL_SOCKET 2
#define CONFIRM_CONTROL_SOCKET 1
#define NEW_PROXY_SOCKET 4
#define RSOCKET_SERVER_NOTICE_LEN 3


#endif // !_PUBLIC_H_