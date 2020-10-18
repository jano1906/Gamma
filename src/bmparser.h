/** @file
 * Interfejs klasy parsującej komendy
 *
 * @author Jan Olszewski
 * @copyright Uniwersytet Warszawski
 * @date 16.05.2020
 */

#ifndef BMPARSER_H
#define BMPARSER_H

#include <stdint.h>


/** @brief typ reprezentujący radzaj zwróconej komendy
 */
enum command_type{Unknown = 0, Move, Golden, Busy, Free, GoldenPossible, Print};

/** @brief Struktura reprezentująca komendę 
 * zawierającą parametry z jakimi ma być wykonane gamma_move i gamma_golden_move
 */
typedef struct{
	long long int nums[3]; ///< tablica liczb będących parametrami
} move_command;

/** @brief Struktura reprezentująca komendę 
 * zawierającą parametry z jakimi ma być wykonane gamma_golden_possible,
 * gamma_free_frields, gamma_busy_fields
 */
typedef struct{
	long long int player; ///< liczba będąca parametrami
} player_info_command;

/** @brief Struktura reprezentująca komendę 
 * zawierającą parametry z jakimi ma być wykonane gamma_new oraz 
 * tryb w jakim ma być prowadzona gra
 */
typedef struct{
	char mode; ///< zawiera informacje o trybie gry
	long long int nums [4]; ///< tablica liczb będących parametrami
} mode_selection_command;

/** @brief Struktura reprezentująca komendę, mówiącą że ma być wykonane
 * gamma_board i wypisanie planszy
 */
typedef struct{
	char dummy; ///< zmienna, która jest tylko alokowana, ale nie używana
} print_command;

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
 mode_selection_command* parse_mode_selection(char* buffer);

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
enum command_type parse_command_type(char* buffer);

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
move_command* parse_move_command(char* buffer);

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
player_info_command* parse_player_info_command(char* buffer);

/** @brief parsuje linię inputu i zwraca komendę
 * param[in] buffer	- linia inputu do przetworzenia
 *
 * @return sparsowana komenda
 */
print_command* parse_print_command(char* buffer);

#endif