/** @file
 * Implementacja interfejsu gamma.h
 *
 * @author Jan Olszewski
 * @copyright Uniwersytet Warszawski
 * @date 16.04.2020
 */
#include <stdio.h>


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/** @brief przeźroczysty kolor - domyślny, na który malujemy pola przy gamma_try_golden_move
 * używamy go, żeby niepotrzebnie nie alokować tablicy rozmiaru planszy na tablicę visited
 * przy używaniu DFSa do kolorowania obszarów
 */
#define TRANSPARENT UINT32_MAX

/** @brief standardowy stdlib::malloc mowiacy czy funkcja zwraca NULL czy też nie.
 * Alokuje pamięć jak standardowy stdlib::malloc, ale 
 * jeżeli zwraca NULL to ustawia success na false
 * @param[in] size 	- ilość pamięci w bajtach do zaalokowania
 * @param[in] success 	- referencja do flagi mówiącej czy udało się 
 * wykonać wszystkie procesy
 *
 * @return Pointer na blok zaalokowanej pamięci lub NULL jeżeli nie udało się jej zaalokować 
 */
static void* safe_malloc(size_t size, bool* success){
	void* r = malloc(size);
	if(r == NULL){
		*success = false;
	}
	return r;
}

/** @brief standardowy stdlib::malloc ale jezeli nie udalo sie zaalokować pamięci wyrzuca exit(1).
 * @param[in] size 	- ilość pamięci w bajtach do zaalokowania
 *
 * @return Pointer na blok zaalokowanej pamięci lub kończy działanie programu
 * z flagą 1 jeżeli nie udało się zaalokować
 */
static void* safer_malloc(size_t size){
	void* r = malloc(size);
	if(r == NULL)
		exit (1);
	else
		return r;

}

/** @brief Zadaje numeryczny odpowiednik kierunków świata (E,N,W,S) na osi OX.
 */
static const int directions_x[4]={1,0,-1,0};
/** @brief Zadaje numeryczny odpowiednik kierunków świata (E,N,W,S) na osi OY.
*/
static const int directions_y[4]={0,1,0,-1};

/** @brief Struktura reprezentująca pole na planszy.
 * utożsamiamy colour = area_id w niektórych opisach
 */
typedef struct{
	uint32_t player_id; 	///< id gracza zajmującego pole
	uint32_t area_id; 	///< id obszaru do którego należy pole
} field;

/** @brief Struktura reprezentująca gracza.
 * utożsamiamy colour = area_id w niektórych opisach
 */
typedef struct{
	bool* does_area_exist; 		///< tablica booli mówiąca czy i-ty obszar istnieje
	uint64_t no_busy_fields; 	///< liczba zajętych pól przez gracza
	bool golden_move_used; 		///< true jeśli golden_move został wykonany, false wpp
	uint32_t no_areas_used; 	///< liczba wystąpieć wartości true w bool[] does_area_exist
	uint32_t first_free_colour; 	///< zwraca id pierwszego nieużytego jeszcze obszaru
} player;

/** @brief Struktura przechowująca stan gry
 */
typedef struct{
	field** board; 		///< 2-wymiarowa tablica pól czyli plansza
	uint32_t width; 	///< szerokość planszy
	uint32_t height; 	///< wysokość planszy
	uint32_t no_players; 	///< liczba graczy uczestniczących w grze
	uint32_t max_no_areas; 	///< maksymalna liczba obszerów jaką może zająć gracz w danej grze
	player* playerlist; 	///< lista graczy uczestniczących w grze
} gamma_t;

/** @brief zwraca true jeśli gracz zajął już maksymalną liczbe obszarów, false wpp
 * param[in] g 		- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, o którego pytamy
 *
 * @return true jeżeli gracz zajął już wszystkie dostępne obszary, false wpp
 */
static bool player_all_areas_used(gamma_t *g, uint32_t player){
	return (g->playerlist)[player].no_areas_used == g->max_no_areas;
}

/** @brief zwraca pole o współżędnych x,y z gry g
 * param[in] g  - wskaźnik na strukturę opisującą stan gry
 * param[in] x 	- odcięta szukanego pola 
 * param[in] y 	- rzędna szukanego pola 
 *
 * @return pole o współrzędnych kartezjańskich (x,y) z planszy w grze g
 */
static field get_field(gamma_t *g, uint32_t x, uint32_t y){
	return (g->board)[y][x];
}

/** @brief zwraca wysokość planszy z gry g
 * param[in] g  - wskaźnik na strukturę opisującą stan gry
 *
 * @return wysokość planszy z gry g
 */
uint32_t get_height(gamma_t* g){
	return g->height;
}

/** @brief zwraca szerokość planszy z gry g
 * param[in] g  - wskaźnik na strukturę opisującą stan gry
 *
 * @return szerokość planszy z gry g
 */
uint32_t get_width(gamma_t* g){
	return g->width;
}

/** @brief zwraca objętość planszy z gry g
 * param[in] g  - wskaźnik na strukturę opisującą stan gry
 *
 * @return objętość planszy (width*height) z gry g
 */
static uint64_t board_size(gamma_t* g){
	return get_height(g)*get_width(g);
}

/** @brief ustala wartość area_id pola o wskazanych koordynatach na zadaną wartość
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] x 		- odcięta pola, które modyfikujemy
 * param[in] y 		- rzędna pola, które modyfikujemy
 * param[in] colour 	- kolor na który malujemy pole, czyli wartość area_id, którą chcemy nadać
 */
static void set_area_id(gamma_t *g, uint32_t x, uint32_t y, uint32_t colour){
	(g->board)[y][x].area_id = colour;
}

/** @brief ustala wartość player_id pola o wskazanych koordynatach na zadaną wartość
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] x 		- odcięta pola, które modyfikujemy
 * param[in] y 		- rzędna pola, które modyfikujemy
 * param[in] player 	- wartość player_id, którą nadajemy polu
 */
static void set_player_id(gamma_t* g, uint32_t x, uint32_t y, uint32_t player){
	(g->board)[y][x].player_id = player;
}

