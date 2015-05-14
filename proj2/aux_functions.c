//=========================
// MC823 - Laboratorio de Redes de Computadores - Projeto 1
//
// Nome: Frank  RA: 070934
// Nome: Suelen RA: 072399
//=========================

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#define DOUBLE_SPACE "  "
#define ONE_SPACE ' '
#define WORD_SIZE_MAX 4486
#define STRING_END '\0'

/**
 * Função que deixa somente um espaço entre os caracteres separados por espaços dentro da string
 *
 * @param string a string a ser modificada
 */
void oneSpaceBetweenChar(char * string) {
  char * temp;
  int sizeString;

  while ((temp = strstr(string, DOUBLE_SPACE)) != NULL) {
     memmove(temp, (temp + 1), strlen(temp));
  }

  // Para nao ter espaco branco no primeiro caracter
  if (string[0] == ONE_SPACE)
    strcpy(&string[0], &string[1]);

  sizeString = strlen(string);
  
  // Para nao ter espaco branco no ultimo caracter
  if (string[sizeString - 1] == ONE_SPACE)
    strcpy(&string[sizeString - 1], &string[sizeString]);
}

/**
 * Retorna a quantidade de vezes que um determinado caracter c aparece na string
 *
 * @param string a string onde deseja efetuar a busca
 * @param c o caracter que deseja contar
 *
 * @return retorna a quantidade de vezes que c aparece na string
 */
int countEspecificChar(char * string, char c) {
  int counter, sizeString, i;

  counter = 0;
  sizeString = strlen(string);

  for(i = 0; i < sizeString; i++){

    if(string[i] == c) {
      counter++;
    }

  }
  
  return counter;
}

/**
 * Quebra a string recebida em duas: uma string que vem antes do
 * primeiro delimitador e outra que vem depois do primeiro delimitador
 *
 * @param string a string que deseja quebrar
 * @param delim o delimitador
 *
 * @return retString uma string que vem antes do primeiro delimitador
 */
char * strTok(char * string, char const delim) {
  char temp[WORD_SIZE_MAX], * retString;
  int i = 0, j = 0, sizeRetString;

  if(strlen(string) == 0) return NULL;
  
  while(string[i] != delim && string[i] != STRING_END) {
    temp[i] = string[i];
    i++;
  }

  if(strlen(string) > i) {

    temp[i] = STRING_END;
    sizeRetString = strlen(temp);
    retString = (char *) malloc (sizeof(char) * (sizeRetString + 1));
    strcpy(retString, temp);
    
    i++;
    while(string[i] != STRING_END) {
      temp[j] = string[i];
      i++;
      j++;
    }
    temp[j] = STRING_END;
    
    strcpy(string, temp);
  } else {

    return string;
  }

  return retString;
}

/**
 * Função que separa a string de acordo com o delimitador passado
 * retornando dentro de uma matriz de char
 * 
 * @param string a string que deseja separar
 * @param delim o delimitador que separa as 'palavras' da string
 *
 * @return um vetor que em cada posição contem uma 'palavra' da string
 */
char ** split(char * string, char delim) {
  int sizeVector, i;
  char ** vector;

  sizeVector = countEspecificChar(string, delim) + 1;

  vector = (char **) malloc (sizeof(char *) * sizeVector);

  for(i = 0; i < sizeVector; i++) {
    
    vector[i] = strTok(string, delim);

    if(vector[i] != NULL) oneSpaceBetweenChar(vector[i]);
  }

  return vector;
}

/**
 *Recebe um ponteiro para um arquivo e le o primeiro registro
 *(caracter a caracter ate encontrar o delimitador de registros)
 *
 *@param file ponteiro para o arquivo a ser lido
 *@param separator o separador de registros
 *
 *@return uma string que contem um "registro"
 */
char * readFileBySeparator(FILE * file, char separator){
  char * ret;
  char letter;
  char temp[WORD_SIZE_MAX];
  int counter = 0;
  int sizeReg;
  
  letter = fgetc(file);
  
  if (letter != EOF) {

    while ((letter != separator)) {
      temp[counter++] = letter;
      letter = fgetc(file);
      if(letter == EOF) {
	return NULL;
      }
    }
    
    temp[counter] = STRING_END;
    sizeReg = strlen(temp);
    ret = (char *) malloc (sizeof(char) * (sizeReg + 1));
    strcpy (ret, temp);
    
    return ret;
    
  } else {
    return NULL;
  }

}

