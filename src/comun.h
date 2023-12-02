#ifndef _COMUN_H_
#define _COMUN_H_

#include "ataque.h"

/*
 * Recibe dos ataques.
 * 
 * Compara sus nombres y devuelve 0 si son iguales, 
 * 0 < si el nombre del ataque1 es mas grande y 0 > 
 * si el nombre del ataque2 es mas grande.
 */
int comparador_abb(void *_ataque1, void *_ataque2);

/*
 * Recibe un pokemon y un nombre.
 *
 * Compara el nombre del pokemon con el recibido por parametro.
 * Devuelve 0 si son iguales.
 */
int comparar_nombres(void *pokemon, void *nombre);

/*
 * Recibe un ataque y un puntero a un abb.
 *
 * Inserta en el abb el ataque pasado por parametro.
 */
void guardar_ataques(const struct ataque *ataque, void *abb);

#endif // _COMUN_H_
