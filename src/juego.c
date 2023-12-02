#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "juego.h"
#include "tipo.h"
#include "pokemon.h"
#include "ataque.h"
#include "hash.h"
#include "abb.h"
#include "comun.h"

#define CANTIDAD_MINIMA 6
#define CANTIDAD_RONDAS 9
#define MAX_POKEMONES 3
#define MULTIPLICADOR_EFECTIVO 3

struct jugador {
	int puntos;
	hash_t *pokemones;
	abb_t *ataques_disponibles;
};

struct juego {
	unsigned ronda;
	informacion_pokemon_t *informacion;
	lista_t *pokemones;
	struct jugador jugador1;
	struct jugador jugador2;
};

/*
 * Recibe la direccion de memoria de un jugador.
 * 
 * Reserva la memoria correspondiente para los campos del jugador.
 * Devuelve false en caso de error.
 */
bool reservar_jugador(struct jugador *jugador)
{
	jugador->pokemones = hash_crear(MAX_POKEMONES);
	jugador->ataques_disponibles = abb_crear(comparador_abb);
	return jugador->pokemones && jugador->ataques_disponibles;
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
	for (int i = 0; i < tamanio - 1; i++) {
		hash_insertar(jugador ? juego->jugador2.pokemones :
					juego->jugador1.pokemones,
			      pokemon_nombre(pokemones[i]), pokemones[i], NULL);
		con_cada_ataque(pokemones[i], guardar_ataques,
				jugador ? juego->jugador2.ataques_disponibles :
					  juego->jugador1.ataques_disponibles);
	}

	hash_insertar(!jugador ? juego->jugador2.pokemones :
				 juego->jugador1.pokemones,
		      pokemon_nombre(pokemones[tamanio - 1]),
		      pokemones[tamanio - 1], NULL);
	con_cada_ataque(pokemones[tamanio - 1], guardar_ataques,
			!jugador ? juego->jugador2.ataques_disponibles :
				   juego->jugador1.ataques_disponibles);

	return true;
}

/*
 * Recibe un hash con los pokemones del jugador, otro abb con los ataques que todavia no uso,
 * la jugada que hizo el jugador, un doble puntero a pokemon y a ataque.
 * 
 * Valida si la jugada hecha por el jugador1 es valida, en caso de serlo modifica el 
 * puntero a pokemon para que apunte al pokemon seleccionado y hace lo mismo con el ataque.
 * 
 * Devuelve true si la jugada es valida y false en caso contrario. 
 */
bool validar_jugada(hash_t *pokemones, abb_t *ataques_disponibles,
		    jugada_t jugada, pokemon_t **pokemon,
		    struct ataque **ataque)
{
	*pokemon = hash_obtener(pokemones, jugada.pokemon);
	if (!*pokemon)
		return false;
	*ataque =
		(struct ataque *)pokemon_buscar_ataque(*pokemon, jugada.ataque);
	if (!*ataque)
		return false;

	return abb_buscar(ataques_disponibles, (void *)*ataque);
}

/*
 * Recibe un juego y los ataques usados por los jugadores.
 * 
 * Elimina del abb el ataque que acaba de ser utilizado por los jugadores.
 * Devuelve false en caso de error.
 */
bool registrar_ataques(juego_t *juego, struct ataque *jugador1,
		       struct ataque *jugador2)
{
	return abb_quitar(juego->jugador1.ataques_disponibles,
			  (void *)jugador1) &&
	       abb_quitar(juego->jugador2.ataques_disponibles,
			  (void *)jugador2);
}

/*
 * Recibe el tipo de un ataque y el tipo del pokemon a atacar. 
 * 
 * Devuelve la efectividad del ataque actual respecto al pokemon.
 */
RESULTADO_ATAQUE efectividad(enum TIPO ataque, enum TIPO pokemon)
{
	if (ataque == NORMAL || pokemon == NORMAL)
		return ATAQUE_REGULAR;

	enum TIPO tipos[] = { FUEGO, PLANTA, ROCA, ELECTRICO, AGUA };
	int pos_actual = 0;
	int pos_adversario = 0;

	for (int i = 0; i < sizeof(tipos) / sizeof(int); i++) {
		if (tipos[i] == ataque)
			pos_actual = i;
		else if (tipos[i] == pokemon)
			pos_adversario = i;
	}

	if (ataque == FUEGO && pokemon == AGUA)
		pos_adversario = pos_actual - 1;

	if (ataque == AGUA && pokemon == FUEGO)
		pos_adversario = pos_actual + 1;

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

	if (!reservar_jugador(&juego->jugador1) ||
	    !reservar_jugador(&juego->jugador2)) {
		juego_destruir(juego);
		return NULL;
	}

	return juego;
}

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

lista_t *juego_listar_pokemon(juego_t *juego)
{
	return juego ? juego->pokemones : NULL;
}

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
		       TODO_OK :
		       ERROR_GENERAL;
}

resultado_jugada_t juego_jugar_turno(juego_t *juego, jugada_t jugada_jugador1,
				     jugada_t jugada_jugador2)
{
	resultado_jugada_t jugada = { .jugador1 = 0, .jugador2 = 0 };
	if (!juego)
		return jugada;

	pokemon_t *pokemon_jugador1;
	struct ataque *ataque_jugador1;
	if (!validar_jugada(juego->jugador1.pokemones,
			    juego->jugador1.ataques_disponibles,
			    jugada_jugador1, &pokemon_jugador1,
			    &ataque_jugador1))
		return jugada;

	pokemon_t *pokemon_jugador2;
	struct ataque *ataque_jugador2;
	if (!validar_jugada(juego->jugador2.pokemones,
			    juego->jugador2.ataques_disponibles,
			    jugada_jugador2, &pokemon_jugador2,
			    &ataque_jugador2))
		return jugada;

	if (!registrar_ataques(juego, ataque_jugador1, ataque_jugador2))
		return jugada;

	jugada.jugador1 = efectividad(ataque_jugador1->tipo,
				      pokemon_tipo(pokemon_jugador2));
	jugada.jugador2 = efectividad(ataque_jugador2->tipo,
				      pokemon_tipo(pokemon_jugador1));

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

	abb_destruir(juego->jugador2.ataques_disponibles);
	hash_destruir(juego->jugador2.pokemones);

	abb_destruir(juego->jugador1.ataques_disponibles);
	hash_destruir(juego->jugador1.pokemones);

	lista_destruir(juego->pokemones);
	pokemon_destruir_todo(juego->informacion);
	free(juego);
}