/** @file
 * Implementacja interfejsu interactive.h
 *
 * @author Jan Olszewski
 * @copyright Uniwersytet Warszawski
 * @date 16.05.2020
 */

#include <termio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "gamma.h"
#include "logic.h"

/** @brief alokuje pamięć i zwraca podsłowo stringa
 * param[in] str 	- przetwarzany string
 * param[in] s 		- start przetwarzania
 * param[in] k 		- koniec przetwarzania
 *
 * @return podsłowo stringa [s,k)
 */
static char* substr(char* str, int s, int k){
	char* str1 = malloc(k-s+1);
	if(!str1)
		return NULL;
	memcpy(str1, &str[s], k-s);
	str1[k-s] = '\0';
	return str1;
}

/** @brief łączy dwa stringi w jeden
 * param[in] str1	- pierwszy string
 * param[in] str2 	- drugi string
 *
 * @return str1 'konkatynacja' str2
 */
static char* append(char* str1, char* str2){
	char* str = malloc(strlen(str1)+strlen(str2)+1);
	if(!str){
		free(str1);
		free(str2);
		return NULL;
	}
	strcpy(str, str1);
	strcpy(&str[strlen(str1)], str2);
	free(str1);
	free(str2);
	return str;
}


/** @brief podświetla str na przedziale [s,k)
 * param[in] str 	- przetwarzany string
 * param[in] s 		- start podświetlania
 * param[in] k 		- koniec podświetlania
 *
 * @return str z podświetlonym tekstem na przedziale [s,k)
 */
static char* highlighted(char* str, uint32_t s, uint32_t k){
	char* str1 = substr(str, 0, s);
	if(!str1)
		return NULL;
	char* str2 = substr(str, s, k);
	if(!str2)
		return NULL;
	char* str3 = substr(str, k, strlen(str));
	if(!str3)
		return NULL;
	char* h1 = malloc(sizeof("\x1b[44m")+1);
	char* h2 = malloc(sizeof("\x1b[0m")+1);
	strcpy(h1, "\x1b[44m");
	strcpy(h2, "\x1b[0m");
	return append(append(append(append(str1, h1), str2), h2), str3);
}

/** @brief podświetla aktualne położenie kursora
 * param[in] game 	- struktura przechowująca stan gry
 * param[in] curX 	- aktualna wsp. x-owa
 * param[in] curY 	- aktualna wsp. y-owa
 *
 * @return plansza z podświetlonym miejscem, w którym znajduje się kursor
 */
static char* highlighted_board(gamma_t* game, uint32_t curX, uint32_t curY){
	uint32_t size_of_pocket = length_of_max_player_id_on_board(game);
	uint32_t w = get_width(game);
	uint32_t h = get_height(game);
	uint64_t pos;
	if(size_of_pocket>1){
		pos = curX*(size_of_pocket+1)+w*(size_of_pocket+1)*(h-1-curY);
	}
	else
		pos = curX + (w+1)*(h-1-curY);
	char* board = gamma_board(game);
	if(!board)
		return NULL;
	char* toReturn = highlighted(board, pos, pos+size_of_pocket);
	if(!toReturn)
		return NULL;
	free(board);
	return toReturn;
}

/** @brief zwraca wsp. x-ową, na której powinien stać kursor na początku gry
 * param[in] game - struktura przechowująca stan gry
 *
 * @return odcięta domyślnego stanu kursora
 */
uint32_t get_curX_default(gamma_t* game){
	return get_width(game)/2;
}

/** @brief zwraca wsp. y-ową, na której powinien stać kursor na początku gry
 * param[in] game - struktura przechowująca stan gry
 *
 * @return rzędna domyślnego stanu kursora
 */
uint32_t get_curY_default(gamma_t* game){
	return get_height(game)/2;
}

/** @brief czyści terminal
 */
void clear(){
	printf("\033[2J");
    printf("\033[J");
    printf("\033[H");
    printf("\033[1;1H");
}

/** @brief wypisuje informacje o graczy
 * param[in] game 	- struktura przechowująca stan gry
 * param[in] curP	- aktualny gracz
 *
 * @return true jeżeli udało się wykonać akcję, false wpp
 */
static void print_player_info(gamma_t* game, uint32_t curP){
	printf("Player: %u, Busy fields: %lu", 
		   curP, gamma_busy_fields(game, curP));
}

/** @brief wypisuje stan planszy oraz statystyki aktualnego gracza
 * param[in] game - struktura przechowująca stan gry
 * param[in] curX - aktualna wsp x-owa kursora
 * param[in] curY - aktualna wsp y-owa kursora
 * param[in] curP - aktualny numer gracza, którego jest tura
 */
void print(gamma_t* game, uint32_t curX, uint32_t curY, uint32_t curP){
	char* toPrint = highlighted_board(game, curX, curY);
	if(!toPrint)
		return err();
	
	printf("%s\n", toPrint);
	free(toPrint);
	print_player_info(game, curP);
	printf(", Free fields: %lu", gamma_free_fields(game, curP));
	if(gamma_golden_possible(game, curP))
		printf(", G");
}

/** @brief wypisuje końcowy stan panszy i listę graczy z ich wynikiem
 * param[in] game 		- struktura przechowująca stan gryB
 */
