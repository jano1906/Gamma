/** @file
 * Interfejs obsługujący logikę gry
 *
 * @author Jan Olszewski
 * @copyright Uniwersytet Warszawski
 * @date 16.05.2020
 */

#ifndef LOGIC_H
#define LOGIC_H

#include "gamma.h"

/** @brief typ mówiący o trybie, w którym ma być rozgrywana rozgrywka
 */
enum mode{Batch, Interactive};


/** @brief struktura przechowująca stan gry 
 * i informacje o trybie, w którym będzie rozgrywana rozgrywka
 */
typedef struct{
	gamma_t* game; ///< struktura przechowująca stan gry
	enum mode mod; ///< zmienna mówiąca o trybie przeprowadzaniej rozgrywki
} game_and_mode;


/** @brief parsuje kolejne linie inputu, jeżeli jest zgodna ze specyfikacją
 * tworzy gre i mówi w jakim trybie będzie przeprowadzana rozgrywka
 * param[in] gm 	- struktura przechowująca stan gry 
 * 					  i informacje o trybie, w którym będzie rozgrywana rozgrywka
 * param[in] line 	- numer przetwarzanej lini
 *
 * @return true jeżeli udało się wykonać akcję, false wpp
 */
bool set_game_and_mode(game_and_mode* gm, int* line);

/** @brief przeprowadza rozgrywkę w trybie wsadowym
 * param[in] game 	- struktura przechowująca stan gry 
 * param[in] line 	- numer przetwarzanej lini
 */
void play_batch(gamma_t* game, int* line);

/** @brief przeprowadza rozgrywkę w trybie interaktywnym
 * param[in] game 	- struktura przechowująca stan gry 
 */
void play_interactive(gamma_t* game);

/** @brief wypisuje komunikat zabija proces
 * param[in] str 	- komunikat jaki ma zostać wypisany przed zabiciem procesu 
 */
void fail(char* str);

/** @brief wypisuje komunikat ERROR line, gdzie 
 * line to numer lini, w której pojawił się błąd
 * param[in] line 	- numer przetwarzanej lini
 */
void errLine(int line);

/** @brief wypisuje komunikat ERROR
 */
void err();
#endif