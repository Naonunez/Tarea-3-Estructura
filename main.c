#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "HashMap.h"
#include "list.h"
#include <math.h>
struct ResultadoBusqueda {
    int id;
    char titulo[100];
};

typedef struct {
  char titulo[100];  
  char nombre_archivo[100];
  char id[100]; 
  HashMap* palabras;
  char autor[100];  
  int caracteres;
  int CantPalabras;
  int titulo_length;
}Libro;
int opciondef(int opcion){
  scanf("%d", &opcion);
  return opcion;
}

int contadorPalabras(char *linea){
  int contador = 0;
  int Palabra = 0;

  for (int i = 0; linea[i] != '\0'; i++) {
    if (linea[i] == ' ' || linea[i] == '\t' || linea[i] == '\n' || linea[i] == '\r') {
      Palabra = 0;
    }
    else {
      if (Palabra == 0) {
        contador++;
        Palabra = 1;
     }
    }
  }
  return contador;
}

int ContadorCaracteres(char *linea){
  int contador = 0;
  for (int i = 0; linea[i] != '\0'; i++) {
    if (linea[i] != '\n') { 
        contador++;
    }
  }

  return contador;
}

void Palabras( char *cadena, HashMap* palabra) {
char *token = strtok(cadena, " ");

while (token != NULL) {
    char cleanedToken[100]; 
    int j = 0;
    for (int i = 0; token[i]; i++) {
        if (isalpha(token[i])) {
            cleanedToken[j] = tolower(token[i]);
            j++;
        }
    }
    cleanedToken[j] = '\0';

    if (strlen(cleanedToken) > 0) { 
        Pair *pair = searchMap(palabra, cleanedToken);
        if (pair != NULL) {
            int *count = (int *)(pair->value);
            (*count)++;
        } else { 
            int *count = (int *)malloc(sizeof(int));
            *count = 1;
            insertMap(palabra, cleanedToken, count);
        }
      
    }

    token = strtok(NULL, " "); 
}
}

// Declarar la variable global CONTLIBROS
int CONTLIBROS = 0;
void mostrarprimero(HashMap* frec){
  Pair* palabra = firstMap(frec);
  printf("%s\n", palabra->key);
}

void CargarDocumento(char* nombreArchivo, HashMap* MapaLibros, HashMap* frecuenciaPalabras) {
  printf("\n");
  char linea[1000];
  char *carpeta = "Libros";
  DIR *ArchivosCarpeta = opendir(carpeta);
  int encontrado = 0;

  // Crear un nuevo libro para almacenar los datos
  Libro *Nuevo = (Libro*)malloc(sizeof(Libro));
  Nuevo->CantPalabras = 0;
  Nuevo->caracteres = 0;
  char cadena[] = "*** START OF ";
  char CadenaFin[] = "End of Project Gutenberg";
  if (ArchivosCarpeta) {
    struct dirent *entrada;
    char RutaArchivo[1000];

    while ((entrada = readdir(ArchivosCarpeta)) != NULL) {
      if (strcmp(entrada->d_name, nombreArchivo) == 0) {
        snprintf(RutaArchivo, sizeof(RutaArchivo), "%s/%s", carpeta, nombreArchivo);

        FILE *Archivo = fopen(RutaArchivo, "r");
        if (Archivo == NULL) {
          printf("No se pudo abrir el archivo: %s\n", nombreArchivo);


          return;
        }
        encontrado = 1;
        while (fgets(linea, sizeof(linea), Archivo)) {
          linea[strcspn(linea, "\n")] = '\0';

          if (strncmp(linea, "Title:", 6) == 0) {
            char *title = linea + 7;
            strcpy(Nuevo->titulo, title);
          }

          if (strncmp(linea, "Author:", 7) == 0) {
            strcpy(Nuevo->autor, linea + 8);
            Nuevo->autor[strlen(Nuevo->autor) - 1] = '\0';
          }

          // Contar palabras y caracteres
          Nuevo->CantPalabras += contadorPalabras(linea);
          Nuevo->caracteres += ContadorCaracteres(linea);

          strcpy(Nuevo->nombre_archivo, nombreArchivo);
          char *subcadenaid = strtok(nombreArchivo, ".");
          strcpy(Nuevo->id, subcadenaid);

          if (strstr(linea, cadena) != NULL) {
            Palabras(linea, frecuenciaPalabras);
            Nuevo->CantPalabras = 0;
            Nuevo->caracteres = 0;
          }

          if (strstr(linea, CadenaFin) != NULL) {
            break;
          }
        }
        // Insertar el libro en el mapa
        insertMap(MapaLibros, Nuevo->id, Nuevo);
        Nuevo->palabras= frecuenciaPalabras;
        fclose(Archivo);

        CONTLIBROS++;

        printf("El archivo %s.txt ha sido cargado correctamente\n", nombreArchivo);
        return;
      }
    }
    if (encontrado == 0) {
      printf("No se ha encontrado el archivo que buscas");

      CONTLIBROS--;

    }
    closedir(ArchivosCarpeta);
  }
}