/** @brief zwiększa wartość player.no_areas_used o 1
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, którego modyfikujemy
 */
static void increase_player_no_areas_used(gamma_t* g, uint32_t player){
	(g->playerlist)[player].no_areas_used++;
}

/** @brief zmniejsza wartość player.no_areas_used o 1
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, którego modyfikujemy
 */
static void decrease_player_no_areas_used(gamma_t* g, uint32_t player){
	(g->playerlist)[player].no_areas_used--;
}

/** @brief zwraca wartość liczbę użytych obszarów przez gracza
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, o którego pytamy
 *
 * @return liczba użytych już obszarów przez gracza
 */
static uint32_t get_player_no_areas_used(gamma_t* g, uint32_t player){
	return (g->playerlist)[player].no_areas_used;
}

/** @brief zwraca pierwszy wolny kolor (area_id), na które możemy pomalować nowy obszar
 * zwraca poprawną wartość tylko w przypadku, gdy jeszcze nie wszystkie obszary zostały wykorzystane
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, o którego pytamy
 *
 * @return pierwszy wolny kolor, na który możemy pomalować nowy obszar
 */
static uint32_t get_player_first_free_colour(gamma_t* g, uint32_t player){
	return (g->playerlist)[player].first_free_colour;
}


/** @brief ustawia wartość player.first_free_colour na zadaną wartość
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, którego modyfikujemy
 * param[in] value 	- wartość jaką chcemy nadać zmiennej first_free_colour
 */
static void set_player_first_free_colour(gamma_t* g, uint32_t player, 
					 uint32_t value){
	(g->playerlist)[player].first_free_colour = value;
}

/** @brief ustawia wartość player.does_area_exist[area_id] na zadaną wartość
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, którego modyfikujemy
 * param[in] area_id 	- indeks w tablicy does_area_exist, który chcemy zmienić
 * param[in] value 	- wartość jaką chcemy nadać zmiennej does_area_exist[area_id]
 */
static void set_player_does_area_exist(gamma_t* g, uint32_t player, 
				uint32_t area_id, bool value){
	(g->playerlist)[player].does_area_exist[area_id] = value;
}

/** @brief zwraca true jeżeli obszar o zadanym id jest już w użyciu, false wpp
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, o którego pytamy
 * param[in] area	- id obszaru, o który pytamy
 * param[in] value 	- wartość jaką chcemy nadać zmiennej does_area_exist[area_id]
 *
 * @return true jeżeli obszar o zadanym id jest już w użyciu, false wpp 
 */
static bool get_player_does_area_exist(gamma_t* g, uint32_t player, uint32_t area){
	return (g->playerlist)[player].does_area_exist[area];
}

/** @brief ustawia wartość player.first_free_colour na następny pierwszy wolny kolor
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, którego modyfikujemy
 */
static void move_player_first_free_colour(gamma_t* g, uint32_t player){
	if(!player_all_areas_used(g, player)){
		uint32_t iterator = get_player_first_free_colour(g, player);	
		while(get_player_does_area_exist(g, player, iterator)){
			iterator = (iterator+1)%(g->max_no_areas);
		}
		set_player_first_free_colour(g, player, iterator);
	}
}

/** @brief zwraca true jeżeli gracz użył już golden_move w danej grze
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, o którego pytamy
 *
 * @return true jeżeli gracz użył już golden_move w danej grze
 */
static bool get_player_golden_move_used(gamma_t* g, uint32_t player){
	return (g->playerlist)[player].golden_move_used;
}

/** @brief ustawia wartość player.golden_mode_used na zadaną wartość
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, o którego modyfikujemy
 * param[in] value 	- wartość, którą nadajemy player.player_golden_move_used
 */
static void set_player_golden_move_used(gamma_t* g, uint32_t player, bool value){
	(g->playerlist)[player].golden_move_used = value;
}

/** @brief zwiększa player.no_busy_fields o 1
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, którego modyfikujemy
 */
static void increase_player_no_busy_fields(gamma_t* g, uint32_t player){
	(g->playerlist)[player].no_busy_fields++;
}

/** @brief zmniejsza player.no_busy_fields o 1
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, którego modyfikujemy
 */
static void decrease_player_no_busy_fields(gamma_t* g, uint32_t player){
	(g->playerlist)[player].no_busy_fields--;
}

/** @brief zwraca liczbę zajętych pól przez gracza
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player 	- id gracza, o którego pytamy
 *
 * @return liczba zajętych pól przez gracza
 */
static uint64_t get_player_no_busy_fields(gamma_t* g, uint32_t player){
	return (g->playerlist)[player].no_busy_fields;
}

/** @brief zwraca liczbę graczy uczestniczących w grze
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 *
 * @return liczba graczy uczestniczących w grze
 */
uint32_t get_no_players(gamma_t* g){
	return g->no_players;
}

/** @brief zwraca maksymalną liczbę obszarów, które może zająć gracz
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 *
 * @return maksymalna liczba obszarów, które może zająć gracz
 */
static uint32_t get_max_no_areas(gamma_t* g){
	return g->max_no_areas;
}

/** @brief zwraca true jeżeli pole należy do innego gracza niż ten, którego podaliśmy
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player  	- id gracza, o którego pytamy
 * param[in] x  	- odcięta pola, o które pytamy
 * param[in] y  	- rzędna pola, o które pytamy
 *
 * @return true jeżeli pole należy do innego gracza niż ten, którego podaliśmy
 */
static bool field_belongs_to_other_player(gamma_t* g,uint32_t player,
				   uint32_t x,uint32_t y){
	field f = get_field(g, x, y);
      	bool flague = true;
	flague = flague && f.player_id != 0;
	flague = flague && f.player_id != player;

	return flague;
}

/** @brief zwraca true jeżeli podane (x,y) mieszczą się w planszy
 * param[in] g  - wskaźnik na strukturę opisującą stan gry
 * param[in] x  - odcięta, o którą pytamy
 * param[in] y  - rzędna, o którą pytamy
 *
 * @return true jeżeli podane (x,y) mieszczą się w planszy
 */
