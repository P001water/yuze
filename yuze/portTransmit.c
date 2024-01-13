#include "portTransmit.h"

// -m yuze_tran -s 7878 -d 127.0.0.1 -e 9999 将7878 端口收到的socks代理请求转交给127.0.0.1打开正向监听的主机
// Forword data to remote host ,remote host should be Forward connection host
int yuze_tran(int refPort, char* destHost, int destPort) {
    SOCKET server_sock;
    printf("[+] Port forwart to RemoteHost 0.0.0.0:%d <--[yuze]--> %s:%d\n", refPort, destHost, destPort);

    server_sock = socket_initServer(refPort, 500);
    for (;;) {
        fd_set readset;
        struct timeval tv = { 60,0 };
        FD_ZERO(&readset);
        FD_SET(server_sock, &readset);

        int selectRet = select(server_sock + 1, &readset, NULL, NULL, &tv);
        if (selectRet < 0)
        {
            puts("[-] yuze_tran -》 Select error");
            break;
        }
        else if (selectRet == 0)
        {
            puts("[-] yuze_tran -》 Select time out");
            continue;
        }
        else if (selectRet) {
            if (FD_ISSET(server_sock, &readset)) {
                SOCKET clnt_sock = socket_acceptClient(server_sock);
                if (clnt_sock == SOCKET_ERROR) {
                    //puts("[+] Client Connected");
                }

                SOCKET dest_sock = socket_connect(destHost, destPort);
                if (dest_sock == SOCKET_ERROR) {
                    //puts("[-] RemoteHost Connected closed");
                    closesocket(dest_sock);
                    exit(EXIT_FAILURE);
                }
                else {
                    //puts("[+] Connect RemoteHost Succeed");
                }
                tunnel_sock_to_sock(clnt_sock, dest_sock);
            }
        }
    }
}