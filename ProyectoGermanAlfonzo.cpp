/*Proyecto de estructura
German Alfonzo C.I:32342610
*/

#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <map>

using namespace std;

/******************************/
/*         CONSTANTES         */
/******************************/
const int MAX_IMPLEMENTOS = 5;
const int NUM_HEROES_A_ELEGIR = 4;
const int MAX_NIVEL = 20;
const int EXPERIENCIA_BASE = 100;
const int MAX_SALAS = 30;
const int PROBABILIDAD_EVENTO = 30; // 30% de chance de evento
const string ARCHIVO_PARTIDA = "partida_rd.save";

/******************************/
/*         ESTRUCTURAS        */
/******************************/

// Nodo plantilla para listas simples
template <typename T>
struct Nodo {
    int idNodo;
    T dato;
    Nodo<T> * siguiente;
};

// Enumeracion de tipos de efectos
enum class TipoEfecto {
    NINGUNO,
    VENENO,
    QUEMADURA,
    CONGELACION,
    BENDICION,
    MALDICION,
    REGENERACION
};

// Estructura para efectos de estado
struct EfectoEstado {
    TipoEfecto tipo;
    int duracion;
    int potencia;
};

// Especie
struct Especie {
    string nombre;
    int fortalezaMax;
    int danoBase;
    int saludMax;
    int rapidez;
    int resistenciaMagica;
    string descripcion;
};

// Implemento
struct Implemento {
//    int idImplemento;
    string nombre;
    string tipo; // ataque, defensa, cura, poder_magico, especial
    int fortalezaNecesaria;
    int valor; // danio, curacion o defensa
    int usos;
    vector<EfectoEstado> efectos;
    string descripcion;
};

// Personaje
struct Personaje {
    Especie especie;
    string nombre;
    int salud;
    int saludMax;
    int fortaleza;
    int fortalezaMax;
    int rapidez;
    int experiencia;
    int nivel;
    bool vivo;
    vector<EfectoEstado> efectos;
    Nodo<Implemento> * mochila;
    
    // Estadisticas adicionales
    int muertes;
    int kills;
    int danioTotal;
    int curacionTotal;
};

// Evento especial en sala
struct EventoSala {
    string nombre;
    string descripcion;
    function<void(Personaje*)> efecto;
    bool positivo;
};

// Sala
struct Sala {
    int idSala;
    string nombre;
    string descripcion;
    Nodo<Sala*> * adyacentes;
    int cantidadOrcos;
    bool tienePuertaDestino;
    bool explorada;
    EventoSala* evento;
};

// Mapa
struct Mapa {
    Nodo<Sala> * salas;
    Sala* salaActual;
    Sala* salaInicial;
};


/******************************/
/* VARIABLES GLOBALES         */
/******************************/
Nodo<Especie> * listaEspecies = nullptr;
Nodo<Personaje> * listaHeroes = nullptr;
Nodo<Implemento> * listaImplementos = nullptr;
Mapa mapaGlobal;
vector<EventoSala> eventosDisponibles;

int turnoGlobal = 0;
bool partidaEnCurso = false;
bool juegoTerminado = false;
bool victoria = false;
int numRegistros = 0;

/******************************/
/* FUNCIONES DE UTILIDAD      */
/******************************/

// Funcion para limpiar la pantalla de forma portable
void limpiarPantalla() {
    #ifdef _WIN32
       system("cls");
    #else
       system("clear");
    #endif
}