static bool x_y_fit_the_board(gamma_t* g, uint32_t x, uint32_t y){
	bool flague = true;
	flague = flague && x<g->width;
        flague = flague && y<g->height;
        return flague;
}

/** @brief zwraca true jeżeli id gracza, które wskazaliśmy mieści się w zakresie graczy danej gry
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player  	- id gracza, o którego pytamy
 *
 * @return true jeżeli id gracza, które wskazaliśmy mieści się w zakresie graczy danej gry
 */
static bool player_fit_the_range(gamma_t* g, uint32_t player){
	return (player>0 && player<=get_no_players(g));
}

/** @brief zwalnia pamięć zaalokowaną na planszę dla danej gry
 * param[in] g  - wskaźnik na strukturę opisującą stan gry
 */
static void free_board(gamma_t* g){
	if(g != NULL && g->board != NULL){
		for(uint32_t i=0;i<g->height;i++){
			if(g->board[i] != NULL)
				free(g->board[i]);
		}
		free(g->board);
	}
}

/** @brief zwalnia pamięć zaalokowaną na listę graczy i każdego z graczy dla danej gry
 * param[in] g  - wskaźnik na strukturę opisującą stan gry
 */
static void free_player_list(gamma_t* g){
	if(g != NULL && g->playerlist != NULL){
		for(uint32_t i=0;i<=g->no_players;i++){
			if(((g->playerlist)[i]).does_area_exist != NULL)
				free(((g->playerlist)[i]).does_area_exist);
		}
		free(g->playerlist);
	}
}

/** @brief Usuwa strukturę przechowującą stan gry.
 * Usuwa z pamięci strukturę wskazywaną przez @p g.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] g       – wskaźnik na usuwaną strukturę.
 */
void gamma_delete(gamma_t* g){
	if(g != NULL){
		free_board(g);
		free_player_list(g);
		free(g);
	}
}

/** @brief sprawdza czy zadany input spełnia założenia gamma_new
 * @param[in] width 	– szerokość planszy
 * @param[in] height 	– wysokość planszy
 * @param[in] players   – liczba graczy
 * @param[in] areas    	– liczba obszarów
 *
 * @return true jeżeli input spełnia założenia, false wpp
 */
static bool gamma_new_valid_input(uint32_t width, uint32_t height,
	                   	  uint32_t players, uint32_t areas){
	bool flague = true;
	flague = flague && (width>0);
	flague = flague && (height>0);
	flague = flague && (players>0);
	flague = flague && (areas>0);
	return flague;
}

/** @brief alokuje pamięć na planszę, ustala success na false jeżeli się nie udało
 * param[in] width 	- szerokość
 * param[in] height 	- wysokość
 * param[in] success 	- referencja do flagi mówiącej czy wszystkie procesy zakończyły się powodzeniem
 *
 * @return wskaźnik na utworzoną planszę
 */
static field** alloc_board(uint32_t width, uint32_t height, bool* success){
	field** board = safe_malloc(height*sizeof(field*), success);
	if(board != NULL){
		for(uint32_t i=0;i<height;i++){
			board[i] = safe_malloc(width*sizeof(field), success);
		}
	}
	return board;
}

/** @brief ustawia wszystkie pola danej planszy na (player_id, area_id) = (0, 0)
 * param[in] board 	- wskaźnik do planszy, którą modyfikujemy
 * param[in] width 	- szerokość
 * param[in] height 	- wysokość
 */
static void fill_board_with_default_values(field** board, 
				    	   uint32_t width, uint32_t height){
	for(uint32_t i=0;i<height;i++){
		for(uint32_t j=0;j<width;j++){
			board[i][j].player_id = 0;
			board[i][j].area_id = 0;
		}
	}
}

/** @brief tworzy planszę z domyślnymi wartościami pól ((0, 0))
 * ustala success na false jeżeli się nie udało
 * param[in] width 	- szerokość
 * param[in] height 	- wysokość
 * param[in] success 	- referencja do flagi mówiącej czy wszystkie procesy zakończyły się powodzeniem
 *
 * @return wskaźnik na utworzoną planszę
 */
static field** make_board(uint32_t width, uint32_t height, bool* success){
	field** board = alloc_board(width, height, success);
	if(*success == true){
		fill_board_with_default_values(board, width, height);
	}
	return board;
}

/** @brief ustawia początkowe wartości danego gracza
 * param[in] p 		- gracz, którego modyfikujemy
 * param[in] areas 	- liczba dostępnych obszarów dla gracza
 */
static void set_player_to_default(player* p, uint32_t areas){
 	p->no_busy_fields = 0;
	p->golden_move_used = false;
	p->no_areas_used = 0;
	p->first_free_colour = 0;
	for(uint32_t i=0;i<areas;i++){
		(p->does_area_exist)[i] = false;
	}
}

/** @brief tworzy i zwraca listę graczy z domyślymi wartościami startowymi
 * param[in] players 	- liczba graczy w grze (długość listy)
 * param[in] areas 	- liczba dostępnych obszarów dla gracza
 * param[in] success 	- referencja do flagi mówiącej czy wszystkie procesy zakończyły się powodzeniem
 *
 * @return wskaźnik na listę graczy z domyślnymi wartościami startowymi
 */
static player* make_playerlist(uint32_t players, uint32_t areas, bool* success){
	player* playerlist = safe_malloc((players+1)*sizeof(player), success);
	if(playerlist != NULL){
		for(uint32_t i=0;i<=players;i++){
			playerlist[i].does_area_exist = NULL;
		}
		for(uint32_t i=0;i<=players;i++){
			bool* bool_tab = safe_malloc(areas*sizeof(bool), success);
			if(bool_tab != NULL){
				playerlist[i].does_area_exist = bool_tab;
				set_player_to_default(&playerlist[i], areas);
			} else{
				for(uint32_t j=0;j<i;j++){
					free(playerlist[j].does_area_exist);
				}
				break;
			}
		}
	}
	return playerlist;
}

