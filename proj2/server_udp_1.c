//=========================
// MC823 - Laboratorio de Redes de Computadores - Projeto 2
//
// Nome: Frank  RA: 070934
// Nome: Suelen RA: 072399
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
#include <time.h>
#include <netdb.h>

#include "aux_functions.h"

#define PORT "4950"  // a porta a qual os usuarios se conectarao
#define MAXBUFLEN 6


#define NUMBER_MOVIES 13
#define MAX_SIZE_BUF_TITLES 331

/**
 *Tamanho do buffer maximo de envio de todas as infos de todos os
 * filmes: 4486 caracteres -> [2(id) + '|' + 20(titulo) + '|' + 300(sinopse)
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

/**
 * Tamanho maximo do buffer para envio de confirmacao de nota dada a um filme
 * 2 caracteres
 * -> 1 (indica sucesso)
 * = 1 caractere + '\0' = 2
 */
#define MAX_SIZE_BUF_RATING 2

/**
 * Tamanho maximo do buffer para envio de media de um filme e clientes
 * que votaram
 * 6 caracteres
 * -> 2(nota)
 * + 2 (sinopse) + '|'
 * = 5 caracteres + '\0' = 6
 */
#define MAX_SIZE_BUF_AVG 6

/**
 * Tamanho do campo 'nota' de movieStruct
 */
#define SIZE_FIELD_NOTA 3

/**
 * Tamanho do campo 'clientesAval' de movieStruct
 */
#define SIZE_FIELD_CLIENTESAVAL 3


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
  char id[3];			// ID - 2 caracteres
  char titulo[22];		// Titulo - no maximo 20 caracteres
  char sinopse[302];	// Sinopse - no maximo 300 caracteres
  char horarios[18];	// Horario - XX:XX (5 caracteres) * 3 horarios = 15 caracteres
  char sala[2];			// Sala de Exibicao - 1 caracter
  char nota[3];			// Nota de um cliente a um determinado filme - 2 caracteres (00 a 10)
  char clientesAval[3];	// Numero de clientes que deram nota a um filme - 2 caracteres (00 a 99)
} Movie;


// vetor de structs 'filme', aonde serao armazenados os filmes
// carregados a partir do arquivo
Movie filmes[NUMBER_MOVIES];


/**
 *  Retorna ao cliente todos os Ids dos filmes com seus respectivos
 *  titulos
 * @param sockfd id do socket que recebeu a requisicao
 * @param their_addr endereco ip + porta do destino
 * @param addrlen tamanho da struct 'their_addr'
*/
void getAllMovieTitles(int sockfd, struct sockaddr_storage their_addr, int addr_len) {
  char buffer[MAX_SIZE_BUF_TITLES];
  int i;

  buffer[0] = END_STRING;
  for(i=0; i < NUMBER_MOVIES; i++) {
    strcat(buffer,filmes[i].id);
    strcat(buffer, FIELD_SEP);
    strcat(buffer,filmes[i].titulo);
    strcat(buffer, REG_SEP);
  }

  sendto(sockfd, buffer, MAX_SIZE_BUF_TITLES, 0, (struct sockaddr *)&their_addr, addr_len);
}


/**
 * Envia ao cliente todas as informacoes de todos os filmes.  
 *
 * @param sockfd socket da conexao que recebeu a requisicao
 * @param their_addr endereco ip + porta do destino
 * @param addrlen tamanho da struct 'their_addr'
*/
void getAllMovies(int sockfd, struct sockaddr_storage their_addr, int addr_len) {
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
  
  sendto(sockfd, buffer, MAX_SIZE_BUF_ALL_MOVIES, 0, (struct sockaddr *)&their_addr, addr_len);
}

/**
 *  Retorna todas as informacoes de um filme especificado pelo Id que o
 *  cliente enviou
 *
 * @param sockfd socket da conexao que fez a requisicao
 * @param opt array que contem o id do filme do qual se quer as infos
 * @param their_addr endereco ip + porta do destino
 * @param addrlen tamanho da struct 'their_addr'
*/
void getMovieById(int sockfd, char opt[], struct sockaddr_storage their_addr, int addr_len) {
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
  
  sendto(sockfd, buffer, MAX_SIZE_BUF_INFO_MOVIE, 0, (struct sockaddr *)&their_addr, addr_len);
}

/**
 * Retorna a sinopse do filme especificado pelo Id que o cliente enviou
 *
 * @param sockfd socket que realizou a requisicao
 * @param opt array que contem o id do filme do qual a sinopse deve 
 *            ser enviada
 * @param their_addr endereco ip + porta do destino
 * @param addrlen tamanho da struct 'their_addr'
 */
