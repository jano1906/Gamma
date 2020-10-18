/** @file
 * Implementacja interfejsu logic.h
 *
 * @author Jan Olszewski
 * @copyright Uniwersytet Warszawski
 * @date 16.05.2020
 */

/** @brief makro potrzebne do korzystania z getline()
 */
#define _XOPEN_SOURCE 700 
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>
#include "gamma.h"
#include "bmparser.h"
#include "bmexecuter.h"
#include "logic.h"
#include "interactive.h"


/** @brief sprawdza, czy dana linia inputu ma być zignorowana
 * param[in] line 	- przetwarzana linia tekstu
 * @return true jeżeli dana linia ma być zignorowana, false wpp
 */
static bool ignore_line(char* line){
	if(line != NULL && strlen(line) > 0){
		return line[0] == '#' || (line[0] == '\n' && strlen(line) == 1);
	} else{
		return false;
	}

}

/** @brief wypisuje komunikat zabija proces
 * param[in] str 	- komunikat jaki ma zostać wypisany przed zabiciem procesu 
 */
void fail(char* str){
	printf("%s", str);
	exit(1);
}

/** @brief wypisuje komunikat ERROR line, gdzie 
 * line to numer lini, w której pojawił się błąd
 * param[in] line 	- numer przetwarzanej lini
 */
void errLine(int line){
	fprintf(stderr, "ERROR %d\n", line);
}

/** @brief wypisuje komunikat ERROR
 */
void err(){
	fprintf(stderr, "ERROR\n");
}

/** @brief wypisuje komunikat OK line, gdzie 
 * line to numer lini, w której ustawiono tryp gry
 * param[in] line 	- numer przetwarzanej lini
 */
static void okLine(int line){
	printf("OK %d\n", line);
}

/** @brief parsuje kolejne linie inputu, jeżeli jest zgodna ze specyfikacją
 * tworzy gre i mówi w jakim trybie będzie przeprowadzana rozgrywka
 * param[in] gm 	- struktura przechowująca stan gry 
 * 					  i informacje o trybie, w którym będzie rozgrywana rozgrywka
 * param[in] line 	- numer przetwarzanej lini
 *
 * @return true jeżeli udało się wykonać akcję, false wpp
 */
bool set_game_and_mode(game_and_mode* gm, int* line){
	char* buffer = malloc(1);
	size_t buffsize = 1;
	if(buffer != NULL){
		bool mode_set = false;
		while(!mode_set && (getline(&buffer, &buffsize, stdin) != -1)){
			(*line)++;
			//printf("line: %d\n", *line);
			if(!ignore_line(buffer)){
				if(buffer != NULL){
					mode_selection_command* com = parse_mode_selection(buffer);
					mode_set = execute_mode_selection(com, gm, *line);
					free(com);
				} else{
					fail("NULL BUFFER");
				}
			}
		}
		if(mode_set){
			okLine(*line);
			free(buffer);
			return true;
		} else{
			free(buffer);
			return false;
		}
	} else{
		fail("NO MEMORY FOR BUFFER");
	}
	return false;
}

/** @brief przeprowadza rozgrywkę w trybie wsadowym
 * param[in] game 	- struktura przechowująca stan gry 
 * param[in] line 	- numer przetwarzanej lini
 */
void play_batch(gamma_t* game, int* line){
	char* buffer = malloc(1);
	size_t buffsize = 1;
	if(buffer != NULL){
		while(getline(&buffer, &buffsize, stdin) != -1){
			(*line)++;
			if(!ignore_line(buffer)){
				if(buffer != NULL){
					enum command_type com_type = parse_command_type(buffer);
					if(com_type == Move){
						move_command* com = parse_move_command(buffer);
						execute_move_command(com, game, *line);
						free(com);
					} else if(com_type == Golden){
						move_command* com = parse_move_command(buffer);
						execute_golden_move_command(com, game, *line);
						free(com);
					} else if(com_type == Busy){
						player_info_command* com = parse_player_info_command(buffer);
						execute_busy_fields_command(com, game, *line);
						free(com);
					} else if(com_type == Free){
						player_info_command* com = parse_player_info_command(buffer);
						execute_free_fields_command(com, game, *line);
						free(com);
					} else if(com_type == GoldenPossible){
						player_info_command* com = parse_player_info_command(buffer);
						execute_golden_possible_command(com, game, *line);
						free(com);
					} else if(com_type == Print){
						print_command* com = parse_print_command(buffer);
						execute_print_command(com, game, *line);
						free(com);
					} else if(com_type == Unknown){
						 errLine(*line);
					} else{
						fail("FAILED TO PARSE COMMAND TYPE");
					}
				} else{
					fail("NULL BUFFER");
				}
			}
		}
	} else{
		fail("NO MEMORY FOR BUFFER");
	}
	free(buffer);
}

/** @brief przeprowadza rozgrywkę w trybie interaktywnym
 * param[in] game 	- struktura przechowująca stan gry 
 */
void play_interactive(gamma_t* game){
	//SETTING TERMINAL
	struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    //GAME
    bool running = true;
    uint32_t curX = get_curX_default(game);
    uint32_t curY = get_curY_default(game);
    uint32_t curP = 1;
    while(running){
    	clear();
    	print(game, curX, curY, curP);
    	action(game, &curX, &curY, &curP, &running);
    }
    
    clear();
    summary(game);
    //RESTORING TERMINAL

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}