/** @brief Tworzy strukturę przechowującą stan gry.
 * Alokuje pamięć na nową strukturę przechowującą stan gry.
 * Inicjuje tę strukturę tak, aby reprezentowała początkowy stan gry.
 * W przypadku niepowodzenia któregoś z procesów inicjalizacji czyści dotychczasowo
 * zaalokowaną pamięć i zwraca NULL
 * @param[in] width   – szerokość planszy, liczba dodatnia,
 * @param[in] height  – wysokość planszy, liczba dodatnia,
 * @param[in] players – liczba graczy, liczba dodatnia,
 * @param[in] areas   – maksymalna liczba obszarów,
 *                      jakie może zająć jeden gracz.
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci lub któryś z parametrów jest niepoprawny.
 */
static gamma_t* make_game_state(uint32_t width, uint32_t height,
			 	uint32_t players, uint32_t areas){
	bool success = true;
	field** board = make_board(width, height, &success);
    player* playerlist = make_playerlist(players, areas, &success);
	gamma_t* game_state = safe_malloc(sizeof(gamma_t), &success);
	if(game_state != NULL){
		game_state->board = board;
		game_state->width = width;
		game_state->height = height;
		game_state->no_players = players;
		game_state->max_no_areas = areas;
		game_state->playerlist = playerlist;
	}
	if(success == false){
		if(game_state != NULL){
			gamma_delete(game_state);
		}
		else{
			if(board != NULL){
				for(uint32_t i=0;i<height;i++){
					free(board[i]);
				}
				free(board);
			}
			if(playerlist != NULL){
				for(uint32_t i=0; i<=players;i++){
					if(((playerlist)[i]).does_area_exist != NULL)
						free(((playerlist)[i]).does_area_exist);
				}
				free(playerlist);
			}
		}
		return NULL;
	}

	return game_state;
}

/** @brief Tworzy strukturę przechowującą stan gry.
 * Alokuje pamięć na nową strukturę przechowującą stan gry.
 * Inicjuje tę strukturę tak, aby reprezentowała początkowy stan gry.
 * @param[in] width   – szerokość planszy, liczba dodatnia,
 * @param[in] height  – wysokość planszy, liczba dodatnia,
 * @param[in] players – liczba graczy, liczba dodatnia,
 * @param[in] areas   – maksymalna liczba obszarów,
 *                      jakie może zająć jeden gracz.
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci lub któryś z parametrów jest niepoprawny.
 */
gamma_t* gamma_new(uint32_t width, uint32_t height,
		   uint32_t players, uint32_t areas){
	if(gamma_new_valid_input(width, height, players, areas)){
		gamma_t* game_state = make_game_state(width, height,
						      players, areas);
		return game_state;
	}
	else
		return NULL;
}

/** @brief zwraca tablicę wartości logicznych reprezentujących sąsiedztwo pola danego gracza z innymi jego polami
 * tablica reprezentuje kierunki świata [0,1,2,3] = [E,N,W,S]
 * tab[i] = true jeżeli w kierunku odpowiadającym i-temu indexowi znajduje się pole gracza.
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player  	- id gracza, o którego pytamy
 * param[in] x  	- odcięta pola, o które pytamy
 * param[in] y  	- rzędna pola, o które pytamy
 *
 * @return tablica wartości logicznych rozmiaru 4 mówiąca, w których kierunkach są inne pola danego gracza
 */
static bool* player_areas_nearby(gamma_t* g, uint32_t player, 
				 uint32_t x, uint32_t y){
	bool* tab = safer_malloc(4*sizeof(bool));
	bool N = false;
	bool S = false;
	bool E = false;
	bool W = false;

	if(x<g->width-1){
		E = (get_field(g, x+1, y).player_id == player)? true : false;
	}
	if(x>0){
		W = (get_field(g, x-1, y).player_id == player)? true : false;
	}
	if(y<g->height-1){
		N = (get_field(g, x, y+1).player_id == player)? true : false;
	}
	if(y>0){
		S = (get_field(g, x, y-1).player_id == player)? true : false;
	}
	tab[0] = E;
	tab[1] = N;
	tab[2] = W;
	tab[3] = S;
	return tab;
}

/** @brief zwraca true jeżeli w sąsiedztwie danego pola istnieje inne pole danego gracza
 * param[in] g  	- wskaźnik na strukturę opisującą stan gry
 * param[in] player  	- id gracza, o którego pytamy
 * param[in] x  	- odcięta pola, o które pytamy
 * param[in] y  	- rzędna pola, o które pytamy
 *
 * @return true jeżeli w sąsiedztwie danego pola istnieje inne pole danego gracza
 */
static bool are_player_areas_nearby(gamma_t* g, uint32_t player, 
				    uint32_t x, uint32_t y){
	bool* tab = player_areas_nearby(g, player, x , y);
	bool flague = false;
	for(int i=0;i<4;i++){
		flague = flague || tab[i];
	}
	free(tab);
	return flague;
}

/** @brief sprawdza czy zadany input spełnia założenia gamma_move
 * @param[in] g 	– wskaźnik na strukturę przechowującą stan gry
 * @param[in] player    – id gracza, który ma wykonać ruch
 * @param[in] x   	– odcięta pola, na które ma zostać wykonany ruch
 * @param[in] y   	– rzędna pola, na które ma zostać wykonany ruch
 *
 * @return zwraca true jeżeli input spełnia założenia, false wpp
 */
static bool gamma_move_valid_input(gamma_t* g, uint32_t player, 
				   uint32_t x, uint32_t y){
	bool flague = true;
	if(g!=NULL){
		flague = flague && player_fit_the_range(g, player);
		flague = flague && x_y_fit_the_board(g, x, y);
		flague = flague && ((g->board)[y][x].player_id == 0);
		flague = flague && (!player_all_areas_used(g, player) ||
				    are_player_areas_nearby(g, player, x, y));
		return flague;
	}
	else 
		return false;
}

