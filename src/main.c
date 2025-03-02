#include "public.h"

#ifdef _WIN32
#include "getopt.h"
#else
#include <getopt.h>
#endif // _WIN32

typedef struct {
    int action;
    int listenPort;
    const char* forward_address; // Change to string for IP:PORT or PORT
    const char* connect_server;
    int socksport;
} ProxyConfig;

char* user = NULL;  // Username
char* password = NULL;  // Password

struct option long_options[] = {
    {"help",        no_argument,       0, 'h'},
    {"version",     no_argument,       0, 'v'},
    {"listenport",  required_argument, 0, 'l'},
    {"connect_server", required_argument, 0, 'c'},
    {"socksport", required_argument, 0, 's'},
    {"user", required_argument, 0, 'u'},
    {"password", required_argument, 0, 'p'},
    {"forward_addr", required_argument, 0, 'f'},
    {0, 0, 0, 0}
};

void banner() {
    printf(" _   _ _   _ _______ \n");
    printf("| | | | | | |_  / _ \\\n");
    printf("| |_| | |_| |/ /  __/\n");
    printf(" \\__, |\\__,_/___\\___|\n");
    printf("  __/ |              \n");
    printf(" |___/               \n");
    printf("                      [+] Version %s\n", YUZE_VERSION);
    printf("                      [+] Error_Report@github P001\n\n");
}

void show_help() {
    puts("Usage: yuze [options]");
    puts("Options:");
    puts("  -h, --help            Show this help message and exit");
    puts("  -v, --version         Show version information");
    puts("  -l, --listenport      Set listening port");
    puts("  -u, --user            username for auth");
    puts("  -p, --password        password for auth");
    puts("  -f, --forward_addr    Set forward addr");
    puts("  -c, --connect_server  Set reverse proxy connect server");
    puts("  -s, --socksport       Set SOCKS port for Reverse Proxy");
}

int main(int argc, char* argv[]) {
    socket_api_init();
    tunnel_Pool_init();
    banner();

    ProxyConfig config = { 0 };

    if (argc < 2) {
        fprintf(stderr, "[-] Missing mode argument\n");
        show_help();
        exit(EXIT_FAILURE);
    }

    const char* mode = argv[1];
    if (strcmp(mode, "proxy") == 0) config.action = 1;
    else if (strcmp(mode, "reverse") == 0) config.action = 2;
    else if (strcmp(mode, "fwd") == 0) config.action = 3;
    else if (strcmp(mode, "rev") == 0) config.action = 7;
    else {
        fprintf(stderr, "[-] Unknown mode: %s\n", mode);
        show_help();
        exit(EXIT_FAILURE);
    }

    int flag;
    while ((flag = getopt_long(argc, argv, "hvl:c:s:u:p:f:", long_options, NULL)) != -1) {
        switch (flag) {
        case 'h':
            show_help();
            exit(EXIT_SUCCESS);
        case 'u':  // User
            user = optarg;
            break;
        case 'p':  // Password
            password = optarg;
            break;
        case 'v':
            printf("\nVERSION : %s \n\n", "yuze.0.1.0");
            exit(EXIT_SUCCESS);
        case 'l':
            config.listenPort = (int)strtol(optarg, NULL, 10);
            break;
        case 'f':
            config.forward_address = optarg;
            break;
        case 'c':
            config.connect_server = optarg;
            break;
        case 's':
            config.socksport = (int)strtol(optarg, NULL, 10);
            break;
        default:
            show_help();
            exit(EXIT_FAILURE);
        }
    }


    switch (config.action) {
    case 1:
        start_proxy(config.listenPort, user, password);
        break;
    case 2:
        reverseProxy(config.listenPort, config.socksport, config.connect_server, user, password);
        break;
    case 3:
        port_forward(config.listenPort, config.forward_address);
        break;
    default:
        fprintf(stderr, "[-] Invalid action specified\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
