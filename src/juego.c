#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "juego.h"
#include "lista.h"
#include "tipo.h"
#include "pokemon.h"
#include "ataque.h"
#include "hash.h"
#include "adversario.h"

#define CANTIDAD_MINIMA 6
#define CANTIDAD_RONDAS 9
#define MAX_POKEMONES 3
#define MAX_ATAQUES 9
#define MULTIPLICADOR_EFECTIVO 3

struct jugador {
	int puntos;
	hash_t *pokemones;
	hash_t *ataques_usados;
};

struct juego {
	int ronda;
	informacion_pokemon_t *informacion;
	lista_t *pokemones;
	struct jugador jugador1;
	struct jugador jugador2;
};

/*
 * Recibe un juego y la capacidad para reservar el hash.
 * 
 * Reserva la memorjugador2 para un hash y devuele un puntero a este.
 * 
 * En caso de error libera toda le memorjugador2 reservada para juego 
 * y devuelve NULL.
 */
hash_t *reservar_hash(juego_t *juego, size_t capacidad)
{
	hash_t *hash = hash_crear(capacidad);
	if (!hash) {
		juego_destruir(juego);
		return NULL;
	}
	return hash;
}

/*
 * Recibe un pokemon y un void* a una lista.
 * 
 * Inserta el pokemon en la ultima posicion de la lista. 
 */
void cargar_lista(pokemon_t *pokemon, void *lista)
{
	lista_insertar((lista_t *)lista, pokemon);
}

/*
 * Recibe un pokemon y un nombre.
 *
 * Compara el nombre del pokemon con el recibido por parametro.
 * Devuelve 0 si son iguales.
 */
int comparar_nombres(void *pokemon, void *nombre)
{
	return strcmp(pokemon_nombre((pokemon_t *)pokemon), (char *)nombre);
}

/*
 * Recibe un juego, jugador y un array de punteros a strings con su respectivo tamanio. 
 * 
 * Devuelve true si alguno de esos nombres se repite entre si,
 * caso contrario devuelve false.
 */
bool pokemon_repetido(struct juego *juego, const char **nombres, int tamanio)
{
	bool repetido = false;
	for (int j = 0; j < tamanio - 1 && !repetido; j++) {
		for (int i = j + 1; i < tamanio && !repetido; i++)
			repetido = !strcmp(nombres[j], nombres[i]);
	}
	return repetido;
}

/*
 * Recibe una lista con pokemones, un array de punteros a strings,
 * un array de punteros a pokemones y un tamanio que comparten ambos arrays.
 * 
 * Hace que cada posicion del array apunte a un pokemon, cuyo nombre coincide 
 * con alguno de los de el array de nombres.
 *
 * Devuelve false si alguna de las posiciones del array es NULL, 
 * en caso contrario devuelve true.
 */
bool existe_pokemon(lista_t *listado, const char **nombres,
		    pokemon_t **pokemones, int tamanio)
{
	bool existe = true;
	for (int i = 0; i < tamanio; i++)
		pokemones[i] = lista_buscar_elemento(listado, comparar_nombres,
						     (void *)nombres[i]);

	for (int i = 0; i < tamanio && existe; i++)
		existe = pokemones[i] != NULL;

	return existe;
}

/*
 * Recibe un juego, un jugador y un array de pokemones con su respectivo tamanio.
 * 
 * Inserta los dos primeros pokemones del array en el jugador y el ultimo lo inserta en el otro.
 * Devuelve false en caso de error.
 */
bool guardar_pokemones(juego_t *juego, JUGADOR jugador, pokemon_t **pokemones,
		       int tamanio)
{
	bool error = false;
	for (int i = 0; i < tamanio - 1 && !error; i++)
		error = !hash_insertar(jugador ? juego->jugador2.pokemones :
						 juego->jugador1.pokemones,
				       pokemon_nombre(pokemones[i]),
				       pokemones[i], NULL);

	if (!error)
		error = !hash_insertar(!jugador ? juego->jugador2.pokemones :
						  juego->jugador1.pokemones,
				       pokemon_nombre(pokemones[tamanio - 1]),
				       pokemones[tamanio - 1], NULL);
	return error;
}

