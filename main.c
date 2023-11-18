#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "src/pokemon.h"
#include "src/ataque.h"
#include "src/juego.h"
#include "src/lista.h"
#include "src/adversario.h"

#define BLANCO "\x1b[37;1m"
#define VERDE "\x1b[32;1m"
#define ROJO "\x1b[31;1m"
#define AMARILLO "\x1b[33;1m"
#define NORMAL "\x1b[0m"
#define MAGNETA "\x1b[35;1m"
#define CYAN "\x1b[36;1m"

enum RESULTADO { ERROR, OK, SALIR };

#define CMD_AYUDA "ayuda"
#define CMD_SALIR "salir"
#define CMD_CLEAR "clear"
#define CMD_SELECCIONAR_POKEMONES "s"
#define CMD_HACER_JUGADA "j"
#define CMD_MOSTRAR_PUNTAJE "p"
#define MAX_CARACTERES 50
#define MAX_ELECCIONES 3

void informar_aviso(char *aviso, bool error)
{
	error ? printf(ROJO "ERROR: " NORMAL) : printf(VERDE "EXITO: " NORMAL);
	printf("%s\n", aviso);		
}

void mostrar_comando(char *comando, char *descripcion)
{
	printf(AMARILLO "%s: ", comando);
	printf(NORMAL "%s\n", descripcion);
}

enum RESULTADO mostrar_comandos_disponibles()
{
	printf("Tenes disponibles los siguientes comandos: \n");
	mostrar_comando(CMD_AYUDA, "Muestra por pantalla los comandos disponibles");
	mostrar_comando(CMD_SALIR, "Sale del programa");
	mostrar_comando(CMD_SELECCIONAR_POKEMONES, "Selecciona los pokemones para comenzar a jugar");
	mostrar_comando(CMD_HACER_JUGADA, "Selecciona el pokemon y ataque para poder realizar una jugada");
	mostrar_comando(CMD_MOSTRAR_PUNTAJE, "Muestra el puntaje de los jugadores");
	return OK;
}

void mostrar_ataques_disponibles(const struct ataque *ataque, void *aux)
{
	printf(MAGNETA "Poder: ");
	printf(NORMAL "%u\t", ataque->poder);
	printf(ROJO "\t Ataque: ");
	printf(NORMAL "%s\n", ataque->nombre);
}

bool mostrar_pokemones_disponibles(void *_pokemon, void *aux)
{
	printf(AMARILLO "Nombre: ");
	printf(NORMAL "%s\n", pokemon_nombre(_pokemon));
	con_cada_ataque(_pokemon, mostrar_ataques_disponibles, aux);
	return true;
}

void listar_pokemones(void *juego)
{
        lista_con_cada_elemento(juego_listar_pokemon((juego_t *)juego), mostrar_pokemones_disponibles, NULL);
}

enum RESULTADO seleccionar_pokemones_usuario(juego_t *juego, char *nombre1, char *nombre2, char *nombre3)
{
	char *nombres[] = { nombre1, nombre2, nombre3 };

	for (int i = 0; i < MAX_ELECCIONES; i++) {
		printf(MAGNETA "TP2> " NORMAL);
		printf("Nombre: ");
		fscanf(stdin, "%s", nombres[i]);
	}

	JUEGO_ESTADO estado = juego_seleccionar_pokemon((juego_t *)juego, JUGADOR1, nombre1, nombre2, nombre3);

        if (estado == POKEMON_INEXISTENTE || estado == POKEMON_REPETIDO) {
		informar_aviso(estado == POKEMON_INEXISTENTE ? "Ingresa pokemones de la lista" : "No se pueden pokemones repetidos", true);
                return seleccionar_pokemones_usuario(juego, nombre1, nombre2, nombre3);
        }
        
	if (estado == ERROR_GENERAL) {
		informar_aviso("No es tu culpa", true);
		return ERROR;
	}

	informar_aviso("Pokemones cargados :)", false);
	return OK;
}

enum RESULTADO seleccionar_pokemones_ia(juego_t *juego, adversario_t *ia)
{
	char *nombre1;
	char *nombre2;
	char *nombre3;
	if (!adversario_seleccionar_pokemon(ia, &nombre1, &nombre2, &nombre3))
		return ERROR;

	if (juego_seleccionar_pokemon(juego, JUGADOR2, nombre1, nombre2, nombre3) == ERROR_GENERAL)
		return ERROR;

	return OK;
}

enum RESULTADO seleccionar_pokemones(juego_t *juego, adversario_t *ia)
{	
        printf("Tenes todos estos pokemones, selecciona tres. Los dos primeros son para vos y el tercero para tu adversario.\n");
	listar_pokemones(juego);

	char nombre1[MAX_CARACTERES];
	char nombre2[MAX_CARACTERES];
	char nombre3[MAX_CARACTERES];

	// el usuario elije pokemones validos y ya se cargan en juego.
	if (seleccionar_pokemones_usuario(juego, nombre1, nombre2, nombre3) == ERROR)
		return ERROR;

