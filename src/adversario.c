#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lista.h"
#include "juego.h"
#include "adversario.h"
#include "pokemon.h"
#include "hash.h"
#include "ataque.h"

#define MAX_POKEMONES 3
#define MAX_ATAQUES 9
#define ROJO "\x1b[31;1m"
#define NORMAL "\x1b[0m"

struct adversario {
	lista_t *pokemones_disponibles;
	lista_t *pokemones;
	hash_t *ataques_usados;
};

struct paquete {
	const struct ataque *ataques[MAX_ATAQUES >> 1];
	int tamanio;
};

/*
 * Recibe hasta que numero se pueden generar numeros.
 *
 * Devuelve el numero generado.
 */
size_t numero_aleatorio(size_t rango)
{
	return (size_t)rand() % rango;
}

/*
 * Recibe una lista con pokemones, un array de pokemon y otro de nombres.
 * 
 * Busca en la lista los pokemones que se encuentren en una posicion aleatoria
 * y guarda esos pokemones en el array de pokemon. Ademas guarda los nombres de estos en el 
 * array de nombres.
 */
void elegir_pokemones(lista_t *pokemones_disponibles,
			   pokemon_t **pokemones, char ***nombres)
{
	for (int i = 0; i < MAX_POKEMONES; i++) {
		pokemones[i] = lista_elemento_en_posicion(
			pokemones_disponibles,
			numero_aleatorio(lista_tamanio(pokemones_disponibles)));
		*nombres[i] = (char *)pokemon_nombre(pokemones[i]);
	}
}

/*
 * Recibe un ataque y un paquete que contiene que contiene 
 * un array de ataques con su respectivo tamanio.
 * 
 * Guarda el ataque en el array de ataques.
 */
void guardar_ataques(const struct ataque *actual, void *_paquete)
{
	struct paquete *paquete = _paquete;
	paquete->ataques[paquete->tamanio] = actual;
	paquete->tamanio++;
}

/*
 * Recibe un pokemon y un nombre.
 *
 * Compara el nombre del pokemon con el recibido por parametro.
 * Devuelve 0 si son iguales.
 */
int _comparar_nombres(void *pokemon, void *nombre) 
{
	return strcmp(pokemon_nombre((pokemon_t *)pokemon), (char *)nombre);
}

/*
 * Recibe un hash con los ataques que fueron usados y un paquete que contiene 
 * un array de ataques con su respectivo tamanio.
 * 
 * Devuelve true si los tres ataques del array ya fueron usados, devuelve false 
 * en caso contrario.
 */
bool pokemon_sin_ataques(hash_t *ataques_usados, struct paquete paquete)
{
	bool los_tiene = true;
	for (int i = 0; i < paquete.tamanio && los_tiene; i++)
		los_tiene = hash_contiene(ataques_usados,
					  paquete.ataques[i]->nombre);

	return los_tiene;
}

// OK
adversario_t *adversario_crear(lista_t *pokemon)
{
	if (!pokemon)
		return NULL;

	struct adversario *ia = calloc(1, sizeof(struct adversario));
	if (!ia)
		return NULL;
	ia->pokemones_disponibles = pokemon;

	ia->pokemones = lista_crear();
	if (!ia->pokemones) {
		free(ia);
		return NULL;
	}

	ia->ataques_usados = hash_crear(MAX_ATAQUES);
	if (!ia->ataques_usados) {
		adversario_destruir(ia);
		return NULL;
	}

	return ia;
}

// OK
bool adversario_seleccionar_pokemon(adversario_t *adversario, char **nombre1,
				    char **nombre2, char **nombre3)
{
	if (!adversario || !nombre1 || !nombre2 || !nombre3)
		return false;

	char **nombres[] = { nombre1, nombre2, nombre3 };
	pokemon_t *pokemon[MAX_POKEMONES];

	// Me agarro 3 pokemones del pokemones_disponibles al azar.
	elegir_pokemones(adversario->pokemones_disponibles, pokemon,
			      nombres);

	// Me guardo los 2 primeros pokemones que elegi en una lista.
	bool error = false;
	for (int i = 0; i < MAX_POKEMONES - 1; i++)
		error = !lista_insertar(adversario->pokemones, pokemon[i]);

	return !error;
}

// OK ?
bool adversario_pokemon_seleccionado(adversario_t *adversario, char *nombre1,
				     char *nombre2, char *nombre3)
{
	if (!adversario)
		return false;

	pokemon_t *pokemon = lista_buscar_elemento(adversario->pokemones_disponibles, _comparar_nombres, nombre3);
	if (!pokemon)
		return false;

	return lista_insertar(adversario->pokemones, pokemon);
}

// OK ?
jugada_t adversario_proxima_jugada(adversario_t *adversario)
{
	jugada_t jugada = { .ataque = "", .pokemon = "" };
	if (!adversario)
		return jugada;

	size_t posicion_pokemon = numero_aleatorio(lista_tamanio(adversario->pokemones));
	pokemon_t *pokemon = lista_elemento_en_posicion(adversario->pokemones, posicion_pokemon);

	struct paquete paquete = { .tamanio = 0 };
	size_t posicion_ataque = numero_aleatorio(MAX_ATAQUES >> 1);
	con_cada_ataque(pokemon, guardar_ataques, &paquete);

	while (hash_obtener(adversario->ataques_usados, paquete.ataques[posicion_ataque]->nombre) == paquete.ataques[posicion_ataque])
		posicion_ataque = numero_aleatorio(MAX_ATAQUES / 3);

	const struct ataque *seleccionado = paquete.ataques[posicion_ataque];
	if (!hash_insertar(adversario->ataques_usados, seleccionado->nombre, (void *)seleccionado, NULL))
		return jugada;

	strcpy(jugada.ataque, seleccionado->nombre);
	strcpy(jugada.pokemon, pokemon_nombre(pokemon));

	printf(ROJO "Pokemon: %s\n" NORMAL, jugada.pokemon);
	printf(ROJO "Ataque: %s\n" NORMAL, jugada.ataque);

	if (pokemon_sin_ataques(adversario->ataques_usados, paquete)) {
		if (!lista_quitar_de_posicion(adversario->pokemones,
					      posicion_pokemon)) {
			memset(&jugada, 0, sizeof(jugada));
			return jugada;
		}
	}

	return jugada;
}

void adversario_informar_jugada(adversario_t *a, jugada_t j)
{
}

// OK
void adversario_destruir(adversario_t *adversario)
{
	if (!adversario)
		return;

	hash_destruir(adversario->ataques_usados);
	lista_destruir(adversario->pokemones);
	free(adversario);
}