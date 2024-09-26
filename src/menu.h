#ifndef MENU_H_
#define MENU_H_
#include <stdlib.h>

typedef enum RESULTADO { ERROR, OK, SALIR, COMANDO_INVALIDO } RESULTADO;

typedef struct menu menu_t;

/*
 * Crea un menu. Devuele NULL en caso de error.
 */
menu_t *menu_crear(size_t capacidad);

/*
 * Recibe un menu, un comando, la descripcion del comando y la funcion que va a ejecutar el comando.
 *
 * Guarda el nuevo comando con sus caracteristicas en el menu.
 * Devuelve NULL en caso de error. 
 */
menu_t *comando_agregar(menu_t *menu, const char *comando, char *descripcion,
			RESULTADO (*f)(void *));

/*
 * Recibe un menu, un comando y un parametro auxiliar para enviar a la funcion del comando.
 *
 * Devuelve ERROR en caso de no poder la ejecucion, sino devuelve lo que devuelva la funcion que 
 * fue guardada para el comando.
 */
enum RESULTADO ejecutar_comando(menu_t *menu, const char *comando, void *aux);

/*
 * Recibe la informacion que contiene el comando y devuelve la descripcion de este.
 * o NULL en caso de error.
 */
char *descripcion_comando(void *info_comando);

/*
 * Recibe un menu.
 * Devuelve la cantidad de comandos que tiene almacenados.
 * En caso de error devuelve 0.
 */
size_t cantidad_comandos(menu_t *menu);

/*
 * Recibe un menu, una funcion de tipo bool y un parametro auxiliar.
 *
 * Aplica a cada comando almacenado la funcion pasada por parametro, 
 * mandando como un parametro el auxiliar recibido.
 * 
 * Devuelve la cantidad de elementos a las que se le aplico la funcion o 0 en caso de error.
 */
size_t con_cada_comando(menu_t *menu,
			bool (*f)(const char *comando, void *info_comando,
				  void *aux),
			void *aux);

/* 
 * Destruye el menu y liberando la memoria que este ocupa.
 */
void menu_destruir(menu_t *menu);

#endif // MENU_H_