int compararLibrosPorTitulo(const void* a, const void* b) {
  const Libro* libroA = *(const Libro**)a;
  const Libro* libroB = *(const Libro**)b;
  return strcmp(libroA->titulo, libroB->titulo);
}

void Mostrardocumentos(HashMap* MapaLibros) {
  printf("Libros cargados: \n");

  Pair* primerLibro = firstMap(MapaLibros);

  // Crear un arreglo para almacenar los libros
  Libro* libros[1000]; // Supongamos un máximo de 1000 libros
  int numLibros = 0;

  while (primerLibro != NULL) {
    Libro* Datos = (Libro*)primerLibro->value;
    libros[numLibros] = Datos;
    numLibros++;
    primerLibro = nextMap(MapaLibros);
  }

  // Ordenar los libros por título
  qsort(libros, numLibros, sizeof(Libro*), compararLibrosPorTitulo);

  // Mostrar los libros ordenados
  for (int i = 0; i < numLibros; i++) {
    Libro* Datos = libros[i];
    printf("Id: %s\n", Datos->id);
    printf("Titulo: %s\n", Datos->titulo);
    printf("Cantidad de palabras: %i\n", Datos->CantPalabras);
    printf("Cantidad de caracteres: %i\n\n", Datos->caracteres);
  }
}

void BuscarLibroPorTitulo(HashMap* MapaLibros, const char* palabrasClave) {
    printf("Resultados de la búsqueda por título:\n");

    // Obtén el primer libro del HashMap
    Pair* primerlibro = firstMap(MapaLibros);

    while (primerlibro != NULL) {
        Libro* Datos = (Libro*)primerlibro->value;
        // Verifica si todas las palabras clave están presentes en el título
        const char* titulo = Datos->titulo;    


        List* ListaPalabras = createList();
        char* copia = strdup(palabrasClave); 
        char* palabra = strtok(copia, " ");

        while (palabra != NULL) {
            pushBack(ListaPalabras, palabra);
            palabra = strtok(NULL, " "); 

        }


        List* ListaPalabrasTitulo = createList();
        char* copia1 = strdup(titulo); 
        char* palabratitulo = strtok(copia1, " ");

        while (palabratitulo != NULL) {
            pushBack(ListaPalabrasTitulo, palabratitulo);
            palabratitulo = strtok(NULL, " "); 

        }


        palabratitulo=firstList(ListaPalabrasTitulo);
        palabra=firstList(ListaPalabras);
        int coincidencia=0;
        while(palabratitulo != NULL){
          while(palabra != NULL){
            if(strcasecmp(palabra, palabratitulo) == 0){
              coincidencia++;
            }
            palabra=nextList(ListaPalabras);
          }
          palabra=firstList(ListaPalabras);
          palabratitulo=nextList(ListaPalabrasTitulo);
        }
        if (coincidencia==contadorPalabras((char*)palabrasClave)){
          printf("%s\n", titulo);
        }
        cleanList(ListaPalabras);
        cleanList(ListaPalabrasTitulo);
        primerlibro = nextMap(MapaLibros);
    }
  printf("\n");
}
void PalabrasConMayorFrecuencia(HashMap* Libros, char* idLibro) {
  // Buscar el libro en el mapa
  Pair* libroPair = searchMap(Libros, idLibro);

  if (libroPair == NULL) {
    printf("El libro con ID %s no se encuentra en la biblioteca.\n", idLibro);
    return;
  }

  // Obtener el libro y su mapa de frecuencia de palabras
  Libro* libro = (Libro*)libroPair->value;
  HashMap* frecuenciaPalabras = libro->palabras;

  // Crear un arreglo de pares para ordenar las palabras por frecuencia
  Pair* pares[1000];
  int numPares = 0;

  // Copiar los pares del mapa de frecuencia a un arreglo
  Pair* palabraPair = firstMap(frecuenciaPalabras);
  while (palabraPair != NULL) {
    pares[numPares] = palabraPair;
    numPares++;
    palabraPair = nextMap(frecuenciaPalabras);
  }

  // Ordenar el arreglo de pares por frecuencia descendente
  for (int i = 0; i < numPares - 1; i++) {
    for (int j = i + 1; j < numPares; j++) {
      int* frecuencia1 = (int*)pares[i]->value;
      int* frecuencia2 = (int*)pares[j]->value;
      if (*frecuencia2 > *frecuencia1) {
        Pair* temp = pares[i];
        pares[i] = pares[j];
        pares[j] = temp;
      }
    }
  }

  // Mostrar las palabras con mayor frecuencia
  printf("Las palabras con mayor frecuencia en el libro %s son:\n", idLibro);
  for (int i = 0; i < numPares; i++) {
    char* palabra = (char*)pares[i]->key;
    int* frecuencia = (int*)pares[i]->value;
    printf("%s: %d veces\n", palabra, *frecuencia);
  }
}

