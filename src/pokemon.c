#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pokemon.h"
#include <stdio.h>
#include "ataque.h"
#include "tipo.h"
#define MAX_NOMBRE 30
#define MAX_ATAQUES 3
#define DELIMITADORES 1
#define CARACTER_NORMAL 'N'
#define CARACTER_FUEGO 'F'
#define CARACTER_AGUA 'A'
#define CARACTER_PLANTA 'P'
#define CARACTER_ELECTRICO 'E'
#define CARACTER_ROCA 'R'
#define INVALIDO -1
#define CARACTER_ESPECIAL '.'
#define FORMATO_POKE "%[^;];%c\n"
#define FORMATO_ATAQUE "%[^;];%c;%u\n"

struct pokemon {
	char nombre[MAX_NOMBRE];
	enum TIPO tipo;
	struct ataque *ataques;
};

struct info_pokemon {
	struct pokemon *pokemones;
	int cantidad_pokemones;
};

/*
 * Devueleve true si la extension del archivo es .txt, .png o .csv.
 */
bool validar_extension(const char *path)
{
	char *ext = strchr(path, CARACTER_ESPECIAL);
	if (ext == NULL)
		return false;

	if (strcmp(ext, ".txt") == 0 || strcmp(ext, ".png") == 0 ||
	    strcmp(ext, ".csv") == 0)
		return true;
	else
		return false;
}

/*
 * Devuelve true si el nombre del archivo recibido tiene una direccion de memoria valida
 * y una extension valida.
 */
bool validar_archivo(const char *path)
{
	return path != NULL && validar_extension(path) ? true : false;
}

/*
 * Determina el tipo del caracter, segun los disponibles en enum TIPO, 
 * si no existe devuelve INVALIDO.
 */
enum TIPO determinar_tipo(const char caracter)
{
	switch (caracter) {
	case CARACTER_NORMAL:
		return NORMAL;
	case CARACTER_FUEGO:
		return FUEGO;
	case CARACTER_AGUA:
		return AGUA;
	case CARACTER_PLANTA:
		return PLANTA;
	case CARACTER_ELECTRICO:
		return ELECTRICO;
	case CARACTER_ROCA:
		return ROCA;
	default:
		return INVALIDO;
	}
}

/*
 * Devuelve true si los datos son validos.
 */
bool validar_datos(char tipo, enum TIPO *asignar, char nombre[MAX_NOMBRE])
{
	*asignar = determinar_tipo(tipo);
	return *asignar != INVALIDO && strlen(nombre) > 0 ? true : false;
}

/*
 * Devuelve true si la linea es valida
 */
bool validar_linea(int lector, int cantidad_a_leer)
{
	return lector != EOF && lector == cantidad_a_leer ? true : false;
}

/*
 * Carga toda la informacion que esta en la direccion de memoria apuntada por aux en 
 * el vector apuntado por pokemones. En caso de error devuelve NULL.
 */
struct pokemon *cargar_pokemones(struct pokemon *pokemones, struct pokemon *aux,
				 int *tope)
{
	void *ptr1 = realloc(pokemones,
			     (unsigned long)(*tope) * sizeof(struct pokemon));
	void *ptr2 = calloc(MAX_ATAQUES, sizeof(struct ataque));

	if (ptr1 == NULL || ptr2 == NULL) {
		(*tope)--;
		free(ptr1);
		free(ptr2);
		return pokemones;
	}
	pokemones = ptr1;
	pokemones[*tope - 1].ataques = ptr2;

	strcpy(pokemones[*tope - 1].nombre, aux->nombre);
	pokemones[*tope - 1].tipo = aux->tipo;
	for (int i = 0; i < MAX_ATAQUES; i++) {
		strcpy(pokemones[*tope - 1].ataques[i].nombre,
		       aux->ataques[i].nombre);
		pokemones[*tope - 1].ataques[i].tipo = aux->ataques[i].tipo;
		pokemones[*tope - 1].ataques[i].poder = aux->ataques[i].poder;
	}
	return pokemones;
}

/*
 * Abre el archivo, carga toda la informacion de los pokemones en un bloque de memoria apuntado
 * por info_pokemones y luego cierra el archivo. En caso de error devuelve NULL.
 */
void *leer_archivo(FILE *archivo, struct info_pokemon *info_pokemones)
{
	struct pokemon *aux = calloc(1, sizeof(struct pokemon));
	if (aux == NULL)
		return NULL;

	aux->ataques = calloc(MAX_ATAQUES, sizeof(struct ataque));
	if (aux->ataques == NULL) {
		free(aux);
		return NULL;
	}

	int tope_pokemon = 0;
	int n = 0;
	int lector = 0;
	char tipo = 'A';
	bool hay_problemas = false;

	while (!hay_problemas) {
		lector = fscanf(archivo, FORMATO_POKE, aux->nombre, &tipo);
		if (!validar_linea(lector, 2 * DELIMITADORES) ||
		    !validar_datos(tipo, &aux->tipo, aux->nombre))
			hay_problemas = true;

		while (n < MAX_ATAQUES && !hay_problemas) {
			lector = fscanf(archivo, FORMATO_ATAQUE,
					aux->ataques[n].nombre, &tipo,
					&aux->ataques[n].poder);
			if (!validar_linea(lector, 3 * DELIMITADORES) ||
			    !validar_datos(tipo, &aux->ataques[n].tipo,
					   aux->nombre))
				hay_problemas = true;
			if (!hay_problemas)
				n++;
		}
		if (n == MAX_ATAQUES)
			tope_pokemon++;
		n = 0;

		if (!hay_problemas) {
			void *ptr = cargar_pokemones(info_pokemones->pokemones,
						     aux, &tope_pokemon);
			if (ptr == NULL)
				hay_problemas = true;
			else
				info_pokemones->pokemones = ptr;
			info_pokemones->cantidad_pokemones = tope_pokemon;
		}
	}
	free(aux->ataques);
	free(aux);
	return tope_pokemon == 0 ? NULL : info_pokemones;
}

