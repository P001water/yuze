#pragma warning(disable:4996)
;
#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include<stdio.h>
#include<string.h>   
#include<stdlib.h> 
#include <time.h>

typedef struct {
    int client_sock;
    const char* user;
    const char* password;
} ThreadArgs;


// 修正跨平台线程创建宏
#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")

#define THREAD_RETURN DWORD WINAPI
#define THREAD_PARAM LPVOID

#define creatThread_multi_Platform(thread_id, funcName, param) \
    thread_id = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)funcName, (LPVOID)param, 0, NULL);

#define HANDLE_ID_multi_Platform HANDLE
#define sleep_multi_Platform(param) Sleep(param)

#else
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>     // 包含 fcntl、F_GETFL、F_SETFL、O_NONBLOCK 的定义
#include <unistd.h>    // 包含 POSIX 接口，如文件描述符操作

#define THREAD_RETURN void*
#define THREAD_PARAM void*

#define creatThread_multi_Platform(thread_id, funcName, param) \
    pthread_create(&thread_id, NULL, funcName, (void*)param);

#define HANDLE_ID_multi_Platform pthread_t

#define sleep_multi_Platform(param) sleep(param)

#endif


// self-write C function module
#include "socks5Proto.h"
#include "reverse.h"
#include "forward.h"
#include "socksAPI.h"
#include "tunnelSock2sock.h"
#include "socks5Proxy.h"
#include "portTransmit.h"
#include "debug.h"

#define YUZE_VERSION "0.4.0"
#define True              1
#define False             0

#define SOCKET_CONNECT_ERROR -1
#define SOCKET_LISTEN_BACKLOG 1000


// SOCKS5 protocol
#define SOCKS5_VERSION 5
#define SOCKS5_CMD_CONNECT 1
#define SOCKS5_CMD_BIND 2
#define SOCKS5_CMD_UDP_ASSOCIATE 3

// Address types
#define SOCKS5_ATYP_IPV4 1
#define SOCKS5_ATYP_DOMAIN 3
#define SOCKS5_ATYP_IPV6 4

// UDP packet format
#define SOCKS5_UDP_HEADER_SIZE 4

// UDP buffer size
#define UDP_BUFFER_SIZE 65535


#endif // !_PUBLIC_H_