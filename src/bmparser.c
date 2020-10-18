/** @file
 * Implementacja interfejsu bmparser.h
 *
 * @author Jan Olszewski
 * @copyright Uniwersytet Warszawski
 * @date 16.05.2020
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "bmparser.h"
#include "logic.h"


/** @brief zwraca długość liczby w zapisie dziesiętnym
 * param[in] n - przetwarzana liczba
 *
 * @return log_10(n) lub 1 jeżeli n == 0
 */
static uint32_t num_len(unsigned long n){
	uint32_t log = 1;
	n/=10;
	while(n!=0){
		n/=10;
		log++;
	}
	return log;
}

/** @brief obcina ostatni znak ze string
 * param[in] str - przetwarzany string
 */
static void trim_last(char* str){
	str[strlen(str)-1] = '\0';
}

/** @brief sprawdza czy dany token jest liczbą
 * param[in] ul	 	- liczba, z którą porównywany jest token
 * param[in] token 	- sprawdzany token
 *
 * @return true jeżeli token jest liczbą, false wpp
 */
static bool valid_num_token(long long int ul, char* token){
	bool flague = true;
	flague &= num_len(ul) == strlen(token);
	if(num_len(ul) == 1){
		flague &= ul == token[0] - '0';
	}
	return flague;
}

/** @brief sprawdza czy dany znak jest białym znakiem różnym od \n
 * param[in] c - przetwarzany znak
 *
 * @return true jeżeli dany znak jest białym znakiem różnym od \n, false wpp
 */
