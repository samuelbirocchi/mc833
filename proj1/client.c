// Projeto 1 - MC833
// Matheus Pinheiro - RA 119920
// Samuel Birocchi - RA 104052 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Porta efêmera que o cliente usará para se conectar
#define PORT "49152" 

// Numero maximo de bytes que cada resposta pode conter
#define MAXDATASIZE 5000

#define REG_SEP '\n'
#define FIELD_SEP '|'

char ** split(char * string, char delim){

  char * pch;
  char *temp = &delim;
  char ** vector;

  vector = (char **) malloc (sizeof(char *) * MAXDATASIZE);

  pch = strtok (string,temp);
  int i = 0;
  while (pch != NULL) {
    vector[i] = pch;
    pch = strtok (NULL, temp);
    i++;
  }
  return vector;

}

void printMenu(int isClientLocadora) {
  printf("\n\n******************************************************\n");
  printf("Escolha uma das opcoes e pressione ENTER\n\n");
  printf("p - Imprimir menu\n");
  printf("0 - Sair\n");
  printf("1 - Listar titulo e ano de lancamento de todos os filmes\n");
  printf("2 - Listar titulo e ano de lancamento de todos os filmes de um genero\n");
  printf("3 - Exibir sinopse de um filme\n");
  printf("4 - Exibir a quantidade de um filme\n");
  printf("5 - Exibir todas as informacoes de um filme\n");
  printf("6 - Listar informacoes de todos os filmes\n");
  if (isClientLocadora) {
    printf("7 - Alterar a quantidade de um filme\n");
  }
  printf("******************************************************\n");
}

// Titulo e ano
void listMovies(char response[]) { 
  char ** temp;
  char ** all_books;
  char ** id_isbn_title;
  int i = 0;

  if (*response != '\0') { 

    temp = split(response, '#');

    int len = atoi(temp[0]); 
    all_books = split(temp[1], '\n');


    for(i = 0; i < len; i++) {
      id_isbn_title = split(all_books[i], '|');
      
      printf("%s | %s\n", id_isbn_title[0], id_isbn_title[1]);

      free(id_isbn_title);
    }
    free(all_books);
    free(temp);
  }
}

void showMovieSynopsis(char response[]){
  printf("%s \n", response);
}

void listAllMovies (char response[]) { 
  char ** temp;
  char ** all_books;
  char ** all_info;
  int i = 0;

  temp = split(response, '#');

  int len = atoi(temp[0]); 
  all_books = split(temp[1], '\n');

  for(i = 0; i < len; i++) {
    all_info = split(all_books[i], '|');
    
    printf("%s | %s | %s | %s | %s | %s\n\n", all_info[0], all_info[1], all_info[2], all_info[3], all_info[4], all_info[5],all_info[6]);

    free(all_info);
  }
  free(all_books);
  free(temp);
}

int main(int argc, char* argv[]) {
    // File descriptor do socket
    int sfd;  
    struct addrinfo hints, *result, *rp;
    int rv;
    char s[INET6_ADDRSTRLEN];
    int isClientLocadora = 0;
    char response[MAXDATASIZE]; // Buffer de resposta
    int ativo;
    
    if (argc <= 2) {
        fprintf(stderr,"usage: client hostname usertype\n");
        exit(1);
    }

    if (argc == 3) {
      if ((strcmp(argv[2],"locadora") == 0)) {
        isClientLocadora = 1;
      }
    }
  
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */

    if ((getaddrinfo(argv[1], PORT, &hints, &result)) != 0) {
        perror("Erro getaddrinfo\n");
        exit(0);
    }
  
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1){
            perror("client: socket");
            continue;
        }

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == -1) {
            close(sfd);
            perror("client: connect");
            continue;    
        }

        break;
    }

    if (rp == NULL) {
        printf("Could not bind\n");
        exit(0);
    }

    freeaddrinfo(result);

    printMenu(isClientLocadora);

    int connected = 1;
    char option[1]; // Armazena opcao escolhida 
    char buffer[20]; // Buffer para envio de requisicao
    char id[10];
    while (connected) {
        
        scanf("%s", &option);
        buffer[0] = (option[0]);
        buffer[1] = '\0';

        switch ( buffer[0] ) {

          // Sair
          case '0' :
            send(sfd, buffer, MAXDATASIZE, 0);
            connected = 0;
            break;

          //Listar titulo e ano de todos os filmes
          case '1' :
            send(sfd, buffer, 12, 0);

            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
              perror("recv");
              exit(1);
            }
            listMovies(response);
            break;

          //Listar titulo e ano dos filmes de um genero
          case '2' :
            printf("Digite o genero: ");
            scanf("%s", id);
            strcat(buffer,id);

            send(sfd, buffer, 12, 0);

            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
              perror("recv");
              exit(1);
            }
            listMovies(response);
            break;

          //Exibir sinopse de um filme
          case '3' :
            printf("Digite o id do filme: ");
            scanf("%s", id);
            strcat(buffer,id);

            send(sfd, buffer, 12, 0);

            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
               perror("recv");
               exit(1);
            }

            showMovieSynopsis(response);
            break;

          //Exibir a quantidade de um filme
          case '4' :
            printf("Digite o id do filme: ");
            scanf("%s", id);
            strcat(buffer,id);

            send(sfd, buffer, 12, 0);

            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
               perror("recv");
               exit(1);
            }

            showMovieSynopsis(response);
            break;

          //Exibir todas informacoes de um filme
          case '5' :
            printf("Digite o id do filme: ");
            scanf("%s", id);
            strcat(buffer,id);

            send(sfd, buffer, 12, 0);

            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
               perror("recv");
               exit(1);
            }

            listAllMovies(response); // TALVEZ PRECISE ALTERAR
            break;

          //Exibir todas informacoes de todos os filmes
          case '6' :
            send(sfd, buffer, 12, 0);
            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
              perror("recv");
              exit(1);
            }
            listAllMovies(response);
            break;

          //Alterar a quantidade de um filme
          case '7' :
            printf("Digite o id do filme: ");
            scanf("%s", id);

            strcat(buffer,"|");
            strcat(buffer,id);
            strcat(buffer,"|");

            printf("Digite a nova quantidade: ");
            scanf("%s", id);

            strcat(buffer,id);
            strcat(buffer,"|");

            char str[1];
            sprintf(str, "%d", isClientLocadora);
            strcat(buffer,str);

            send(sfd, buffer, 20, 0);
            break;

          case 'p' :
            printMenu(isClientLocadora);
            break;

          default:
            printf("Opção do cliente inválida\n");
            break;
          }
    }
    
    close(sfd);
    return 0;
}
