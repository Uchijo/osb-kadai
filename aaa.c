#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void exit_with_error(char *errorMessage, int line, char *file);

char *get_result(char *message) {
    int port = 5000;
    char *ip = "127.0.0.1";
    int len = strlen(message);
    int sock;

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        exit_with_error("", __LINE__, __FILE__);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        exit_with_error("", __LINE__, __FILE__);
    }

    if (send(sock, message, len, 0) != len) {
        exit_with_error("", __LINE__, __FILE__);
    }

    int num;
    char buf[512];
    if ((num = recv(sock, buf, 511, 0)) <= 0) {
        exit_with_error("", __LINE__, __FILE__);
    }

    buf[num] = '\0';

    close(sock);
    return buf;
}

// depends on rpc_t
int hoge(int a, int b, double c) {
    // generate message
    char *mes = "hoge 100 1 1.01";
    
    // get result
    char *result = get_result(mes);

    // parse result
    int ret;
    sscanf(result, "%d", &ret);
    return ret;
}

char *generate_hoge_message(int a, int b, double c) {
    char *mes = malloc(100 * sizeof(char));
    sprintf(mes, "hoge %d %d %f", a, b, c);
    return mes;
}
