#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTA 8080
#define TAM_BUFFER 1024
#define MAX_TENTATIVAS 3 
#define ENDERECO_SERVIDOR "127.0.0.1" 

int main() {
	int sockid = socket(AF_INET, SOCK_STREAM, 0);

	if (sockid < 0) {
        printf("Erro ao criar o socket\n");
        exit(EXIT_FAILURE);
	}

	struct sockaddr_in servidor;
	unsigned long endereco_servidor;

	bzero(&(servidor), sizeof(servidor));
	inet_pton(AF_INET, ENDERECO_SERVIDOR, &endereco_servidor);

	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(PORTA);
	servidor.sin_addr.s_addr = endereco_servidor;

    // Estabelecendo a conexão com o servidor
    int con = connect(sockid, (struct sockaddr *)&servidor, sizeof(servidor));
    if (con < 0) {
        printf("\nErro ao conectar ao servidor\n");
        close(sockid);
        exit(EXIT_FAILURE);
    }

	char enviado[TAM_BUFFER];

	while (1) {
		printf("Digite uma mensagem (ou 'sair' para encerrar): ");
		fgets(enviado, TAM_BUFFER, stdin);

		// Remover quebra de linha
		enviado[strcspn(enviado, "\n")] = 0;

		if (strcmp(enviado, "sair") == 0) {
		    printf("Encerrando o cliente.\n");

            send(
                sockid,
                (const char *)enviado, 
                strlen(enviado),
                0
            );

            break;
		}

        int status_envio = -1;
        for (int i = 0; i < MAX_TENTATIVAS; i++) {
            if (status_envio > 0)
                break;

            status_envio =
                send(
                    sockid,
                    (const char *)enviado,
                    strlen(enviado),
                    0
                );
        }


        if (status_envio < 0) {
            printf(
                "Falha ao enviar a mensagem após %i tentativas!\n",
                MAX_TENTATIVAS
            );

            continue;
        }

        int tam_msg_recebido =
            recv(
                sockid,
                (char *)enviado,
                TAM_BUFFER,
                0
            );

        if (tam_msg_recebido < 0) {
            printf("Falha ao receber o eco!\n");
            continue;
        }
			
        // Adiciona o caractere '\0' para marcar o final da string
        enviado[tam_msg_recebido] = '\0';

        printf("Servidor: %s\n", enviado);
	}

	close(sockid);

	return 0;
}
