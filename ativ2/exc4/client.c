#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTA 8080
#define TAM_BUFFER 1024
#define MAX_TENTATIVAS 3 
#define ENDERECO_SERVIDOR "127.0.0.1" 


int main(void) {
    // Criação do socket TCP do transmissor
	int sockid = socket(AF_INET, SOCK_STREAM, 0);

	if (sockid < 0) {
        printf("Erro ao criar o socket\n");
        exit(EXIT_FAILURE);
	}

    // Inicialização das estruturas do servidor e cliente
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

    // Criação de buffers para a comunicação
	char enviado[TAM_BUFFER];
	char mensagem_original[TAM_BUFFER];
	while (1) {
        // Captura da mensagem do cliente a ser enviada
		printf("Digite uma mensagem (ou 'sair' para encerrar): ");
		fgets(enviado, TAM_BUFFER, stdin);

        // Remover quebra de linha da mensagem ao pressionar "Enter"
		enviado[strcspn(enviado, "\n")] = 0;

        // Guardando uma cópia da mensagem a ser enviada
		strcpy(mensagem_original, enviado); 

        // Verificação pela string de saída para parar a comunicação
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

		int tentativas = 0;
		int confirmacao = 0;
        // Loop para tentar ao máximo MAX_TENTATIVAS enviar e receber o eco
		while (tentativas < MAX_TENTATIVAS) {
            // Tentativa de envio da mensagem ao servidor
			int status_envio =
				send(
					sockid,
					(const char *)enviado, 
					strlen(enviado),
					0
				);

			if (status_envio < 0) {
				tentativas++;

				printf(
					"Erro ao enviar a mensagem. Tentativa %d de %d.\n",
					tentativas,
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

            // Tentativa de uma recepção de resposta (eco) do servidor
			if (tam_msg_recebido < 0) {
				tentativas++;

				printf(
					"Erro ao enviar a mensagem. Tentativa %d de %d.\n",
					tentativas,
					MAX_TENTATIVAS
				);

				continue;
			}
			
            // Marcação de final da string no buffer de recebido
			enviado[tam_msg_recebido] = '\0'; 

			printf("Servidor: %s\n", enviado);

            // Verificação se o eco recebido condiz com a mensagem original
			if (strcmp(enviado, mensagem_original) == 0) {
				confirmacao = 1;
				break;

            // Caso não esteja de acordo aponta a divergência
			} else {
				tentativas++;

				printf(
					"Divergência na mensagem. Tentativa %d de %d.\n",
					tentativas,
					MAX_TENTATIVAS
				);
			}
		}
		 
        // Verificação se houve sucesso nas tentativas
		if (!confirmacao) {
			printf(
				"Falha ao receber a mensagem correta após %d tentativas.\n",
				MAX_TENTATIVAS
			);
		}
	}

	close(sockid);

	return 0;
}
