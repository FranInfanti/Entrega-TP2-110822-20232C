#include <string.h>
#include "comun.h"
#include "pokemon.h"
#include "abb.h"

int comparador_abb(void *_ataque1, void *_ataque2)
{
	struct ataque *ataque1 = _ataque1;
	struct ataque *ataque2 = _ataque2;

	return strcmp(ataque1->nombre, ataque2->nombre);
}

int comparar_nombres(void *pokemon, void *nombre)
{
	return strcmp(pokemon_nombre((pokemon_t *)pokemon), (char *)nombre);
}

void guardar_ataques(const struct ataque *ataque, void *abb)
{
	abb_insertar((abb_t *)abb, (void *)ataque);
}