void getMovieSynById(int sockfd, char opt[], struct sockaddr_storage their_addr, int addr_len) {
  char buffer[MAX_SIZE_BUF_SYNOPSIS];
  char opt2[3];
  
  opt2[0] = opt[1];
  opt2[1] = opt[2];
  opt2[2] = END_STRING;

  buffer[0] = END_STRING;
  strcat(buffer, filmes[atoi(opt2)-1].sinopse);
  strcat(buffer, REG_SEP);
  
  sendto(sockfd, buffer, MAX_SIZE_BUF_SYNOPSIS, 0, (struct sockaddr *)&their_addr, addr_len);
}

/**
 * Dá nota a um filme especificado pelo Id que o cliente enviou e atualiza sua media
 *
 * @param sockfd socket que realizou a requisicao
 * @param opt array que contem o id do filme ao qual se deve dar nota
 * @param their_addr endereco ip + porta do destino
 * @param addrlen tamanho da struct 'their_addr'
 */
void rateMovieById(int sockfd, char opt[], struct sockaddr_storage their_addr, int addr_len) {
  char buffer[MAX_SIZE_BUF_RATING];
  char opt_index[3], opt_rating[3];
  int index_movie, rate, clientesAval, oldRating;
  
  opt_index[0] = opt[1];
  opt_index[1] = opt[2];
  opt_index[2] = END_STRING;
  index_movie = atoi(opt_index-1);
  opt_rating[0] = opt[3];
  opt_rating[1] = opt[4];
  opt_rating[2] = END_STRING;
  rate = atoi(opt_rating-1);
  
  oldRating = atoi(filmes[index_movie].nota);
  clientesAval = atoi(filmes[index_movie].clientesAval);  
  clientesAval++;
  
  rate = (oldRating + rate)/clientesAval;
  snprintf(filmes[index_movie].nota, SIZE_FIELD_NOTA, "%d", rate);
  filmes[index_movie].nota[3] = END_STRING;
  snprintf(filmes[index_movie].clientesAval, SIZE_FIELD_CLIENTESAVAL, "%d", clientesAval);
  filmes[index_movie].clientesAval[3] = END_STRING;

  buffer[0] = '1';
  buffer[1] = END_STRING;
  sendto(sockfd, buffer, MAX_SIZE_BUF_RATING, 0, (struct sockaddr *)&their_addr, addr_len);
}

/**
 * Retorna a (media da) nota de um filme especificado pelo Id que o cliente enviou e o 
 * numero de clientes que votaram
 *
 * @param sockfd socket que realizou a requisicao
 * @param opt array que contem o id do filme do qual a nota e numero de clientes 
 *            devem ser enviados
 * @param their_addr endereco ip + porta do destino
 * @param addrlen tamanho da struct 'their_addr'
 */
