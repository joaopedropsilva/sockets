#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTA 8080
#define TAM_BUFFER 1024
#define MAXCONN_BACKLOG 3


int main() {
    // Criação do socket TCP receptor
	int sockid = socket(AF_INET, SOCK_STREAM, 0);

	if (sockid < 0) {
		printf("Erro ao criar o socket\n");
		exit(EXIT_FAILURE);
	}

    // Inicialização das estruturas do servidor e cliente
	struct sockaddr_in servidor; 
	struct sockaddr_in cliente; 

	bzero(&(servidor), sizeof(servidor));
	bzero(&(cliente), sizeof(cliente));

	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(PORTA);
	servidor.sin_addr.s_addr = INADDR_ANY;

    // Fazendo bind na porta definida para o servidor
	if (bind(sockid, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
		printf("Erro ao fazer bind\n");

		close(sockid);
		exit(EXIT_FAILURE);
	}

    // Colocando o socket passivo para ouvir e disponibilizar
    // um máximo de conexões numa fila (backlog)
    if (listen(sockid, MAXCONN_BACKLOG) == -1) {
		printf("Erro ao fazer listen\n");

		close(sockid);
		exit(EXIT_FAILURE);
    }

    // Buffer para o conteúdo da comunicação
    char recebido[TAM_BUFFER];
    while (1) {
        // Aceitando conexões no socket do servidor
        int socklen = sizeof(struct sockaddr_in);
        int accept_id =
            accept(
                sockid,
                (struct sockaddr *)&cliente,
                (socklen_t *)&socklen
            );

        if (accept_id < 0) {
            printf("Erro ao aceitar conexão\n");

            close(accept_id);
            close(sockid);
            exit(EXIT_FAILURE);
        }

        while (1) {
            // Tentativa de recepção de uma mensagem
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


            // Marcação de final da string no buffer de recebido
            recebido[tam_msg_recebido] = '\0'; 

            // Verificação de desconexão do cliente
            // "" abrange o caso de suspensão forçada do processo cliente
            if (strcmp(recebido, "sair") == 0 || strcmp(recebido, "") == 0) {
                break;
            }

            printf(
                "Recebido de %s:%d - %s\n",
                inet_ntoa(cliente.sin_addr),
                ntohs(cliente.sin_port),
                recebido
            );

            // Tentativa de envio do eco
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

        // Fechamento da conexão para o cliente
        close(accept_id);
    }

    close(sockid);

	return 0;
}