// Funcion para pausar la ejecucion
void pausar() {
    std::cout << "\n Presione ENTER para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Funcion para generar nUmeros aleatorios en un rango
int aleatorioEnRango(int min, int max) {
    static bool semillaInicializada = false;
    if (!semillaInicializada) {
        srand(time(nullptr));
        semillaInicializada = true;
    }
    return min + (rand() % (max - min + 1));
}

// Funcion para capitalizar strings
string capitalizar(const string& str) {
    string resultado = str;
    if (!resultado.empty()) {
        resultado[0] = toupper(resultado[0]);
        for (size_t i = 1; i < resultado.size(); ++i) {
            resultado[i] = tolower(resultado[i]);
        }
    }
    return resultado;
}

/******************************/
/* FUNCIONES DE LECTURA Y VALIDAcion */
/******************************/

int leerEntero(string mensaje, int min, int max) {
    int num;
    while (true) {
        std::cout << mensaje;
        cin >> num;
        if (cin.fail() || num < min || num > max) {
            std::cout << "Entrada invalida. Intente nuevamente (valor entre " << min << " y " << max << ").\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return num;
        }
    }
}

string leerCadena(string mensaje, bool capitalizarEntrada = true) {
    string entrada;
    while (true) {
        std::cout << mensaje;
        std::getline(cin, entrada);
        
        // Eliminar espacios en blanco al inicio y final
        entrada.erase(entrada.find_last_not_of(" \t\n\r\f\v") + 1);
        entrada.erase(0, entrada.find_first_not_of(" \t\n\r\f\v"));
        
        if (entrada.empty()) {
            std::cout << "La entrada no puede estar vacia. Intente nuevamente.\n";
        } else {
            return capitalizarEntrada ? capitalizar(entrada) : entrada;
        }
    }
}

bool leerSiNo(string mensaje) {
    string entrada;
    while (true) {
        std::cout << mensaje << " (s/n): ";
        std::getline(cin, entrada);
        if (entrada.empty()) {
            std::cout << "Por favor ingrese 's' para si o 'n' para no.\n";
            continue;
        }
        
        char c = tolower(entrada[0]);
        if (c == 's') return true;
        if (c == 'n') return false;
        
        std::cout << "Entrada invalida. Por favor ingrese 's' o 'n'.\n";
    }
}

/******************************/
/* FUNCIONES DE LISTAS SIMPLES*/
/******************************/

template <typename T>
void agregarOrdenadamente(Nodo<T> * &lista, T dato, int idDelNodoNuevo) {
    Nodo<T> * nuevo = new Nodo<T>{idDelNodoNuevo, dato, nullptr};
    if (!lista) {
        lista = nuevo;
        //lista->idNodo = idDelNodoNuevo;
    } else {
        Nodo<T> * temp = lista;
        Nodo<T> * prev = lista;
        if(lista->idNodo > idDelNodoNuevo){
            nuevo->siguiente = lista;
            lista = nuevo;
            //prev = nuevo;
        }else{
            while (temp->siguiente && (temp->idNodo < idDelNodoNuevo)) {
                prev = temp;
                temp = temp->siguiente;
            }
            if (temp->idNodo < idDelNodoNuevo){
                temp->siguiente = nuevo;
            }else{
                prev->siguiente = nuevo;
                nuevo->siguiente = temp;
            }
        }    
    }
}


template <typename T>
void agregarAlFinal(Nodo<T> * &lista, T dato, int idDelNodoNuevo) {
    Nodo<T> * nuevo = new Nodo<T>{idDelNodoNuevo, dato, nullptr};
    if (!lista) {
        lista = nuevo;
    } else {
        Nodo<T> * temp = lista;
        while (temp->siguiente){ 
            temp = temp->siguiente;
        }
        temp->siguiente = nuevo;
        temp->idNodo = idDelNodoNuevo;
    }
}

template <typename T>
bool eliminarPorPosicion(Nodo<T> * &lista, int pos) {

    if (!lista || pos < 1) return false;
    
    Nodo<T> * temp = lista;
    if (pos == 1) {
        lista = lista->siguiente;
        delete temp;
        return true;
    }
    
    Nodo<T> * prev = nullptr;
    int idx = 1;
    while (temp && idx < pos) {
        prev = temp;
        temp = temp->siguiente;
        idx++;
    }
    
    if (temp) {
        prev->siguiente = temp->siguiente;
        delete temp;
        return true;
    }
    return false;
}

template <typename T>
bool eliminarPorID(Nodo<T> * &lista, int idAEliminar) {
    if (!lista || idAEliminar < 1) return false;
    
    Nodo<T> * temp = lista;
    if (temp->idNodo == idAEliminar) {
        lista = lista->siguiente;
        delete temp;
        return true;
    }
    
    Nodo<T> * prev = nullptr;
    int idx = 1;
    while (temp && temp->idNodo != idAEliminar) {
        prev = temp;
        temp = temp->siguiente;
        //idx++;
    }
    
    if (temp) {
        prev->siguiente = temp->siguiente;
        delete temp;
        return true;
    }
    return false;
}

template <typename T>
int contarLista(Nodo<T> * lista) {
    int cuenta = 0;
    Nodo<T> * temp = lista;
    while (temp) {
        cuenta++;
        temp = temp->siguiente;
    }
    return cuenta;
}

//Esa funcion recorre la lista y devuelve el mayor de los ID
template <typename T>
int mayorID(Nodo<T> * lista) {
    int mayorID = 0;
    Nodo<T> * temp = lista;
    while (temp) {
        if (temp->idNodo > mayorID){
            mayorID = temp->idNodo;
        }
        temp = temp->siguiente;
    }
    return mayorID;
}

template <typename T>
T* obtenerPorPosicion(Nodo<T> * lista, int pos) {
    if (pos < 1 || !lista) return nullptr;
    
    Nodo<T> * temp = lista;
    for (int i = 1; i < pos && temp; i++) {
        temp = temp->siguiente;
    }
    
    return temp ? &(temp->dato) : nullptr;
}

template <typename T>
T* obtenerPorID(Nodo<T> * lista, int idSolicitado) {
    if (idSolicitado < 1 || !lista) return nullptr;
    Nodo<T> * temp = lista;
    while (temp) {
        if (temp->idNodo == idSolicitado){
            return temp ? &(temp->dato) : nullptr;
        }
        temp = temp->siguiente;
    }
    return temp ? &(temp->dato) : nullptr;
}

template <typename T>
void liberarLista(Nodo<T> * &lista) {
    while (lista) {
        Nodo<T> * temp = lista;
        lista = lista->siguiente;
        delete temp;
    }
}

template <typename T>
Nodo<T> * obtenerApuntadorANodoPorID(Nodo<T> * lista, int idSolicitado) {
    if (idSolicitado == 0 || !lista) return nullptr;
    Nodo<T> * temp = lista;
    while (temp) {
        cout << "temp->idNodo: " << temp->idNodo << "  idSolicitado: " << idSolicitado << "\n";
        if (temp->idNodo == idSolicitado){
            cout << "Estoy en obtenerApuntadorANodoPorID, encontre el ID solicitado: " << idSolicitado << "\n";
            pausar();
            return temp ? temp : nullptr;
        }
        temp = temp->siguiente;
    }
    //return temp ? &(temp->dato) : nullptr;
    return temp ? temp : nullptr;
}

template <typename T>
Nodo<T> * obtenerApuntadorANodoPorNombre(Nodo<T> * lista, string nombreSolicitado) {
    if (nombreSolicitado.empty() || !lista) return nullptr;
    Nodo<T> * temp = lista;
    while (temp) {
        if (temp->dato.nombre == nombreSolicitado){
            pausar();
            return temp ? temp : nullptr;
        }
        temp = temp->siguiente;
    }
    //return temp ? &(temp->dato) : nullptr;
    return temp ? temp : nullptr;
}


/******************************/
/* FUNCIONES DE MOSTRAR DATOS */
/******************************/

void mostrarEfecto(TipoEfecto efecto) {
    switch (efecto) {
        case TipoEfecto::VENENO: std::cout << "Veneno"; break;
        case TipoEfecto::QUEMADURA: std::cout << "Quemadura"; break;
        case TipoEfecto::CONGELACION: std::cout << "Congelacion"; break;
        case TipoEfecto::BENDICION: std::cout << "Bendicion"; break;
        case TipoEfecto::MALDICION: std::cout << "Maldicion"; break;
        case TipoEfecto::REGENERACION: std::cout << "Regeneracion"; break;
        default: std::cout << "Ninguno"; break;
    }
}

void mostrarEspecies(bool mostrarDetalles = false) {
    if (!listaEspecies) {
        std::cout << "No hay especies registradas.\n";
        return;
    }
    
    std::cout << "\n=== LISTADO DE ESPECIES ===\n";
    if (mostrarDetalles) {
        std::cout << string(120, '-') << "\n";
        std::cout << " ID | Nombre           | Fortaleza | Danio | Salud | Rapidez | Res.Mag | Descripcion\n";
        std::cout << string(120, '-') << "\n";
    } else {
        std::cout << string(70, '-') << "\n";
        std::cout << " ID | Nombre           | Fortaleza | Danio | Salud | Rapidez\n";
        std::cout << string(70, '-') << "\n";
    }
    
    Nodo<Especie> * actual = listaEspecies;
    int idEspecie = 0;
    while (actual) {
        idEspecie = actual->idNodo;
        if (idEspecie < 10){
            std::cout << " ";            
        }
        std::cout << " " << idEspecie << " | " << actual->dato.nombre;
        std::cout << string(16 - actual->dato.nombre.size(), ' ') << " | ";
        std::cout << actual->dato.fortalezaMax << string(9 - to_string(actual->dato.fortalezaMax).size(), ' ') << " | ";
        std::cout << actual->dato.danoBase << string(5 - to_string(actual->dato.danoBase).size(), ' ') << " | ";
        std::cout << actual->dato.saludMax << string(5 - to_string(actual->dato.saludMax).size(), ' ') << " | ";
        std::cout << actual->dato.rapidez;
        
        if (mostrarDetalles) {
            std::cout << string(7 - to_string(actual->dato.rapidez).size(), ' ') << " | ";
            std::cout << actual->dato.resistenciaMagica << string(7 - to_string(actual->dato.resistenciaMagica).size(), ' ') << " | ";
            std::cout << (actual->dato.descripcion.size() > 48 ? actual->dato.descripcion.substr(0, 45) + "..." : actual->dato.descripcion);
        }
        
        std::cout << "\n";
        actual = actual->siguiente;
    }
    std::cout << "\n";
}

void mostrarImplementos(Nodo<Implemento> * lista, bool mostrarDetalles = false) {
    if (!lista) {
        std::cout << "No hay implementos disponibles.\n";
        return;
    }
    std::cout << "\n=== LISTADO DE IMPLEMENTOS ===\n";
    if (mostrarDetalles) {
        std::cout << string(120, '-') << "\n";
        std::cout << " ID | Nombre                 | Tipo            | Fort.Req | Valor | Usos | Efectos          | Descripcion\n";
        std::cout << string(120, '-') << "\n";
    } else {
        std::cout << " ID | Nombre                 | Tipo            | Fort.Req | Valor | Usos \n";
        std::cout << string(74, '-') << "\n";
    }
    Nodo<Implemento> * actual = lista;
    while (actual) {
        if (actual->idNodo < 10){
            cout << " ";
        }
        std::cout << " " << actual->idNodo << " | " << actual->dato.nombre;
        std::cout << string(22 - actual->dato.nombre.size(), ' ') << " | ";
        std::cout << actual->dato.tipo;
        std::cout << string(15 - actual->dato.tipo.size(), ' ') << " | ";
        std::cout << actual->dato.fortalezaNecesaria;
        std::cout << string(8 - to_string(actual->dato.fortalezaNecesaria).size(), ' ') << " | ";
        std::cout << actual->dato.valor;
        std::cout << string(5 - to_string(actual->dato.valor).size(), ' ') << " | ";
        std::cout << actual->dato.usos;
        std::cout << string(4 - to_string(actual->dato.usos).size(), ' ') << " | ";
        if (mostrarDetalles) {
            //std::cout << string(5 - to_string(actual->dato.valor).size(), ' ') << " | ";
            if (actual->dato.efectos.empty()) {
                std::cout << "Ninguno";
            } else {
                for (size_t i = 0; i < actual->dato.efectos.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    mostrarEfecto(actual->dato.efectos[i].tipo);
                    std::cout << "(" << actual->dato.efectos[i].duracion << ")";
                }
            }
            std::cout << string(16 - 7, ' ') << " | "; // Aproximacion para alineacion
            std::cout << (actual->dato.descripcion.size() > 30 ? actual->dato.descripcion.substr(0, 27) + "..." : actual->dato.descripcion);
        }
        std::cout << "\n";
        actual = actual->siguiente;
    }
    std::cout << "\n";
}


void mostrarPersonajeDetallado(const Personaje& p) {
    std::cout << "\n=== FICHA DE PERSONAJE ===\n";
    std::cout << "Nombre: " << p.nombre << "\n";
    std::cout << "Especie: " << p.especie.nombre << " (Nivel " << p.nivel << ")\n";
    std::cout << "Salud: " << p.salud << "/" << p.saludMax << "\n";
    std::cout << "Fortaleza: " << p.fortaleza << "/" << p.fortalezaMax << "\n";
    std::cout << "Rapidez: " << p.rapidez << "\n";
    std::cout << "Experiencia: " << p.experiencia << "/" << (EXPERIENCIA_BASE * p.nivel) << "\n";
    std::cout << "Estado: " << (p.vivo ? "Vivo" : "Muerto") << "\n";
    
    // Mostrar efectos activos
    if (!p.efectos.empty()) {
        std::cout << "Efectos activos: ";
        for (size_t i = 0; i < p.efectos.size(); ++i) {
            if (i > 0) std::cout << ", ";
            mostrarEfecto(p.efectos[i].tipo);
            std::cout << "(" << p.efectos[i].duracion << ")";
        }
        std::cout << "\n";
    }
    
    // Mostrar estadisticas
    std::cout << "\nEstadisticas:\n";
    std::cout << "Enemigos derrotados: " << p.kills << "\n";
    std::cout << "Veces muerto: " << p.muertes << "\n";
    std::cout << "Daño total causado: " << p.danioTotal << "\n";
    std::cout << "Curacion total recibida: " << p.curacionTotal << "\n";
    
    // Mostrar mochila
    std::cout << "\nMochila (" << contarLista(p.mochila) << "/" << MAX_IMPLEMENTOS << "):\n";
    if (p.mochila) {
        mostrarImplementos(p.mochila);
    } else {
        std::cout << "Vacia\n";
    }
}


void mostrarPersonajes(bool resumido = true) {
    if (!listaHeroes) {
        std::cout << "No hay personajes creados.\n";
        return;
    }
    
    if (resumido) {
        std::cout << "\n=== LISTADO DE PERSONAJES ===\n";
        std::cout << " ID | Nombre        | Especie       | Salud      | Fort.    | Rapidez | Nivel | Estado\n";
        std::cout << string(90, '-') << "\n";
        
        Nodo<Personaje> * actual = listaHeroes;
        while (actual) {
            if (actual->idNodo < 10) {
                std::cout << " ";
            }
            std::cout << " " << actual->idNodo << " | " << actual->dato.nombre;
            std::cout << string(13 - actual->dato.nombre.size(), ' ') << " | ";
            std::cout << actual->dato.especie.nombre;
            std::cout << string(13 - actual->dato.especie.nombre.size(), ' ') << " | ";
            std::cout << actual->dato.salud << "/" << actual->dato.saludMax;
            std::cout << string(10 - (to_string(actual->dato.salud).size() + to_string(actual->dato.saludMax).size() + 1), ' ') << " | ";
            std::cout << actual->dato.fortaleza << "/" << actual->dato.fortalezaMax;
            std::cout << string(8 - (to_string(actual->dato.fortaleza).size() + to_string(actual->dato.fortalezaMax).size() + 1), ' ') << " | ";
            std::cout << actual->dato.rapidez;
            std::cout << string(7 - to_string(actual->dato.rapidez).size(), ' ') << " | ";
            std::cout << actual->dato.nivel;
            std::cout << string(5 - to_string(actual->dato.nivel).size(), ' ') << " | ";
            std::cout << (actual->dato.vivo ? "Vivo" : "Muerto");
            std::cout << "\n";
            actual = actual->siguiente;
        }
        std::cout << string(90, '-') << "\n" << "\n";
    } else {
        Nodo<Personaje> * actual = listaHeroes;
        while (actual) {
            std::cout << "=== PERSONAJE #" << actual->idNodo << " ===\n";
            mostrarPersonajeDetallado(actual->dato);
            std::cout << "\n";
            actual = actual->siguiente;
        }
    }
}


void mostrarSalas() {
    if (!mapaGlobal.salas) {
        std::cout << "No hay salas en el mapa.\n";
        return;
    }
    
    std::cout << "\n === MAPA ACTUAL ===\n";
    std::cout << "Sala actual: " << (mapaGlobal.salaActual ? mapaGlobal.salaActual->nombre : "Ninguna") << "\n";
    std::cout << "   # | Nombre Sala                              | Orkos | Puerta | Explorada | Adyacentes\n";
    std::cout << string(88, '-') << "\n";
    
    Nodo<Sala> * temp = mapaGlobal.salas;
    int idx = 1;
    while (temp) {
        if (idx < 10){
            std::cout << " ";
        }
        std::cout << "  " << temp->dato.idSala << " | " << temp->dato.nombre;
        std::cout << string(40 - temp->dato.nombre.size(), ' ') << " | ";
        std::cout << temp->dato.cantidadOrcos;
        std::cout << string(5 - to_string(temp->dato.cantidadOrcos).size(), ' ') << " | ";
        std::cout << (temp->dato.tienePuertaDestino ? "Si " : "No ");
        std::cout << "    | ";
        std::cout << (temp->dato.explorada ? "Si      " : "No      ");
        std::cout << "  | ";
        
        // Mostrar adyacentes
        Nodo<Sala*> * ady = temp->dato.adyacentes;
        if (!ady) {
            std::cout << "Ninguna";
        } else {
            while (ady) {
                if (ady != temp->dato.adyacentes) std::cout << ", ";
                std::cout << ady->dato->nombre;
                ady = ady->siguiente;
            }
        }
        
        std::cout << "\n";
        temp = temp->siguiente;
        idx++;
    }
}


/******************************/
/* FUNCIONES DE SELECcion     */
/******************************/

Especie* seleccionarEspecie() {
    if (!listaEspecies) {
        std::cout << "No hay especies existentes.\n";
        return nullptr;
    }
    
    mostrarEspecies();
    int especieSeleccionada = leerEntero("Seleccione especie (0 para cancelar): ", 0, mayorID(listaEspecies));
    if (especieSeleccionada == 0) return nullptr;
    
    return obtenerPorID(listaEspecies, especieSeleccionada);
}

// Dado un nombre de Especie, esta funcion lo busca en la lista de especies y devuelve su apuntador
Especie* encontrarEspeciePorNombre(string nombreABuscar) {
    if (!listaEspecies) {
        std::cout << "No hay especies existentes.\n";
        return nullptr;
    }
    // buscar la especie por su nombre en la lista de especies existentes
    Nodo<Especie>* temp = listaEspecies;
    while (temp) {
        if (temp->dato.nombre == nombreABuscar) {
            //Encontre la especie que estaba buscando
            return temp ? &(temp->dato) : nullptr;
        }
        temp = temp->siguiente;
    }
    return nullptr;
 }

Implemento* seleccionarImplemento(Nodo<Implemento> * lista) {
    if (!lista) {
        std::cout << "No hay implementos disponibles.\n";
        return nullptr;
    }
    
    mostrarImplementos(lista);
    int implementoSeleccionado = leerEntero("Seleccione implemento (0 para cancelar): ", 0, mayorID(lista));
    if (implementoSeleccionado == 0) return nullptr;
    
    return obtenerPorID(lista, implementoSeleccionado);
}

Personaje* seleccionarPersonaje(bool soloVivos = false) {
    if (!listaHeroes) {
        std::cout << "No hay personajes creados.\n";
        return nullptr;
    }
    
    // Contar personajes disponibles
    int disponibles = 0;
    Nodo<Personaje> * temp = listaHeroes;
    while (temp) {
        if (!soloVivos || temp->dato.vivo) disponibles++;
        temp = temp->siguiente;
    }
    
    if (disponibles == 0) {
        std::cout << (soloVivos ? "No hay personajes vivos." : "No hay personajes.") << "\n";
        return nullptr;
    }
    
    mostrarPersonajes();
    int choice;
    do {
        //choice = leerEntero("Seleccione personaje (0 para cancelar): ", 0, contarLista(listaHeroes));
        choice = leerEntero("Seleccione personaje (0 para cancelar): ", 0, mayorID(listaHeroes));
        if (choice == 0) return nullptr;
        
        //Personaje* p = obtenerPorPosicion(listaHeroes, choice);
        Personaje* p = obtenerPorID(listaHeroes, choice);
        if (!soloVivos || (p && p->vivo)) return p;
        
        std::cout << "Ese personaje no esta vivo. Seleccione otro.\n";
    } while (true);
}

Sala* seleccionarSala(bool excluirActual = false) {
    if (!mapaGlobal.salas) {
        std::cout << "No hay salas en el mapa.\n";
        return nullptr;
    }
    
    mostrarSalas();
    int choice;
    do {
        choice = leerEntero("Seleccione sala (0 para cancelar): ", 0, contarLista(mapaGlobal.salas));
        if (choice == 0) return nullptr;
        
        Sala* s = obtenerPorPosicion(mapaGlobal.salas, choice);
        if (!excluirActual || (s && s != mapaGlobal.salaActual)) return s;
        
        std::cout << "No puede seleccionar la sala actual. Elija otra.\n";
    } while (true);
}

/*****************************************************************************************************************/
/*                                 FUNCIONES DE LECTURA Y ALMACENADO EN ARCHIVOS                                 */
/*****************************************************************************************************************/

void cargarEspeciesDesdeArchivo(){
    fstream archivoDatos; 
    string nombreArchivo;
    nombreArchivo = "especies.txt";  
    cout << "Cargando Especies desde archivo: " << nombreArchivo << "\n";
    archivoDatos.open(nombreArchivo);
    if (archivoDatos.is_open()){
        archivoDatos >> numRegistros;
        if (numRegistros > 0){
            string linea;
            string dato;
            int idLeido = 0;
            string nombreLeido;
            int fortalezaLeida = 0;
            int danoLeido = 0;
            int saludLeida = 0;
            int rapidezLeida = 0;
            int resistenciaMagicaLeida = 0;
            string descripcionLeida;
            int posicionPrimerEspacio = 0;
            int i = 0;
            std::getline(archivoDatos, linea);
            std::getline(archivoDatos, linea);
            while (std::getline(archivoDatos, linea)){
                posicionPrimerEspacio = linea.find(" ");
                dato = linea.substr(0, posicionPrimerEspacio);
                if (dato == "---"){
                    i = 0;
                } else {
                    switch(i) {
                        case 0:
                            idLeido = stoi(dato);
                            break;
                        case 1:
                            nombreLeido = dato;
                            break;
                        case 2:
                            if (dato == "-"){
                                dato = "0";
                            }    
                            fortalezaLeida = stoi(dato);
                            break;
                        case 3:
                            if (dato == "-"){
                                dato = "0";
                            }    
                            danoLeido = stoi(dato);
                            break;
                        case 4:
                            if (dato == "-"){
                                dato = "0";
                            }    
                            saludLeida = stoi(dato);
                            break;
                        case 5:
                            if (dato == "-"){
                                dato = "0";
                            }    
                            rapidezLeida = stoi(dato);
                            break;
                        case 6:
                            if (dato == "-"){
                                dato = "0";
                            }    
                            resistenciaMagicaLeida = stoi(dato);
                            break;
                        case 7:
                            descripcionLeida = linea;
                            // insertar la especie en la lista
                            Especie especieLeida = {nombreLeido, fortalezaLeida, danoLeido, saludLeida, rapidezLeida, resistenciaMagicaLeida, descripcionLeida};
                            agregarOrdenadamente(listaEspecies, especieLeida, idLeido);
                            break;
                    }
                    i++;
                }
            }
        }
    }     else     {
        std::cout << "No se pudo abrir el archivo " << nombreArchivo << "\n";
        return;    
    }
}

void cargarImplementosDesdeArchivo(){
    fstream archivoDatos; 
    string nombreArchivo;
    nombreArchivo = "implementos.txt";  
    cout << "Cargando Implementos desde archivo: " << nombreArchivo << "\n";
    archivoDatos.open(nombreArchivo);
    if (archivoDatos.is_open()){
        archivoDatos >> numRegistros;
        if (numRegistros > 0){
            string linea;
            string dato;
            int idLeido;
            string nombreLeido;
            string tipoLeido;
            int fortalezaLeida = 0;
            int valorLeido = 0;
            int usosRestantesLeido = 0;
            int posicionPrimerEspacio = 0;
            int i = 0;
            std::getline(archivoDatos, linea);
            std::getline(archivoDatos, linea);
            while (std::getline(archivoDatos, linea)){
                posicionPrimerEspacio = linea.find(" ");
                dato = linea.substr(0, posicionPrimerEspacio);
                if (dato == "---"){
                    i = 0;
                } else {
                    switch(i) {
                        case 0:
                            idLeido = stoi(dato);
                            break;
                        case 1:
                            nombreLeido = linea;
                            break;
                        case 2:
                            if (dato == "-"){
                                dato = "0";
                            }
                            tipoLeido = dato;
                            break;
                        case 3:
                            fortalezaLeida = stoi(dato);
                            break;
                        case 4:
                            valorLeido = stoi(dato);
                            break;
                        case 5:
                            usosRestantesLeido = stoi(dato);
                            // Ahora voy a crear el Implemento y a guardarlo en la lista
                            Implemento implementoLeido = {nombreLeido, tipoLeido, fortalezaLeida, valorLeido, usosRestantesLeido};
                            agregarOrdenadamente(listaImplementos, implementoLeido, idLeido);
                            break;
                    }
                    i++;
                }
            }
        }
    }     else     {
        std::cout << "No se pudo abrir el archivo " << nombreArchivo << "\n";
        return;    
    }
}

/********************************************************************************************************************************************************************************  
    El primer parametro es un int que contiene el ID de la sala a la cual se le van a agregar las adyacencias y el segundo parametro es un string que contiene las adyacencias 
    tomadas del archivo .txt, es decir contiene los IDs de las salas que son adyacentes a la que viene en el int. 
    Se van tomando los IDs de las adyacencias del string, se buscan en la lista mapaGlobal que es la contiene las Salas, se toman los apuntadores a esas Salas y se mandan en
    la funcion agregarOrdenadamente para que inserte la adyacencia en la lista ed Salas adyacentes de la Sala a modificar
*********************************************************************************************************************************************************************************/

 void agregarAdyacencias(int idSalaACargarAdyacentes, string lineaDeAdyacencias){
    int adyacenciaTomada = 0;
    string restoDeLaLinea = lineaDeAdyacencias;
    int posicionDosPuntos = 0;
    int posicionSeparador = 0;
    while (!restoDeLaLinea.empty() && posicionSeparador > -1){
        posicionDosPuntos = restoDeLaLinea.find(":");
        adyacenciaTomada = stoi(restoDeLaLinea.substr(0, posicionDosPuntos));
        posicionSeparador = restoDeLaLinea.find("|");
        restoDeLaLinea.erase(0,posicionSeparador+1);
        Sala * salaACargarAdyacentes = obtenerPorID(mapaGlobal.salas, idSalaACargarAdyacentes);
        Sala * salaAdyacente = obtenerPorID(mapaGlobal.salas, adyacenciaTomada);
        agregarOrdenadamente(salaACargarAdyacentes->adyacentes, salaAdyacente, adyacenciaTomada);
    }
}


void cargarSalasDesdeArchivo(){
    fstream archivoDatos; 
    string nombreArchivo;
    nombreArchivo = "salas.txt";  
    cout << "Cargando Salas desde archivo: " << nombreArchivo << "\n";
    archivoDatos.open(nombreArchivo);
    if (archivoDatos.is_open()){
        archivoDatos >> numRegistros;
        if (numRegistros > 0){
            string linea;
            string dato;
            int idLeido = 0;
            string nombreLeido;
            string adyacenciasLeidas;
            int primeraAdyacenciaLeida = 0;
            int posicionPrimerEspacio = 0;
            int posicionPrimerDosPuntos = 0;
            int i = 0;
            std::getline(archivoDatos, linea);
            std::getline(archivoDatos, linea);
            while (std::getline(archivoDatos, linea)){
                posicionPrimerEspacio = linea.find(" ");
                dato = linea.substr(0, posicionPrimerEspacio);
                if (dato == "---"){
                    i = 0;
                } else {
                    switch(i) {
                        case 0:
                            idLeido = stoi(dato);
                            break;
                        case 1:
                            nombreLeido = linea;
                            break;
                        case 2:
                            break;
                        case 3:
                            adyacenciasLeidas = linea;
                            posicionPrimerDosPuntos = dato.find(":");
                            primeraAdyacenciaLeida = stoi(dato.substr(0, posicionPrimerDosPuntos));
                            Sala nuevaSala = {idLeido, nombreLeido, adyacenciasLeidas, nullptr, 0, false, false, nullptr};
                            agregarOrdenadamente(mapaGlobal.salas, nuevaSala, idLeido);
                            break;
                    }
                    i++;
                }
            }
            /* Ahora debo recorrer nuevamente el archivo para poder cargar las adyacencias, estas no se pueden cargar en el primer recorrido porque algunas no existen todavia,
            por ejemplo, cuando cargo la sala con el ID 1, esta tiene como adyacencias las salas 2, 3 y 60, estas salas todavia no se han cargado.
            Para hacer esto voy a cerrar el archivo y abrirlo nuevamente, luego lo voy a recorrer con otro While.           */

            archivoDatos.close();
            archivoDatos.open(nombreArchivo);
            if (archivoDatos.is_open()){
                i = 0;
                std::getline(archivoDatos, linea);
                std::getline(archivoDatos, linea);
                while (std::getline(archivoDatos, linea)){
                    posicionPrimerEspacio = linea.find(" ");
                    dato = linea.substr(0, posicionPrimerEspacio);
                    if (dato == "---"){
                        i = 0;
                    } else {
                        switch (i){
                            case 0:
                                idLeido = stoi(dato);
                                break;
                            case 1:
                                nombreLeido = linea;
                                break;
                            case 2:
                                break;
                            case 3:
                                adyacenciasLeidas = linea;
                                posicionPrimerDosPuntos = dato.find(":");
                                primeraAdyacenciaLeida = stoi(dato.substr(0, posicionPrimerDosPuntos));
                                Sala* nuevaSala = obtenerPorID(mapaGlobal.salas, idLeido);
                                agregarAdyacencias(idLeido, linea);
                                break;
                        }
                        i++;
                    }
                }
            } else {
                std::cout << "No se pudo abrir el archivo la segunda vez" << nombreArchivo << "\n";
                return;
            }
        }
    } else {
        std::cout << "No se pudo abrir el archivo " << nombreArchivo << "\n";
        return;    
    }
}

/****************************************************************************************************************************************
 * NO SE ESTA GUARDANDO LA DESCRIPCION, LOS ARCHIVOS ORIGINALES COMPARTIDOS POR EL PROFESOR NO CONTIENEN DESCRIPCION DE LOS IMPLEMENTOS *
****************************************************************************************************************************************/
void guardarImplementosEnArchivo(){
    // Primero borro el archivo actual y luego creo el nuevo con el mismo nombre
    const char* archivoABorrar = "implementos.txt";
    int archivoBorrado = remove(archivoABorrar);
    //Necesito saber la cantidad de registros para colocarla en la primera linea
    int cantidadRegistros = contarLista(listaImplementos);
    // Procedo a crear el nuevo archivo
    const char* archivoNuevo = "implementos.txt";
    std::ofstream archivoDatos(archivoNuevo);
    if (archivoDatos.is_open()){
        // Escribo el numero de registros en el archivo. Cuando elimino todos los archivos queda en cero y debo guardarlo porque si no hago nada quedaria el valor anterior en el archivo
        archivoDatos << cantidadRegistros << "\n";
        // Si hay registros debo guardarlos en el archivo
        if (cantidadRegistros > 0){
            string linea;
            string dato;
            string nombreAEscribir;
            string tipoAGuardar;
            int fortalezaAGuardar = 0;
            int valorAGuardar = 0;
            int usosRestantesAGuardar = 0;
            int i = 0;
            Nodo<Implemento> * actual = listaImplementos;
            while (actual) {
                //DEBO TOMAR LOS VALORES DE LA LISTA DE IMPLEMENTOS Y GUARDARLOSEN EL ARCHIVO, UNO EN CADA LINEA CON SUS SEPARADORES
                archivoDatos << "---\n";
                archivoDatos << actual->idNodo <<"\n";
                archivoDatos << actual->dato.nombre <<"\n";
                archivoDatos << actual->dato.tipo <<"\n";
                archivoDatos << actual->dato.fortalezaNecesaria <<"\n";
                archivoDatos << actual->dato.valor <<"\n";
                archivoDatos << actual->dato.usos <<"\n";
                actual = actual->siguiente;
            }
        } else {
            // Como no hay registros que guardar, guardo el archivo con un 0 en la primera linea y mas nada
            std::cout << "No hay implementos que guardar \n";
            pausar();
        }
        cout << "Archivo: " << archivoABorrar << " creado";
        archivoDatos.close();
        return;    
    } else {
    std::cout << "No se pudo abrir el archivo " << archivoNuevo << "\n";
    pausar();
    return;    
    }
}


void guardarEspeciesEnArchivo(){
    // Primero borro el archivo actual y luego creo el nuevo con el mismo nombre
    const char* archivoABorrar = "especies.txt";
    int archivoBorrado = remove(archivoABorrar);
    //Necesito saber la cantidad de registros para colocarla en la primera linea del archivo
    int cantidadRegistros = contarLista(listaEspecies);
    // Procedo a crear el nuevo archivo
    const char* archivoNuevo = "especies.txt";
    std::ofstream archivoDestino(archivoNuevo);
    if (archivoDestino.is_open()){
        // Escribo el numero de registros en el archivo. Cuando elimino todos los archivos queda en cero y debo guardarlo porque si no hago nada quedaria el valor anterior en el archivo
        archivoDestino << cantidadRegistros << "\n";
        // Si hay registros debo guardarlos en el archivo
        if (cantidadRegistros > 0){
            Nodo<Especie> * actual = listaEspecies;
            while (actual) {
                //DEBO TOMAR LOS VALORES DE LA LISTA DE IMPLEMENTOS Y GUARDARLOSEN EL ARCHIVO, UNO EN CADA LINEA CON SUS SEPARADORES
                archivoDestino << "---\n";
                archivoDestino << actual->idNodo <<"\n";
                archivoDestino << actual->dato.nombre <<"\n";
                archivoDestino << actual->dato.fortalezaMax <<"\n";
                archivoDestino << actual->dato.danoBase <<"\n";
                archivoDestino << actual->dato.saludMax <<"\n";
                archivoDestino << actual->dato.rapidez <<"\n";
                archivoDestino << actual->dato.resistenciaMagica <<"\n";
                archivoDestino << actual->dato.descripcion <<"\n";
                actual = actual->siguiente;
            }
        } else {
            // Como no hay registros que guardar, guardo el archivo con un 0 en la primera linea y mas nada
            std::cout << "No hay implementos que guardar \n";
            pausar();
        }
        std::cout << "Archivo: " << archivoNuevo << " creado";
        archivoDestino.close();
        return;    
    } else {
        std::cout << "No se pudo abrir el archivo " << archivoNuevo << "\n";
        pausar();
        return;    
    }
}


void cargarPersonajesDesdeArchivo(){
    if (!listaEspecies) {
        std::cout << "No hay especies disponibles para crear personajes.\n";
        pausar();
        return;
    }
    fstream archivoDatos; 
    string nombreArchivo = "personajes.txt";
    archivoDatos.open(nombreArchivo);
    cout << "Cargando Personajes desde archivo: " << nombreArchivo << "\n";
    if (archivoDatos.is_open()){
        archivoDatos >> numRegistros;
        if (numRegistros > 0){
            string linea;
            string dato;
            int idLeido;
            string especieLeida;
            string nombreLeido;
            int posicionPrimerEspacio = 0;
            int i = 0;
            getline(archivoDatos, linea);
            getline(archivoDatos, linea);
            while (getline(archivoDatos, linea)){
                posicionPrimerEspacio = linea.find(" ");
                dato = linea.substr(0, posicionPrimerEspacio);
                if (dato == "---"){
                    i = 0;
                } else {
                    switch(i) {
                        case 0:
                            idLeido = stoi(dato);
                            break;
                        case 1:
                            especieLeida = dato;
                            break;
                        case 2:
                            nombreLeido = dato;
                            // Verificamos que no exista el ID, si ya existe no podemos crear el personaje
                            Nodo<Personaje>* temp = listaHeroes;
                            while (temp) {
                                if (temp->idNodo == idLeido) {
                                    std::cout << "Ya existe un personaje con el ID: " << idLeido << "\n";
                                    break;
                                }
                                temp = temp->siguiente;
                            }
                            // Verificamos que no exista el nombre, si ya existe no podemos crear el personaje
                            while (temp) {
                                if (temp->dato.nombre == nombreLeido) {
                                    std::cout << "Ya existe un personaje con el nombre: " << nombreLeido << "\n";
                                    break;
                                }
                                temp = temp->siguiente;
                            }
                            // Verificamos que exista la especie, si no existe no podemos crear el personaje
                            Especie* especie = encontrarEspeciePorNombre(especieLeida);
                            if (!especie) {
                                std::cout << "Creacion del personaje '" << nombreLeido << "' cancelada, No existe la especie '" << especieLeida << "'.\n";
                                break;
                            }
                            Personaje nuevoPersonaje;
                            nuevoPersonaje.nombre = nombreLeido;
                            nuevoPersonaje.especie = *especie;
                            nuevoPersonaje.saludMax = especie->saludMax;
                            nuevoPersonaje.salud = nuevoPersonaje.saludMax;
                            nuevoPersonaje.fortalezaMax = especie->fortalezaMax;
                            nuevoPersonaje.fortaleza = nuevoPersonaje.fortalezaMax;
                            nuevoPersonaje.rapidez = especie->rapidez;
                            nuevoPersonaje.experiencia = 0;
                            nuevoPersonaje.nivel = 1;
                            nuevoPersonaje.vivo = true;
                            nuevoPersonaje.efectos.clear();
                            nuevoPersonaje.mochila = nullptr;
                            nuevoPersonaje.muertes = 0;
                            nuevoPersonaje.kills = 0;
                            nuevoPersonaje.danioTotal = 0;
                            nuevoPersonaje.curacionTotal = 0;
                            agregarOrdenadamente(listaHeroes, nuevoPersonaje, idLeido);
                            break;
                    }
                    i++;
                }
            }
        }
    }     else     {
        std::cout << "No se pudo abrir el archivo de Personajes \n";
        return;    
    }
    mostrarPersonajes(true);
}

/******************************/
/* FUNCIONES DE GESTIoN       */
/******************************/

void inicializarDatosBase() {
    
    // Solo inicializar si no hay datos
    if (listaEspecies) return;
    cargarEspeciesDesdeArchivo();
    if (!listaImplementos) cargarImplementosDesdeArchivo();
    if (!mapaGlobal.salas) cargarSalasDesdeArchivo();
    if (!listaHeroes) cargarPersonajesDesdeArchivo();
    pausar();

    // Crear eventos base
    eventosDisponibles = {
        {"Fuente curativa", "Una fuente magica emana energias sanadoras.", 
         [](Personaje* p) { 
             int curacion = p->saludMax * 0.3;
             p->salud = min(p->salud + curacion, p->saludMax);
             std::cout << p->nombre << " se cura " << curacion << " puntos de salud por la fuente.\n";
         }, true},
         
        {"Trampa de pinchos", "Cuidado con el suelo!", 
         [](Personaje* p) { 
             int danio = p->saludMax * 0.2;
             p->salud = max(1, p->salud - danio);
             std::cout << p->nombre << " sufre " << danio << " puntos de danio por la trampa.\n";
         }, false},
         
        {"Altar misterioso", "Un antiguo altar emana una energia desconocida.", 
         [](Personaje* p) { 
             if (aleatorioEnRango(1, 100) > 50) {
                 p->efectos.push_back({TipoEfecto::BENDICION, 3, 10});
                 std::cout << p->nombre << " recibe una bendicion del altar!\n";
             } else {
                 p->efectos.push_back({TipoEfecto::MALDICION, 3, 5});
                 std::cout << p->nombre << " es maldecido por el altar!\n";
             }
         }, false},
         
        {"Cofre del tesoro", "Un cofre antiguo espera ser abierto.", 
         [](Personaje* p) { 
             if (contarLista(p->mochila) < MAX_IMPLEMENTOS) {
                 int idx = aleatorioEnRango(1, contarLista(listaImplementos));
                 Implemento* imp = obtenerPorPosicion(listaImplementos, idx);
                 if (imp) {
 //   ********************************                   agregarAlFinal(p->mochila, *imp);
                    std::cout << p->nombre << " obtiene un " << imp->nombre << " del cofre!\n";
                 }
             } else {
                 std::cout << p->nombre << " encuentra un tesoro pero no tiene espacio en la mochila.\n";
             }
         }, true}
    };
}

void agregarEspecie() {
    
    int idLeido = leerEntero("Ingrese el ID de la especie: ",1 , 100);
    // Verificar unicidad del ID
    Nodo<Especie> * temp = listaEspecies;
    while (temp) {
        if (temp->idNodo == idLeido) {
            std::cout << "Ya existe una especie con ese ID.\n";
            pausar();
            return;
        }
        temp = temp->siguiente;
    }
    string nombreLeido = leerCadena("Ingrese nombre de la especie: ");
    // Verificar unicidad del nombre
    while (temp) {
        if (temp->dato.nombre == nombreLeido) {
            std::cout << "Ya existe una especie con ese nombre.\n";
            pausar();
            return;
        }
        temp = temp->siguiente;
    }
    int fortalezaLeida = leerEntero("Ingrese Fortaleza Maxima (1-1000): ", 1, 1000);
    int danoLeido = leerEntero("Ingrese Danio base (0-500): ", 0, 500);
    int saludLeida = leerEntero("Ingrese Salud maxima (1-10000): ", 1, 10000);
    int rapidezLeida = leerEntero("Ingrese Rapidez (1-100): ", 1, 100);
    int resistenciaMagicaLeida = leerEntero("Ingrese Resistencia Magica (0-100): ", 0, 100);
    string descripcionLeida = leerCadena("Ingrese descripcion: ", false);
    
    Especie especieNueva = {nombreLeido, fortalezaLeida, danoLeido, saludLeida, rapidezLeida, resistenciaMagicaLeida, descripcionLeida};
    agregarOrdenadamente(listaEspecies, especieNueva, idLeido);
    std::cout << "Especie '" << nombreLeido << "' creada correctamente.\n";
    pausar();
}

void modificarEspecie() {
    if (!listaEspecies) {
        std::cout << "No hay especies para modificar.\n";
        pausar();
        return;
    }
    mostrarEspecies();
    int idAModificar = leerEntero("Seleccione especie la modificar (0 para cancelar): ", 0, mayorID(listaEspecies));
    if (idAModificar == 0) return;
    
    Especie* especieAModificar = obtenerPorID(listaEspecies, idAModificar);
    if (!especieAModificar) {
        std::cout << "Error al seleccionar especie.\n";
        pausar();
        return;
    }
    std::cout << "\nModificando especie: " << especieAModificar->nombre << "\n";
    std::cout << "(Deje en blanco para mantener el valor actual)\n";
    string nuevoNombre = leerCadena("Nuevo nombre [" + especieAModificar->nombre + "]: ");
    if (!nuevoNombre.empty()) {
        // Verificar que el nuevo nombre no exista

        // REVISAR
        Nodo<Especie> * temp = listaEspecies;
        while (temp) {
            if (temp->dato.nombre == nuevoNombre && &(temp->dato) != especieAModificar) {
                std::cout << "Ya existe otra especie con ese nombre.\n";
                pausar();
                return;
            }
            temp = temp->siguiente;
        }
        especieAModificar->nombre = nuevoNombre;
    }
    string input;
    std::cout << "Nueva fortaleza maxima [" << especieAModificar->fortalezaMax << "]: ";
    std::getline(cin, input);
    if (!input.empty()) especieAModificar->fortalezaMax = stoi(input);
    std::cout << "Nuevo danio base [" << especieAModificar->danoBase << "]: ";
    std::getline(cin, input);
    if (!input.empty()) especieAModificar->danoBase = stoi(input);
    std::cout << "Nueva salud maxima [" << especieAModificar->saludMax << "]: ";
    std::getline(cin, input);
    if (!input.empty()) especieAModificar->saludMax = stoi(input);
    std::cout << "Nueva rapidez [" << especieAModificar->rapidez << "]: ";
    std::getline(cin, input);
    if (!input.empty()) especieAModificar->rapidez = stoi(input);
    std::cout << "Nueva resistencia magica [" << especieAModificar->resistenciaMagica << "]: ";
    std::getline(cin, input);
    if (!input.empty()) especieAModificar->resistenciaMagica = stoi(input);
    string nuevaDesc = leerCadena("Nueva descripcion [" + especieAModificar->descripcion + "]: ", false);
    if (!nuevaDesc.empty()) {
        especieAModificar->descripcion = nuevaDesc;
    }
    std::cout << "Especie modificada con éxito.\n";
    pausar();
}


void eliminarEspecie() {
    if (!listaEspecies) {
        std::cout << "No hay especies para eliminar.\n";
        pausar();
        return;
    }
    mostrarEspecies();
    int idAEliminar = leerEntero("Seleccione el ID de la Especie a eliminar (0 para cancelar): ", 0, mayorID(listaEspecies));
    if (idAEliminar == 0) return;
    // Verificar que no haya personajes usando esta especie
    Nodo<Personaje> * temp = listaHeroes;
    Especie* especieAEliminar = obtenerPorID(listaEspecies, idAEliminar);
    if (especieAEliminar) {
        while (temp) {
           if (temp->dato.especie.nombre == especieAEliminar->nombre) {
                std::cout << "No se puede eliminar: hay personajes de esta especie (" << temp->dato.nombre << ").\n";
                pausar();
                return;
            }
            temp = temp->siguiente;
        }
        // Mandamos a eliminar la especie
        eliminarPorID(listaEspecies, idAEliminar);
        std::cout << "\n Especie eliminada con exito.\n";
        pausar();
    } 
}

void agregarSala() {
    std::string nombre = leerCadena("Ingrese nombre de la sala: ");
    string descripcion = leerCadena("Ingrese descripcion de la sala: ");
    int numeroSala;
    Sala nuevaSala = {numeroSala, nombre, descripcion, nullptr, 0, false, false, nullptr};
// **********    agregarAlFinal(mapaGlobal.salas, nuevaSala);
    std::cout << "Sala '" << nombre << "' creada correctamente.\n";
    pausar();
}

void modificarSala() {
    if (!mapaGlobal.salas) 
    {
       std::cout << "No hay salas para modificar.\n"; // En esta Linea da error
       pausar();
       return;
    }
    mostrarSalas();
    int pos = leerEntero("Seleccione sala a modificar (0 para cancelar): ", 0, contarLista(mapaGlobal.salas));
    if (pos == 0) return;
    Sala* sala = obtenerPorPosicion(mapaGlobal.salas, pos);
    if (!sala) {
       std::cout << "Error al seleccionar sala.\n"; // En esta Linea da error
       pausar();
       return;
    }
    std::cout << "\nModificando sala: " << sala->nombre << "\n";
    std::cout << "(Deje en blanco para mantener el valor actual)\n";
    string nuevoNombre = leerCadena("Nuevo nombre [" + sala->nombre + "]: ");
    if (!nuevoNombre.empty()) sala->nombre = nuevoNombre;
    string nuevaDescripcion = leerCadena("Nueva descripcion [" + sala->descripcion + "]: ", false);
    if (!nuevaDescripcion.empty()) sala->descripcion = nuevaDescripcion;
    std::cout << "Sala modificada con éxito.\n";
    pausar();
}

void eliminarSala() {
    if (!mapaGlobal.salas) {
        std::cout << "No hay salas para eliminar.\n";
        pausar();
        return;
    }
    mostrarSalas();     
    int pos = leerEntero("Seleccione sala a eliminar (0 para cancelar): ", 0, contarLista(mapaGlobal.salas));
    if (pos == 0) return;
    eliminarPorPosicion(mapaGlobal.salas, pos);
    std::cout << "Sala eliminada con éxito.\n";
    pausar();
}

void agregarImplemento() {
    //Pedimos la informacion al usuario
    int idImplemento = leerEntero("Ingrese el ID del implemento: ", 1, 100);
    string nombre = leerCadena("Ingrese el nombre del implemento: ");
    string tipo = leerCadena("Ingrese el tipo (ataque, defensa, cura, poder_magico, especial): ");
    int fortalezaNecesaria = leerEntero("Ingrese la fortaleza necesaria: ", 0, 100);
    int valor = leerEntero("Ingrese el valor (danio, curacion o defensa): ", 0, 1000);
    int usos = leerEntero("Ingrese la cantidad de usos: ", 1, 100);
    string descripcion = leerCadena("Ingrese la descripcion: ", false);
    //Creamos el Implemento y lo mandamos a agregar a la Lista
    Implemento nuevoImplemento = {nombre, tipo, fortalezaNecesaria, valor, usos, {}, descripcion};
    agregarOrdenadamente(listaImplementos, nuevoImplemento, idImplemento);
    std::cout << "Implemento '" << nombre << "' creado correctamente.\n";
    pausar();
}


void modificarImplemento() {
    if (!listaImplementos) {
        std::cout << "No hay implementos para modificar.\n";
        pausar();
        return;
    }
    mostrarImplementos(listaImplementos);
    int idAModificar = leerEntero("Seleccione implemento a modificar (0 para cancelar): ", 0, mayorID(listaImplementos));
    if (idAModificar == 0) return;
    Implemento* imp = obtenerPorID(listaImplementos, idAModificar);
    if (!imp) {
        std::cout << "Error al seleccionar implemento.\n";
        pausar();
        return;
    }
    std::cout << "\nModificando implemento: " << imp->nombre << "\n";
    std::cout << "(Deje en blanco para mantener el valor actual)\n";
    string input;
    std::cout << "Nuevo nombre [" << imp->nombre << "]: ";
    std::getline(cin, input);
    if (!input.empty()) imp->nombre = input;

    std::cout << "Nuevo tipo [" << imp->tipo << "]: ";       
    getline(cin, input);
    if (!input.empty()) imp->tipo = input;
    std::cout << "Nueva fortaleza necesaria [" << imp->fortalezaNecesaria << "]: ";
    getline(cin, input);
    if (!input.empty()) imp->fortalezaNecesaria = stoi(input);      
    std::cout << "Nuevo valor [" << imp->valor << "]: ";
    getline(cin, input);
    if (!input.empty()) imp->valor = stoi(input);
    std::cout << "Nueva cantidad de usos [" << imp->usos << "]: ";
    getline(cin, input);        
    if (!input.empty()) imp->usos = stoi(input);
    std::cout << "Nueva descripcion [" << imp->descripcion << "]: ";
    getline(cin, input);
    if (!input.empty()) imp->descripcion = input;
    std::cout << "Implemento modificado con éxito.\n";       
    pausar();
}


void eliminarImplemento() {     
    if (!listaImplementos) {
        std::cout << "No hay implementos para eliminar.\n";
        pausar();
        return;
    }
    // Le preguntamos al usuario cual es el que quiere eliminar
    mostrarImplementos(listaImplementos);
    int idABorrar = leerEntero("Seleccione el ID del implemento a eliminar (0 para cancelar): ", 0, mayorID(listaImplementos));
    if (idABorrar == 0) return;       // Si la respuesta es 0, no quiere eliminar ninguno

    // Verificar que ningUn personaje tenga este implemento en la mochila
    Implemento* imp = obtenerPorID(listaImplementos, idABorrar);
    if (imp) {
        Nodo<Personaje>* tempPersonajes = listaHeroes;
        while (tempPersonajes) {
            Nodo<Implemento>* impMochila = tempPersonajes->dato.mochila;
            while (impMochila) {
                if (impMochila->dato.nombre == imp->nombre) {
                    std::cout << "No se puede eliminar: un personaje tiene este implemento en la mochila (" 
                         << tempPersonajes->dato.nombre << ").\n";
                    pausar();
                    return;
                }
                impMochila = impMochila->siguiente;
            }
            tempPersonajes = tempPersonajes->siguiente;
        }
    }
    //eliminarPorPosicion(listaImplementos, pos);
    eliminarPorID(listaImplementos, idABorrar);
    std::cout << "Implemento eliminado con exito.\n";
    pausar();
}


/* ***************************************************************************************************
// Se piden los datos al usuario, incluyendo el ID, se valida la unicidad del ID y del nombre
// el Nuevo personaje se inserta Ordenadamente en listaHeroes, pasando con el idLeido de la pantalla
*************************************************************************************************** */
int agregarPersonaje() {
    if (!listaEspecies) {
        std::cout << "No hay especies disponibles para crear personajes.\n";
        pausar();
        return 0;
    }

    int idLeido = leerEntero("Ingrese el ID del personaje: ",1 , 100);
    // Verificar unicidad del ID
    Nodo<Personaje> * temp = listaHeroes;
    while (temp) {
        if (temp->idNodo == idLeido) {
            std::cout << "Ya existe un personaje con ese ID.\n";
            pausar();
            return 0;
        }
        temp = temp->siguiente;
    }

    string nombreLeido = leerCadena("Ingrese nombre del personaje: ");
    // Verificar unicidad de nombre
    while (temp) {
        if (temp->dato.nombre == nombreLeido) {
            std::cout << "Ya existe un personaje con ese nombre.\n";
            pausar();
            return 0;
        }
        temp = temp->siguiente;
    }

    Especie* especie = seleccionarEspecie();
    if (!especie) {
        std::cout << "La especie no existe. Creacion de personaje cancelada.\n";
        return 0;
    }

    Personaje nuevoPersonaje;
    nuevoPersonaje.nombre = nombreLeido;
    nuevoPersonaje.especie = *especie;
    nuevoPersonaje.saludMax = especie->saludMax;
    nuevoPersonaje.salud = nuevoPersonaje.saludMax;
    nuevoPersonaje.fortalezaMax = especie->fortalezaMax;
    nuevoPersonaje.fortaleza = nuevoPersonaje.fortalezaMax;
    nuevoPersonaje.rapidez = especie->rapidez;
    nuevoPersonaje.experiencia = 0;
    nuevoPersonaje.nivel = 1;
    nuevoPersonaje.vivo = true;
    nuevoPersonaje.efectos.clear();
    nuevoPersonaje.mochila = nullptr;
    nuevoPersonaje.muertes = 0;
    nuevoPersonaje.kills = 0;
    nuevoPersonaje.danioTotal = 0;
    nuevoPersonaje.curacionTotal = 0;

    agregarOrdenadamente(listaHeroes, nuevoPersonaje, idLeido);
    std::cout << "Personaje '" << nombreLeido << "' creado correctamente.\n";
    pausar();
    return idLeido;
}

void modificarPersonaje() {
    if (!listaHeroes) {
        std::cout << "No hay personajes para modificar.\n";
        pausar();
        return;
    }

    mostrarPersonajes();
    int pos = leerEntero("Seleccione personaje a modificar (0 para cancelar): ", 0, contarLista(listaHeroes));
    if (pos == 0) return;

    Personaje* p = obtenerPorPosicion(listaHeroes, pos);
    if (!p) {
        std::cout << "Error al seleccionar personaje.\n";
        pausar();
        return;
    }

    std::cout << "\nModificando personaje: " << p->nombre << "\n";
    std::cout << "(Deje en blanco para mantener el valor actual)\n";

    string input;

    std::cout << "Nuevo nombre [" << p->nombre << "]: ";
    getline(cin, input);
    if (!input.empty()) p->nombre = input;

    // Permitir cambiar especie
    std::cout << "Desea cambiar la especie? (s/n): ";
    getline(cin, input);
    if (!input.empty() && (tolower(input[0]) == 's')) {
        Especie* especieNueva = seleccionarEspecie();
        if (especieNueva) {
            p->especie = *especieNueva;
            p->saludMax = especieNueva->saludMax;
            p->salud = p->saludMax;
            p->fortalezaMax = especieNueva->fortalezaMax;
            p->fortaleza = p->fortalezaMax;
            p->rapidez = especieNueva->rapidez;
        }
    }

    std::cout << "Salud actual [" << p->salud << "], Salud maxima [" << p->saludMax << "]\n";
    std::cout << "Ingrese nueva salud (0 para mantener actual): ";
    getline(cin, input);
    if (!input.empty()) {
        int val = stoi(input);
        if (val > 0 && val <= p->saludMax) p->salud = val;
    }

    std::cout << "Fortaleza actual [" << p->fortaleza << "], Fortaleza maxima [" << p->fortalezaMax << "]\n";
    std::cout << "Ingrese nueva fortaleza (0 para mantener actual): ";
    getline(cin, input);
    if (!input.empty()) {
        int val = stoi(input);
        if (val > 0 && val <= p->fortalezaMax) p->fortaleza = val;
    }

    std::cout << "Personaje modificado con éxito.\n";
    pausar();
}

void eliminarPersonaje() {
    if (!listaHeroes) {
        std::cout << "No hay personajes para eliminar.\n";
        pausar();
        return;
    }

    mostrarPersonajes();
    int pos = leerEntero("Seleccione personaje a eliminar (0 para cancelar): ", 0, contarLista(listaHeroes));
    if (pos == 0) return;

    Personaje* p = obtenerPorPosicion(listaHeroes, pos);
    if (p) {
        // Liberar la mochila
        if (p->mochila) {
            liberarLista(p->mochila);
            p->mochila = nullptr;
        }
    }

    eliminarPorPosicion(listaHeroes, pos);
    std::cout << "Personaje eliminado con éxito.\n";
    pausar();
}

// Funciones para gestionar la mochila de un personaje

void agregarImplementoAMochila(Personaje* personaje) {
    if (!personaje) return;

    int cantidadImplementosEnMochila = contarLista(personaje->mochila);
    if (cantidadImplementosEnMochila >= MAX_IMPLEMENTOS) {
        std::cout << "La mochila esta llena. No se pueden agregar mas implementos.\n";
        pausar();
        return;
    }

    Implemento* implementoSeleccionado = seleccionarImplemento(listaImplementos);
    // como quiero utilizar en la mochila el mismo id que tiene el implemento en listaImplementos debo obtener al apuntador al nodo que lo contiene para tomar el id de alli
    Nodo<Implemento> *nodoImplemento = obtenerApuntadorANodoPorNombre(listaImplementos, implementoSeleccionado->nombre);

    if (!implementoSeleccionado) {
        std::cout << "Seleccion cancelada.\n";
        return;
    }

    // Validar fortaleza necesaria
    if (personaje->fortaleza < implementoSeleccionado->fortalezaNecesaria) {
        std::cout << "No tiene la fortaleza necesaria para usar este implemento.\n";
        pausar();
        return;
    }

    // Verificar si ya esta en mochila
    Nodo<Implemento>* temp = personaje->mochila;
    while (temp) {
        if (temp->dato.nombre == implementoSeleccionado->nombre) {
            std::cout << "Este implemento ya esta en la mochila.\n";
            pausar();
            return;
        }
        temp = temp->siguiente;
    }
    agregarOrdenadamente(personaje->mochila, *implementoSeleccionado, nodoImplemento->idNodo);
    std::cout << "Implemento '" << implementoSeleccionado->nombre << "' agregado a la mochila de " << personaje->nombre << ".\n";
    pausar();
}

void eliminarImplementoDeMochila(Personaje* p) {
    if (!p) return;

    if (!p->mochila) {
        std::cout << "La mochila esta vacia.\n";
        pausar();
        return;
    }

    mostrarImplementos(p->mochila);
    int pos = leerEntero("Seleccione implemento a eliminar de la mochila (0 para cancelar): ", 0, contarLista(p->mochila));
    if (pos == 0) return;

    eliminarPorPosicion(p->mochila, pos);
    std::cout << "Implemento eliminado de la mochila.\n";
    pausar();
}

/******************************/
/*     FUNCIONES DE MENU      */
/******************************/

// Funcion para el submenu de especies
void menuEspecies() {
    while (true) {
        limpiarPantalla();
        std::cout << "\n=== SUBMENU DE ESPECIES ===\n";
        std::cout << "1. Mostrar Especies Existentes\n";
        std::cout << "2. Agregar Nueva Especie\n";
        std::cout << "3. Modificar datos de una Especie\n";
        std::cout << "4. Eliminar Especie\n";
        std::cout << "5. Volver al MenU Principal\n";
        
        int opcion = leerEntero("Seleccione una opcion: ", 1, 5);
        
        switch (opcion) {
            case 1:
                mostrarEspecies(true);
                pausar();
                break;
            case 2:
                agregarEspecie();
                guardarEspeciesEnArchivo();
                break;
            case 3:
                modificarEspecie();
                guardarEspeciesEnArchivo();
                break;
            case 4:
                eliminarEspecie();
                guardarEspeciesEnArchivo();
                break;
            case 5:
                return;
        }
    }
}

// Funcion para el submenu de mapa
void menuMapa() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== SUBMENU DE MAPA ===\n";
        std::cout << "1. Mostrar el Mapa Actual\n";
        std::cout << "2. Agregar nueva sala dentro del mapa\n";
        std::cout << "3. Modificar los datos de una Sala dentro del Mapa\n";
        std::cout << "4. Eliminar una Sala del Mapa\n";
        std::cout << "5. Volver al Menu Principal\n";
        
        int opcion = leerEntero("Seleccione una opcion: ", 1, 5);
        
        switch (opcion) {
            case 1:
                mostrarSalas();
                pausar();
                break;
            case 2:
                agregarSala();
                break;
            case 3:
                modificarSala();
                break;
            case 4:
                eliminarSala();
                break;
            case 5:
                return;
        }
    }
}

