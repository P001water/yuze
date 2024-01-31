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
    signal(SIGPIPE, SIG_IGN); // ignore signal to avoid
#endif
    return 1;
}

// creat server socket and listen
int socket_initListenServer(int port, int backlog)
{
    int s, yes = 1;
    struct sockaddr_in serverAddr;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(s, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("[-] Bind failed\n");
        socket_close(s);
        return -1;
    }


    if (listen(s, SOMAXCONN) < 0) // backlog = SOMAXCONN , wait quene depend on system
    {
        printf("[-] Listen failed\n");
        socket_close(s);
        return -1;
    }
    return s;
}

/*
 * If the listening socket signaled there is a new connection ready to
 * be accepted, we accept it and return -1 on error or the new client
 * socket on success.
 */
int socket_acceptClient(int server_socket) {
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
int socket_connect(char* serverName, int port)
{
    int ConnectSocket;
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket < 0)
    {
        puts("[-] Create Socket Failed");
        return -1;
    }

    struct hostent* hostInfo;
    hostInfo = gethostbyname(serverName);
    if (hostInfo == NULL)
    {
        printf("[-] Cannot retrieve the server IP for %s\n", serverName);
        return -1;
    }

    struct sockaddr_in clntService;
    clntService.sin_family = AF_INET;
#ifdef WIN32
    clntService.sin_addr = *((struct in_addr*)hostInfo->h_addr);
#else
    clntService.sin_addr = *((struct in_addr*)hostInfo->h_addr_list[0]);
#endif // 


    clntService.sin_port = htons(port);
    int iResult = connect(ConnectSocket, (struct sockaddr*)&clntService, sizeof(clntService));
    if (iResult == 0) {
        return ConnectSocket;
    }

    for (;;) {
        fd_set writeset;
        struct timeval tv = { 2,0 };

        FD_ZERO(&writeset);
        FD_SET(ConnectSocket, &writeset);

        iResult = select(ConnectSocket + 1, NULL, &writeset, NULL, &tv);
        if (iResult < 0) {
            printf("[-] Connect failed");
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

    return -1;
}

// socket_recv(int£¬buff£¬length)
int socket_recv(int s, char* buf, int len)
{
    return recv(s, buf, len, 0);
}

// socket_send(int£¬buff£¬length)
int socket_send(int s, char* buf, int len)
{
    //int rc = send(s, buf, len, 0);
    //if (errno == EPIPE) {
    //    puts("[-] remote Host socket close");
    //    return -1;
    //}
    //else
    //{
    //    return rc;
    //}
    return send(s, buf, len, 0);
}

int socket_close(int socket) {
    if (socket > 0) {
#ifdef WIN32
        closesocket(socket);
#else
        close(socket);
#endif
    }
    return 1;
}

int Reply_Cannot_Build_Target_Now(int s, char cmd)
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

int Reply_Cannot_Build_Target_Reason(int s) {
#ifdef WIN32
    int Error = WSAGetLastError();
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
#else
    switch (errno) {
    case ENETUNREACH:
        Reply_Cannot_Build_Target_Now(s, 0x03);
        break;
    case EHOSTUNREACH:
        Reply_Cannot_Build_Target_Now(s, 0x04);
        break;
    case ECONNREFUSED:
        Reply_Cannot_Build_Target_Now(s, 0x05);
        break;
    case ETIMEDOUT:
        Reply_Cannot_Build_Target_Now(s, 0x06);
        break;
    }
#endif
    return 1;
}


int Reply_Build_Target_OK(int s)
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