/*
 * El tope_pokemones debe ser igual a la cantidad de elementos del vector pokemones.
 * Y odrdena de manera ascendente segun el nombre el vector pokemones.
 */
void bubble_sort(struct pokemon *pokemones, int tope)
{
	struct pokemon aux;
	for (int j = 1; j < tope; j++) {
		for (int i = 0; i < tope - j; i++) {
			if (strcmp(pokemones[i].nombre,
				   pokemones[i + 1].nombre) > 0) {
				aux = pokemones[i];
				pokemones[i] = pokemones[i + 1];
				pokemones[i + 1] = aux;
			}
		}
	}
}

informacion_pokemon_t *pokemon_cargar_archivo(const char *path)
{
	if (!validar_archivo(path))
		return NULL;

	struct info_pokemon *info_pokemones =
		calloc(1, sizeof(struct info_pokemon));
	if (info_pokemones == NULL)
		return NULL;

	FILE *archivo = fopen(path, "r");
	if (archivo == NULL) {
		free(info_pokemones);
		return NULL;
	}

	void *resultado_lectura = leer_archivo(archivo, info_pokemones);
	if (resultado_lectura == NULL) {
		free(info_pokemones);
		fclose(archivo);
		return NULL;
	}
	fclose(archivo);
	bubble_sort(info_pokemones->pokemones,
		    info_pokemones->cantidad_pokemones);
	return info_pokemones;
}

/*
 * El tope debe tener el numero de la cantidad de elementos del vector pokemones.
 * Devuelve la posicion en el vector en la cual se encuentra el nombre buscado,
 * si no lo encuentra devuelve INVALIDO.
 */
int busqueda_binaria(struct pokemon *pokemones, int tope, const char *nombre)
{
	int inicio = 0;
	int fin = tope;
	int posicion = INVALIDO;
	while (inicio <= fin && posicion == INVALIDO) {
		int centro = (inicio + fin) / 2;
		if (strcmp(pokemones[centro].nombre, nombre) > 0)
			fin = centro - 1;
		else if (strcmp(pokemones[centro].nombre, nombre) < 0)
			inicio = centro + 1;
		else
			posicion = centro;
	}
	return posicion;
}

pokemon_t *pokemon_buscar(informacion_pokemon_t *ip, const char *nombre)
{
	if (ip == NULL || nombre == NULL)
		return NULL;

	int posicion =
		busqueda_binaria(ip->pokemones, ip->cantidad_pokemones, nombre);
	return posicion == INVALIDO ? NULL : &ip->pokemones[posicion];
}

int pokemon_cantidad(informacion_pokemon_t *ip)
{
	return ip == NULL ? 0 : ip->cantidad_pokemones;
}

const char *pokemon_nombre(pokemon_t *pokemon)
{
	return pokemon == NULL ? NULL : pokemon->nombre;
}

enum TIPO pokemon_tipo(pokemon_t *pokemon)
{
	return pokemon == NULL ? NORMAL : pokemon->tipo;
}

const struct ataque *pokemon_buscar_ataque(pokemon_t *pokemon,
					   const char *nombre)
{
	if (pokemon == NULL || nombre == NULL)
		return NULL;

	int posicion = INVALIDO;
	for (int i = 0; i < MAX_ATAQUES && posicion == INVALIDO; i++) {
		if (strcmp(pokemon->ataques[i].nombre, nombre) == 0)
			posicion = i;
	}
	return posicion == INVALIDO ? NULL : &pokemon->ataques[posicion];
}

int con_cada_pokemon(informacion_pokemon_t *ip, void (*f)(pokemon_t *, void *),
		     void *aux)
{
	if (ip == NULL || f == NULL)
		return 0;

	for (int i = 0; i < ip->cantidad_pokemones; i++)
		f(&ip->pokemones[i], aux);
	return ip->cantidad_pokemones;
}

int con_cada_ataque(pokemon_t *pokemon,
		    void (*f)(const struct ataque *, void *), void *aux)
{
	if (pokemon == NULL || f == NULL)
		return 0;

	for (int i = 0; i < MAX_ATAQUES; i++)
		f(&pokemon->ataques[i], aux);
	return MAX_ATAQUES;
}

void pokemon_destruir_todo(informacion_pokemon_t *ip)
{
	if (ip == NULL)
		return;

	for (int i = 0; i < ip->cantidad_pokemones; i++)
		free(ip->pokemones[i].ataques);
	free(ip->pokemones);
	free(ip);
}