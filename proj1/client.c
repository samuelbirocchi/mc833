//=========================
// MC823 - Laboratorio de Redes de Computadores - Projeto 1
//
// Nome: Davi   RA: 097464
// Nome: Fabio  RA: 073048
//=========================

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

void printMenu(int isClientLibrary) {
  printf("\n\n******************************************************\n");
  printf("Catálogo de livros! Entre com uma das opções abaixo e pressione ENTER\n\n");
  printf("p - Imprimir esse menu\n");
  printf("0 - Sair\n");
  printf("1 - Listar ISBN e título de todos os livros\n");
  printf("2 - Exibir descrição de um livro\n");
  printf("3 - Exibir todas informacoes de um livro\n");
  printf("4 - Exibir todas informacoes de todos os livros\n");
  printf("5 - Exibir a quantidade de um livro\n");
  if (isClientLibrary) {
    printf("6 - Alterar a quantidade de um livro\n");
  }
  printf("******************************************************\n");
}

void listAllBooks(char response[]) { 
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

void listAllBooksInfo (char response[]) { 
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

void showBookDesc(char response[]){

  printf("%s \n", response);

}


int main(int argc, char* argv[]) {
    // File descriptor do socket
    int sfd;  
    struct addrinfo hints, *result, *rp;
    int rv;
    char s[INET6_ADDRSTRLEN];
    int isClientLibrary = 0;
    
    char response[MAXDATASIZE]; // Buffer de resposta

    int ativo;

    
    if (argc <= 2) {
        fprintf(stderr,"usage: client hostname usertype\n");
        exit(1);
    }

    if (argc == 3) {
      if ((strcmp(argv[2],"library") == 0)) {
        isClientLibrary = 1;
      }
    }
  
    // hints define o tipo de endereço que estamos procurando no getaddrinfo
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */

    /* getaddrinfo() retorna uma lista de structs contendo endereços do tipo especificado em "hints". */
    if ((getaddrinfo(argv[1], PORT, &hints, &result)) != 0) {
        perror("Erro getaddrinfo\n");
        exit(0);
    }
  
    // Percorre todos os endereços encontrados no getaddrinfo
    // Faz o "bind" para o primeiro socket criado com sucesso
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        // socket() retorna um inteiro similar a um descritor de arquivos relacionado ao socket criado, 
        // através do qual ele pode ser referenciado
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
        /* No address succeeded */              
        printf("Could not bind\n");
        exit(0);
    }

    freeaddrinfo(result); // all done with this structure
    
    // char buf[100]; int numbytes;
    // if ((numbytes=recv(sfd, buf, 100, 0)) == -1) {
    //     perror("recv");
    //     exit(1);
    // }
    // printf("%s \n", buf);

    printMenu(isClientLibrary);

    int connected = 1;
    char option[1]; // Armazena opcao escolhida 
    char buffer[20]; // Buffer para envio de requisicao
    char isbn[10];
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

          //Listar ISBN e título de todos os livros
          case '1' :

            send(sfd, buffer, 12, 0);

            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
              perror("recv");
              exit(1);
            }
            listAllBooks(response);
            break;

          //Exibir descrição de um livro
          case '2' :
            printf("Digite o isbn do LIVRO: ");
            scanf("%s", isbn);
            strcat(buffer,isbn);

            send(sfd, buffer, 12, 0);

            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
               perror("recv");
               exit(1);
            }

            showBookDesc(response);
            break;

          //Exibir todas informacoes de um livro
          case '3' :
            printf("Digite o isbn do LIVRO: ");
            scanf("%s", isbn);
            strcat(buffer,isbn);

            send(sfd, buffer, 12, 0);

            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
               perror("recv");
               exit(1);
            }

            showBookDesc(response);

          break;

          //Exibir todas informacoes de todos os livros
          case '4' :
            send(sfd, buffer, 12, 0);
            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
              perror("recv");
              exit(1);
            }
            listAllBooksInfo(response);
            break;

          break;

          //Exibir a quantidade de um livro
          case '5' :
            printf("Digite o isbn do LIVRO: ");
            scanf("%s", isbn);
            strcat(buffer,isbn);

            send(sfd, buffer, 12, 0);

            if (recv(sfd, response, MAXDATASIZE, 0) == -1) {
               perror("recv");
               exit(1);
            }

            showBookDesc(response);

          break;

          //Alterar a quantidade de um livro
          case '6' :
            printf("Digite o isbn do LIVRO: ");
            scanf("%s", isbn);

            strcat(buffer,"|");
            strcat(buffer,isbn);
            strcat(buffer,"|");

            printf("Digite a nova quantidade: ");
            scanf("%s", isbn);

            strcat(buffer,isbn);
            strcat(buffer,"|");

            char str[1];
            sprintf(str, "%d", isClientLibrary);
            strcat(buffer,str);

            send(sfd, buffer, 20, 0);

          break;

          case 'p' :
            printMenu(isClientLibrary);
            break;

          default:
            printf("Opção do cliente inválida\n");
            break;
          }
    }
        close(sfd);

        return 0;
    }

    /* 
  int ativo = 0;
  while(ativo) {

    scanf("%c", &option);
    buffer[0] = option;
    buffer[1] = '\0';

    switch ( option ) {
    case '1' :
      // Exibe id e titulo de todos os filmes
      send(sfd, buffer, 6, 0);
      recv(sfd, response, MAXDATASIZE, 0);
      listAllMovies(response);
      getchar();
      break;
      
    case '2': 
      // Exibe a sinopse do filme
      printf("Digite o número do filme: ");
      scanf("%s", aux);
      strcat(buffer,aux);
      send(sfd, buffer, 6, 0);
      recv(sfd, response, MAXDATASIZE, 0);
      printf("\n=======================================================\n");
      printf("\n\nSINOPSE: %s\n", response);
      printf("\n=======================================================\n");
      getchar();
      break;
      
    case '3': 
      // Exibe todas as informacoes de um filme
      printf("Digite o número do filme: ");
      scanf("%s", aux);
      strcat(buffer,aux);
      send(sfd, buffer, 6, 0);
      recv(sfd, response, MAXDATASIZE, 0);
      findMovieById(response);
      getchar();
      break;
      
    case '4':
      // Exibe todas as informacoes de todos os filmes
      send(sfd, buffer, 6, 0);
      recv(sfd, response, MAXDATASIZE, 0);
      findAllMovies(response);
      getchar();
      break;
      
    case '5':
      // Envia solicitacao de encerramento de conexao
      send(sfd, buffer, 6, 0);
      ativo = 0;
      break;

    default:
      printf("Opção inválida. Digite novamente:");
      getchar();
      break;
    }
    
  }
  
  close(sfd);
  
  return 0;
}
*/