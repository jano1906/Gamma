/** @file
 * Interfejs klasy wykonującej komendy
 *
 * @author Jan Olszewski
 * @copyright Uniwersytet Warszawski
 * @date 16.05.2020
 */

#ifndef BMEXECUTER_H
#define BMEXECUTER_H

#include <stdbool.h>
#include "bmparser.h"
#include "gamma.h"
#include "logic.h"

/** @brief ustawia tryb gry na podstawie podanej komendy
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o trybie
 * @param[in] gm    – wskaźnik na strukturę do której zapiszemy utworzoną grę 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 * @return true jeżeli gamma_new zwróciło grę, false jeżeli zwróciło NULL 
 */
bool execute_mode_selection(mode_selection_command* com, game_and_mode* gm, int line);

/** @brief wykonuje gamma_move z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_move_command(move_command* com, gamma_t* game, int line);

/** @brief wykonuje gamma_golden_move z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_golden_move_command(move_command* com, gamma_t* game, int line);

/** @brief wykonuje gamma_busy_fields z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_busy_fields_command(player_info_command* com, gamma_t* game, int line);

/** @brief wykonuje gamma_free_fields z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_free_fields_command(player_info_command* com, gamma_t* game, int line);

/** @brief wykonuje gamma_golden_possible z parametrami zapisanymi w com
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_golden_possible_command(player_info_command* com, gamma_t* game, int line);

/** @brief wykonuje gamma_board z parametrami zapisanymi w com i wypisuje stan planszy
 * @param[in] com 	– sparsowana komenda zawierajaca informacje o wykonywanej akcji
 * @param[in] game  – wskaźnik na strukturę przechowującą stan gry 
 * 					  i informację o trybie, w którym będziemy grać
 * @param[in] line 	– wskaźnik na licznik przetworzonych lini
 */
void execute_print_command(print_command* com, gamma_t* game, int line);

#endif