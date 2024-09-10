#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTA 8080
#define TAM_BUFFER 1024
#define MAXCONN_BACKLOG 512

int main() {
	int sockid = socket(AF_INET, SOCK_STREAM, 0);

	if (sockid < 0) {
		printf("Erro ao criar o socket\n");
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

    while (1) {
        int socklen = sizeof(struct sockaddr_in);
        int accept_id = accept(sockid, (struct sockaddr *)&cliente, (socklen_t *)&socklen);
        if (accept_id < 0) {
            printf("Erro ao aceitar conexão\n");

            close(accept_id);
            close(sockid);
            exit(EXIT_FAILURE);
        }

        char recebido[TAM_BUFFER];
        while (1) {
            int tam_msg_recebido =
                recv(
                        accept_id,
                        (char *)recebido,
                        TAM_BUFFER,
                        0
                    );

            if (tam_msg_recebido < 0) {
                printf("Erro ao receber mensagem\n");

                continue;
            }


            // Adiciona o caractere '\0' para marcar o final da string
            recebido[tam_msg_recebido] = '\0'; 

            // Caso o cliente queira desconectar-se
            if (strcmp(recebido, "sair") == 0 || strcmp(recebido, "") == 0) {
                break;
            }

            printf("Recebido de %s:%d - %s\n", inet_ntoa(cliente.sin_addr), ntohs(cliente.sin_port), recebido);

            int send_status =
                send(
                        accept_id,
                        (const char *)recebido,
                        strlen(recebido),
                        0
                    );

            if (send_status < 0) {
                printf("Erro ao ecoar mensagem\n");

                continue;
            }

            printf("Mensagem ecoada para o cliente.\n");
        }

        close(accept_id);
    }

    close(sockid);

	return 0;
}