/** @brief koloruje dane pole na nowy, jeszcze nie użyty kolor
 * inkrementuje player.no_areas_used
 * po wykonaniu malowania aktualizuje player.first_free_colour
 * @param[in] g 	– wskaźnik na strukturę przechowującą stan gry
 * @param[in] player    – id gracza, do którego będzie należeć pokolorowane pole
 * @param[in] x   	– odcięta pola, które kolorujemy
 * @param[in] y   	– rzędna pola, które kolorujemy  
 */
static void colour_new(gamma_t* g, uint32_t player, uint32_t x, uint32_t y){
	increase_player_no_areas_used(g, player);
	uint32_t first_free_colour = get_player_first_free_colour(g, player);
	set_area_id(g, x, y, first_free_colour);
	set_player_does_area_exist(g, player , first_free_colour, true);
	if(!player_all_areas_used(g, player)){
		move_player_first_free_colour(g, player);
	}
}

/** @brief koloruje caly obszar na zadany kolor
 * implementacja na podstawie DFS, bez tablicy visited, bo zawsze malujemy na "świerzy" kolor
 * @param[in] g 	– wskaźnik na strukturę przechowującą stan gry
 * @param[in] player    – id gracza, do którego będzie należeć pokolorowane pole
 * @param[in] x   	– odcięta pola, które kolorujemy
 * @param[in] y   	– rzędna pola, które kolorujemy 
 * @param[in] colour 	- kolor na który kolorujemy obszar 
 */
static void colour_area(gamma_t* g, uint32_t player, 
		 	uint32_t x, uint32_t y, uint32_t colour){
	set_area_id(g, x, y, colour);
	bool* tab = player_areas_nearby(g, player, x, y); 
	for(int i=0;i<4;i++){
		if(tab[i]){
			uint32_t new_x = x+directions_x[i];
			uint32_t new_y = y+directions_y[i];		
			if(colour != get_field(g, new_x, new_y).area_id){
				colour_area(g, player, new_x, new_y, colour);
			}
		}
	}
	free(tab);
}

/** @brief koloruje obszar na zadany kolor i dba o aktualizację parametrów danego gracza
 * @param[in] g 	– wskaźnik na strukturę przechowującą stan gry
 * @param[in] player    – id gracza, do którego będzie należeć pokolorowane pole
 * @param[in] x   	– odcięta pola, które kolorujemy
 * @param[in] y   	– rzędna pola, które kolorujemy 
 * @param[in] colour 	- kolor na który kolorujemy obszar 
 */
static void perform_area_colouring(gamma_t* g, uint32_t player,
	       		           uint32_t x, uint32_t y, uint32_t colour){
	set_area_id(g, x, y, colour);	
	bool* tab = player_areas_nearby(g, player, x, y); 
	for(int i=0;i<4;i++){
		if(tab[i]){
			uint32_t new_x = x+directions_x[i];
			uint32_t new_y = y+directions_y[i];
			uint32_t field_colour = get_field(g, new_x, new_y).area_id;
			if(colour != field_colour){
				decrease_player_no_areas_used(g, player);
				set_player_does_area_exist(g, player, 
							   field_colour, false);
				colour_area(g, player, new_x, new_y, colour);
				move_player_first_free_colour(g, player);
			}
		}
	}
	free(tab);
}

/** @brief Wykonuje ruch przy założeniu, że input jest poprawny
 * Ustawia pionek gracza @p player na polu (@p x, @p y).
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 */
static void gamma_make_move(gamma_t *g, uint32_t player, 
			    uint32_t x, uint32_t y){
	set_player_id(g, x, y, player);
	bool* tab = player_areas_nearby(g, player, x, y);
	uint32_t colour;
	bool is_colour_choosen = false;
	for(int i=0;i<4;i++){
		if(tab[i]){
			uint32_t new_x = x+directions_x[i];
			uint32_t new_y = y+directions_y[i];
			colour = get_field(g, new_x, new_y).area_id;
			is_colour_choosen = true;
		}
	}
	if(is_colour_choosen){
		perform_area_colouring(g, player, x, y, colour);
	}
	else
		colour_new(g, player, x, y);
	free(tab);
}

/** @brief Wykonuje ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y).
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy ruch jest nielegalny lub któryś z parametrów jest niepoprawny.
 */
bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y){
	if(gamma_move_valid_input(g, player, x, y)){
		gamma_make_move(g, player, x, y);
		increase_player_no_busy_fields(g, player);
		return true;
	}	
	else
		return false;
}

/** @brief sprawdza czy inni gracze zajeli już jakieś pola
 * @param[in] g 	– wskaźnik na strukturę przechowującą stan gry
 * @param[in] player    – id gracza, o którego pytamy
 *
 * @return true jeżeli inni gracze posiadają zajęte pola, false wpp
 */
static bool other_players_have_busy_fields(gamma_t *g, uint32_t player){
	uint32_t no_players = get_no_players(g);
	bool flague = false;
	for(uint32_t i=1;i<=no_players;i++){
		if(i!=player && get_player_no_busy_fields(g, i)>0){
			flague = true;
			break;
		}
	}
	return flague;
}

/** @brief sprawdza czy zadany input spełnia założenia gamma_golden_possible
 * @param[in] g 	– wskaźnik na strukturę przechowującą stan gry
 * @param[in] player    – id gracza, o którego pytamy
 *
 * @return true jeżeli input spełnia założenia, false wpp
 */
static bool gamma_golden_possible_valid_input(gamma_t* g, uint32_t player){
	bool flague = true;
	if(g != NULL){
		flague = flague && player_fit_the_range(g, player);
		return flague;
	}
	else
		return false;
}



/** @brief symuluje złoty ruch. Zwraca true jeżeli dało się go wykonać
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli dało się wykonać ruch
 */
