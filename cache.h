#define BITS_DIR_MEM 16
#define MEMORY_SIZE 65536
#define OK 0
#define ERROR 1
#define NO_MISS_RATE '-'
#define EFES 0xFFFFFFFF


typedef struct sVia
{
    int time_stamp;
    int cargada;
    int tag;
    int is_dirty;
	char * datos;
}sVia;


typedef struct sBloque
{
 
    struct sVia* vias;
}sBloque;

typedef struct sCache
{
    
    int cantidadvias;
    int cantidadbloques;
    int capaciadbloques;
    
    
    int lecturas;
    int misses;
	int timestampgen;
    
	int bitsoffset;
    int bitsindex;
    int bitstag;
    struct sBloque* Bloque;
}sCache;

char * Memoria_principal;
sCache cache;


/*
La funcion begin inicializa la estructura segun los parametros. Si hay un error de memoria devuelve NULL
*/
int begin (int block_size, int ways, int cache_size);

/*
La funcion init() debe inicializar los bloques de la cache como invalidos,
la memoria simulada en 0 y la tasa de misses a 0.
*/
void init();

/*
La funcion end() debe desalocar los bloques de la cache.
*/
void end();

/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
*/
unsigned int find_offset(unsigned int address);




/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
*/
unsigned int find_set(unsigned int address);


/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
*/
unsigned int find_tag(unsigned int address);



/*
La funcion find block(unsigned int address) debe devolver el bloque
de memoria correspondiente a la direccion address.
*/

unsigned int find_block (unsigned int address);



/*
La funcion find earliest(unsigned int setnum) debe devolver el
bloque mas 'antiguo' dentro de un conjunto, utilizando el campo correspondiente
de los metadatos de los bloques del conjunto.
*/
unsigned int find_earliest(unsigned int setnum);


/*
La funcion is dirty(unsigned int way,unsigned int setnum) debe
devolver el estado del bit D del bloque correspondiente.
*/
unsigned int is_dirty(unsigned int way, unsigned int setnum);


/*
La funcion write block(unsigned int way, unsigned int
setnum) debe escribir en las posiciones correspondientes de memoria
el contenido del bloque setnum de la via way.
*/
void write_block(unsigned int way, unsigned int setnum);


/*
La funcion read block(unsigned int blocknum) debe leer el bloque
blocknum de memoria y guardarlo en el lugar que le corresponda en
la memoria cache.
*/

void read_block(unsigned int blocknum);



/*
La funcion find_via(unsigned int address) busca la via en la que esta guardada address 
y devuelve 0xFFFFFFFF si no esta
*/
int find_via(unsigned int address);

/*
La funcion read byte(unsigned int address, char *hit) debe retornar
el valor correspondiente a la posicioon de memoria address,
buscnandolo primero en el cache, y escribir 1 en *hit si es un hit y 0 si
es un miss.
*/
char read_byte(unsigned int address, unsigned char *hit);


/*
La funcion write byte(unsigned int address, char value) debe
escribir el valor value en la posicion correcta del bloque que corresponde
a address, si esta en el cache, y escribir 1 en *hit si es un hit
y 0 si es un miss.
*/
void write_byte(unsigned int address, char value, unsigned char *hit);

/*
La funcion get miss rate() debe devolver el porcentaje de misses
desde que se inicializo el cache.
read byte() y write byte() solo deben interactuar con la memoria
a traves de las otras primitivas.
*/
char get_miss_rate();

