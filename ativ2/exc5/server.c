#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTA 8080
#define TAM_BUFFER 1024
#define MAXCONN_BACKLOG 18

int main(void) {
    int sockid = socket(AF_INET, SOCK_STREAM, 0);
    if (sockid < 0) {
        printf("Falha ao abrir o socket do servidor\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servidor;
    struct sockaddr_in cliente;

    bzero(&(servidor), sizeof(servidor));
    bzero(&(cliente), sizeof(cliente));

	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(PORTA);
	servidor.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockid, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
		printf("Erro ao fazer bind\n");

		close(sockid);
		exit(EXIT_FAILURE);
	}

    if (listen(sockid, MAXCONN_BACKLOG) == -1) {
		printf("Erro ao fazer listen\n");

		close(sockid);
		exit(EXIT_FAILURE);
    }

    
    char recebido[TAM_BUFFER];
    int socklen = sizeof(struct sockaddr);
    int accept_id;
    while (1) {
        accept_id = accept(sockid, (struct sockaddr *)&cliente, (socklen_t *)&socklen);
        if (accept_id < 0) {
            printf("Erro ao aceitar conexão\n");

            continue;
        }

        int pid = fork();
        if (pid < 0) {
            printf("Erro ao criar um socket para atender a conexão\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            close(sockid);

            while (1) {
                int tam_mensagem_recebido =
                    recv(
                            accept_id,
                            (char *)recebido,
                            TAM_BUFFER,
                            0
                        );

                if (tam_mensagem_recebido < 0) {
                    printf("Erro ao receber a mensagem do cliente\n");

                    continue;
                }

                recebido[tam_mensagem_recebido] = '\0';

                if (strcmp(recebido, "sair") == 0) {
                    close(accept_id);
                    exit(EXIT_SUCCESS);
                } else if (strcmp(recebido, "") == 0) {
                    close(accept_id);
                    exit(EXIT_FAILURE);
                }

                printf(
                        "Recebido de %s:%d - %s\n",
                        inet_ntoa(cliente.sin_addr),
                        ntohs(cliente.sin_port),
                        recebido
                      );

                if (strcmp(recebido, "data_apache") == 0) {
                    char ls_command[] = "ls /var/www/";

                    FILE* pipe = popen(ls_command, "r");

                    if (pipe == NULL) {
                        char msg_erro[] = "Falha ao recuperar dados do Apache";

                        memset(recebido, 0, TAM_BUFFER);
                        strcpy(recebido, msg_erro);
                    } else {
                        memset(recebido, 0, TAM_BUFFER);
                        recebido[0] = '\n';
                        recebido[1] = '\t';

                        char buffer[TAM_BUFFER];
                        while (fgets(buffer, TAM_BUFFER, pipe) != NULL) {
                            if (strlen(recebido) + strlen(buffer) > TAM_BUFFER)
                                break;

                            buffer[strcspn(buffer, "\n")] = ' ';
                            strcat(recebido, buffer);
                        }
                    }

                    int r = pclose(pipe);
                    if (r < 0) {
                        printf("Erro ao fechar o pipe\n");
                        exit(EXIT_FAILURE);
                    }
                } else if (strcmp(recebido, "status_connection") == 0) {
                    char netstat_command[40];
                    sprintf(
                        netstat_command,
                        "netstat -an | grep %d | sed 's/.* //'",
                        ntohs(cliente.sin_port)
                    );

                    FILE* pipe = popen(netstat_command, "r");

                    if (pipe == NULL) {
                        char msg_erro[] = "Falha ao recuperar dados da conexão";

                        memset(recebido, 0, TAM_BUFFER);
                        strcpy(recebido, msg_erro);
                    } else {
                        memset(recebido, 0, TAM_BUFFER);

                        char buffer[TAM_BUFFER];
                        while (fgets(buffer, TAM_BUFFER, pipe) != NULL) {
                            if (strlen(recebido) + strlen(buffer) > TAM_BUFFER)
                                break;

                            buffer[strcspn(buffer, "\n")] = '_';
                            strcat(recebido, buffer);
                        }

                        recebido[strlen(recebido) - 1] = 0;
                        char* separador = strrchr(recebido, '_');
                        char* copia = separador ? separador + 1 : recebido;
                        strcpy(recebido, copia);
                    }

                    int r = pclose(pipe);
                    if (r < 0) {
                        printf("Erro ao fechar o pipe\n");
                        exit(EXIT_FAILURE);
                    }
                }
                int send_status =
                    send(
                            accept_id,
                            (const char *)recebido,
                            strlen(recebido),
                            0
                        );

                if (send_status < 0) {
                    printf("Erro ao ecoar a mensagem\n");

                    continue;
                }

                printf("Mensagem ecoada para o cliente.\n");
            }
        }

        close(accept_id);
    }

    close(sockid);

    return 0;
}

