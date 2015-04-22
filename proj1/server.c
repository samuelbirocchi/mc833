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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "aux_functions.h"

#define PORT "49152"  // porta de entrada dos clientes
#define BACKLOG 10   // define o numero maximo de conexoes pendentes

#define NUMBER_MOVIES 13
#define MAX_SIZE_BUF_TITLES 331

/**
 *Tamanho do buffer maximo de envio de todas as infos de todos os
 * filmes: 3 caracteres -> [2(id) + '|' + 20(titulo) + '|' + 300(sinopse)
 * + '|' + 17(horarios) + '|' + 1(sala) + '|'] * 13 = 4485 caracteres
 * + '\0'= 4486
 */
#define MAX_SIZE_BUF_ALL_MOVIES 4486

/**
 * Tamanho do buffer maximo de envio de todas as infos de um unico filme
 *  346 caracteres 
 *  -> 2(id) + '|' 
 *  + 20(titulo) + '|' 
 *  + 300(sinopse) + '|'
 *  + 17 (horarios) + '|' 
 *  + 1(sala) + '|' 
 *  = 345 caracteres + '\0' = 346
*/
#define MAX_SIZE_BUF_INFO_MOVIE 346

/**
 * Tamanho maximo do buffer para envio de sinopse de um filme
 */
#define MAX_SIZE_BUF_SYNOPSIS 301

#define FIELD_SEP "|"
#define REG_SEP "\n"
#define END_STRING '\0'


/**
 *   struct que contem os dados do filme: Id, Titulo, Sinopse, Horario
 *  de exibicao e Sala de exibicao 
 * Obs: todos os vetores tem um bytes a
 * mais, devido ao fato da funcao fgets() armazenar tambem um '\0' ao
 * final da string (os vetores com tamanho variavel possuem ainda um
 * byte a mais, para o caso de terem o maximo de caracteres, mais a
 * quebra de linha
*/
typedef struct movieStruct {
  char id[3];		// ID - 2 caracteres
  char titulo[22];	// Titulo - no maximo 20 caracteres
  char sinopse[302];	// Sinopse - no maximo 300 caracteres
  char horarios[18];	// Horario - XX:XX (5 caracteres) * 3 horarios = 15 caracteres
  char sala[2];		// Sala de Exibicao - 1 caracter
} Movie;


// vetor de structs 'filme', aonde serao armazenados os filmes
// carregados a partir do arquivo
Movie filmes[NUMBER_MOVIES];

void loadBooksFromDB(){
  int i=0;

  // Array que armazenara cada linha do arquivo de livros, para tratar
  // cada campos posteriormente
  char * lineRead;

  // Matriz onde cada posicao equivale a um campo de um registro do
  // arquivo de livros
  char ** matrixConfig;

  // Carrega todos os filmes contidos no arquivo 'movies.txt' para a
  // memoria (total de 13 filmes)
  FILE *arqFilmes = NULL;

  arqFilmes = fopen("movies.txt","r");
  
  // Le a primeira linha do arquivo de filmes
  lineRead = readFileBySeparator(arqFilmes, '\n');
 
  // Para cada linha do arquivo (que corresponde a um filme) separa os
  // campos e usa cada um deles para preencher a estrutura de filmes
  // adequadamente
  while(lineRead != NULL) {
    
    matrixConfig = split(lineRead, '|');

    strcpy(filmes[i].id, matrixConfig[0]);
    strcpy(filmes[i].titulo, matrixConfig[1]);
    strcpy(filmes[i].sinopse, matrixConfig[2]);
    strcpy(filmes[i].sala, matrixConfig[3]);
    strcpy(filmes[i].horarios, matrixConfig[4]);
    i++;

    free(matrixConfig);

    lineRead = readFileBySeparator(arqFilmes, '\n');
   }

  free(lineRead);

  fclose(arqFilmes);

}


/**
 *  Retorna ao cliente todos os Ids dos filmes com seus respectivos
 *  titulos
 * @param new_fd id do socket que recebeu a requisicao
*/
void getAllMovieTitles(int new_fd) {
  char buffer[MAX_SIZE_BUF_TITLES];
  int i;

  buffer[0] = END_STRING;
  for(i=0; i < NUMBER_MOVIES; i++) {
    strcat(buffer,filmes[i].id);
    strcat(buffer, FIELD_SEP);
    strcat(buffer,filmes[i].titulo);
    strcat(buffer, REG_SEP);
  }

  send(new_fd, buffer, MAX_SIZE_BUF_TITLES, 0);
}


/**
 * Envia ao cliente todas as informacoes de todos os filmes.  
 *
 * @param new_fd socket da conexao que recebeu a requisicao
*/
void getAllMovies(int new_fd) {
  char buffer[MAX_SIZE_BUF_ALL_MOVIES];
  int i;

  buffer[0] = END_STRING;
  for(i = 0; i < NUMBER_MOVIES; i++) {
  	
    strcat(buffer,filmes[i].id);
    strcat(buffer,FIELD_SEP);
    strcat(buffer,filmes[i].titulo);
    strcat(buffer,FIELD_SEP);
    strcat(buffer,filmes[i].sinopse);
    strcat(buffer,FIELD_SEP);
    strcat(buffer,filmes[i].sala);
    strcat(buffer,FIELD_SEP);
    strcat(buffer,filmes[i].horarios);

    if(i != NUMBER_MOVIES - 1)
      strcat(buffer,REG_SEP);
  }
  
  send(new_fd, buffer, MAX_SIZE_BUF_ALL_MOVIES, 0);
}

