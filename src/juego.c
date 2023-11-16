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
 * Recibe el tipo de un ataque y el tipo de ataque de su adversario. 
 * 
 * Devuelve la efectividad del ataque actual respecto al adversario.
 */
RESULTADO_ATAQUE determinar_efectividad(enum TIPO actual, enum TIPO adversario)
{
	if (actual == FUEGO && adversario == PLANTA)
		return ATAQUE_EFECTIVO;

	if (actual == PLANTA && adversario == ROCA)
		return ATAQUE_EFECTIVO;

	if (actual == ROCA && adversario == ELECTRICO)
		return ATAQUE_EFECTIVO;

	if (actual == ELECTRICO && adversario == AGUA)
		return ATAQUE_EFECTIVO;

	if (actual == AGUA && adversario == FUEGO)
		return ATAQUE_EFECTIVO;

	if (actual == FUEGO && adversario == AGUA)
		return ATAQUE_INEFECTIVO;

	if (actual == AGUA && adversario == ELECTRICO)
		return ATAQUE_INEFECTIVO;

	if (actual == ELECTRICO && adversario == ROCA)
		return ATAQUE_INEFECTIVO;

	if (actual == ROCA && adversario == PLANTA)
		return ATAQUE_INEFECTIVO;

	if (actual == PLANTA && adversario == FUEGO)
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
		juego_destruir(juego);
		return NULL;
	}

	juego->usuario.pokemones = hash_crear(MAX_POKEMONES);
	if (!juego->usuario.pokemones) {
		juego_destruir(juego);
		return NULL;
	}

	juego->usuario.ataques_usados = hash_crear(MAX_ATAQUES);
	if (!juego->usuario.ataques_usados) {
		juego_destruir(juego);
		return NULL;
	}

	juego->ia.pokemones = hash_crear(MAX_POKEMONES);
	if (!juego->ia.pokemones) {
		juego_destruir(juego);
		return NULL;
	}

	juego->ia.ataques_usados = hash_crear(MAX_ATAQUES);
	if (!juego->ia.ataques_usados) {
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

	if (hash_contiene(juego->usuario.ataques_usados,
			  jugada_jugador1.ataque))
		return jugada;

	if (hash_contiene(juego->ia.ataques_usados, jugada_jugador2.ataque))
		return jugada;

	pokemon_t *pokemon_jugador1 =
		hash_obtener(juego->usuario.pokemones, jugada_jugador1.pokemon);
	pokemon_t *pokemon_jugador2 =
		hash_obtener(juego->ia.pokemones, jugada_jugador2.pokemon);
	if (!pokemon_jugador1 || !pokemon_jugador2)
		return jugada;

	const struct ataque *ataque_jugador1 =
		pokemon_buscar_ataque(pokemon_jugador1, jugada_jugador1.ataque);
	const struct ataque *ataque_jugador2 =
		pokemon_buscar_ataque(pokemon_jugador2, jugada_jugador2.ataque);
	if (!ataque_jugador1 || !ataque_jugador2)
		return jugada;

	jugada.jugador1 = determinar_efectividad(ataque_jugador1->tipo,
						 ataque_jugador2->tipo);
	jugada.jugador2 = determinar_efectividad(ataque_jugador2->tipo,
						 ataque_jugador1->tipo);

	hash_insertar(juego->usuario.ataques_usados, jugada_jugador1.ataque, (void *)ataque_jugador1, NULL);
	hash_insertar(juego->ia.pokemones, jugada_jugador2.ataque, (void *)ataque_jugador2, NULL);

	juego->usuario.puntos += determinar_puntos((int)ataque_jugador1->poder, jugada.jugador1);
	juego->ia.puntos += determinar_puntos((int)ataque_jugador2->poder, jugada.jugador2);
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