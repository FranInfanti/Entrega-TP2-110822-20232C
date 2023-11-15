#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "juego.h"
#include "lista.h"
#include "tipo.h"
#include "pokemon.h"
#include "ataque.h"
#include "hash.h"
#include "adversario.h"

#define CANTIDAD_MINIMA 6
#define CANTIDAD_RONDAS 9 

struct jugador {
	int puntos;
	hash_t *pokemones;
	hash_t *ataques_usados;
};

struct juego {
	int ronda;
	informacion_pokemon_t *info; // NO ME GUSTA
	lista_t* pokemones;
	struct jugador usuario;
	struct jugador ia;
};

void cargar_lista(pokemon_t *pokemon, void *lista)
{
	lista_insertar((lista_t *)lista, pokemon);
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

	juego->usuario.pokemones = hash_crear(3);
	if (!juego->usuario.pokemones) {
		juego_destruir(juego);
		return NULL;
	}

	juego->usuario.ataques_usados = hash_crear(3);
	if (!juego->usuario.ataques_usados) {
		juego_destruir(juego);
		return NULL;
	}

	juego->ia.pokemones = hash_crear(3);
	if (!juego->ia.pokemones) {
		juego_destruir(juego);
		return NULL;		
	}

	juego->ia.ataques_usados = hash_crear(3);
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

	juego->info = pokemon_cargar_archivo(archivo);
	if (!juego->info)
		return ERROR_GENERAL;
	
	if (pokemon_cantidad(juego->info) < CANTIDAD_MINIMA) {
		pokemon_destruir_todo(juego->info);
		return POKEMON_INSUFICIENTES;
	}
	
	con_cada_pokemon(juego->info, cargar_lista, juego->pokemones);
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
	
	/* Verficiar que no esten repetidos */
	
	JUEGO_ESTADO estado = ERROR_GENERAL;

	if (jugador) 
		/* Le guardo los pokemones a la ia, pero para mi */
		return 0;
	else if (!jugador)
		/* Le guardo los pokemones a jugador */
		return 0;
		
	return estado;
}

resultado_jugada_t juego_jugar_turno(juego_t *juego, jugada_t jugada_jugador1,
				     jugada_t jugada_jugador2)
{
	resultado_jugada_t resultado;
	resultado.jugador1 = ATAQUE_ERROR;
	resultado.jugador2 = ATAQUE_ERROR;
	return resultado;
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
	pokemon_destruir_todo(juego->info);
	lista_destruir(juego->pokemones);
	free(juego);
}
