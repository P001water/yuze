#include "public.h"
#include<getopt.h> 

struct option long_options[] = {
    {"help",        no_argument,       0, 'h'},
    {"version",     no_argument,       0, 'v'},
    {"mode",           required_argument, 0, 'm'},
    {"listenport",  required_argument, 0, 'l'},
    {"refhost",     required_argument, 0, 'r'},
    {"refport",     required_argument, 0, 's'},
    {"desthost",    required_argument, 0, 'd'},
    {"destport",    required_argument, 0, 'e'},
    {0, 0, 0, 0} // This must be the last element in the array
};

void banner() {
    printf(" _   _ _   _ _______ \n");
    printf("| | | | | | |_  / _ \\\n");
    printf("| |_| | |_| |/ /  __/\n");
    printf(" \\__, |\\__,_/___\\___|\n");
    printf("  __/ |              \n");
    printf(" |___/               \n");
    printf("                      [+] Error_Report@github P001");
    printf("\n\n");
}


/*
* main func and parse commandline is here
*/
int main(int argc, char* argv[], const char** envp) {
    socket_api_init(); // windows platform init socket api call
    tunnel_Pool_init();
    banner();

    while ((flag = getopt_long(argc, argv, "hvm:l:r:s:d:e:", long_options, NULL)) != EOF) {
        switch (flag) {
        case 'h':
            puts("[+] no time to help");
            exit(EXIT_SUCCESS);
        case 'v':
            printf("\nVERSION : %s \n\n", "yuze 1.0"); break;
        case 'm':
            if (!strcmp("s_server", optarg)) {
                action = 1;
                break;
            }
            if (!strcmp("r_server", optarg)) {
                action = 2;
                break;
            }
            if (!strcmp("yuze_listen", optarg)) {
                action = 3;
                break;
            }
            if (!strcmp("yuze_tran", optarg)) {
                action = 4;
                break;
            }
            if (!strcmp("yuze_slave", optarg)) {
                action = 5;
                break;
            }
        case 'l':
            listenPort = (int)strtol(optarg, NULL, 10);
            break;
        case 'r':
            strcpy(refHost, optarg, sizeof(refHost) - 1);
            refHost[sizeof(refHost) - 1] = '\0'; // 确保终结符
            break;
        case 's':
            refPort = atol(optarg);
            break;
        case 'd':
            strcpy(destHost, optarg, sizeof(destHost) - 1);
            destHost[sizeof(destHost) - 1] = '\0';
            //printf("%s", connHost);
            break;
        case 'e':
            destPort = atol(optarg);
            break;
        case 't':
            puts("set time is no longer supported");
            break;
        default:
            puts("[-] No input Check action");
            exit(EXIT_FAILURE);
        }
    }

    // main action is here 注意break的语序
    switch (action)
    {
    case 1:
        create_socks_server(listenPort);
        break;
    case 2:
        create_rsocks_client(refHost, refPort);
        break;
    case 3:
        yuze_listen(refPort, destPort);
        break;
    case 4:
        yuze_tran(refPort, destHost, destPort);
        break;
    case 5:
        yuze_slave(refHost, refPort, destHost, destPort);
        break;
    default:
        puts("[-] No input  Learn more and use");
        exit(EXIT_FAILURE);
    }
}