/*
 * Recibe un hash con los pokemones del jugador, otro hash con los ataques que ya uso,
 * la jugada que hizo el jugador, un doble puntero a pokemon y a ataque.
 * 
 * Valida si la jugada hecha por el jugador1 es valida, en caso de serlo modifica el 
 * puntero a pokemon para que apunte al pokemon seleccionado y hace lo mismo con el ataque.
 * 
 * Devuelve true si la jugada es valida y false en caso contrario. 
 */
bool validar_jugada(hash_t *pokemones, hash_t *ataques_usados, jugada_t jugada,
		    pokemon_t **pokemon, const struct ataque **ataque)
{
	if (hash_contiene(ataques_usados, jugada.ataque))
		return false;

	*pokemon = hash_obtener(pokemones, jugada.pokemon);
	if (!*pokemon)
		return false;
	*ataque = pokemon_buscar_ataque(*pokemon, jugada.ataque);

	return !!pokemon && !!ataque;
}

/*
 * Recibe un juego y los ataques usados por los jugadores.
 * 
 * Guarda en los ataques que fueron usados para que no puedan volver
 * a ser utilizados.
 * Devuelve false en caso de error.
 */
bool registrar_ataques(juego_t *juego, const struct ataque *jugador1,
		       const struct ataque *jugador2)
{
	return hash_insertar(juego->jugador1.ataques_usados, jugador1->nombre,
			     (void *)jugador1, NULL) &&
	       hash_insertar(juego->jugador2.ataques_usados, jugador2->nombre,
			     (void *)jugador2, NULL);
}

/*
 * Recibe el tipo de un ataque y el tipo de ataque de su adversario. 
 * 
 * Devuelve la efectividad del ataque actual respecto al adversario.
 */
RESULTADO_ATAQUE efectividad(enum TIPO actual, enum TIPO adversario)
{
	if (actual == NORMAL)
		return ATAQUE_REGULAR;

	enum TIPO tipos[] = { FUEGO, PLANTA, ROCA, ELECTRICO, AGUA };
	int pos_actual;
	int pos_adversario;

	for (int i = 0; i < sizeof(tipos) / sizeof(int); i++) {
		if (tipos[i] == actual)
			pos_actual = i;
		else if (tipos[i] == adversario)
			pos_adversario = i;
	}

	if ((actual == FUEGO && adversario == AGUA))
		pos_adversario = -1;

	if (actual == AGUA && adversario == FUEGO)
		pos_actual = 5;

	if (pos_actual - pos_adversario == -1)
		return ATAQUE_EFECTIVO;

	if (pos_actual - pos_adversario == 1)
		return ATAQUE_INEFECTIVO;

	return ATAQUE_REGULAR;
}

/*
 * Recibe un poder y el resultado de un ataque.
 * 
 * Calcula y devuelve el nuevo poder en base al resultado del ataque.
 */
int determinar_puntos(int poder, RESULTADO_ATAQUE ataque)
{
	if (ataque == ATAQUE_REGULAR)
		return poder;
	if (ataque == ATAQUE_EFECTIVO)
		return poder * MULTIPLICADOR_EFECTIVO;
	return poder % 2 == 0 ? poder >> 1 : (poder >> 1) + 1;
}

// OK
juego_t *juego_crear()
{
	juego_t *juego = calloc(1, sizeof(struct juego));
	if (!juego)
		return NULL;

	juego->pokemones = lista_crear();
	if (!juego->pokemones) {
		free(juego);
		return NULL;
	}

	juego->jugador1.pokemones = reservar_hash(juego, MAX_POKEMONES);
	if (!juego->jugador1.pokemones)
		return NULL;

	juego->jugador1.ataques_usados = reservar_hash(juego, MAX_ATAQUES);
	if (!juego->jugador1.ataques_usados)
		return NULL;

	juego->jugador2.pokemones = reservar_hash(juego, MAX_POKEMONES);
	if (!juego->jugador2.pokemones)
		return NULL;

	juego->jugador2.ataques_usados = reservar_hash(juego, MAX_ATAQUES);
	if (!juego->jugador2.ataques_usados)
		return NULL;

	return juego;
}