// Funcion para el submenu de implementos
void menuImplementos() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== SUBMENU DE IMPLEMENTOS ===\n";
        std::cout << "1. Mostrar los implementos que existen actualmente\n";
        std::cout << "2. Agregar un nuevo implemento\n";
        std::cout << "3. Modificar los datos de un Implemento\n";
        std::cout << "4. Eliminar uno de los Implementos\n";
        std::cout << "5. Volver al MenU Principal\n";
        
        int opcion = leerEntero("Seleccione una opcion: ", 1, 5);
        
        switch (opcion) {
            case 1:
                mostrarImplementos(listaImplementos, true);
                pausar();
                break;
            case 2:
                agregarImplemento();
                guardarImplementosEnArchivo();
                break;
            case 3:
                modificarImplemento();
                guardarImplementosEnArchivo();
                break;
            case 4:
                eliminarImplemento();
                guardarImplementosEnArchivo();
                break;
            case 5:
                return;
        }
    }
}


void seleccionarPersonajes() {
    for (int i = 0; i < NUM_HEROES_A_ELEGIR; ++i) {
        std::cout << "Seleccionando personaje " << (i + 1) << ":\n";
        int idNuevoPersonaje = agregarPersonaje(); // Llama a la funcion existente para agregar un personaje

        // Asignar 5 implementos al personaje
        Personaje* p = obtenerPorID(listaHeroes, idNuevoPersonaje); // Obtiene el último personaje agregado
        for (int j = 0; j < MAX_IMPLEMENTOS; ++j) {
            std::cout << "Asignando implemento " << (j + 1) << " a " << p->nombre << ":\n";
            agregarImplementoAMochila(p); // Llama a la funcion para agregar un implemento a la mochila
            mostrarPersonajes();
        }
    }
}

