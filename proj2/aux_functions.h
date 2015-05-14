//=========================
// MC823 - Laboratorio de Redes de Computadores - Projeto 1
//
// Nome: Frank  RA: 070934
// Nome: Suelen RA: 072399
//=========================

#ifndef AUX_FUNCTIONS_H
#define AUX_FUNCTIONS_H

/**
 * Função que deixa somente um espaço entre os caracteres separados
 * por espaços dentro da string
 *
 * @param string a string a ser modificada
 *
 */
void oneSpaceBetweenChar(char * string);

/**
 * Retorna a quantidade de vezes que um determinado caracter c aparece
 * na string
 *
 * @param string a string onde deseja efetuar a busca
 * @param c o caracter que deseja contar
 *
 * @return retorna a quantidade de vezes que c aparece na string
 *
 */
int countEspecificChar(char * string, char c);

/**
 * Quebra a string recebida em duas: uma string que vem antes do
 * primeiro delimitador e outra que vem depois do primeiro delimitador
 *
 * @param string a string que deseja quebrar
 * @param delim o delimitador
 *
 * @return retString uma string que vem antes do primeiro delimitador
 *
 */
char * strTok(char * string, char const delim);

/**
 * Função que separa a string de acordo com o delimitador passado
 * retornando dentro de uma matriz de char
 * 
 * @param string a string que deseja separar
 * @param delim o delimitador que separa as 'palavras' da string
 *
 * @return um vetor que em cada posição contem uma 'palavra' da string
 * 
 */
char ** split(char * string, char delim);


/**
 *Recebe um ponteiro para um arquivo e le o primeiro registro
 *(caracter a caracter ate encontrar o delimitador de registros)
 *
 *@param file ponteiro para o arquivo a ser lido
 *@param separator o separador de registros
 *
 *@return uma string que contem um "registro"
 */
char * readFileBySeparator(FILE * file, char separator);

#endif