void PalabrasMasRelevantes(HashMap* Libros, char* idLibro) {
    // Buscar el libro en el mapa
    Pair* libroPair = searchMap(Libros, idLibro);

    if (libroPair == NULL) {
        printf("El libro con ID %s no se encuentra en la biblioteca.\n", idLibro);
        return;
    }

    // Obtener el libro y su mapa de frecuencia de palabras
    Libro* libro = (Libro*)libroPair->value;
    HashMap* frecuenciaPalabras = libro->palabras;

    // Calcular el número total de documentos
    int numDocumentos = CONTLIBROS;

    // Crear un arreglo de pares para calcular la relevancia de las palabras
    Pair* pares[1000];
    int numPares = 0;

    // Copiar los pares del mapa de frecuencia a un arreglo
    Pair* palabraPair = firstMap(frecuenciaPalabras);
    while (palabraPair != NULL) {
        pares[numPares] = palabraPair;
        numPares++;
        palabraPair = nextMap(frecuenciaPalabras);
    }

    // Calcular la relevancia de las palabras
    double relevancia[1000];
    for (int i = 0; i < numPares; i++) {
        char* palabra = (char*)pares[i]->key;
        int* frecuencia = (int*)pares[i]->value;

        // Calcular la relevancia utilizando la fórmula dada
        double palabraRelevancia = ((double)(*frecuencia) / libro->CantPalabras) *
         log((double)numDocumentos);

        relevancia[i] = palabraRelevancia;
    }

    // Ordenar las palabras por relevancia descendente
    for (int i = 0; i < numPares - 1; i++) {
        for (int j = i + 1; j < numPares; j++) {
            if (relevancia[j] > relevancia[i]) {
                Pair* tempPair = pares[i];
                pares[i] = pares[j];
                pares[j] = tempPair;

                double tempRelevancia = relevancia[i];
                relevancia[i] = relevancia[j];
                relevancia[j] = tempRelevancia;
            }
        }
    }

    // Mostrar las 10 palabras más relevantes
    printf("Las 10 palabras más relevantes en el libro %s son:\n", idLibro);
    for (int i = 0; i < 10 && i < numPares; i++) {
        char* palabra = (char*)pares[i]->key;
        printf("%s: %.4lf\n", palabra, relevancia[i]);
    }
}







