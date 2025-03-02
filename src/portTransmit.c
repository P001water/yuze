#include "portTransmit.h"
#include "public.h" 


// -m yuze_tran -s 7878 -d 127.0.0.1 -e 9999 将7878 端口收到的socks代理请求转交给127.0.0.1打开正向监听的主机
int yuze_tran(int refPort, char* destHost, int destPort) {
    //int server_sock;
    //printf("[+] Proxy forward to RemoteHost 0.0.0.0:%d <--[yuze]--> %s:%d\n", refPort, destHost, destPort);

    //server_sock = create_tcp_listening_socket(refPort, 500);
    //for (;;) {
    //    fd_set readset;
    //    struct timeval tv = { 60,0 };
    //    FD_ZERO(&readset);
    //    FD_SET(server_sock, &readset);

    //    int selectRet = select(server_sock + 1, &readset, NULL, NULL, &tv);
    //    if (selectRet < 0)
    //    {
    //        puts("[-] yuze_tran -》 Select error");
    //        break;
    //    }
    //    else if (selectRet == 0)
    //    {
    //        puts("[-] yuze_tran -》 Select time out");
    //        continue;
    //    }
    //    else if (selectRet) {
    //        if (FD_ISSET(server_sock, &readset)) {
    //            int clnt_sock = socket_acceptClient(server_sock);
    //            if (clnt_sock == -1) {
    //                //puts("[+] Client Connected");
    //            }

    //            int dest_sock = socket_connect(destHost, destPort);
    //            if (dest_sock == -1) {
    //                //puts("[-] RemoteHost Connected closed");
    //                socket_close(dest_sock);
    //                exit(1);
    //            }
    //            else {
    //                //puts("[+] Connect RemoteHost Succeed");
    //            }
    //            tunnel_sock_to_sock(clnt_sock, dest_sock);
    //        }
    //    }
    //}
    return -1;
}


// -m yuze_slave -r 127.0.0.1 -s 8888 -d 127.0.0.1 -e 9999
int yuze_slave(char* ref_host, int ref_port, char* dest_host, int dest_port) {
//    int readSize;
//    char newSocketNotice[RSOCKET_SERVER_NOTICE_LEN];
//    slaveStructalias slaveConfig;
//    printf("[+] yuze_slave %s:%d <--[yuze]--> %s:%d\n", ref_host, ref_port, dest_host, dest_port);
//
//    int control_socket = rsocksClient_init_ControlSocket(ref_host, ref_port);
//    if (control_socket < 0) {
//        printf("[-] Can not get Control_socket connect\n");
//        socket_close(control_socket);
//        return -1;
//    }
//
//    for (;;) {
//        fd_set readset;
//        struct timeval tv = { 60,0 }; // set select func tiemout time 
//        int retval;
//
//        FD_ZERO(&readset);
//        FD_SET(control_socket, &readset);
//
//        int maxfd = control_socket + 1;
//        retval = select(maxfd, &readset, NULL, NULL, &tv);
//        if (retval == -1) {
//            perror("[-] yuze_slave func select() error");
//            exit(1);
//        }
//        else if (retval == 0)
//        {
//            puts("[-] wait time-out!");
//        }
//        else {
//            if (FD_ISSET(control_socket, &readset)) {
//                readSize = socket_recv(control_socket, newSocketNotice, RSOCKET_SERVER_NOTICE_LEN);
//                if (readSize == RSOCKET_SERVER_NOTICE_LEN && newSocketNotice[0] == True && newSocketNotice[1] == NEW_PROXY_SOCKET) {
//                    strncpy(slaveConfig.refHost, ref_host, 300);
//                    slaveConfig.refport = ref_port;
//                    strncpy(slaveConfig.destHost, dest_host, 300);
//                    slaveConfig.destport = dest_port;
//
//                    slaveConfig.tunnel_id = newSocketNotice[2] - '0' + 48;
//#ifndef WIN32
//                    pthread_t thread_id;
//#else
//                    HANDLE thread_id;
//#endif
//                    //creatThread_multi_Platform(thread_id, yuzeslave_build_tunnel, slaveConfig)
//                }
//                else
//                {
//                    printf("[-] Error Happended on %d\n", readSize);
//                    yuze_slave(ref_host, ref_port, dest_host, dest_port);
//                    return -1;
//                }
//            }
//        }
//    }
    return -1;
}

int yuzeslave_build_tunnel(slaveStructalias* slavesConfig) {
    //char rhost[300], dhost[300];
    //char NewSocketNoticebuff[RSOCKET_SERVER_NOTICE_LEN];

    //strncpy(rhost, slavesConfig->refHost, 300);
    //int rport = slavesConfig->refport;
    //strncpy(dhost, slavesConfig->destHost, 300);
    //int dport = slavesConfig->destport;
    //char tunnel_id = slavesConfig->tunnel_id;

    //int proxySocket = socket_connect(rhost, rport);
    //FillinSocketbuff(NewSocketNoticebuff);
    //NewSocketNoticebuff[0] = True;
    //NewSocketNoticebuff[1] = NEW_PROXY_SOCKET;
    //NewSocketNoticebuff[2] = tunnel_id;
    //int sendlen = socket_send(proxySocket, NewSocketNoticebuff, RSOCKET_SERVER_NOTICE_LEN);
    //if (sendlen != RSOCKET_SERVER_NOTICE_LEN) {
    //    puts("[-] ");
    //}

    //int dest_socket = socket_connect(dhost, dport);
    //if (dest_socket > 0) { // Check the sockets proto version
    //    tunnel_sock_to_sock(proxySocket, dest_socket);
    //}
    return -1;
}



