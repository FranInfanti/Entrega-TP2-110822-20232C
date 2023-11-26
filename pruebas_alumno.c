#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "src/pokemon.h"
#include "src/ataque.h"
#include "src/juego.h"
#include "src/lista.h"
#include "src/tipo.h"
#include "src/hash.h"
#include "pa2m.h"
#include "src/adversario.h"


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

void no_se_puede_seleccionar_pokemones_inexistentes()
{
        juego_t *juego = juego_crear();
        juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

        char *nombre1 = "Toto";
        char *nombre2 = "Pikachu";
        char *nombre3 = "Charmander";

        pa2m_afirmar(juego_seleccionar_pokemon(juego, JUGADOR1, nombre1, nombre2, nombre3) == POKEMON_INEXISTENTE, "No se puede seleccionar pokemones que no existen");
        juego_destruir(juego);
}

void no_se_puede_seleccionar_pokemones_repetidos()
{
        juego_t *juego = juego_crear();
        juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

        char *nombre1 = "Charmander";
        char *nombre2 = "Pikachu";
        char *nombre3 = "Charmander";

        pa2m_afirmar(juego_seleccionar_pokemon(juego, JUGADOR1, nombre1, nombre2, nombre3) == POKEMON_REPETIDO, "No se puede seleccionar pokemones repetidos entre si");
        juego_destruir(juego);  
}

void se_puede_selecionnar_pokemones_validos()
{
        juego_t *juego = juego_crear();
        juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

        char *nombre1 = "Cacnea";
        char *nombre2 = "Pikachu";
        char *nombre3 = "Charmander";

        pa2m_afirmar(juego_seleccionar_pokemon(juego, JUGADOR1, nombre1, nombre2, nombre3) == TODO_OK, "Se puede seleccionar pokemones validos correctamente");
        juego_destruir(juego); 
}

void prueba_de_ataque()
{
        juego_t *juego = juego_crear();
        juego_cargar_pokemon(juego, "ejemplos/correcto.txt");
        
        juego_seleccionar_pokemon(juego, JUGADOR1, "Pikachu", "Charmander", "Cacnea");
        juego_seleccionar_pokemon(juego, JUGADOR2, "Floatzel", "Pikachu", "Togepi");

        jugada_t jugador1 = { .pokemon = "Pikachu", .ataque = "Latigo" };
        jugada_t jugador2 = { .pokemon = "Floatzel", .ataque = "Retribucion" };

        resultado_jugada_t jugada = juego_jugar_turno(juego, jugador1, jugador2);
        pa2m_afirmar(jugada.jugador1 == ATAQUE_REGULAR && jugada.jugador2 == ATAQUE_REGULAR, "Un ataque electrico, hecho por el jugador1, contra uno de agua, del jugador2, resulta efectivo para el jugador1 e inefectivo para el jugador2");
        pa2m_afirmar(juego_obtener_puntaje(juego, JUGADOR1) == 1 && juego_obtener_puntaje(juego, JUGADOR2) == 2, "Los puntajes son correctos");
        pa2m_afirmar(juego_finalizado(juego) == false, "EL juego sigue en curso");
        
        juego_destruir(juego);
}

void pruebas_de_adversario()
{
        juego_t *juego = juego_crear();
        juego_cargar_pokemon(juego, "ejemplos/correcto.txt");

        adversario_t *adversario = adversario_crear(juego_listar_pokemon(juego));

        char *nombre1;
        char *nombre2;
        char *nombre3;
        adversario_seleccionar_pokemon(adversario, &nombre1, &nombre2, &nombre3);

        printf("nombre1: %s\n", nombre1);
        printf("nombre2: %s\n", nombre2);
        printf("nombre3: %s\n", nombre3);

        adversario_destruir(adversario);
        juego_destruir(juego);
}

int main()
{
        srand (( unsigned)time(NULL));
        pa2m_nuevo_grupo("Pruebas de Seleccion");
        no_se_puede_seleccionar_pokemones_inexistentes();
        no_se_puede_seleccionar_pokemones_repetidos();
        se_puede_selecionnar_pokemones_validos();

        pa2m_nuevo_grupo("Pruebas de Ataque");
        prueba_de_ataque();

        pa2m_nuevo_grupo("Pruebas de Adversario");
        pruebas_de_adversario();

        return pa2m_mostrar_reporte();
}