static bool gamma_try_golden_move_no_sideeffect(gamma_t* g, uint32_t x, uint32_t y){
	field primal_field = get_field(g, x, y);
	uint32_t primal_player = primal_field.player_id;
	uint32_t primal_colour = primal_field.area_id;
	bool* tab = player_areas_nearby(g, primal_player, x, y);	
	set_area_id(g, x, y, 0);
	set_player_id(g, x, y, 0);
	set_player_does_area_exist(g, primal_player, primal_colour, false);
	decrease_player_no_areas_used(g, primal_player);
	move_player_first_free_colour(g, primal_player);
	uint32_t no_areas_that_will_be_added = 0;
	for(int i=0;i<4;i++){
		if(tab[i]){
			uint32_t new_x = x+directions_x[i];
			uint32_t new_y = y+directions_y[i];
			if(get_field(g, new_x, new_y).area_id != TRANSPARENT){
				no_areas_that_will_be_added++;
				colour_area(g, primal_player, new_x, new_y, 
					    TRANSPARENT);
			}
		}
		
	}
	free(tab);
	bool answ = false; 
	if(get_player_no_areas_used(g, primal_player)+
	   no_areas_that_will_be_added <= get_max_no_areas(g)){
		answ = true;
	}

	 //undo things
	set_area_id(g, x, y, primal_colour);
    set_player_id(g, x, y, primal_player);
   	set_player_does_area_exist(g, primal_player, primal_colour, true);
	increase_player_no_areas_used(g, primal_player);
	move_player_first_free_colour(g, primal_player);
	colour_area(g, primal_player, x, y, primal_colour);
	return answ;
	
}

//tylko deklaracja, opis niżej
static bool gamma_golden_move_valid_input(gamma_t *g, uint32_t player, 
				   	  uint32_t x, uint32_t y);


/** @brief iteruje try_golden_move_no_sideeddect.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Wartość @p true, jeżeli udałoby się wykonać 
 * try_golden_move_no_sideeddect no chodź jednym polu
 */
static bool is_golden_move_makable_anywhere(gamma_t* g, uint32_t player){
	for(uint32_t x=0; x<get_width(g); x++){
		for(uint32_t y=0; y<get_height(g); y++){
			if(gamma_golden_move_valid_input(g, player, x, y)){
				 if(gamma_try_golden_move_no_sideeffect(g, x, y)){
				 	return true;
				 }
			}
		}
	}	
	return false;
}


/** @brief Sprawdza, czy gracz może wykonać złoty ruch.
 * Sprawdza, czy gracz @p player jeszcze nie wykonał w tej rozgrywce złotego
 * ruchu i jest przynajmniej jedno pole zajęte przez innego gracza.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli gracz jeszcze nie wykonał w tej rozgrywce
 * złotego ruchu i jest przynajmniej jedno pole zajęte przez innego gracza,
 * a @p false w przeciwnym przypadku.
 */
bool gamma_golden_possible(gamma_t *g, uint32_t player){
	bool flague = true;
	if(gamma_golden_possible_valid_input(g, player)){
		flague = flague && !get_player_golden_move_used(g, player);
		flague = flague && other_players_have_busy_fields(g, player);
		if(flague) //expensive operation
			flague = flague && is_golden_move_makable_anywhere(g, player);
		return flague;
	}
	else
		return false;
}

/** @brief sprawdza czy zadany input spełnia założenia gamma_golden_move
 * @param[in] g 	– wskaźnik na strukturę przechowującą stan gry
 * @param[in] player    – id gracza, który ma wykonać ruch
 * @param[in] x   	– odcięta pola, na które ma zostać wykonany ruch
 * @param[in] y   	– rzędna pola, na które ma zostać wykonany ruch
 *
 * @return zwraca true jeżeli input spełnia założenia, false wpp
 */
static bool gamma_golden_move_valid_input(gamma_t *g, uint32_t player, 
				   	  uint32_t x, uint32_t y){
	bool flague = true;
	if(g!=NULL){
		flague = flague && player_fit_the_range(g, player);
		flague = flague && !get_player_golden_move_used(g, player);
		flague = flague && x_y_fit_the_board(g, x, y);
		if(flague){
     	  		flague = flague && (!player_all_areas_used(g, player) ||
        	                    are_player_areas_nearby(g, player, x, y));
			flague = flague && field_belongs_to_other_player(g, player, x, y);
		}
       	 	return flague;
	}
	else
		return false;
}

/** @brief funkcja pomocnicza do gamma_make_move
 * koloruje obszar na zadany kolor i dba o aktualizację parametrów
 * @param[in] g 		- wskaźnik na strukturę przechowującą stan gry
 * @param[in] primal_player 	- id gracza, którego modyfikujemy
 * @param[in] new_x 		- odcięta pola, które modyfikujemy
 * @param[in] new_y 		- rzędna pola, które modyfikujemy
 */
static void perform_transparent_colouring(gamma_t* g, uint32_t primal_player, 
				 	  uint32_t new_x, uint32_t new_y){
	increase_player_no_areas_used(g, primal_player);
        uint32_t fst_col = get_player_first_free_colour(g, primal_player);
        set_player_does_area_exist(g, primal_player, fst_col, true);
        colour_area(g, primal_player, new_x, new_y, fst_col);
        move_player_first_free_colour(g, primal_player);
}

/** @brief Wykonuje złoty ruch przy założeniu, że input jest poprawny, nie naruszy on limitu obszarów
 * Ustawia pionek gracza @p player na polu (@p x, @p y) zajętym przez innego
 * gracza, usuwając pionek innego gracza.
 * @param[in,out] g   		- wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  		- numer gracza, liczba dodatnia niewiększa od wartości
 *                      	  @p players z funkcji @ref gamma_new,
 * @param[in] primal_player 	- id gracza, do którego pierwotnie należało pole
 * @param[in] x       		- numer kolumny, liczba nieujemna mniejsza od wartości
 *                      	  @p width z funkcji @ref gamma_new,
 * @param[in] y       		- numer wiersza, liczba nieujemna mniejsza od wartości
 *                      	  @p height z funkcji @ref gamma_new.
 */
