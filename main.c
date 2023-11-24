#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#include "src/pokemon.h"
#include "src/ataque.h"
#include "src/juego.h"
#include "src/lista.h"
#include "src/adversario.h"
#include "src/hash.h"
#include "src/comun.h"
#include "src/menu.h"

#define BLANCO "\x1b[37;1m"
#define VERDE "\x1b[32;1m"
#define ROJO "\x1b[31;1m"
#define AZUL "\x1b[34;1m"
#define AMARILLO "\x1b[33;1m"
#define COMUN "\x1b[0m"
#define MAGNETA "\x1b[35;1m"
#define CYAN "\x1b[36;1m"
#define NARANJA "\x1b[38;2;255;128;0m"
#define GRIS "\x1b[38;2;176;174;174m"

#define CMD_AYUDA "ayuda"
#define CMD_SALIR "salir"
#define CMD_CLEAR "clear"
#define CMD_SELECCIONAR "s"
#define CMD_JUGADA "j"
#define CMD_PUNTAJE "p"
#define CMD_MOSTRAR "m"
#define TOTAL_COMANDOS 7

#define MAX_CARACTERES 50
#define MAX_ELECCIONES 3

struct paquete {
	menu_t *menu;
	juego_t *juego;
	adversario_t *ia;
	lista_t *pokemones_usuario;
};

void tolower_str(char *str, size_t tamanio) 
{
	for (size_t i = 0; i < tamanio; i++)
		str[i] = (char )tolower(str[i]);

}

void informar_aviso(char *aviso, bool error)
{
	error ? printf(ROJO "ERROR: " COMUN) : printf(VERDE "EXITO: " COMUN);
	printf("%s\n", aviso);		
}

bool mostrar_comando(const char *comando, void *info_comando, void *aux)
{
	printf(AMARILLO "%s: ", comando);
	printf(COMUN "%s\n", descripcion_comando(info_comando));
	return true;
}

void determinar_color(enum TIPO tipo, const char *str)
{	
	switch (tipo)
	{
	case NORMAL:
		printf(NARANJA "%s\n" COMUN, str);
		break;
	case FUEGO:
		printf(ROJO "%s 🔥\n" COMUN, str);
		break;
	case AGUA:
		printf(AZUL "%s 💧\n" COMUN, str);
		break;
	case PLANTA:
		printf(VERDE "%s 🌿\n" COMUN, str);
		break;
	case ELECTRICO:
		printf(CYAN "%s ⚡\n" COMUN, str);
		break;
	case ROCA:
		printf(GRIS "%s 🗻\n" COMUN, str);
		break;
	default:
		break;
	}
}

void mostrar_ataques_disponibles(const struct ataque *ataque, void *aux)
{
	printf(BLANCO "Poder: ");
	printf(COMUN "%u", ataque->poder);
	printf(BLANCO "\t Ataque: ");
	determinar_color(ataque->tipo, ataque->nombre);
}

bool mostrar_pokemones_disponibles(void *pokemon, void *aux)
{
	printf(AMARILLO "Nombre: ");
	determinar_color(pokemon_tipo(pokemon), pokemon_nombre(pokemon));
	con_cada_ataque(pokemon, mostrar_ataques_disponibles, aux);
	return true;
}

RESULTADO listar_pokemones(void *lista)
{
        size_t aplicados = lista_con_cada_elemento(lista, mostrar_pokemones_disponibles, NULL);
	return aplicados == lista_tamanio(lista) ? OK : ERROR;
}

RESULTADO seleccionar_pokemones_usuario(juego_t *juego, adversario_t *ia, lista_t *pokemones_usuario)
{
	char nombre1[MAX_CARACTERES];
	char nombre2[MAX_CARACTERES];
	char nombre3[MAX_CARACTERES];
	char *nombres[] = { nombre1, nombre2, nombre3 };

	for (int i = 0; i < MAX_ELECCIONES; i++) {
		printf(MAGNETA "==TP2== ");
		printf(AMARILLO "Nombre: " COMUN);
		fscanf(stdin, "%s", nombres[i]);
	}

	JUEGO_ESTADO estado = juego_seleccionar_pokemon((juego_t *)juego, JUGADOR1, nombre1, nombre2, nombre3);

        if (estado == POKEMON_INEXISTENTE || estado == POKEMON_REPETIDO) {
		informar_aviso(estado == POKEMON_INEXISTENTE ? "Ingresa pokemones de la lista" : "No se pueden pokemones repetidos", true);
                return seleccionar_pokemones_usuario(juego, ia, pokemones_usuario);
        }
        
	if (estado == ERROR_GENERAL) {
		informar_aviso("No es tu culpa", true);
		return ERROR;
	}

	if (!adversario_pokemon_seleccionado(ia, nombre1, nombre2, nombre3))
		return ERROR;

	lista_insertar(pokemones_usuario, lista_buscar_elemento(juego_listar_pokemon(juego), comparar_nombres, nombre1));
	lista_insertar(pokemones_usuario, lista_buscar_elemento(juego_listar_pokemon(juego), comparar_nombres, nombre2));

	informar_aviso("Pokemones cargados :)", false);
	return OK;
}