void summary(gamma_t* game){
	char* board = gamma_board(game);
	if(!board)
		return err();

	printf("%s", board);
	for(uint32_t i=1;i<=get_no_players(game);i++, printf("\n")){
		print_player_info(game, i);
	}
	free(board);
}

/** @brief zmienia wsp. x-ową, symuluje ruch kursora w lewo
 * param[in] curX	- aktualny wsp x-owa
 */
static void left_move(uint32_t* curX){
    if (*curX > 0)
        (*curX)--;
}

/** @brief zmienia wsp. x-ową, symuluje ruch kursora w prawo
 * param[in] curX 	- aktualny wsp x-owa
 * param[in] width 	- szerokość planszy
 */
static void right_move(uint32_t* curX, uint32_t width){
    if (*curX < width-1)
        (*curX)++;
}

/** @brief zmienia wsp. y-ową, symuluje ruch kursora w dół
 * param[in] curY 	- aktualny wsp y-owa
 */
static void down_move(uint32_t* curY){
    if (*curY > 0)
        (*curY)--;
}

/** @brief zmienia wsp. y-ową, symuluje ruch kursora w górę
 * param[in] curY 	- aktualny wsp y-owa
 * param[in] height - wysokość planszy
 */
static void up_move(uint32_t* curY, uint32_t height){
    if (*curY < height-1)
        (*curY)++;
}

/** @brief inkrementuje podany numer gracza
 * param[in] game - struktura przechowująca stan gry
 * param[in] curY - gracz do zinkrementowania 
 */
static void incrementP(gamma_t* game, uint32_t* curP){
	(*curP) = ((*curP)%get_no_players(game))+1;	
}

/** @brief wykonuje gamma_move z podanymi parametrami
 * param[in] game - struktura przechowująca stan gry
 * param[in] curX - aktualny wsp x-owa
 * param[in] curY - aktualny wsp y-owa
 * param[in] curP - aktualny gracz wykonujący ruch - 
  	 				inkrementowany jeżeli akcja udała się
 *
 * @return true jeżeli udało się wykonać akcję, false wpp
 */
static void move(gamma_t* game, uint32_t curX, uint32_t curY, uint32_t *curP){
	if(gamma_move(game, *curP, curX, curY))
		incrementP(game, curP);
}

/** @brief wykonuje gamma_golden_move z podanymi parametrami
 * param[in] game - struktura przechowująca stan gry
 * param[in] curX - aktualny wsp x-owa
 * param[in] curY - aktualny wsp y-owa
 * param[in] curP - aktualny gracz wykonujący ruch - 
  	 				inkrementowany jeżeli akcja udała się
 *
 * @return true jeżeli udało się wykonać akcję, false wpp
 */
static void golden(gamma_t* game, uint32_t curX, uint32_t curY, uint32_t *curP){
	if(gamma_golden_move(game, *curP, curX, curY))
		incrementP(game, curP);
}

/** @brief sprawdza czy gracz może wykonać jeszcze jakikolwiek ruch
 * param[in] game - struktura przechowująca stan gry
 * param[in] curP - gracz, o którego pytamy
 *
 * @return true jeżeli gracz może wykonać ruch, false wpp
 */
static bool player_has_action(gamma_t* game, uint32_t curP){
	return (gamma_free_fields(game, curP) != 0 || 
		    gamma_golden_possible(game, curP));
}

/** @brief sprawdza czy jakikolwiek gracz może wykonać jeszcze jakikolwiek ruch
 * param[in] game - struktura przechowująca stan gry
 *
 * @return true jeżeli jakiś gracz może wykonać ruch, false wpp
 */
static bool any_player_has_action(gamma_t* game){
	for(uint32_t i=1;i<=get_no_players(game);i++){
		if(player_has_action(game, i))
			return true;
	}
	return false;
}

/** @brief wczytuje znak i wykonuje odpowiednią akcję, ustawia running na false
 * jeżeli kontynuowanie gry jest niemożliwe
 * param[in] game 		- struktura przechowująca stan gry
 * param[in] curX 		- aktualna wsp x-owa kursora
 * param[in] curY 		- aktualna wsp y-owa kursora
 * param[in] curP 		- aktualny numer gracza, którego jest tura
 * param[in] running 	- zmienna mówiąca czy dana gra jeszcze trwa, 
 * 					  	  czy już powinna się zakończyć
 */
void action(gamma_t* game, uint32_t* curX, uint32_t* curY, 
			uint32_t* curP, bool* running){
	*running = any_player_has_action(game);
	if(*running && player_has_action(game, *curP)){
		char k = getchar();
	    if (k == '\033'){
	        k = getchar();
	        if (k == '['){
	            k = getchar();
	            if (k == 'A')
	                up_move(curY, get_height(game));
	            else if (k == 'B')
	                down_move(curY);
	            else if (k == 'C')
	                right_move(curX, get_width(game));
	            else if (k == 'D')
	                left_move(curX);
	        }
	    } else if (k == ' '){
	    	move(game, *curX, *curY, curP);
	    } else if (k == 'g' || k == 'G'){
	    	golden(game, *curX, *curY, curP);
	    } else if (k == 'c' || k == 'C'){
			incrementP(game, curP);
	    } else if (k == '\4'){
	        (*running) = false;;
	    }
	} else{
		(*curP) = ((*curP)%get_no_players(game))+1;
	}
}