/**
 *  Retorna todas as informacoes de um filme especificado pelo Id que o
 *  cliente enviou
 *
 * @param new_fd socket da conexao que fez a requisicao
 * @param opt array que contem o id do filme do qual se quer as infos
*/
void getMovieById(int new_fd, char opt[]) {
  char buffer[MAX_SIZE_BUF_INFO_MOVIE];
  char opt2[3];
  int index_movie;
    
  opt2[0] = opt[1];
  opt2[1] = opt[2];
  opt2[2] = END_STRING;

  index_movie = atoi(opt2) - 1;

  buffer[0] = END_STRING;

  strcat(buffer, filmes[index_movie].id);
  strcat(buffer, FIELD_SEP);
  strcat(buffer, filmes[index_movie].titulo);
  strcat(buffer, FIELD_SEP);
  strcat(buffer, filmes[index_movie].sinopse);
  strcat(buffer, FIELD_SEP);
  strcat(buffer, filmes[index_movie].sala);
  strcat(buffer, FIELD_SEP);
  strcat(buffer, filmes[index_movie].horarios);
  
  send(new_fd, buffer, MAX_SIZE_BUF_INFO_MOVIE, 0);
}

/**
 * Retorna a sinopse do filme especificado pelo Id que o cliente enviou
 *
 * @param new_fd socket que realizou a requisicao
 * @param opt array que contem o id do filme do qual a sinopse deve 
 *            ser enviada
 */
void getMovieSynById(int new_fd, char opt[]) {
  char buffer[MAX_SIZE_BUF_SYNOPSIS];
  char opt2[3];
  
  opt2[0] = opt[1];
  opt2[1] = opt[2];
  opt2[2] = END_STRING;

  buffer[0] = END_STRING;
  strcat(buffer, filmes[atoi(opt2)-1].sinopse);
  strcat(buffer, REG_SEP);
  
  send(new_fd, buffer, MAX_SIZE_BUF_SYNOPSIS, 0);
}


/**
 * Pega todos os processos 'mortos'
 * 
 * @param s 
 */
void sigchld_handler(int s) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

/**
 * Define se o sockaddr é IPv4 ou IPv6
 *
 * @param sa socket
 */
void *get_in_addr(struct sockaddr *sa) {

    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * Principal
 */
int main(int argc, char * argv[]) {
  
  //sockfd refere-se à escuta, new_fd a novas conexoes
  int sockfd, new_fd;  			
  struct addrinfo hints, *servinfo, *p;

  //informacao de endereco dos conectores
  struct sockaddr_storage their_addr; 	

  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  int ativo; // Booleano que indica se a conexao deve continuar ativa
  
  // Vetor que contera a opcao do cliente (mais o id do filme, se for o
  // caso)
  char opt[4];		                

  loadBooksFromDB();
    
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM; // Stream socket
  hints.ai_flags = AI_PASSIVE;
  
  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  
  // Percorre a lista ligada e realiza 'bind' ao primeiro que for possivel
  // Cria todos os file descriptors dos sockets, dando nome a eles
  for(p = servinfo; p != NULL; p = p->ai_next) {

    //Função SOCKET: cria um socket, dando acesso ao serviço da camada de transporte
    if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    
    //Função bind: atribui um nome ao socket
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    
    break;
  }
  
  // Debug de erro
  if (p == NULL)  {
    fprintf(stderr, "servidor: falha ao realizar 'bind'\n");
    return 2;
  }
  
  // Necessario devido à chamada 'getaddrinfo' acima
  freeaddrinfo(servinfo); 
  
  // Anuncia que está apto para receber conexões
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
  
  printf("servidor: aguardando conexoes...\n");
  
  // Loop de aceitacao principal
  while(1) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      // perror("accept");
      continue;
    }
    
    inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("servidor: conexao obtida de %s\n", s);
    
    // Processo filho
    if (!fork()) { 			  

      // Processo filho nao precisa da escuta
      close(sockfd); 			  

      ativo = 1;
      while(ativo){

      	// Recebe a opcao do client
      	if(recv(new_fd,opt, 4, 0) == -1); 

	      perror("recv");

	       switch(opt[0]){
  	case '1':			 
  	  // Listar todos os Ids dos filmes com seus respectivos
  	  // titulos
  	  getAllMovieTitles(new_fd);
  	  break;
   				
  	case '2': 
  	  // Dado o Id de um filme, retornar a sinopse
  	  getMovieSynById(new_fd, opt);  
  	  break;

  	case '3':			  
  	  // Dado o Id de um filme, retornar todas as informações
  	  // desse filme
  	  getMovieById(new_fd, opt);
  	  break;

  	case '4':
  	  // Listar todas as informações de todos os filmes;			  
  	   getAllMovies(new_fd);
  	  break;

  	case '5':
  	  // Finaliza conexao
  	  ativo = 0;
  	  break;
  	default:
  	  printf("Opcao nao valida. Tente novamente\n");
  	  break;
	       }
      }
      close(new_fd);
      exit(0);
    }

    // processo pai nao precisa da nova conexao
    close(new_fd); 
  }
  
  return 0;
}
