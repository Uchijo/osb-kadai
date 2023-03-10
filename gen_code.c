#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gen_code.h"

void exit_with_error(char *errorMessage, int line, char *file);

char *generate_server(rpc_t_list *rpc_data) {
    char *fragment1 = malloc(FRAGMENT_LENGTH * sizeof(char));
    char *fragment2;

    fragment1[0] = '\0';
    fragment2 =
"#include <arpa/inet.h>\n"
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"#include <string.h>\n"
"#include <sys/socket.h>\n"
"#include <unistd.h>\n"
"\n"
"#include \"tools/message_manager.h\"\n"
"void exit_with_error(char *errorMessage, int line, char *file);\n"
"void client_handler(int sock);\n"
"\n"
"// headers for rpc functions\n\0"
;

    append_to_last(fragment1, fragment2, FRAGMENT_LENGTH);
    for (int i=0; i<rpc_data->length; i++) {
        append_to_last(fragment1, generate_header(rpc_data->rpc[i]), FRAGMENT_LENGTH);
    }

    fragment2 =
""
"int main() {\n"
"    int port = 5000;\n"
"    struct sockaddr_in client;\n"
"    struct sockaddr_in server;\n"
"    int server_sock;\n"
"    int client_sock;\n"
"\n"
"    if ((server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {\n"
"        exit_with_error(\"unable to get socket server.\", __LINE__, __FILE__);\n"
"    }\n"
"\n"
"    server.sin_family = AF_INET;\n"
"    server.sin_addr.s_addr = htonl(INADDR_ANY);\n"
"    server.sin_port = htons(port);\n"
"\n"
"    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {\n"
"        exit_with_error(\"failed to bind socket\", __LINE__, __FILE__);\n"
"    }\n"
"\n"
"    if (listen(server_sock, 5) < 0) {\n"
"        exit_with_error(\"\", __LINE__, __FILE__);\n"
"    }\n"
"\n"
"    for (;;) {\n"
"        int size = sizeof(client);\n"
"        if ((client_sock =\n"
"                 accept(server_sock, (struct sockaddr *)&client, &size)) < 0) {\n"
"            exit_with_error(\"\", __LINE__, __FILE__);\n"
"        }\n"
"\n"
"        client_handler(client_sock);\n"
"    }\n"
"    return 1;\n"
"}\n"
"\n"
;

    strcat(fragment1, fragment2);
    for (int i=0; i<rpc_data->length; i++) {
        strcat(fragment1, request_handler_generator(rpc_data->rpc[i]));
    }

    fragment2 = 
"void client_handler(int sock) {\n"
"    char buf[1024];\n"
"    int mes_size;\n"
"\n"
"    if ((mes_size = recv(sock, buf, 1024 * sizeof(char), 0)) < 0) {\n"
"        exit_with_error(\"\", __LINE__, __FILE__);\n"
"    }\n"
"    buf[mes_size] = '\\0';\n"
"\n"
"    printf(\"message received: %s\\n\", buf);\n"
"\n"
"    func_call *func_data = decode_func_call(buf);\n"
"    // varies on .rpch file\n"
;
    strcat(fragment1, fragment2);

    // deal with handler invocation
    for (int i=0; i<rpc_data->length; i++) {
        strcat(fragment1, generate_handler_invoker(rpc_data->rpc[i]));
    }

    fragment2 =
"    free_func_call(func_data);\n"
"\n"
"    printf(\"closing connection on socket %d\\n\", sock);\n"
"    close(sock);\n"
"    puts(\"closed!\\n\");\n"
"}\n"
"\n"
;
    strcat(fragment1, fragment2);

    return fragment1;
}

char *generate_client(rpc_t_list *rpc_data) {
    char *fragment1 = malloc(FRAGMENT_LENGTH * sizeof(char));
    char *fragment2 = malloc(FRAGMENT_LENGTH * sizeof(char));

    fragment1[0] = '\0';
    char *tmp =
"#include <arpa/inet.h>\n"
"#include <stdio.h>\n"
"#include <stdlib.h>\n"
"#include <string.h>\n"
"#include <sys/socket.h>\n"
"#include <unistd.h>\n"
"\n"
"void exit_with_error(char *errorMessage, int line, char *file);\n"
"\n"
"char *get_result(char *message) {\n"
"    int port = 5000;\n"
"    char *ip = \"127.0.0.1\";\n"
"    int len = strlen(message);\n"
"    int sock;\n"
"\n"
"    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {\n"
"        exit_with_error(\"\", __LINE__, __FILE__);\n"
"    }\n"
"\n"
"    struct sockaddr_in addr;\n"
"    addr.sin_family = AF_INET;\n"
"    addr.sin_addr.s_addr = inet_addr(ip);\n"
"    addr.sin_port = htons(port);\n"
"\n"
"    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {\n"
"        exit_with_error(\"\", __LINE__, __FILE__);\n"
"    }\n"
"\n"
"    if (send(sock, message, len, 0) != len) {\n"
"        exit_with_error(\"\", __LINE__, __FILE__);\n"
"    }\n"
"\n"
"    int num;\n"
"    char *buf = malloc(sizeof(char) * 512);\n"
"    if ((num = recv(sock, buf, 511, 0)) <= 0) {\n"
"        exit_with_error(\"\", __LINE__, __FILE__);\n"
"    }\n"
"\n"
"    buf[num] = '\\0';\n"
"\n"
"    close(sock);\n"
"    return buf;\n"
"}\n\n";
    append_to_last(fragment1, tmp, FRAGMENT_LENGTH);

    // generate function itself
    for (int i=0; i<rpc_data->length; i++) {
        rpc_t *elem = rpc_data->rpc[i];
        sprintf(fragment2, "%s %s(\n", elem->return_type, elem->name);
        append_to_last(fragment1, fragment2, FRAGMENT_LENGTH);
        for (int i=0; i<elem->args_length; i++) {
            sprintf(fragment2, "    %s %s", elem->args[i].type, elem->args[i].name);
            append_to_last(fragment1, fragment2, FRAGMENT_LENGTH);
            if (i != elem->args_length - 1) {
                append_to_last(fragment1, ",\n", FRAGMENT_LENGTH);
            }
        }
        append_to_last(fragment1, "\n) {\n", FRAGMENT_LENGTH);
        append_to_last(fragment1, generate_message_generator(elem), FRAGMENT_LENGTH);
        append_to_last(fragment1, "    char *retval = get_result(message);\n", FRAGMENT_LENGTH);
        append_to_last(fragment1, generate_retval_decoder(elem), FRAGMENT_LENGTH);
        append_to_last(fragment1, "}\n\n", FRAGMENT_LENGTH);
    }

    return fragment1;
}

char *generate_server_template(rpc_t_list *rpc_data) {
    char *accum = malloc(FRAGMENT_LENGTH * sizeof(char));

    accum[0] = '\0';
    for (int i=0; i<rpc_data->length; i++) {
        append_to_last(accum, generate_function(rpc_data->rpc[i]), FRAGMENT_LENGTH);
        append_to_last(accum, "\n", FRAGMENT_LENGTH);
    }

    return accum;
}

char *generate_client_header(rpc_t_list *rpc_data) {
    char *accum = malloc(FRAGMENT_LENGTH * sizeof(char));

    accum[0] = '\0';
    for (int i=0; i<rpc_data->length; i++) {
        append_to_last(accum, generate_header(rpc_data->rpc[i]), FRAGMENT_LENGTH);
        append_to_last(accum, "\n", FRAGMENT_LENGTH);
    }

    return accum;
}
