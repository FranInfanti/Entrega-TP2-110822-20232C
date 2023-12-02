#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "juego.h"
#include "adversario.h"
#include "lista.h"
#include "pokemon.h"
#include "ataque.h"
#include "hash.h"
#include "abb.h"
#include "comun.h"

#define MAX_POKEMONES 3
#define MAX_ATAQUES 3
#define CANTIDAD_ADVERSARIOS 1
#define ROJO "\x1b[31;1m"
#define COMUN "\x1b[0m"

struct adversario {
	lista_t *pokemones_disponibles;
	lista_t *pokemones;
	abb_t *ataques;
};

struct paquete {
	struct ataque *ataques[MAX_ATAQUES];
	int tamanio;
};

/*
 * Recibe un array de posiciones y un rango entre el cual se pueden generar numeros.
 * 
 * Devuelve un array cargado con posiciones unicas dentro del rango.
 */
void cargar_posiciones(size_t *posiciones, size_t tamanio, size_t rango)
{
	for (size_t i = 0; i < tamanio; i++)
		posiciones[i] = ((size_t)rand() % rango);

	bool repetido = false;
	size_t i = 0;
	while (i < tamanio && !repetido) {
		size_t j = i + 1;
		while (j < tamanio && !repetido) {
			repetido = posiciones[i] == posiciones[j];
			j++;
		}
		i++;
	}
	
	if (repetido)
		cargar_posiciones(posiciones, tamanio, rango);
}

/*
 * Recibe una lista con pokemones, un array de pokemon y otro de nombres.
 * 
 * Busca en la lista los pokemones que se encuentren en una posicion aleatoria
 * y guarda esos pokemones en el array de pokemon. Ademas guarda los nombres de estos en el 
 * array de nombres.
 */
void elegir_pokemones(adversario_t *adversario, pokemon_t **pokemones,
		      char ***nombres, size_t *posiciones)
{
	for (size_t i = 0; i < MAX_POKEMONES; i++) {
		pokemones[i] = lista_elemento_en_posicion(
			adversario->pokemones_disponibles, posiciones[i]);
		*nombres[i] = (char *)pokemon_nombre(pokemones[i]);
	}
}

/*
 * Recibe un ataque y un paquete que contiene que contiene 
 * un array de ataques con su respectivo tamanio.
 * 
 * Guarda el ataque en el array de ataques.
 */
void adversario_cargar_ataques(const struct ataque *actual, void *_paquete)
{
	struct paquete *paquete = _paquete;
	paquete->ataques[paquete->tamanio] = (struct ataque *)actual;
	paquete->tamanio++;
}

/*
 * Recibe un abb con los ataques del adversario y un paquete con 
 * un array de ataques y su respectivo tamaño.
 * 
 * Verifica si los ataques del array siguien estando en el abb.
 * Devuelve true si niguno de los ataques del array esta en el abb, 
 * false en caso de que haya por lo menos uno.
 */
bool pokemon_sin_ataques(abb_t *ataques, struct paquete paquete)
{
	int ataques_no_encontrados = 0;
	for (int i = 0; i < paquete.tamanio; i++)
		ataques_no_encontrados +=
			!abb_buscar(ataques, paquete.ataques[i]);

	return ataques_no_encontrados == paquete.tamanio;
}

adversario_t *adversario_crear(lista_t *pokemon)
{
	if (!pokemon)
		return NULL;

	adversario_t *adversario = calloc(CANTIDAD_ADVERSARIOS, sizeof(struct adversario));
	if (!adversario)
		return NULL;
	adversario->pokemones_disponibles = pokemon;

	adversario->pokemones = lista_crear();
	if (!adversario->pokemones) {
		adversario_destruir(adversario);
		return NULL;
	}

	adversario->ataques = abb_crear(comparador_abb);
	if (!adversario->ataques) {
		adversario_destruir(adversario);
		return NULL;
	}

	return adversario;
}

bool adversario_seleccionar_pokemon(adversario_t *adversario, char **nombre1,
				    char **nombre2, char **nombre3)
{
	if (!adversario || !nombre1 || !nombre2 || !nombre3)
		return false;

	char **nombres[] = { nombre1, nombre2, nombre3 };
	pokemon_t *pokemones[MAX_POKEMONES];
	size_t posiciones[MAX_POKEMONES];

	cargar_posiciones(posiciones, MAX_POKEMONES,
			  lista_tamanio(adversario->pokemones_disponibles));
	elegir_pokemones(adversario, pokemones, nombres, posiciones);

	for (int i = 0; i < MAX_POKEMONES - 1; i++) {
		lista_insertar(adversario->pokemones, pokemones[i]);
		con_cada_ataque(pokemones[i], guardar_ataques,
				adversario->ataques);
	}

	printf(ROJO "%s\n" COMUN, *nombre1);
	printf(ROJO "%s\n" COMUN, *nombre2);
	printf(ROJO "%s\n" COMUN, *nombre3);

	return true;
}

bool adversario_pokemon_seleccionado(adversario_t *adversario, char *nombre1,
				     char *nombre2, char *nombre3)
{
	if (!adversario)
		return false;

	pokemon_t *pokemon = lista_buscar_elemento(
		adversario->pokemones_disponibles, comparar_nombres, nombre3);
	if (!pokemon)
		return false;

	con_cada_ataque(pokemon, guardar_ataques, adversario->ataques);
	return lista_insertar(adversario->pokemones, pokemon);
}

void seleccionar_jugada(adversario_t *adversario, pokemon_t **pokemon, struct ataque **ataque)
{
	size_t posicion_pokemon;
	cargar_posiciones(&posicion_pokemon, 1, lista_tamanio(adversario->pokemones));
	*pokemon = lista_elemento_en_posicion(adversario->pokemones, posicion_pokemon);

	struct paquete paquete = { .tamanio = 0 };
	con_cada_ataque(*pokemon, adversario_cargar_ataques, &paquete);

	size_t posicion_ataque;
	cargar_posiciones(&posicion_ataque, 1, MAX_ATAQUES);

	int i = 0;
	while (!abb_buscar(adversario->ataques, paquete.ataques[posicion_ataque]) && i < MAX_ATAQUES) {
		cargar_posiciones(&posicion_ataque, 1, MAX_ATAQUES);
		i++;
	}
	*ataque = paquete.ataques[posicion_ataque];
	
	if (i == MAX_ATAQUES)
		seleccionar_jugada(adversario, pokemon, ataque);	
}

jugada_t adversario_proxima_jugada(adversario_t *adversario)
{
	jugada_t jugada = { .ataque = "", .pokemon = "" };
	if (!adversario)
		return jugada;

	pokemon_t *pokemon;
	struct ataque *ataque;

	seleccionar_jugada(adversario, &pokemon, &ataque);

	abb_quitar(adversario->ataques, ataque);
	strcpy(jugada.ataque, ataque->nombre);
	strcpy(jugada.pokemon, pokemon_nombre(pokemon));

	return jugada;
}

void adversario_informar_jugada(adversario_t *a, jugada_t j)
{
}

void adversario_destruir(adversario_t *adversario)
{
	if (!adversario)
		return;

	abb_destruir(adversario->ataques);
	lista_destruir(adversario->pokemones);
	free(adversario);
}