void menuJuego() {
    while (true) {
        limpiarPantalla();
        std::cout << "\n=== MENÚ DEL JUEGO ===\n";
        std::cout << "1. Revisar Mapa\n";
        std::cout << "2. Hacer Movimiento dentro del Mapa\n";
        std::cout << "3. Gestion de Equipo\n";
        std::cout << "4. Consultar datos del Equipo\n";
        std::cout << "5. Bitacora\n";
        std::cout << "6. Rendirse (Salir del Juego)\n";

        int opcion = leerEntero("Seleccione una opcion: ", 1, 6);

        switch (opcion) {
            case 1:
                mostrarSalas();
                pausar();
                break;
            case 2:
                // Aqui iria la logica para mover al jugador en el mapa
                std::cout << "Funcionalidad de movimiento aún no implementada.\n";
                pausar();
                break;
            case 3:
                // Gestionar equipo (podrias mostrar un submenu con opciones para agregar, modificar, etc.)
                std::cout << "Gestion de equipo aún no implementada.\n";
                pausar();
                break;
            case 4:
                mostrarPersonajes(false); // Mostrar detalles completos del equipo
                pausar();
                break;
            case 5:
                // Mostrar bitacora o eventos pasados
                std::cout << "Bitacora aún no implementada.\n";
                pausar();
                break;
            case 6:
                std::cout << "Rendirse y salir del juego.\n";
                return; // Sale del menuJuego y vuelve al menu principal o finaliza
        }
    }
}

