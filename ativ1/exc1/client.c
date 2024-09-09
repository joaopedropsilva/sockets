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
    int sockid = socket(AF_INET, SOCK_DGRAM, 0);

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

    char enviado[TAM_BUFFER];
    char mensagem_original[TAM_BUFFER];

    while (1) {
        printf("Digite uma mensagem (ou 'sair' para encerrar): ");
        fgets(enviado, TAM_BUFFER, stdin);

        // Remover quebra de linha
        enviado[strcspn(enviado, "\n")] = 0;

        if (strcmp(enviado, "sair") == 0) {
            printf("Encerrando o cliente.\n");
            break;
        }

        strcpy(mensagem_original, enviado); 
        sendto(
            sockid,
            (const char *)enviado, 
            strlen(enviado),
            0,
            (const struct sockaddr *)&servidor,
            sizeof(servidor)
        );


        int len = sizeof(servidor);
        int tam_msg_recebido =
            recvfrom(
                sockid,
                (char *)enviado,
                TAM_BUFFER,
                0,
                (struct sockaddr *)&servidor,
                &len
            );

        if (tam_msg_recebido < 0) {
            printf("Erro ao receber resposta.\n");
            continue;
        } 

        // Adiciona o caractere '\0' para marcar o final da string
        enviado[tam_msg_recebido] = '\0'; 

        printf("Servidor: %s\n", enviado);
    }

    close(sockid);

    return 0;
}



