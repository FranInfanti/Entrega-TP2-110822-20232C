#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "menu.h"
#include "hash.h"
#include "juego.h"

struct info_comando {
	char *descripcion;
	RESULTADO (*f)(void *);
};

struct menu {
	hash_t *comandos;
};

/*
 * Crea un bloque donde se guarda la descripcion del comando y la funcion que ejecuta.
 * Devuelve NULL en caso de error.
 */
struct info_comando *cargar_info(char *descripcion, RESULTADO (*f)(void *))
{
	struct info_comando *info_comando = malloc(sizeof(struct info_comando));
	if (!info_comando)
		return NULL;

	info_comando->descripcion = descripcion;
	info_comando->f = f;
	return info_comando;
}

/*
 * Libera la memoria que esta ocupando el comando.
 */
void destruir_comandos(void *comando)
{
	free((struct info_comando *)comando);
}

menu_t *menu_crear(size_t capacidad)
{
	struct menu *menu = calloc(1, sizeof(struct menu));
	if (!menu)
		return NULL;

	menu->comandos = hash_crear(capacidad);
	if (!menu->comandos) {
		free(menu);
		return NULL;
	}

	return menu;
}

menu_t *comando_agregar(menu_t *menu, const char *comando, char *descripcion,
			RESULTADO (*f)(void *))
{
	if (!menu || !comando || !descripcion || !f)
		return NULL;

	struct info_comando *info_comando = cargar_info(descripcion, f);
	if (!info_comando)
		return NULL;

	if (!hash_insertar(menu->comandos, comando, (void *)info_comando, NULL))
		return NULL;

	return menu;
}

RESULTADO ejecutar_comando(menu_t *menu, const char *comando, void *aux)
{
	if (!menu || !comando)
		return ERROR;

	struct info_comando *info_comando =
		hash_obtener(menu->comandos, comando);
	if (!info_comando)
		return COMANDO_INVALIDO;

	return info_comando->f(aux);
}

char *descripcion_comando(void *info_comando)
{
	if (!info_comando)
		return NULL;

	struct info_comando *informacion_comando = info_comando;
	return informacion_comando->descripcion;
}

size_t cantidad_comandos(menu_t *menu)
{
	return hash_cantidad(menu->comandos);
}

size_t con_cada_comando(menu_t *menu,
			bool (*f)(const char *comando, void *info_comando,
				  void *aux),
			void *aux)
{
	if (!menu || !f)
		return 0;

	return hash_con_cada_clave(menu->comandos, f, aux);
}

void menu_destruir(menu_t *menu)
{
	if (!menu)
		return;

	hash_destruir_todo(menu->comandos, destruir_comandos);
	free(menu);
}
