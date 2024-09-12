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
    // Criação do socket UDP transmissor
    int sockid = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockid < 0) {
        printf("Erro ao criar o socket\n");
        exit(EXIT_FAILURE);
    }

    // Inicialização da estrutura com informações do servidor
    struct sockaddr_in servidor;
    unsigned long endereco_servidor;

    bzero(&(servidor), sizeof(servidor));
    inet_pton(AF_INET, ENDERECO_SERVIDOR, &endereco_servidor);

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(PORTA);
    servidor.sin_addr.s_addr = endereco_servidor;

    // Instanciação de um buffer para a comunicação
    char enviado[TAM_BUFFER];
    while (1) {
        // Captura da mensagem do cliente a ser enviada
        printf("Digite uma mensagem (ou 'sair' para encerrar): ");
        fgets(enviado, TAM_BUFFER, stdin);

        // Remover quebra de linha da mensagem ao pressionar "Enter"
        enviado[strcspn(enviado, "\n")] = 0;

        // Verificação pela string de saída para parar a comunicação
        if (strcmp(enviado, "sair") == 0) {
            printf("Encerrando o cliente.\n");
            break;
        }

        // Envio da mensagem ao servidor
        sendto(
            sockid,
            (const char *)enviado, 
            strlen(enviado),
            0,
            (const struct sockaddr *)&servidor,
            sizeof(servidor)
        );


        // Recepção de uma resposta (eco) do servidor
        int len = sizeof(servidor);
        int tam_msg_recebido =
            recvfrom(
                sockid,
                (char *)enviado,
                TAM_BUFFER,
                0,
                (struct sockaddr *)&servidor,
                (socklen_t *)&len
            );

        // Marcação de final da string no buffer de recebido
        enviado[tam_msg_recebido] = '\0'; 

        printf("Servidor: %s\n", enviado);
    }

    close(sockid);

    return 0;
}