	// se le carga el tercer pokemon al adversario.
	if (!adversario_pokemon_seleccionado(ia, nombre1, nombre2, nombre3))
		return ERROR;

	// el adversario elije pokemones y se guardan en el juego.
	if (seleccionar_pokemones_ia(juego, ia) == ERROR)
		return ERROR;

	return OK;
}

jugada_t realizar_jugada_usuario(juego_t *juego)
{
	jugada_t jugada = { .ataque = "", .pokemon = "" };

	return jugada;
}

char *resultado_ataque(RESULTADO_ATAQUE resultado)
{
	if (resultado == ATAQUE_REGULAR)
		return AMARILLO "regular" NORMAL;
	if (resultado == ATAQUE_EFECTIVO)
		return VERDE "efectivo" NORMAL;
	return ROJO "inefectivo" NORMAL;
}

void mostrar_resultado_ataque(RESULTADO_ATAQUE resultado, jugada_t usuario, jugada_t ia)
{
	printf("El ataque %s fue %s contra el ataque %s\n", usuario.ataque, resultado_ataque(resultado), ia.ataque);
}

enum RESULTADO jugar_ronda(juego_t *juego, adversario_t *ia)
{
	jugada_t jugada_usuario;
	jugada_t jugada_ia;

	resultado_jugada_t resultado = { .jugador1 = ATAQUE_ERROR, .jugador2 = ATAQUE_ERROR };

	while (resultado.jugador1 == ATAQUE_ERROR || resultado.jugador2 == ATAQUE_ERROR) {
		if (resultado.jugador1 == ATAQUE_ERROR)
			jugada_usuario = realizar_jugada_usuario(juego);
		if (resultado.jugador2 == ATAQUE_ERROR)
			jugada_ia = adversario_proxima_jugada(ia);
		
		resultado = juego_jugar_turno(juego, jugada_usuario, jugada_ia);
	}

	mostrar_resultado_ataque(resultado.jugador1, jugada_usuario, jugada_ia);

	informar_aviso("Ataque exitoso :)", false);
	return OK;
}

enum RESULTADO mostrar_puntaje(juego_t *juego)
{
	printf(AMARILLO "Usuario: " NORMAL);
	printf("%i\n", juego_obtener_puntaje(juego, JUGADOR1));
	printf(AMARILLO "Ia: " NORMAL);
	printf("%i\n", juego_obtener_puntaje(juego, JUGADOR2));
	return OK;
}

enum RESULTADO ejecutar_comando(char *comando, juego_t *juego, adversario_t *ia)
{
	if (strcmp(comando, CMD_AYUDA) == 0)
		return mostrar_comandos_disponibles();

	if (strcmp(comando, CMD_SALIR) == 0)
		return SALIR;

	if (strcmp(comando, CMD_CLEAR) == 0) {
		system(CMD_CLEAR);
		return OK;
	}
		
	if (strcmp(comando, CMD_SELECCIONAR_POKEMONES) == 0) 
		return seleccionar_pokemones(juego, ia);
		
//	if (strcmp(comando, CMD_HACER_JUGADA) == 0)
//		return jugar_ronda(juego, ia);

	if (strcmp(comando, CMD_MOSTRAR_PUNTAJE) == 0)
		return mostrar_puntaje(juego);
	
	informar_aviso(("El comando no existe, intenta con 'ayuda'"), true);
	return ERROR;
}

void liberar_todo(juego_t *juego, adversario_t *ia)
{
        adversario_destruir(ia);
        juego_destruir(juego);        
}

int main(int argc, char *argv[])
{
	//if (argc != 2) {
	//	informar_aviso("Ingrese el nombre del archivo de pokemones :)", true);
        //	return -1;
      	//}

        juego_t *juego = juego_crear();
	if (!juego) 
		return -1;

	//JUEGO_ESTADO estado = juego_cargar_pokemon(juego, *(argv + 1));
	JUEGO_ESTADO estado = juego_cargar_pokemon(juego, "ejemplos/correcto.txt");
        if (estado != TODO_OK) {
		informar_aviso(estado == ERROR_GENERAL ? "error el archivo no existe" : "La cantidad de pokemones es invalida, intenta con otro archivo", true);
                liberar_todo(juego, NULL);
                return -1;
        }

	adversario_t *ia = adversario_crear(juego_listar_pokemon(juego));
	if (!ia) {
		liberar_todo(juego, NULL);
		return -1;
	}
 
	printf("Ingrese 'ayuda' para ver los comandos disponibles\n");
	enum RESULTADO resultado = OK;

	while (!juego_finalizado(juego) && resultado != SALIR) {
		printf(MAGNETA "==TP2== " NORMAL);
		char comando[MAX_CARACTERES];
		fscanf(stdin, "%s", comando);

		resultado = ejecutar_comando(comando, juego, ia);
	}

        liberar_todo(juego, ia);
        return 0;
}