#include <stdio.h>
#include <stdlib.h>
#include "logic.h"

int main(){

	game_and_mode* gm = malloc(sizeof(game_and_mode));
	int line = 0;

	if(gm != NULL){
		if(set_game_and_mode(gm, &line)){
			if(gm->game != NULL){
				if(gm->mod == Batch){
					play_batch(gm->game, &line);
					gamma_delete(gm->game);
					free(gm);
				}
				else if(gm->mod == Interactive){
					play_interactive(gm->game);
					gamma_delete(gm->game);
					free(gm);
				}
				else{
					fail("SET_GAME_AND_MODE FAILED TO SET MODE PROPERLY");
				}
			} else{
				fail("SET_GAME_AND_MODE FAILED TO SET GAME PROPERLY");
			}
		} else{
			free(gm);
		}
	}

	return 0;
}	