void getRatingById(int sockfd, char opt[], struct sockaddr_storage their_addr, int addr_len) {
  char buffer[6];
  char opt2[3];
  int index_movie;
      
  opt2[0] = opt[1];
  opt2[1] = opt[2];
  opt2[2] = END_STRING;
  index_movie = atoi(opt2-1);

  buffer[0] = END_STRING;
  strcat(buffer, filmes[index_movie].nota);
  strcat(buffer, FIELD_SEP);
  strcat(buffer, filmes[index_movie].clientesAval);
  
  sendto(sockfd, buffer, MAX_SIZE_BUF_AVG, 0, (struct sockaddr *)&their_addr, addr_len);
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
  int sockfd;	
  struct addrinfo hints, *servinfo, *p;
  int rv, numbytes;
  //informacao de endereco dos conectores
  struct sockaddr_storage their_addr;
  char buf[MAXBUFLEN];
  size_t addr_len;
  char s[INET6_ADDRSTRLEN];
  int i=0;
  int ativo; // Booleano que indica se a conexao deve continuar ativa

  clock_t start, end;
  double elapsed, t1, t2;
  struct timeval tv1, tv2;

  // arquivo que armazenara os tempos de processamento de requisicao
  FILE * relatorio;
  
  // Vetor que contera a opcao do cliente (mais o id do filme e a nota dada ao mesmo, se for o
  // caso)
  char opt[6];		                

  // Array que armazenara cada linha do arquivo de filmes, para tratar
  // cada campos posteriormente
  char * lineRead;

  // Matriz onde cada posicao equivale a um campo de um registro do
  // arquivo de filmes
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
  //fim do carregamento de filmes
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  
  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  
  // Percorre a lista ligada e realiza 'bind' ao primeiro que for
  // possivel
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
			 p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }
    
    break;
  }
  
  if (p == NULL)  {
    fprintf(stderr, "listener: falha ao realizar 'bind'\n");
    return 2;
  }
  
  // Necessario devido à chamada 'getaddrinfo' acima
  freeaddrinfo(servinfo); 
  
  printf("listener: esperando chamada 'recvfrom'...\n");

  

  /*
  printf("listener: got packet from %s\n",
      inet_ntop(their_addr.ss_family,
          get_in_addr((struct sockaddr *)&their_addr),
          s, sizeof s));
  printf("listener: packet is %d bytes long\n", numbytes);
  buf[numbytes] = '\0';
  printf("listener: packet contains \"%s\"\n", buf);
  */
  
  

  return 0;
  
  ativo = 1;
  while(ativo){
	
	// Recebe a opcao do client
	addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, opt, MAXBUFLEN , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    } 
    else {
	  
	  switch(opt[0]){
	  case '1':			 
	    // Registra tempo antes de processar a requisicao	    
	    gettimeofday(&tv1, NULL);
	    t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

	    // Listar todos os Ids dos filmes com seus respectivos
	    // titulos
	    getAllMovieTitles(sockfd, their_addr, addr_len); 
	    
	    // Registra tempo apos processar requisicao
	    gettimeofday(&tv2, NULL);
	    t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;

	    // Calcula tempo gasto
	    elapsed = t2 - t1;
	    
	    // Armazena resultado em arquivo
	    relatorio = fopen("relatorio_1.txt","a+");
	    fprintf(relatorio, "%f\n", elapsed);
	    fclose(relatorio);

	    break;
	    
	  case '2': 
	    // Registra tempo antes de processar a requisicao	    
	    gettimeofday(&tv1, NULL);
	    t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

	    // Dado o Id de um filme, retornar a sinopse
	    getMovieSynById(sockfd, opt, their_addr, addr_len);  

	    // Registra tempo apos processar requisicao
	    gettimeofday(&tv2, NULL);
	    t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;

	    // Calcula tempo gasto
	    elapsed = t2 - t1;
	    
	    // Armazena resultado em arquivo
	    relatorio = fopen("relatorio_2.txt","a+");
	    fprintf(relatorio, "%f\n", elapsed);
	    fclose(relatorio);

	    break;
	    
	  case '3':
	    // Registra tempo antes de processar a requisicao	    
	    gettimeofday(&tv1, NULL);
	    t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

	    // Dado o Id de um filme, retornar todas as informações
	    // desse filme
	    getMovieById(sockfd, opt, their_addr, addr_len);

	    // Registra tempo apos processar requisicao
	    gettimeofday(&tv2, NULL);
	    t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;

	    // Calcula tempo gasto
	    elapsed = t2 - t1;
	    
	    // Armazena resultado em arquivo
	    relatorio = fopen("relatorio_3.txt","a+");
	    fprintf(relatorio, "%f\n", elapsed);
	    fclose(relatorio);
	    break;
	    
	  case '4':
	     // Registra tempo antes de processar a requisicao	    
	    gettimeofday(&tv1, NULL);
	    t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

	    // Listar todas as informações de todos os filmes;			  
	    getAllMovies(sockfd, their_addr, addr_len);

	    // Registra tempo apos processar requisicao
	    gettimeofday(&tv2, NULL);
	    t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;

	    // Calcula tempo gasto
	    elapsed = t2 - t1;
	    
	    // Armazena resultado em arquivo
	    relatorio = fopen("relatorio_4.txt","a+");
	    fprintf(relatorio, "%f\n", elapsed);
	    fclose(relatorio);

	    break;
	    
	  case '5':
	     // Registra tempo antes de processar a requisicao	    
	    gettimeofday(&tv1, NULL);
	    t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

	    // Dar nota a um filme e atualizar sua media		  
	    rateMovieById(sockfd, opt, their_addr, addr_len);

	    // Registra tempo apos processar requisicao
	    gettimeofday(&tv2, NULL);
	    t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;

	    // Calcula tempo gasto
	    elapsed = t2 - t1;
	    
	    // Armazena resultado em arquivo
	    relatorio = fopen("relatorio_5.txt","a+");
	    fprintf(relatorio, "%f\n", elapsed);
	    fclose(relatorio);

	    break;
	    
	  case '6':
	     // Registra tempo antes de processar a requisicao	    
	    gettimeofday(&tv1, NULL);
	    t1 = (double)(tv1.tv_sec) + (double)(tv1.tv_usec)/ 1000000.00;

	    // Retorna a nota de um filme e quantos clientes votaram		  
	    getRatingById(sockfd, opt, their_addr, addr_len);

	    // Registra tempo apos processar requisicao
	    gettimeofday(&tv2, NULL);
	    t2 = (double)(tv2.tv_sec) + (double)(tv2.tv_usec)/ 1000000.00;

	    // Calcula tempo gasto
	    elapsed = t2 - t1;
	    
	    // Armazena resultado em arquivo
	    relatorio = fopen("relatorio_6.txt","a+");
	    fprintf(relatorio, "%f\n", elapsed);
	    fclose(relatorio);

	    break;
	    
	  case '7':
	    // Finaliza conexao
	    ativo = 0;
	    break;
	  default:
	    printf("Opcao nao valida %c. Tente novamente\n", opt[0]);
	    break;
	  }
	  
	}
  }
      close(sockfd);
      return 0;
}
