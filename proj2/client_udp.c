#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>

#include <arpa/inet.h>

#define SERVERPORT "49152"

// Numero maximo de bytes que cada resposta pode conter
#define MAXDATASIZE 5000


char ** split(char * string, char delim){
  // Função auxiliar que recebe uma string e divide ela em arrays, dado um char separador
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

    printf("%s | %s | %s | %s | %s | %s | %s\n\n", all_info[0], all_info[1], all_info[2], all_info[3], all_info[4], all_info[5],all_info[6]);

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
//  char s[INET6_ADDRSTRLEN];
  int isClientLibrary = 0;

  char response[MAXDATASIZE]; // Buffer de resposta

  //informacao de endereco dos conectores
  struct sockaddr_storage their_addr;

  socklen_t addr_len=sizeof(their_addr) ;

  clock_t start, end;
  double elapsed, t1, t2;
  struct timeval tv1, tv2;

  // Arquivo onde será salvo os tempos de cada request
  FILE * relatorio;
  
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
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  
  // getaddrinfo() retorna uma lista de structs contendo endereços do tipo especificado em "hints". 
  if ((getaddrinfo(argv[1], SERVERPORT, &hints, &result)) != 0) {
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

      break;
  }

  if (rp == NULL) {
      //No address succeeded             
      printf("Could not bind\n");
      exit(0);
  }

  addr_len = sizeof their_addr;
  printMenu(isClientLibrary);
  
  int connected = 1; // controla a saída do loop de conexao
  char option[1]; // Armazena opcao escolhida 
  char buffer[20]; // Buffer para envio de requisicao
  char isbn[10];
  int buffersize;
  while(connected) {

    scanf("%s", option);

    buffer[0] = (option[0]);
    buffer[1] = '\0';

    printf("buffer: %s \n",buffer);
    
    switch ( buffer[0] ) {
      
      case '0' :
        // Sair

        connected = 0;
        break;

      case '1' :
        // Exibe id e titulo de todos os filmes

        gettimeofday(&tv1, NULL);
        t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;
        
        buffersize = strlen(buffer);

        if (sendto(sfd, buffer, buffersize, 0, rp->ai_addr, rp->ai_addrlen)==-1)
          perror("sendto()");

        if (recvfrom(sfd, response, MAXDATASIZE , 0, (struct sockaddr *)&their_addr, &addr_len) ==-1)
          perror("recvfrom()");

        listAllBooks(response);

        gettimeofday(&tv2, NULL);
        t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;
        
        // Calcula tempo gasto
        elapsed = t2 - t1;
        
        relatorio = fopen("relatorio_com_1.txt","a+");
        fprintf(relatorio, "%f\n", elapsed);
        fclose(relatorio);

        break;    
    
      case '2': 
        //Exibir descrição de um livro
        printf("Digite o isbn do LIVRO: ");
        scanf("%s", isbn);
        strcat(buffer,isbn);

        gettimeofday(&tv1, NULL);
        t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

        buffersize = strlen(buffer);

        if (sendto(sfd, buffer, buffersize, 0, rp->ai_addr, rp->ai_addrlen)==-1)
          perror("sendto()");

        if (recvfrom(sfd, response, MAXDATASIZE , 0, (struct sockaddr *)&their_addr, &addr_len)==-1)
          perror("recvfrom()");

        gettimeofday(&tv2, NULL);
        t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;
        
        elapsed = t2 - t1;

        showBookDesc(response);
        
        relatorio = fopen("relatorio_com_2.txt","a+");
        fprintf(relatorio, "%f\n", elapsed);
        fclose(relatorio);

        break;
      
    case '3': 
      //Exibir todas informacoes de um livro
      printf("Digite o isbn do LIVRO: ");
      scanf("%s", isbn);
      strcat(buffer,isbn);

      gettimeofday(&tv1, NULL);
      t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;
            
      buffersize = strlen(buffer);

      if (sendto(sfd, buffer, buffersize, 0, rp->ai_addr, rp->ai_addrlen)==-1)
        perror("sendto()");

      if (recvfrom(sfd, response, MAXDATASIZE , 0, (struct sockaddr *)&their_addr, &addr_len)==-1)
        perror("recvfrom()");

	    showBookDesc(response);

      gettimeofday(&tv2, NULL);
      t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;
      
      elapsed = t2 - t1;
      
      relatorio = fopen("relatorio_com_3.txt","a+");
      fprintf(relatorio, "%f\n", elapsed);
      fclose(relatorio);

      break;
      
    
    case '4':
      //Exibir todas informacoes de todos os livros

      gettimeofday(&tv1, NULL);
      t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

      buffersize = strlen(buffer);

      if (sendto(sfd, buffer, buffersize, 0, rp->ai_addr, rp->ai_addrlen)==-1)
        perror("sendto()");

      if (recvfrom(sfd, response, MAXDATASIZE , 0, (struct sockaddr *)&their_addr, &addr_len)==-1)
        perror("recvfrom()");

      listAllBooksInfo(response);

      gettimeofday(&tv2, NULL);
      t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;
      
      // Calcula tempo gasto
      elapsed = t2 - t1;
      
      relatorio = fopen("relatorio_com_4.txt","a+");
      fprintf(relatorio, "%f\n", elapsed);
      fclose(relatorio);

      break;

      
	  case '5':
      //Exibir a quantidade de um livro
      printf("Digite o isbn do LIVRO: ");
      scanf("%s", isbn);
      strcat(buffer,isbn);


      gettimeofday(&tv1, NULL);
      t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

      
      buffersize = strlen(buffer);

      if (sendto(sfd, buffer, buffersize, 0, rp->ai_addr, rp->ai_addrlen)==-1)
        perror("sendto()");

      if (recvfrom(sfd, response, MAXDATASIZE , 0, (struct sockaddr *)&their_addr, &addr_len)==-1)
        perror("recvfrom()");
      

      showBookDesc(response);

      gettimeofday(&tv2, NULL);
      t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;
      
      // Calcula tempo gasto
      elapsed = t2 - t1;
      
      relatorio = fopen("relatorio_com_5.txt","a+");
      fprintf(relatorio, "%f\n", elapsed);
      fclose(relatorio);

      break;

      
	  case '6':
	    //Alterar a quantidade de um livro
      printf("Digite o isbn do LIVRO: ");
      scanf("%s", isbn);

      gettimeofday(&tv1, NULL);
      t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

      
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

      buffersize = strlen(buffer);
      
      if (sendto(sfd, buffer, buffersize, 0, rp->ai_addr, addr_len)==-1)
          perror("sendto()");


      gettimeofday(&tv2, NULL);
      t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;
      
      elapsed = t2 - t1;
      
      relatorio = fopen("relatorio_com_6.txt","a+");
      fprintf(relatorio, "%f\n", elapsed);
      fclose(relatorio);

      break;
    
    case 'p' :
      printMenu(isClientLibrary);
      break;

    default:
      printf("Opção inválida. Digite novamente:");
      getchar();
      break;
    }
    
  }
  freeaddrinfo(result); // all done with this structure
  close(sfd);
  
  return 0;
}
