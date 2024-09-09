#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTA 8080
#define TAM_BUFFER 1024

        
int main() {
    int sockid = socket(AF_INET, SOCK_DGRAM, 0);

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

    if (bind(sockid, (const struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
        printf("Erro ao fazer bind\n");

        close(sockid);
        exit(EXIT_FAILURE);
    }

    char enviado[TAM_BUFFER];

    while (1) {
        int len = sizeof(cliente);

        int tam_msg_recebido =
            recvfrom(
                sockid,
                (char *)enviado,
                TAM_BUFFER,
                0,
                (struct sockaddr *)&cliente,
                &len
            );

        // Adiciona o caractere '\0' para marcar o final da string
        enviado[tam_msg_recebido] = '\0'; 

        printf("Recebido de %s:%d - %s\n", inet_ntoa(cliente.sin_addr), ntohs(cliente.sin_port), enviado);

        sendto(
            sockid,
            (const char *)enviado,
            strlen(enviado),
            0,
            (const struct sockaddr *)&cliente,
            len
        );

        printf("Mensagem ecoada para o cliente.\n");
    }

    close(sockid);
    return 0;
}
