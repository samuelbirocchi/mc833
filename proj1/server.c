// Projeto 1 - MC833
// Matheus Pinheiro - RA 119920
// Samuel Birocchi - RA 104052 

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

#define PORT "49152" 
#define BACKLOG 10   

#define NUMBER_MOVIES 10
#define MAX_SIZE_BUF_TITLES 221

#define MAX_SIZE_BUF_ALL_MOVIES 3540

#define MAX_SIZE_BUF_INFO_MOVIE 354

#define MAX_SIZE_BUF_SYNOPSIS 301

#define FIELD_SEP "|"
#define REG_SEP "\n"
#define END_STRING '\0'


typedef struct movieStruct {
  char id[3];		
  char titulo[22];	
  char sinopse[302];	
  char genero[18];	
  char quantidade[3];	
  char ano[6];
} Movie;


Movie filmes[NUMBER_MOVIES];

void loadMovies(){
  int i=0;

  
  
  char * lineRead;

  
  
  char ** matrixConfig;

  
  
  FILE *arqFilmes = NULL;

  arqFilmes = fopen("movies.txt","r");
  
  
  lineRead = readFileBySeparator(arqFilmes, '\n');
 
  
  
  
  while(lineRead != NULL) {
    

    matrixConfig = split(lineRead, '|');


    strcpy(filmes[i].id, matrixConfig[0]);
    strcpy(filmes[i].titulo, matrixConfig[1]);
    strcpy(filmes[i].sinopse, matrixConfig[2]);
    strcpy(filmes[i].genero, matrixConfig[3]);
    strcpy(filmes[i].quantidade, matrixConfig[4]);
    strcpy(filmes[i].ano, matrixConfig[5]);
    i++;

    free(matrixConfig);

    lineRead = readFileBySeparator(arqFilmes, '\n');
   }

  free(lineRead);

  fclose(arqFilmes);

}


void getAllMovieTitles(int new_fd) {
  char buffer[MAX_SIZE_BUF_TITLES];
  int i;

  buffer[0] = END_STRING;
  strcat(buffer, "10#");
  for(i=0; i < NUMBER_MOVIES; i++) {
    strcat(buffer,filmes[i].titulo);
    strcat(buffer, FIELD_SEP);
    strcat(buffer,filmes[i].ano);
    if(i != NUMBER_MOVIES - 1)
      strcat(buffer,REG_SEP);
  }

  send(new_fd, buffer, MAX_SIZE_BUF_TITLES, 0);
}


void getAllMovies(int new_fd) {
  printf("Função getAllMovies\n");

  char buffer[MAX_SIZE_BUF_ALL_MOVIES];
  int i;

  buffer[0] = END_STRING;
  strcat(buffer, "10#");
  for(i = 0; i < NUMBER_MOVIES; i++) {
  	
    strcat(buffer,filmes[i].id);
    strcat(buffer,FIELD_SEP);
    strcat(buffer,filmes[i].titulo);
    strcat(buffer,FIELD_SEP);
    strcat(buffer,filmes[i].sinopse);
    strcat(buffer,FIELD_SEP);
    strcat(buffer,filmes[i].genero);
    strcat(buffer,FIELD_SEP);
    strcat(buffer,filmes[i].quantidade);
    strcat(buffer,FIELD_SEP);
    strcat(buffer,filmes[i].ano);

    if(i != NUMBER_MOVIES - 1)
      strcat(buffer,REG_SEP);
  }
  
  send(new_fd, buffer, MAX_SIZE_BUF_ALL_MOVIES, 0);
}

void getMovieById(int new_fd, char opt[]) {
  char buffer[MAX_SIZE_BUF_INFO_MOVIE];
  char opt2[3];
  int index_movie;
    
  opt2[0] = opt[1];
  opt2[1] = opt[2];
  opt2[2] = END_STRING;

  index_movie = atoi(opt2) - 1;

  buffer[0] = END_STRING;

  strcat(buffer, "1#");

  strcat(buffer, filmes[index_movie].id);
  strcat(buffer, FIELD_SEP);
  strcat(buffer, filmes[index_movie].titulo);
  strcat(buffer, FIELD_SEP);
  strcat(buffer, filmes[index_movie].sinopse);
  strcat(buffer, FIELD_SEP);
  strcat(buffer,filmes[index_movie].genero);
  strcat(buffer,FIELD_SEP);
  strcat(buffer,filmes[index_movie].quantidade);
  strcat(buffer,FIELD_SEP);
  strcat(buffer,filmes[index_movie].ano);
  strcat(buffer, REG_SEP);
  
  send(new_fd, buffer, MAX_SIZE_BUF_INFO_MOVIE, 0);
}

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