static void gamma_make_golden_move(gamma_t* g, uint32_t player, 
				   uint32_t primal_player,
	       	                   uint32_t x, uint32_t y){
	set_area_id(g, x, y, 0);
	set_player_id(g, x, y, 0);
	gamma_move(g, player, x, y);
	bool* tab = player_areas_nearby(g, primal_player, x, y);
	for(int i=0;i<4;i++){
		if(tab[i]){
			uint32_t new_x = x+directions_x[i];
			uint32_t new_y = y+directions_y[i];
			field new_field = get_field(g, new_x, new_y);
			if(new_field.area_id == TRANSPARENT){
				perform_transparent_colouring(g, primal_player,
							      new_x, new_y);
			}
		}
	}
	set_player_golden_move_used(g, player, true);
	decrease_player_no_busy_fields(g, primal_player);
	free(tab);
}

/** @brief Wykonuje złoty ruch przy założeniu, że input jest poprawny
 * Sprawdza czy golden_move nie naruszy limitu obszarów w danej, grze
 * Ustawia pionek gracza @p player na polu (@p x, @p y) zajętym przez innego
 * gracza, usuwając pionek innego gracza.
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy gracz wykorzystał już swój złoty ruch, ruch jest nielegalny
 * lub któryś z parametrów jest niepoprawny.
 */
static bool gamma_try_golden_move(gamma_t* g, uint32_t player, 
		 		  uint32_t x, uint32_t y){
	field primal_field = get_field(g, x, y);
	uint32_t primal_player = primal_field.player_id;
	uint32_t primal_colour = primal_field.area_id;
	bool* tab = player_areas_nearby(g, primal_player, x, y);	
	set_area_id(g, x, y, 0);
	set_player_id(g, x, y, 0);
	set_player_does_area_exist(g, primal_player, primal_colour, false);
	decrease_player_no_areas_used(g, primal_player);
	move_player_first_free_colour(g, primal_player);
	uint32_t no_areas_that_will_be_added = 0;
	for(int i=0;i<4;i++){
		if(tab[i]){
			uint32_t new_x = x+directions_x[i];
			uint32_t new_y = y+directions_y[i];
			if(get_field(g, new_x, new_y).area_id != TRANSPARENT){
				no_areas_that_will_be_added++;
				colour_area(g, primal_player, new_x, new_y, 
					    TRANSPARENT);
			}
		}
		
	}
	free(tab);
	if(get_player_no_areas_used(g, primal_player)+
	   no_areas_that_will_be_added <= get_max_no_areas(g)){
		gamma_make_golden_move(g, player, primal_player, x, y);
		return true;
	}
	else{ //undo things
		set_area_id(g, x, y, primal_colour);
    	    	set_player_id(g, x, y, primal_player);
   		set_player_does_area_exist(g, primal_player, primal_colour, true);
		increase_player_no_areas_used(g, primal_player);
		move_player_first_free_colour(g, primal_player);
		colour_area(g, primal_player, x, y, primal_colour);
		return false;
	}
}

/** @brief Wykonuje złoty ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y) zajętym przez innego
 * gracza, usuwając pionek innego gracza.
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy gracz wykorzystał już swój złoty ruch, ruch jest nielegalny
 * lub któryś z parametrów jest niepoprawny.
 */
bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y){
	if(gamma_golden_move_valid_input(g, player, x, y)){
		return gamma_try_golden_move(g, player, x, y);
	}
	else
		return false;
}

/** @brief sprawdza czy zadany input spełnia założenia gamma_busy_fields
 * @param[in] g 	– wskaźnik na strukturę przechowującą stan gry
 * @param[in] player    – id gracza, o którego pytamy
 *
 * @return true jeżeli input spełnia założenia, false wpp
 */
static bool gamma_busy_fields_valid_input(gamma_t* g, uint32_t player){
	bool flague = true;
	if(g != NULL){
		flague = flague && player_fit_the_range(g, player);
		return flague;
	}
	else
		return false;
}

/** @brief Podaje liczbę pól zajętych przez gracza.
 * Podaje liczbę pól zajętych przez gracza @p player.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól zajętych przez gracza lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
uint64_t gamma_busy_fields(gamma_t *g, uint32_t player){
	if(gamma_busy_fields_valid_input(g, player)){
		return get_player_no_busy_fields(g, player);
	}
	else
		return 0;
}

/** @brief zwraca liczbę wszystkich zajętych pól przez wszystkich graczy w grze
 * @param[in] g - wskaźnik na strukturę przechowującą stan gry
 *
 * @return liczba wszystkich zajętych pól przez wszystkich graczy w grze
 */
static uint64_t sum_of_all_players_busy_fields(gamma_t* g){
	uint64_t sum = 0;
	for(uint32_t i=1;i<=get_no_players(g);i++){
		sum+= get_player_no_busy_fields(g, i);
	}
	return sum;
}

/** @brief zwraca liczbę wszystkich pól danego gracza będących przegiem dowolnego obszaru
 * @param[in] g 	- wskaźnik na strukturę przechowującą stan gry
 * @param[in] player 	- id gracza, o którego pytamy
 *
 * @return liczba wszystkich pól danego gracza będących przegiem dowolnego obszaru
 */
static uint64_t count_boarder_size(gamma_t* g, uint32_t player){
	uint64_t count = 0;
	for(uint32_t i=0;i<get_height(g);i++){
		for(uint32_t j=0;j<get_width(g);j++){
			field f = get_field(g, j, i);
			bool flague = true;
			flague = flague && (f.player_id == 0);
			flague = flague && (are_player_areas_nearby(g, player,
								    j, i));
			if(flague)
				count++;
		}
	}
	return count;
}

/** @brief sprawdza czy zadany input spełnia założenia gamma_free_fields
 * @param[in] g 	– wskaźnik na strukturę przechowującą stan gry
 * @param[in] player   – id gracza, o którego pytamy
 *
 * @return zwraca true jeżeli input spełnia założenia, false wpp
 */
static bool gamma_free_fields_valid_input(gamma_t* g, uint32_t player){
	if(g != NULL){
		bool flague = true;
		flague = flague && player_fit_the_range(g, player);
		return flague;
	}
	else
		return false;
}