RESULTADO seleccionar_pokemones_ia(juego_t *juego, adversario_t *ia, lista_t *pokemones_usuario)
{
	char *nombre1;
	char *nombre2;
	char *nombre3;
	if (!adversario_seleccionar_pokemon(ia, &nombre1, &nombre2, &nombre3))
		return ERROR;

	if (juego_seleccionar_pokemon(juego, JUGADOR2, nombre1, nombre2, nombre3) == ERROR_GENERAL)
		return ERROR;

	lista_insertar(pokemones_usuario, lista_buscar_elemento(juego_listar_pokemon(juego), comparar_nombres, nombre3));
	return OK;
}

jugada_t realizar_jugada_usuario(juego_t *juego)
{
	jugada_t jugada = { .ataque = "", .pokemon = "" };
	char pokemon[MAX_CARACTERES];
	char ataque[MAX_CARACTERES];

	printf("Selecciona un pokemon y el ataque que quieras usar de este\n");

	printf(AMARILLO "Pokemon: " COMUN);
	fscanf(stdin, "%s", pokemon);
	printf(AMARILLO "Ataque: " COMUN);
	fscanf(stdin, "%s", ataque);

	strcpy(jugada.pokemon, pokemon);
	strcpy(jugada.ataque, ataque);

	return jugada;
}

char *resultado_ataque(RESULTADO_ATAQUE resultado)
{
	if (resultado == ATAQUE_REGULAR)
		return AMARILLO "regular" COMUN;
	if (resultado == ATAQUE_EFECTIVO)
		return VERDE "efectivo" COMUN;
	return ROJO "inefectivo" COMUN;
}

void mostrar_resultado_ataque(resultado_jugada_t resultado, jugada_t usuario, jugada_t ia)
{
	printf("El ataque ");
	printf(BLANCO "%s " COMUN, usuario.ataque);
	printf("fue %s contra el pokemon de tu adversario\n", resultado_ataque(resultado.jugador1));
	printf("El ataque ");
	printf(BLANCO "%s " COMUN, ia.ataque);
	printf("fue %s contra tu pokemon\n", resultado_ataque(resultado.jugador2));
}

RESULTADO listar_comandos(void *_paquete)
{
	struct paquete *paquete = _paquete;
	size_t aplicada = con_cada_comando(paquete->menu, mostrar_comando, NULL);
	return aplicada == cantidad_comandos(paquete->menu) ? OK : ERROR;
}

RESULTADO salir_programa(void *_paquete)
{
	return SALIR;
}

RESULTADO limpiar_pantalla(void *_paquete)
{
	system("clear");
	return OK;
}

RESULTADO seleccionar_pokemones(void *_paquete)
{	
	struct paquete *paquete = _paquete;
	juego_t *juego = paquete->juego;
	adversario_t *ia = paquete->ia;
	lista_t *pokemones = paquete->pokemones_usuario;

        printf("Tenes todos estos pokemones, selecciona tres. Los dos primeros son para vos y el tercero para tu adversario\n");
	printf("El color del pokemon y ataque corresponde al tipo de este\n");
	listar_pokemones(juego_listar_pokemon(juego));

	if (seleccionar_pokemones_usuario(juego, ia, pokemones) == ERROR)
		return ERROR;

	if (seleccionar_pokemones_ia(juego, ia, pokemones) == ERROR)
		return ERROR;

	return OK;
}

RESULTADO jugar_ronda(void *_paquete)
{
	struct paquete *paquete = _paquete;
	juego_t *juego = paquete->juego;
	adversario_t *ia = paquete->ia;

	jugada_t jugada_ia = adversario_proxima_jugada(ia);
	jugada_t jugada_usuario = { .ataque = "", .pokemon = "" };
	resultado_jugada_t resultado = { .jugador1 = ATAQUE_ERROR, .jugador2 = ATAQUE_ERROR };

	while (resultado.jugador1 == ATAQUE_ERROR) {
		jugada_usuario = realizar_jugada_usuario(juego);
		resultado = juego_jugar_turno(juego, jugada_usuario, jugada_ia);
		if (resultado.jugador1 == ATAQUE_ERROR)
			informar_aviso("Fijate de estar ingresando el pokemon correcto y acordate de que no podes repetir ataques", true);
	}	

	mostrar_resultado_ataque(resultado, jugada_usuario, jugada_ia);
	return OK;
}

RESULTADO mostrar_puntaje(void *_paquete)
{
	struct paquete *paquete = _paquete;
	printf(AMARILLO "Usuario: " COMUN);
	printf("%i\n", juego_obtener_puntaje(paquete->juego, JUGADOR1));
	printf(AMARILLO "Ia: " COMUN);
	printf("%i\n", juego_obtener_puntaje(paquete->juego, JUGADOR2));
	return OK;
}