void getMovieQtById(int new_fd, char opt[]) {
  char buffer[MAX_SIZE_BUF_SYNOPSIS];
  char opt2[3];
  
  opt2[0] = opt[1];
  opt2[1] = opt[2];
  opt2[2] = END_STRING;

  buffer[0] = END_STRING;
  strcat(buffer, filmes[atoi(opt2)-1].quantidade);
  strcat(buffer, REG_SEP);
  
  send(new_fd, buffer, MAX_SIZE_BUF_SYNOPSIS, 0);
}

void alterQt(int new_fd, char opt[]){
  char *args;
  char buffer[1];

  buffer[0] = '0';
  args = split(opt, '|');
  if(atoi(args[3])){
    strcpy(filmes[atoi(args[1])-1].quantidade, args[2]);
    buffer[0] = '1';
  }

  send(new_fd, buffer, 1, 0);
}

void getMovieByGenre(int new_fd, char opt[]){
  
  char buffer[MAX_SIZE_BUF_ALL_MOVIES], genre[18];
  char *size;
  int i, j = 0, cmp;

  buffer[0] = END_STRING;
  strcpy(genre, split(opt, '|')[1]);

  for(i = 0; i < NUMBER_MOVIES; i++) {
    cmp = strcmp(genre, filmes[i].genero);
    printf("%d\n", cmp);
    if (cmp == 0)
    {
      strcat(buffer,filmes[i].titulo);
      strcat(buffer,FIELD_SEP);
      strcat(buffer,filmes[i].ano);

      j++;

      if(i != NUMBER_MOVIES - 1)
        strcat(buffer,REG_SEP);
    }
  }
  strcat(buffer, "#");
  sprintf(size, "%d", j);
  strcat(buffer, size);
  printf("%s\n", buffer);
  send(new_fd, buffer, MAX_SIZE_BUF_ALL_MOVIES, 0);
}

void sigchld_handler(int s) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa) {

    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char * argv[]) {
  
  
  int sockfd, new_fd;  			
  struct addrinfo hints, *servinfo, *p;

  
  struct sockaddr_storage their_addr; 	

  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  int ativo; 
  
  
  
  char opt[20];		                

  loadMovies();
    
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM; 
  hints.ai_flags = AI_PASSIVE;
  
  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  
  
  
  for(p = servinfo; p != NULL; p = p->ai_next) {

    
    if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    
    
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    
    break;
  }
  
  
  if (p == NULL)  {
    fprintf(stderr, "servidor: falha ao realizar 'bind'\n");
    return 2;
  }
  
  
  freeaddrinfo(servinfo); 
  
  
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
  
  
  while(1) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      
      continue;
    }
    
    inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("servidor: conexao obtida de %s\n", s);
    
    
    if (!fork()) { 			  

      
      close(sockfd); 			  

      ativo = 1;
      while(ativo){

      	
      	if(recv(new_fd,opt, 20, 0) == -1);

	      perror("recv");

	       switch(opt[0]){
  	case '0':
      
      ativo = 0;
      break;
    case '1':			 
  	  getAllMovieTitles(new_fd);
  	  break;
  	case '2': 
  	  
      getMovieByGenre(new_fd, opt);
  	  break;
  	case '3':
      getMovieSynById(new_fd, opt);
  	  break;
  	case '4':
  	  getMovieQtById(new_fd, opt);
  	  break;
    case '5':
      getMovieById(new_fd, opt);
      break;
    case '6':
      getAllMovies(new_fd);
      break;
    case '7':
      alterQt(new_fd, opt);
      break;
  	default:
  	  printf("Opcao nao valida. Tente novamente\n");
  	  break;
	       }
      }
      close(new_fd);
      exit(0);
    }

    
    close(new_fd); 
  }
  
  return 0;
}
