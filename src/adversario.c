#include <time.h>
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

#define MAX_POKEMONES 3
#define MAX_ATAQUES 3
#define ROJO "\x1b[31;1m"
#define NORMAL "\x1b[0m"

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
 * Recibe dos ataques.
 * 
 * Compara sus nombres y devuelve 0 si son iguales, 
 * 0 < si el nombre del ataque1 es mas grande y 0 > 
 * si el nombre del ataque2 es mas grande.
 */
int comparador_abb_adversario(void *_ataque1, void *_ataque2)
{
	struct ataque *ataque1 = _ataque1;
	struct ataque *ataque2 = _ataque2;

	return strcmp(ataque1->nombre, ataque2->nombre);
}

/*
 * Recibe un pokemon y un nombre.
 *
 * Compara el nombre del pokemon con el recibido por parametro.
 * Devuelve 0 si son iguales.
 */
int adversario_comparar_nombres(void *pokemon, void *nombre) 
{
	return strcmp(pokemon_nombre((pokemon_t *)pokemon), (char *)nombre);
}

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
	for (size_t i = 0; i < tamanio && !repetido; i++) {
		for (size_t j = i + 1; j < tamanio && !repetido; j++)
			repetido = posiciones[i] == posiciones[j];
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
void elegir_pokemones(adversario_t *adversario, pokemon_t **pokemones, char ***nombres, size_t *posiciones)
{
	for (size_t i = 0; i < MAX_POKEMONES; i++) {
		pokemones[i] = lista_elemento_en_posicion(adversario->pokemones_disponibles, posiciones[i]);
		*nombres[i] = (char *)pokemon_nombre(pokemones[i]);
	}
}

/*
 *
 *
 */
void adversario_guardar_ataques(const struct ataque *ataque, void *abb)
{
	abb_insertar((abb_t *)abb, (void *)ataque);
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
 * Devuelve true si el pokemon se quedo sin ataques.
 */
bool pokemon_sin_ataques(abb_t *ataques, struct paquete paquete)
{	
	int ataques_no_encontrados = 0;
	for (int i = 0; i < paquete.tamanio ; i++) 
		ataques_no_encontrados += !!!abb_buscar(ataques, paquete.ataques[i]);
	
	return ataques_no_encontrados == paquete.tamanio;
}



adversario_t *adversario_crear(lista_t *pokemon)
{
	if (!pokemon)
		return NULL;
	
	adversario_t *adversario = calloc(1, sizeof(struct adversario));
	if (!adversario)
		return NULL;
	
	adversario->pokemones_disponibles = pokemon;

	adversario->pokemones = lista_crear();
	if (!adversario->pokemones) {
		adversario_destruir(adversario);
		return NULL;
	}

	adversario->ataques = abb_crear(comparador_abb_adversario);
	if (!adversario->ataques) {
		adversario_destruir(adversario);
		return NULL;
	}

	return adversario;
}

bool adversario_seleccionar_pokemon(adversario_t *adversario, char **nombre1, char **nombre2, char **nombre3)
{
	if (!adversario || !nombre1 || !nombre2 || !nombre3)
		return false;

	char **nombres[] = { nombre1, nombre2, nombre3 };
	pokemon_t *pokemones[MAX_POKEMONES];
	size_t posiciones[MAX_POKEMONES];

	// Carga el vector con posiciones aleatorias unicas.
	cargar_posiciones(posiciones, MAX_POKEMONES, lista_tamanio(adversario->pokemones_disponibles));

	// Cargo los pokemones de las posiciones definidas de antes.
	elegir_pokemones(adversario, pokemones, nombres, posiciones);

	// Me guardo los 2 primeros pokemones que elegi en una lista.
	for (int i = 0; i < MAX_POKEMONES - 1; i++) {
		lista_insertar(adversario->pokemones, pokemones[i]);
		con_cada_ataque(pokemones[i], adversario_guardar_ataques, adversario->ataques);
	}
		
	return true;
}

bool adversario_pokemon_seleccionado(adversario_t *adversario, char *nombre1,
				     char *nombre2, char *nombre3)
{
	if (!adversario)
		return false;

	pokemon_t *pokemon = lista_buscar_elemento(adversario->pokemones_disponibles, adversario_comparar_nombres, nombre3);
	if (!pokemon)
		return false;

	con_cada_ataque(pokemon, adversario_guardar_ataques, adversario->ataques);

	// Me guardo el 3er pokemon que le dio el usuario.
	return lista_insertar(adversario->pokemones, pokemon);
}

jugada_t adversario_proxima_jugada(adversario_t *adversario)
{
	jugada_t jugada = { .ataque = "", .pokemon = "" };
	if (!adversario)
		return jugada;

	size_t posicion_pokemon;
	cargar_posiciones(&posicion_pokemon, 1, lista_tamanio(adversario->pokemones));

	pokemon_t *pokemon = lista_elemento_en_posicion(adversario->pokemones, posicion_pokemon);

	struct paquete paquete = { .tamanio = 0 };
	con_cada_ataque(pokemon, adversario_cargar_ataques, &paquete);

	size_t posicion_ataque; 
	cargar_posiciones(&posicion_ataque, 1, MAX_ATAQUES);

	int i = 0;
	while (!abb_buscar(adversario->ataques, paquete.ataques[posicion_ataque]) && i < MAX_ATAQUES) {
		cargar_posiciones(&posicion_ataque, 1, MAX_ATAQUES);
		i++;
	}
		
	if (i == MAX_ATAQUES)
		return adversario_proxima_jugada(adversario);

 	struct ataque *seleccionado = paquete.ataques[posicion_ataque];
	abb_quitar(adversario->ataques, seleccionado);

	strcpy(jugada.ataque, seleccionado->nombre);
	strcpy(jugada.pokemon, pokemon_nombre(pokemon));

	if (pokemon_sin_ataques(adversario->ataques, paquete)) 
		lista_quitar_de_posicion(adversario->pokemones, posicion_pokemon);	

	printf(ROJO "Pokemon: %s\n" NORMAL, jugada.pokemon);
	printf(ROJO "Ataque: %s\n" NORMAL, jugada.ataque);

	printf(ROJO "Le quedan: %li ataques\n" NORMAL, abb_tamanio(adversario->ataques));
	printf(ROJO "Le quedan %li pokemones con ataques\n" NORMAL, lista_tamanio(adversario->pokemones));

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