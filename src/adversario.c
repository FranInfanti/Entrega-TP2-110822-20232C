#include <time.h>
#include <stdlib.h>
#include "lista.h"
#include "juego.h"
#include "adversario.h"
#include "pokemon.h"
#include "hash.h"

struct adversario {
	hash_t *pokemones;
	lista_t *listado;
};

adversario_t *adversario_crear(lista_t *pokemon)
{
	if (!pokemon)
		return NULL;

	struct adversario *ia = calloc(1, sizeof(struct adversario));
	if (!ia)
		return NULL;

	ia->listado = pokemon;
	ia->pokemones = hash_crear(3);
	if (!ia->pokemones) {
		free(ia);
		return NULL;
	}

	return ia;
}

bool adversario_seleccionar_pokemon(adversario_t *adversario, char **nombre1,
				    char **nombre2, char **nombre3)
{
	if (!adversario)
		return false;
/* 
	int posicion[] = {0,0,0};
	for (int i = 0; i < 3; i++) 
		posicion[i] = (int)rand() % lista_tamanio(adversario->listado);
		
	void **elementos[] = {NULL, NULL, NULL};
	for (int i = 0; i < 3; i++)
		elementos[i] = lista_elemento_en_posicion(adversario->listado, posicion[i]);

	if (nombre1)
		*nombre1 = pokemon_nombre(elementos[0]);
	if (nombre2)
		*nombre2 = pokemon_nombre(elementos[1]);
	if (nombre3)
		*nombre3 = pokemon_nombre(elementos[2]);
*/
	return true;
}

bool adversario_pokemon_seleccionado(adversario_t *adversario, char *nombre1,
				     char *nombre2, char *nombre3)
{
	/* Le chupa tres huevos a la ia lo que le elijeron, me importa a mi al juego */
	return false;
}

jugada_t adversario_proxima_jugada(adversario_t *adversario)
{
	jugada_t j = { .ataque = "", .pokemon = "" };
	return j;
}

void adversario_informar_jugada(adversario_t *a, jugada_t j)
{
}

void adversario_destruir(adversario_t *adversario)
{
	if (!adversario)
		return;

	hash_destruir(adversario->pokemones);
	free(adversario);
}