// Funcion para iniciar el juego: seleccion de personajes e inicio del menú del juego
void iniciarJuego() {
    seleccionarPersonajes(); // Funcion para seleccionar 4 personajes y asignarles 5 implementos cada uno
    menuJuego();          
}

void menuPrincipal() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== MENU PRINCIPAL ===\n";
        std::cout << "1. Iniciar Juego\n";
        std::cout << "2. Submenu de Especies\n";
        std::cout << "3. Submenu de Mapa\n";
        std::cout << "4. Submenu de Implementos\n";
        std::cout << "5. Salir del Juego\n";

        int opcion = leerEntero("Seleccione una opcion: ", 1, 5);

        switch (opcion) {
            case 1:
                iniciarJuego();
                break;
            case 2:
                menuEspecies();
                break;
            case 3:
                menuMapa();
                break;
            case 4:
                menuImplementos();
                break;
            case 5:
                std::cout << "Saliendo del juego...\n";
                return;
        }
    }
}

// Menú mostrado durante una batalla activa
void menuCombate() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== MENU DE COMBATE ===\n";
        std::cout << "1. Atacar\n";
        std::cout << "2. Usar objeto\n";
        std::cout << "3. Usar Poder Magico\n";
        std::cout << "4. Analizar estado de la batalla\n";
        std::cout << "5. Huir\n";

        int opcion = leerEntero("Seleccione una opcion: ", 1, 5);

        switch (opcion) {
            case 1:
                std::cout << "Has seleccionado atacar.\n";
                // Aqui incluir la logica para ejecutar un ataque
                pausar();
                break;
            case 2:
                std::cout << "Has seleccionado usar un objeto.\n";
                // Logica para usar objetos
                pausar();
                break;
            case 3:
                std::cout << "Has seleccionado usar poder magico.\n";
                // Logica para usar poderes magicos
                pausar();
                break;
            case 4:
                std::cout << "Estado actual de la batalla:\n";
                // Mostrar detalles de salud, estados, etc.
                pausar();
                break;
            case 5:
                std::cout << "Has huido de la batalla.\n";
                return; // Salir del menú combate
        }
    }
}


//******************************************************************************************//
/*                                     FUNCION PRINCIPAL                                    */
//******************************************************************************************//

int main() {
    inicializarDatosBase(); // Inicializa las especies y los implementos base
    menuPrincipal(); // Llama al menU principal

    // Liberar memoria antes de salir
    //liberarLista(listaEspecies);
    //liberarLista(listaHeroes);
    //liberarLista(listaImplementos);
    //liberarLista(mapaGlobal.salas);

    return 0;
}