static bool is_space(char c){
	return (isspace(c) != 0) && (c != '\n');
}

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
mode_selection_command* parse_mode_selection(char* buffer){
	mode_selection_command* com = malloc(sizeof(mode_selection_command));
	if(buffer != NULL && strlen(buffer) > 0){
		char* bufferCpy = malloc(strlen(buffer)+1);
		strcpy(bufferCpy, buffer);
		trim_last(bufferCpy);
		char* token = strtok(bufferCpy, " \t\v\f\r");
		if(token != NULL){
			if(strcmp(token, "B")*strcmp(token, "I") != 0 || is_space(buffer[0])){
				//printf("wrong mode\n");
				free(bufferCpy);
				free(com);
				return NULL;
			} else{
				if(com != NULL){
					com->mode = token[0];
					for(int i=0;i<4;i++){
						token = strtok(NULL, " \t\v\f\r");
						if(token != NULL){
							long long int ul = strtoull(token, NULL, 10);
							if(valid_num_token(ul, token)){
								com->nums[i] = ul;
							} else{
								//printf("not a number\n"); 
								free(com); 
								free(bufferCpy); 
								return NULL;
							}
						} else{
							//printf("too short\n"); 
							free(com); 
							free(bufferCpy); 
							return NULL;
						}
					}
					token = strtok(NULL, " \t\v\f\r");
					if(token == NULL){
						free(bufferCpy);
						return com;
					} else{
						//printf("too long\n"); 
						free(com); 
						free(bufferCpy); 
						return NULL;
					}
				} else{
					//printf("couldn't alloc\n"); 
					free(bufferCpy); 
					return NULL;
				}
			}
		} else{
			//printf("Empty line\n");
			free(com);
			free(bufferCpy);
			return NULL;
		}
	} else{
		free(com);
		return NULL;
	}
}

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
enum command_type parse_command_type(char* buffer){
	enum command_type type = Unknown;
	if(buffer != NULL && strlen(buffer) > 0){
		char* bufferCpy = malloc(strlen(buffer)+1);
		strcpy(bufferCpy, buffer);
		trim_last(bufferCpy);
		if(bufferCpy != NULL && strlen(bufferCpy) > 0 && !is_space(buffer[0])){
			char* token = strtok(bufferCpy, " \t\v\f\r");
			if(token != NULL){
				if(strcmp(token, "m") == 0)
					type = Move;
				else if(strcmp(token, "g") == 0)
					type = Golden;
				else if(strcmp(token, "b") == 0)
					type = Busy;
				else if(strcmp(token, "f") == 0)
					type = Free;
				else if(strcmp(token, "q") == 0)
					type = GoldenPossible;
				else if(strcmp(token, "p") == 0)
					type = Print;
			}
		}
		free(bufferCpy);
	}
	return type;
}

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
move_command* parse_move_command(char* buffer){
	move_command* com = malloc(sizeof(move_command));
	if(com != NULL){
		if(buffer != NULL && strlen(buffer) > 0){
			char* bufferCpy = malloc(strlen(buffer)+1);
			strcpy(bufferCpy, buffer);
			trim_last(bufferCpy);
			if(bufferCpy != NULL && strlen(bufferCpy) > 0){
				char* token = strtok(bufferCpy, " \t\v\f\r");

				for(int i=0;i<3;i++){
					token = strtok(NULL, " \t\v\f\r");
					if(token != NULL){
						long long int ul = strtoull(token, NULL, 10);
						if(valid_num_token(ul, token)){
							com->nums[i] = ul;
						} else{
							//printf("not a number\n"); 
							free(com); 
							free(bufferCpy); 
							return NULL;
						}
					} else{
						//printf("too short\n"); 
						free(com); 
						free(bufferCpy); 
						return NULL;
					}
				}
				token = strtok(NULL, " \t\v\f\r");
				if(token == NULL){
					free(bufferCpy);
					return com;
				} else{
					//printf("too long\n"); 
					free(com); 
					free(bufferCpy); 
					return NULL;
				}
			} else{
				//printf("empty line \n");
				free(bufferCpy);
				free(com);
				return NULL;
			}
		} else{
			//printf("empty buffer \n");
			free(com);
			return NULL;
		}
	} else{
		fail("NO MEMORY FOR COMMAND");
	}
	return NULL;
}

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
player_info_command* parse_player_info_command(char* buffer){
	player_info_command* com = malloc(sizeof(player_info_command));
	if(com != NULL){
		if(buffer != NULL && strlen(buffer) > 0){
			char* bufferCpy = malloc(strlen(buffer)+1);
			strcpy(bufferCpy, buffer);
			trim_last(bufferCpy);
			if(bufferCpy != NULL && strlen(bufferCpy) > 0){
				char* token = strtok(bufferCpy, " \t\v\f\r");

				token = strtok(NULL, " \t\v\f\r");
				if(token != NULL){
					long long int ul = strtoull(token, NULL, 10);
					if(valid_num_token(ul, token)){
						com->player = ul;
					} else{
						//printf("not a number\n"); 
						free(com); 
						free(bufferCpy); 
						return NULL;
					}
				} else{
					//printf("too short\n"); 
					free(com); 
					free(bufferCpy); 
					return NULL;
				}
				token = strtok(NULL, " \t\v\f\r");
				if(token == NULL){
					free(bufferCpy);
					return com;
				} else{
					//printf("too long\n"); 
					free(com); 
					free(bufferCpy); 
					return NULL;
				}
			} else{
				//printf("empty line\n");
				free(bufferCpy);
				free(com);
				return NULL;
			}
		} else{
			//printf("empty buffer\n");
			free(com);
			return NULL;
		}
	} else{
		fail("NO MEMORY FOR COMMAND");
	}
	return NULL;
}

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
print_command* parse_print_command(char* buffer){
	print_command* com = malloc(sizeof(print_command));
	if(com != NULL){
		if(buffer != NULL && strlen(buffer) > 0){
			char* bufferCpy = malloc(strlen(buffer)+1);
			strcpy(bufferCpy, buffer);
			trim_last(bufferCpy);
			if(bufferCpy != NULL && strlen(bufferCpy) > 0){
				char* token = strtok(bufferCpy, " \t\v\f\r");
				if(token != NULL)
					com->dummy = token[0];

				token = strtok(NULL, " \t\v\f\r");
				if(token == NULL){
					free(bufferCpy);
					return com;
				} else{
					//printf("too long\n"); 
					free(com); 
					free(bufferCpy); 
					return NULL;
				}
			} else{
				//printf("empty line\n");
				free(bufferCpy);
				free(com);
				return NULL;
			}
		} else{
			//printf("empty buffer\n");
			free(com);
			return NULL;
		}
	} else{
		fail("NO MEMORY FOR COMMAND");
	}
	return NULL;
}
