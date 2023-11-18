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
	struct jugador usuario;
	struct jugador ia;
};

/*
 * Recibe un juego y la capacidad para reservar el hash.
 * 
 * Reserva la memoria para un hash y devuele un puntero a este.
 * 
 * En caso de error libera toda le memoria reservada para juego 
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
 * Recibe dos punteros void* a un string.
 *
 * Devuelve 0 si son iguales, > 0 si el _nombre1 es mas grande 
 * y 0 <  si el _nombre2 es mas grande.
 */
int comparar_nombres(void *nombre1, void *nombre2)
{
	return strcmp((char *)nombre1, (char *)nombre2);
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
 * Recibe un hash con los pokemones del jugador, otro hash con los ataques que ya uso,
 * la jugada que hizo el jugador, un doble puntero a pokemon y a ataque.
 * 
 * Valida si la jugada hecha por el usuario es valida, en caso de serlo modifica el 
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

	juego->usuario.pokemones = reservar_hash(juego, MAX_POKEMONES);
	if (!juego->usuario.pokemones)
		return NULL;

	juego->usuario.ataques_usados = reservar_hash(juego, MAX_ATAQUES);
	if (!juego->usuario.ataques_usados)
		return NULL;

	juego->ia.pokemones = reservar_hash(juego, MAX_POKEMONES);
	if (!juego->ia.pokemones)
		return NULL;

	juego->ia.ataques_usados = reservar_hash(juego, MAX_ATAQUES);
	if (!juego->ia.ataques_usados)
		return NULL;

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

	bool error = false;
	for (int i = 0; i < tamanio - 1 && !error; i++)
		error = !hash_insertar(jugador ? juego->ia.pokemones :
						 juego->usuario.pokemones,
				       nombres[i], pokemones[i], NULL);

	if (!error)
		error = !hash_insertar(!jugador ? juego->ia.pokemones :
						  juego->usuario.pokemones,
				       nombre3, pokemones[tamanio - 1], NULL);

	return error ? ERROR_GENERAL : TODO_OK;
}

resultado_jugada_t juego_jugar_turno(juego_t *juego, jugada_t jugada_jugador1,
				     jugada_t jugada_jugador2)
{
	resultado_jugada_t jugada = { .jugador1 = 0, .jugador2 = 0 };
	if (!juego)
		return jugada;

	pokemon_t *pokemon_jugador1;
	const struct ataque *ataque_jugador1;
	if (!validar_jugada(juego->usuario.pokemones,
			    juego->usuario.ataques_usados, jugada_jugador1,
			    &pokemon_jugador1, &ataque_jugador1))
		return jugada;

	pokemon_t *pokemon_jugador2;
	const struct ataque *ataque_jugador2;
	if (!validar_jugada(juego->ia.pokemones, juego->ia.ataques_usados,
			    jugada_jugador2, &pokemon_jugador2,
			    &ataque_jugador2))
		return jugada;

	jugada.jugador1 =
		efectividad(ataque_jugador1->tipo, ataque_jugador2->tipo);
	jugada.jugador2 =
		efectividad(ataque_jugador2->tipo, ataque_jugador1->tipo);

	hash_insertar(juego->usuario.ataques_usados, jugada_jugador1.ataque,
		      (void *)ataque_jugador1, NULL);
	hash_insertar(juego->ia.ataques_usados, jugada_jugador2.ataque,
		      (void *)ataque_jugador2, NULL);

	juego->usuario.puntos +=
		determinar_puntos((int)ataque_jugador1->poder, jugada.jugador1);
	juego->ia.puntos +=
		determinar_puntos((int)ataque_jugador2->poder, jugada.jugador2);
	juego->ronda++;
	return jugada;
}

int juego_obtener_puntaje(juego_t *juego, JUGADOR jugador)
{
	if (juego == NULL)
		return 0;
	return jugador ? juego->ia.puntos : juego->usuario.puntos;
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

	hash_destruir(juego->ia.ataques_usados);
	hash_destruir(juego->ia.pokemones);
	hash_destruir(juego->usuario.ataques_usados);
	hash_destruir(juego->usuario.pokemones);
	pokemon_destruir_todo(juego->informacion);
	lista_destruir(juego->pokemones);
	free(juego);
}