/** @brief Podaje liczbę pól, jakie jeszcze gracz może zająć.
 * Podaje liczbę wolnych pól, na których w danym stanie gry gracz @p player może
 * postawić swój pionek w następnym ruchu.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól, jakie jeszcze może zająć gracz lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
uint64_t gamma_free_fields(gamma_t *g, uint32_t player){
	if(gamma_free_fields_valid_input(g, player)){
		if(player_all_areas_used(g, player)){
			return count_boarder_size(g, player);
		}
		else
			return board_size(g)-sum_of_all_players_busy_fields(g);
	}
	else
		return 0;
}

/** @brief Daje napis opisujący stan planszy jeżeli liczba graczy <10
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy. Przykład znajduje się w pliku gamma_test.c.
 * Funkcja wywołująca musi zwolnić ten bufor.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry.
 * 
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
static char* print_board_without_space(gamma_t* g){
	if(g != NULL){
		char* buffer = malloc((board_size(g)+get_height(g)+1)*sizeof(char));
		if(buffer != NULL){
			uint32_t h = get_height(g);
			uint32_t w = get_width(g);
			uint64_t it = 0;
			for(int i=h-1;i>=0;i--){
				for(uint32_t j=0;j<w;j++){
					field f = get_field(g,j,i);
					buffer[it] = f.player_id == 0? '.' : (f.player_id+'0');
					it++;
				}
				buffer[it] = '\n';
				it++;
			}
			buffer[it]='\0';
		}
		return buffer;
	} else{
		return NULL;
	}
}

/** @brief logarytm dziesiętny zaokrąglony w dół lub 1 jeżeli n == 0
 * param[in] n 	- liczba, o której długość pytamy
 *
 * @return log(n) lub 1 jeżeli n == 0
 */
static uint32_t len(uint32_t n){
	int log=0;
	if(n == 0)
		return 1;
	while(n!=0){
		n/=10;
		log++;
	}
	return log;
}

/** @brief zwraca największe id gracza jakie występuje na planszy
 * param[in] g 	- wskaźnik na strukturę przechowującą stan gry
 *
 * @return największe id gracza jakie występuje na planszy
 */
static uint32_t max_player_id_on_board(gamma_t* g){
	uint32_t p = 0;
	for(uint32_t i = get_no_players(g); i!=0; i--){
		if(gamma_busy_fields(g, i) != 0){
			p=i;
			break;
		}
	}
	/*if(g != NULL){
		for(uint32_t i=0;i<get_height(g);i++){
			for(uint32_t j=0;j<get_width(g);j++){
				field f = get_field(g, j, i);
				if(f.player_id>p)
					p=f.player_id;
			}
		}
	}*/
	return p;
}

/** @brief zwraca długość napisu, jaki zajmuje największy gracz do wypisania
 * param[in] g 	- wskaźnik na strukturę przechowującą stan gry
 *
 * @return log(największy aktywny gracz na planszy) zaokrąglony w dół
 */
uint32_t length_of_max_player_id_on_board(gamma_t* g){
	uint32_t max_player_id = max_player_id_on_board(g);
	return len(max_player_id);
}

/** @brief wpisuje id gracza w ustalone miejsce (ptr), a reszte uzupełnia spacjami
 * Dba o wypełnienie wszystkich miejsc, które powinny zostać wypełnione, a nie są
 * przez spacje. Id gracza jest zapisywane jako ciąg charów, pojedynczych cyfr.
 * param[in, out] buffer 		- pointer na buffer, do którego wpisujemy
 * param[in] ptr 			- pierwszy indeks, który mamy nadpisać
 * param[in] no_places_it_should_take 	- ile miejsc powinniśmy zapisać w bufferze
 * param[in] player_id 			- id gracza, którego wpisujemy
 */
static void insert_player_id_in_buffer(char* buffer, uint32_t ptr,
				       uint32_t no_places_it_should_take,
				       uint32_t player_id){
	if(player_id != 0){
		uint32_t player_length = len(player_id);
		for(int i=player_length-1;i>=0;i--){
			buffer[ptr+i] = player_id%10 + '0';
			player_id/=10;
		}
		for(uint32_t i=player_length;i<no_places_it_should_take;i++){
			buffer[ptr+i] = ' ';
		}
	}
	else{
		buffer[ptr] = '.';
		for(uint32_t i=1;i<no_places_it_should_take;i++)
			buffer[ptr+i] = ' ';
	}
}

/** @brief Daje napis opisujący stan planszy jeżeli w grze aktywny jest gracz o id >=10
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy. Przykład znajduje się w pliku gamma_test.c.
 * Funkcja wywołująca musi zwolnić ten bufor.
 * @param[in] g       		- wskaźnik na strukturę przechowującą stan gry.
 * @param[in] size_of_pocket 	- liczba znaków jaką powinna zająć każda wypisana liczba
 *
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
static char* print_board_with_space(gamma_t* g, uint32_t size_of_pocket){
	uint32_t h = get_height(g);
	uint32_t w = get_width(g);
	char* buffer = malloc((h*w*(size_of_pocket+1)+1)*sizeof(char));
	if(buffer == NULL)
		return NULL;
	uint32_t it = 0;
	for(int i=h-1;i>=0;i--){
		for(uint32_t j=0;j<w;j++){
			field f = get_field(g,j,i);
			insert_player_id_in_buffer(buffer, it, size_of_pocket, 
						   f.player_id);
			it+= size_of_pocket;
			if(j!=w-1){
				buffer[it] = ' ';
				it++;
			}
		}
		buffer[it] = '\n';
		it++;
	}
	buffer[it] = '\0';
	return buffer;
}

/** @brief Daje napis opisujący stan planszy.
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy. Przykład znajduje się w pliku gamma_test.c.
 * Funkcja wywołująca musi zwolnić ten bufor.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry.
 * 
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
char* gamma_board(gamma_t *g){
	if(g != NULL){
		uint32_t size_of_pocket = length_of_max_player_id_on_board(g);
	 	if(size_of_pocket>1){
	 		return print_board_with_space(g, size_of_pocket);
		 }
		 else{
			return print_board_without_space(g);
		 }
	}
	else
		return NULL;
 }