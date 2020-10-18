/** @file
 * Implementacja interfejsu bmexecuter.h
 *
 * @author Jan Olszewski
 * @copyright Uniwersytet Warszawski
 * @date 16.04.2020
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bmparser.h"
#include "logic.h"
#include "gamma.h"

/** @brief wypisuje 0
 */
static void print0(){
	printf("%d\n", 0);
}

/** @brief wypisuje 1
 */
static void print1(){
	printf("%d\n", 1);
}

/** @brief sprawdza czy podana tablica liczb zawiera liczby w zakresie [0, UINT32_MAX]
 * @param[in] nums[] 	– tablica liczb, którą badamy 
 * @param[in] size	 	– rozmiar tablicy
 * @return true wszystkie liczby mieszczą się w zakresie [0, UINT32_MAX], false wpp
 */
static bool valid_nums(long long int nums[], int size){
	bool flague = true;
	for(int i=0;i<size;i++){
		flague &= nums[i]>=0 && nums[i]<= UINT32_MAX;
	}
	return flague;
}

/** @brief sprawdza czy liczba mieści się w zakresie [0, UINT32_MAX]
 * @param[in] num 	– liczba, którą sprawdzamy
 * @return true jeżeli num mieści się w zakresie [0, UINT32_MAX]
 */
static bool valid_num(long long int num){
	return num>=0 && num<= UINT32_MAX;
}

/** @brief ustawia tryb gry na podstawie podanej komendy
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o trybie
 * @param[in] gm    – wskaźnik na strukturę do której zapiszemy utworzoną grę 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 * @return true jeżeli gamma_new zwróciło grę, false jeżeli zwróciło NULL 
 */
bool execute_mode_selection(mode_selection_command* com, game_and_mode* gm, int line){

	if(com != NULL && valid_nums(com->nums, 4)){
		if(com->mode == 'I'){
			gm->mod = Interactive;
		} else if(com->mode == 'B'){
			gm->mod = Batch;
		} else{
			fail("PARSER FAILED TO PARSE MODE_SELECTION_COMMAND PROPERLY");
		}
		gamma_t* g = gamma_new(com->nums[0], com->nums[1], com->nums[2], com->nums[3]);
		if(g == NULL){
			errLine(line);
			return false;
		} else{
			gm->game = g;
			return true;
		}
	} else{
		errLine(line);
		return false;
	}
}

/** @brief wykonuje gamma_move z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_move_command(move_command* com, gamma_t* game, int line){
	if(com != NULL && valid_nums(com->nums, 3)){
		if(gamma_move(game, com->nums[0], com->nums[1], com->nums[2])){
			print1();
		} else{
			print0();
		}
	} else{
		errLine(line);
	}
}

/** @brief wykonuje gamma_golden_move z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_golden_move_command(move_command* com, gamma_t* game, int line){
	if(com != NULL && valid_nums(com->nums, 3)){
		if(gamma_golden_move(game, com->nums[0], com->nums[1], com->nums[2])){
			print1();
		} else{
			print0();
		}
	} else{
		errLine(line);
	}
}

/** @brief wykonuje gamma_busy_fields z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_busy_fields_command(player_info_command* com, gamma_t* game, int line){
	if(com != NULL && valid_num(com->player)){
		printf("%lu\n", gamma_busy_fields(game, com->player));
	} else{
		errLine(line);
	}
}

/** @brief wykonuje gamma_free_fields z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_free_fields_command(player_info_command* com, gamma_t* game, int line){
	if(com != NULL && valid_num(com->player)){
		printf("%lu\n", gamma_free_fields(game, com->player));
	} else{
		errLine(line);
	}
}

/** @brief wykonuje gamma_golden_possible z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_golden_possible_command(player_info_command* com, gamma_t* game, int line){
	if(com != NULL && valid_num(com->player)){
		if(gamma_golden_possible(game, com->player)){
			print1();
		} else{
			print0();
		}
	} else{
		errLine(line);
	}
}

/** @brief wykonuje gamma_board z parametrami zapisanymi w com i wypisuje stan planszy
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_print_command(print_command* com, gamma_t* game, int line){
	if(com != NULL){
		char* board = gamma_board(game);
		if(board != NULL){
			printf("%s", board);
			free(board);
		}
	} else{
		errLine(line);
	}
}