RESULTADO mostrar_tus_pokemones(void *_paquete)
{
	struct paquete *paquete = _paquete;
	return listar_pokemones(paquete->pokemones_usuario);
}

bool verificar_comando(char *comando, bool *selecciono)
{
	if (!strcmp(comando, CMD_JUGADA) && !*selecciono) {
		informar_aviso("No tenes pokemones para jugar, intenta con 's'", true);
		return false;
	} else if (!strcmp(comando, CMD_SELECCIONAR) && *selecciono) {
		informar_aviso("No podes volver a seleccionar pokemones hasta terminar la partida", true);
		return false;
	} else if (!strcmp(comando, CMD_SELECCIONAR)) {
		*selecciono = true;
		return true;
	} else if (!strcmp(comando, CMD_MOSTRAR) && !*selecciono) {
		informar_aviso("No tenes pokemones asignados, intenta con 's'", true);
		return true;
	}
	return true;	
}

void liberar_todo(void *menu, void *juego, void *ia, void *pokemones)
{
	lista_destruir(pokemones);
        adversario_destruir(ia);
        juego_destruir(juego);  
	menu_destruir(menu);      
}

bool agregar_comandos(menu_t *menu)
{
	if (!menu)
		return false;

	comando_agregar(menu, CMD_AYUDA, "Muestra por pantalla los comandos disponibles", listar_comandos);
	comando_agregar(menu, CMD_SALIR, "Sale del programa", salir_programa);
	comando_agregar(menu, CMD_CLEAR, "Limpia la pantalla", limpiar_pantalla);
	comando_agregar(menu, CMD_SELECCIONAR, "Selecciona los pokemones para comenzar a jugar", seleccionar_pokemones);
	comando_agregar(menu, CMD_JUGADA, "Selecciona el pokemon y ataque para poder realizar una jugada", jugar_ronda);
	comando_agregar(menu, CMD_PUNTAJE, "Muestra el puntaje de ambos jugadores", mostrar_puntaje);
	comando_agregar(menu, CMD_MOSTRAR, "Muestra por pantalla los pokemones propios", mostrar_tus_pokemones);
	return cantidad_comandos(menu) ==  TOTAL_COMANDOS;
}

int main(int argc, char *argv[])
{
	srand(( unsigned)time(NULL));

	menu_t *menu = menu_crear();
	if (!agregar_comandos(menu)) {
		liberar_todo(menu, NULL, NULL, NULL);
		return -1;
	}

        juego_t *juego = juego_crear();
	if (!juego) {
		liberar_todo(menu, NULL, NULL, NULL);
		return -1;
	}
	
	JUEGO_ESTADO estado = juego_cargar_pokemon(juego, "ejemplos/correcto.txt");
        if (estado != TODO_OK) {
		informar_aviso(estado == ERROR_GENERAL ? "El archivo no existe" : "La cantidad de pokemones es invalida, intenta con otro archivo", true);
                liberar_todo(menu, juego, NULL, NULL);
		return -1;
        }

	adversario_t *ia = adversario_crear(juego_listar_pokemon(juego));
	if (!ia) {
		liberar_todo(menu, juego, NULL, NULL);
		return -1;
	}

	lista_t *pokemones_usuario = lista_crear();
	if (!pokemones_usuario) {
		liberar_todo(menu, juego, ia, NULL);
		return -1;	
	}

	RESULTADO resultado = OK;
	bool selecciono = false;
	struct paquete paquete = { .menu = menu, .juego = juego, .ia = ia, .pokemones_usuario = pokemones_usuario};

	printf("Ingrese 'ayuda' para ver los comandos disponibles\n");
	while (!juego_finalizado(juego) && (resultado == OK || resultado == COMANDO_INVALIDO)) {
		char comando[MAX_CARACTERES];
		printf(MAGNETA "==TP2== " COMUN);
		fscanf(stdin, "%s", comando);
		tolower_str(comando, strlen(comando));

		if (verificar_comando(comando, &selecciono))
			resultado = ejecutar_comando(menu, comando, &paquete); 

		if (resultado == COMANDO_INVALIDO)
			informar_aviso("El comando no existe, intenta con 'ayuda'", true);
	}

	if (resultado == OK) {
		ejecutar_comando(menu, CMD_PUNTAJE, &paquete);
		printf("Parece que %s\n", juego_obtener_puntaje(juego, JUGADOR1) > juego_obtener_puntaje(juego, JUGADOR2) ? VERDE "ganaste, felicitaciones :)" COMUN : ROJO "perdiste, mala suerte :(" COMUN);
		printf(VERDE "Gracias por jugar\n" COMUN);
	}
	
        liberar_todo(menu, juego, ia, pokemones_usuario);
        return resultado == OK ? 0 : -1;
}