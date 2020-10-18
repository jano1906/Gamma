/** @file
 * Interfejs obsługujący tryb interaktywny
 *
 * @author Jan Olszewski
 * @copyright Uniwersytet Warszawski
 * @date 16.05.2020
 */

#ifndef INTERACTIVE_H
#define INTERACTIVE_H

/** @brief zwraca wsp. x-ową, na której powinien stać kursor na początku gry
 * param[in] game - struktura przechowująca stan gry
 *
 * @return odcięta domyślnego stanu kursora
 */
uint32_t get_curX_default(gamma_t* game);

/** @brief zwraca wsp. y-ową, na której powinien stać kursor na początku gry
 * param[in] game - struktura przechowująca stan gry
 *
 * @return rzędna domyślnego stanu kursora
 */
uint32_t get_curY_default(gamma_t* game);

/** @brief czyści terminal
 */
void clear();

/** @brief wypisuje stan planszy oraz statystyki aktualnego gracza
 * param[in] game - struktura przechowująca stan gry
 * param[in] curX - aktualna wsp x-owa kursora
 * param[in] curY - aktualna wsp y-owa kursora
 * param[in] curP - aktualny numer gracza, którego jest tura
 */
void print(gamma_t* game, uint32_t curX, uint32_t curY, uint32_t curP);

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
			uint32_t* curP, bool* running);

/** @brief wypisuje końcowy stan panszy i listę graczy z ich wynikiem
 * param[in] game 		- struktura przechowująca stan gryB
 */
void summary(gamma_t* game);

#endif