// OK
JUEGO_ESTADO juego_cargar_pokemon(juego_t *juego, char *archivo)
{
	if (!juego || !archivo)
		return ERROR_GENERAL;

	juego->informacion = pokemon_cargar_archivo(archivo);
	if (!juego->informacion)
		return ERROR_GENERAL;

	if (pokemon_cantidad(juego->informacion) < CANTIDAD_MINIMA) {
		pokemon_destruir_todo(juego->informacion);
		juego->informacion = NULL;
		return POKEMON_INSUFICIENTES;
	}

	con_cada_pokemon(juego->informacion, cargar_lista, juego->pokemones);
	return TODO_OK;
}

// OK
lista_t *juego_listar_pokemon(juego_t *juego)
{
	return juego ? juego->pokemones : NULL;
}

// OK
JUEGO_ESTADO juego_seleccionar_pokemon(juego_t *juego, JUGADOR jugador,
				       const char *nombre1, const char *nombre2,
				       const char *nombre3)
{
	if (!juego || !nombre1 || !nombre2 || !nombre3)
		return ERROR_GENERAL;

	pokemon_t *pokemones[MAX_POKEMONES];
	const char *nombres[] = { nombre1, nombre2, nombre3 };
	int tamanio = MAX_POKEMONES;

	if (pokemon_repetido(juego, nombres, tamanio))
		return POKEMON_REPETIDO;

	if (!existe_pokemon(juego->pokemones, nombres, pokemones, tamanio))
		return POKEMON_INEXISTENTE;

	return guardar_pokemones(juego, jugador, pokemones, tamanio) ?
		       ERROR_GENERAL :
		       TODO_OK;
}

// OK
resultado_jugada_t juego_jugar_turno(juego_t *juego, jugada_t jugada_jugador1,
				     jugada_t jugada_jugador2)
{
	resultado_jugada_t jugada = { .jugador1 = 0, .jugador2 = 0 };
	if (!juego)
		return jugada;

	pokemon_t *pokemon_jugador1;
	const struct ataque *ataque_jugador1;
	if (!validar_jugada(juego->jugador1.pokemones,
			    juego->jugador1.ataques_usados, jugada_jugador1,
			    &pokemon_jugador1, &ataque_jugador1))
		return jugada;

	pokemon_t *pokemon_jugador2;
	const struct ataque *ataque_jugador2;
	if (!validar_jugada(juego->jugador2.pokemones,
			    juego->jugador2.ataques_usados, jugada_jugador2,
			    &pokemon_jugador2, &ataque_jugador2))
		return jugada;

	if (!registrar_ataques(juego, ataque_jugador1, ataque_jugador2))
		return jugada;

	jugada.jugador1 =
		efectividad(ataque_jugador1->tipo, ataque_jugador2->tipo);
	jugada.jugador2 =
		efectividad(ataque_jugador2->tipo, ataque_jugador1->tipo);

	juego->jugador1.puntos +=
		determinar_puntos((int)ataque_jugador1->poder, jugada.jugador1);
	juego->jugador2.puntos +=
		determinar_puntos((int)ataque_jugador2->poder, jugada.jugador2);

	juego->ronda++;
	return jugada;
}

int juego_obtener_puntaje(juego_t *juego, JUGADOR jugador)
{
	if (juego == NULL)
		return 0;
	return jugador ? juego->jugador2.puntos : juego->jugador1.puntos;
}

bool juego_finalizado(juego_t *juego)
{
	if (juego == NULL)
		return true;
	return juego->ronda == CANTIDAD_RONDAS;
}

void juego_destruir(juego_t *juego)
{
	if (!juego)
		return;

	hash_destruir(juego->jugador2.ataques_usados);
	hash_destruir(juego->jugador2.pokemones);
	hash_destruir(juego->jugador1.ataques_usados);
	hash_destruir(juego->jugador1.pokemones);
	pokemon_destruir_todo(juego->informacion);
	lista_destruir(juego->pokemones);
	free(juego);
}