void BuscarPorPalabra(HashMap* Libros, const char* palabraClave) {
    printf("Resultados de la búsqueda por palabra clave: %s\n", palabraClave);

    // Obtén el primer libro del HashMap
    Pair* primerLibro = firstMap(Libros);

    while (primerLibro != NULL) {
        Libro* Datos = (Libro*)primerLibro->value;

        // Leer el contenido del libro desde el archivo
        char contenido[10000]; // Suponiendo una longitud máxima de contenido de 10,000 caracteres
        FILE* archivoLibro = fopen(Datos->nombre_archivo, "r");
        if (archivoLibro == NULL) {
            // Manejar un error si no se puede abrir el archivo del libro
            perror("Error al abrir el archivo del libro");
            return;
        }

        fread(contenido, 1, sizeof(contenido), archivoLibro);
        fclose(archivoLibro);

        // Comprobar si la palabra clave se encuentra en el contenido del libro
        if (strstr(contenido, palabraClave) != NULL) {
            printf("ID: %s\n", Datos->id);
            printf("Título: %s\n", Datos->titulo);
        }

        primerLibro = nextMap(Libros);
    }

    printf("\n");
}







int main(){

  int opcion;
  int opciondefinida;
  Libro biblioteca[100]; 
  int numLibros = 0;
  char NombreArchivo[100];
  HashMap* Libros = NULL;
  Libros = createMap(1000);



  printf("Menú\n");
  printf("1.Cargar Documentos\n");
  printf("2.Mostrar documentos ordenados.\n");
  printf("3.Buscar un libro por título.\n");
  printf("4.Palabras con mayor frecuencia\n");
  printf("5.Palabras más relevantes.\n");
  printf("6.Buscar por palabra. \n");
  printf("7.Mostrar palabra en su contexto dentro del libro.\n");
  printf("8.Salir\n");
  opciondefinida = opciondef(opcion);

  while(opciondefinida != 8){

    switch(opciondefinida){
      case 1:
        printf("Ingrese el nombre del archivo a cargar\n");
        scanf("%s",NombreArchivo);
        HashMap* FrecPalabras = NULL;
        FrecPalabras = createMap(1000);

        CargarDocumento(NombreArchivo, Libros, FrecPalabras);
        printf("\n");

        break;


        printf("\n");
        break;
      case 2:
        printf("\n");
        Mostrardocumentos(Libros);

        break;

        case 3:
      printf("Ingrese las palabras clave separadas por espacios:\n");
      char palabrasClave[200];  // Asume un límite de 200 caracteres para palabras clave
      scanf(" %[^\n]", palabrasClave);  // Leer toda la línea

      BuscarLibroPorTitulo(Libros, palabrasClave);
      break;




      case 4:
      /*printf("Ingrese el numero id del libro a buscar en:\n");
      int idBusca;
      scanf("%i", &idBusca);*/
      printf("Ingrese el ID del libro para encontrar palabras con mayor frecuencia: ");
      char idLibro[100];
      scanf("%s", idLibro);

      PalabrasConMayorFrecuencia(Libros, idLibro);




      break;

      case 5:
      printf("Ingrese el ID del libro para encontrar las palabras más relevantes: ");
      char palabrita[100];
      scanf("%s", palabrita);
      
      PalabrasMasRelevantes(Libros, palabrita);

      break;
      


      case 6:
      printf("Ingrese la palabra clave a buscar:\n");
      char palabraClave[200];  // Asume un límite de 200 caracteres para la palabra clave
      scanf(" %[^\n]", palabraClave);  // Leer toda la línea

      BuscarPorPalabra(Libros, palabraClave);
      break;

      case 7:

      case 8:
        {



        }
        break;


      default:
        {
          printf("Opcion no valida. Intentelo de nuevo porfavor.\n");
        }

        break;
    }
    printf("Menú\n");
    printf("1.Cargar Documentos\n");
    printf("2.Mostrar documentos ordenados.\n");
    printf("3.Buscar un libro por título.\n");
    printf("4.Palabras con mayor frecuencia\n");
    printf("5.Palabras más relevantes.\n");
    printf("6.Buscar por palabra. \n");
    printf("7.Mostrar palabra en su contexto dentro del libro.\n");
    printf("8.Salir\n");


    opciondefinida = opciondef(opcion);
  }


  return 0;
}