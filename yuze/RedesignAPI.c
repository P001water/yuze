#include "RedesignAPI.h"


int socket_api_init()
{
#ifdef WIN32  // init socket lib for win
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
    {
        puts("[-] Windows WSAStartup failed");
        return 1;
    }
#else
    signal(SIGPIPE, SIG_IGN);
#endif
    return 1;
}

// creat server socket and listen
int socket_initListenServer(int port, int backlog)
{
    int s, yes = 1;
    struct sockaddr_in serverAddr;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(s, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[-] Bind failed with error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return -1;
    }


    if (listen(s, SOMAXCONN) == SOCKET_ERROR) // backlog = SOMAXCONN , wait quene depend on system
    {
        printf("[-] Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return -1;
    }
    return s;
}

/*
 * If the listening socket signaled there is a new connection ready to
 * be accepted, we accept it and return -1 on error or the new client
 * socket on success.
 */
SOCKET socket_acceptClient(SOCKET server_socket) {
    int s;

    while (True) {
        struct sockaddr_in sa;
        int slen = sizeof(sa);
        s = accept(server_socket, (struct sockaddr*)&sa, &slen);
        if (s == -1) {
            if (errno == EINTR)
                continue; /* Try again. */
            else
                return -1;
        }
        break;
    }
    return s;
}

// Connect to remote server socket
SOCKET socket_connect(char* serverName, int port)
{
    SOCKET ConnectSocket;
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET)
    {
        puts("[-] Create Socket Failed");
        return -1;
    }

    u_long iMode = 0;
    // If iMode = 0, blocking is enabled;
    // If iMode != 0, non-blocking mode is enabled.
    // function in ioctlsocket
    int iResult = ioctlsocket(ConnectSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
        printf("ioctlsocket failed with error: %ld\n", iResult);

    struct hostent* hostInfo;
    hostInfo = gethostbyname(serverName);
    if (hostInfo == NULL)
    {
        printf("[-] Cannot retrieve the server IP for %s\n", serverName);
        return -1;
    }

    struct sockaddr_in clntService;
    clntService.sin_family = AF_INET;
    clntService.sin_addr = *((struct in_addr*)hostInfo->h_addr);
    clntService.sin_port = htons(port);
    iResult = connect(ConnectSocket, (struct sockaddr*)&clntService, sizeof(clntService));
    if (iResult == 0) {
        return ConnectSocket;
    }

    if (iMode != 0) {
        //因为是非阻塞的，这个时候错误码应该是WSAEWOULDBLOCK，Linux下是EINPROGRESS
        if (iResult < 0 && WSAGetLastError() != WSAEWOULDBLOCK) {
            printf("[-] Connect failed with error: %ld\n", WSAGetLastError());
            return -1;
        }

        for (;;) {
            fd_set writeset;
            struct timeval tv = { 2,0 };

            FD_ZERO(&writeset);
            FD_SET(ConnectSocket, &writeset);

            iResult = select(ConnectSocket + 1, NULL, &writeset, NULL, &tv);
            if (iResult < 0) {
                printf("[-] Connect failed with error: %ld\n", WSAGetLastError());
                break;
            }
            else if (iResult == 0) {
                puts("[-] Connect timeout");
                break;
            }
            else {
                if (FD_ISSET(ConnectSocket, &writeset)) {
                    return ConnectSocket;
                }
            }
        }
    }
    return -1;
}

// socket_recv(SOCKET，buff，length)
int socket_recv(SOCKET s, char* buf, int len)
{
    return recv(s, buf, len, 0);
}

// socket_send(SOCKET，buff，length)
int socket_send(SOCKET s, char* buf, int len)
{
    return send(s, buf, len, 0);
}


int Reply_Cannot_Build_Target_Now(SOCKET s, char cmd)
{
    char replaybuf[4]; // Buffer that will be sent
    int msglen = 4; // Represents length of the message

    // Preparing the message
    replaybuf[0] = 5;
    replaybuf[1] = cmd;
    replaybuf[2] = 0;
    replaybuf[3] = 1;

    socket_send(s, replaybuf, msglen);
    return 1;
}

int Reply_Cannot_Build_Target_Reason(SOCKET s)
{
    int Error; // eax

    Error = WSAGetLastError();
    if (Error == 10060)
    {
        Reply_Cannot_Build_Target_Now(s, 6);
    }
    else if (Error > 10060)
    {
        if (Error == 10061)
        {
            Reply_Cannot_Build_Target_Now(s, 5);
        }
        else if (Error == 10065)
        {
            Reply_Cannot_Build_Target_Now(s, 4);
        }
    }
    else if (Error == 10051)
    {
        Reply_Cannot_Build_Target_Now(s, 3);
    }
    return 1;
}


int Reply_Build_Target_OK(SOCKET s)
{
    char replaybuf[10]; // Buffer that will be sent
    int msglen = 10; // Represents length of the message

    // Preparing the message
    replaybuf[0] = 5;
    replaybuf[1] = 0;
    replaybuf[2] = 0;
    replaybuf[3] = 1;

    memset(&replaybuf[4], 65, 6);

    socket_send(s, replaybuf, msglen);
    return 1;
}