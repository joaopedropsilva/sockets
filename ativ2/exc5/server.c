#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORTA 8080
#define TAM_BUFFER 1024
#define MAXCONN_BACKLOG 512


int main(void) {
    // Criação do socket TCP receptor
    int sockid = socket(AF_INET, SOCK_STREAM, 0);

    if (sockid < 0) {
        printf("Falha ao abrir o socket do servidor\n");
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
    int socklen = sizeof(struct sockaddr);
    int accept_id;
    while (1) {
        // Aceitando conexões no socket do servidor
        accept_id =
            accept(
                sockid,
                (struct sockaddr *)&cliente,
                (socklen_t *)&socklen
            );

        if (accept_id < 0) {
            printf("Erro ao aceitar conexão\n");

            continue;
        }

        // Criação de processos filhos para atender as conexões
        int pid = fork();

        if (pid < 0) {
            printf("Erro ao criar um socket para atender a conexão\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // Fechando o socket ouvinte no processo filho
            close(sockid);

            while (1) {
                // Tentativa de recepção de uma mensagem
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

                // Marcação de final da string no buffer de recebido
                recebido[tam_mensagem_recebido] = '\0';

                // Verificação de desconexão do cliente
                // "" abrange o caso de suspensão forçada do processo cliente
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

                // Tratativa para a solicitação de dados do apache
                if (strcmp(recebido, "data_apache") == 0) {
                    // Declaração do comando de recuperação de dados
                    char ls_command[] = "ls /var/www/html/";

                    // Abrindo um pipe que recebe as informações
                    // de retorno do comando executado
                    FILE* pipe = popen(ls_command, "r");

                    if (pipe == NULL) {
                        // Em caso de erro repassa uma mensagem ao cliente
                        char msg_erro[] = "Falha ao recuperar dados do Apache";

                        // Limpando o buffer de recebido
                        memset(recebido, 0, TAM_BUFFER);
                        // Copiando a mensagem de erro para o buffer
                        strcpy(recebido, msg_erro);
                    } else {
                        // Limpando o buffer de recebido
                        memset(recebido, 0, TAM_BUFFER);
                        // Definindo espaçamento para uma melhor 
                        // visualização pelo cliente
                        recebido[0] = '\n';
                        recebido[1] = '\t';

                        // Declaração de um buffer para recuperação de
                        // dados da stream recebida no comando
                        char buffer[TAM_BUFFER];
                        // Leitura da stream de dados recebida
                        while (fgets(buffer, TAM_BUFFER, pipe) != NULL) {
                            // Se o tamanho da leitura recebida exceder
                            // o máximo do buffer o resultado é truncado 
                            if (strlen(recebido) + strlen(buffer) > TAM_BUFFER)
                                break;

                            // Substituição da quebra de linha
                            // para melhorar a visualização
                            buffer[strcspn(buffer, "\n")] = ' ';
                            // Concatenação de cada dado recebido
                            // num buffer composto
                            strcat(recebido, buffer);
                        }
                    }

                    int r = pclose(pipe);
                    if (r < 0) {
                        printf("Erro ao fechar o pipe\n");
                        exit(EXIT_FAILURE);
                    }
                } else if (strcmp(recebido, "status_connection") == 0) {
                    // Declaração do comando de recuperação de dados
                    char netstat_command[40];
                    sprintf(
                        netstat_command,
                        "netstat -an | grep %d | sed 's/.* //'",
                        ntohs(cliente.sin_port)
                    );

                    // Abrindo um pipe que recebe as informações
                    // de retorno do comando executado
                    FILE* pipe = popen(netstat_command, "r");

                    if (pipe == NULL) {
                        // Em caso de erro repassa uma mensagem ao cliente
                        char msg_erro[] = "Falha ao recuperar dados da conexão";

                        // Limpando o buffer de recebido
                        memset(recebido, 0, TAM_BUFFER);
                        // Copiando a mensagem de erro para o buffer
                        strcpy(recebido, msg_erro);
                    } else {
                        // Limpando o buffer de recebido
                        memset(recebido, 0, TAM_BUFFER);

                        // Declaração de um buffer para recuperação de
                        // dados da stream recebida no comando
                        char buffer[TAM_BUFFER];
                        // Leitura da stream de dados recebida
                        while (fgets(buffer, TAM_BUFFER, pipe) != NULL) {
                            // Se o tamanho da leitura recebida exceder o
                            // máximo do buffer o resultado é truncado
                            if (strlen(recebido) + strlen(buffer) > TAM_BUFFER)
                                break;

                            // Substituição da quebra de linha por
                            // um marcador para futuro tratamento
                            // da string recebida
                            buffer[strcspn(buffer, "\n")] = '_';
                            // Concatenação de cada dado recebido
                            // num buffer composto
                            strcat(recebido, buffer);
                        }

                        // Marcação dee final de string no buffer de mensagem
                        recebido[strlen(recebido) - 1] = 0;
                        // Substituição do marcador adicionado pela
                        // string desejada no retorno
                        // A string recebida pelo resultado do comando repetia
                        // o estado da conexão como: "ESTABLISHED_ESTABLISHED"
                        // Essa é uma maneira de tratar essa questão
                        char* separador = strrchr(recebido, '_');
                        char* copia = separador ? separador + 1 : recebido;

                        // Copiando a string final para o buffer de saída
                        strcpy(recebido, copia);
                    }

                    int r = pclose(pipe);
                    if (r < 0) {
                        printf("Erro ao fechar o pipe\n");
                        exit(EXIT_FAILURE);
                    }
                }

                // Tentativa de envio da mensagem ao cliente
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

                printf("Mensagem enviada para o cliente.\n");
            }
        }

        // Fechamento da conexão para o cliente
        close(accept_id);
    }

    close(sockid);

    return 0;
}

