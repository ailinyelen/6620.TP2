#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ERROR -1
#define BITS_DIR_MEM 20


char * ByteMemoria;

typedef struct sVia
{
    int time_stamp;
    int cargada;
    int tag;
    int is_dirty;
	char * datos;
};


typedef struct sBloque
{
 
    struct sVia* vias;
};

typedef struct sMemoria
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
};


const unsigned int EFES = 0xFFFFFFFF;

struct sMemoria memoria;

/*
La funcion init() debe inicializar los bloques de la cache como invalidos,
la memoria simulada en 0 y la tasa de misses a 0.
*/
void init()
{
	
	memoria.Bloque = malloc(sizeof(struct sBloque) * memoria.cantidadbloques);
	int cont = 0;
	int contvias = 0;
	
	for (cont = 0; cont < memoria.cantidadbloques; cont++)
	{
		memoria.Bloque[cont].vias = malloc(sizeof(struct sVia) * memoria.cantidadvias);
		for (contvias = 0; contvias < memoria.cantidadvias; contvias++)
		{
			memoria.Bloque[cont].vias[contvias].is_dirty = 0;
			memoria.Bloque[cont].vias[contvias].cargada = 0;
			memoria.Bloque[cont].vias[contvias].tag = 0;
			memoria.Bloque[cont].vias[contvias].time_stamp = -1;
			memoria.Bloque[cont].vias[contvias].datos = malloc(sizeof(char) * memoria.capaciadbloques);
		}
	}
}


/*
La funcion end() debe desalocar los bloques de la cache.
*/
void end()
{
	
	int cont = 0;
	int contvias = 0;
	
	for (cont = 0; cont < memoria.cantidadbloques; cont++)
	{
		
		for (contvias = 0; contvias < memoria.cantidadvias; contvias++)
		{
			
			free(memoria.Bloque[cont].vias[contvias].datos);
		}
		
		free(memoria.Bloque[cont].vias);
		
	}
	free(memoria.Bloque);
}


/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
*/
unsigned int find_offset(unsigned int address)
{
	
	unsigned int efes = EFES;
	efes = (efes >> (32 - memoria.bitsoffset));
	return address & efes;	
}




/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
*/
unsigned int find_set(unsigned int address)
{
	unsigned int sinoffset = (address >> memoria.bitsoffset);
	unsigned int efes = EFES;
	efes = (efes >> (32 - memoria.bitsindex));
	unsigned int ret = efes & sinoffset;
	return ret;
}


/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
*/
unsigned int find_tag(unsigned int address)
{	
	return (address >> (memoria.bitsoffset + memoria.bitsindex));	
}



/*
La funcion find block(unsigned int address) debe devolver el bloque
de memoria correspondiente a la direccion address.
*/

unsigned int find_block (unsigned int address)
{
	/// EL INDICE DE BLOQUE EN MEMORIA FISICA? QUE DIFERENCIA HAY CON find_set
	unsigned int sinoffset = (address >> memoria.bitsoffset);
	return sinoffset;
}



/*
La funcion find earliest(unsigned int setnum) debe devolver el
bloque mas 'antiguo' dentro de un conjunto, utilizando el campo correspondiente
de los metadatos de los bloques del conjunto.
*/
unsigned int find_earliest(unsigned int setnum)
{
	struct sBloque bloque = memoria.Bloque[setnum];
	int cont = 0;
	struct sVia viaearliest;

	// LE ASIGNA EL MAYOR NUMERO POSIBLE
	unsigned int ret = EFES;
	
	
	for (cont = 0; cont < memoria.cantidadvias; cont++)
	{
		
		struct sVia viacandidata = bloque.vias[cont];
		
		if (viacandidata.cargada == 0)
		{
			ret = cont;
			viaearliest = viacandidata;
		}
		else
		{
			if (ret == EFES)
			{
				ret = cont;
				viaearliest = viacandidata;
			}
			else
			{
				if 
					((viaearliest.cargada == 1)
					&& (viaearliest.time_stamp > viacandidata.time_stamp))
				{
					ret = cont;
					viaearliest = viacandidata;
				}
			}
			
		}
		
		
	}
	
	return ret;
	
}


/*
La funcion is dirty(unsigned int way,unsigned int setnum) debe
devolver el estado del bit D del bloque correspondiente.
*/
unsigned int is_dirty(unsigned int way, unsigned int setnum)
{
	return memoria.Bloque[setnum].vias[way].is_dirty;
}


/*
La funcion write block(unsigned int way, unsigned int
setnum) debe escribir en las posiciones correspondientes de memoria
el contenido del bloque setnum de la via way.
*/
void write_block(unsigned int way, unsigned int setnum)
{
	unsigned int cont; 
	
    struct sVia via = memoria.Bloque[setnum].vias[way];
 
 
	// Desplazo los BITS de TAG y le sumo el numero de bloque
	unsigned int init_memory = via.tag << memoria.bitsindex;
	init_memory += setnum;
	
	// Desplazo los BITS de OFFSET
	init_memory = init_memory << memoria.bitsoffset;
	
	// COPIA LOS DATOS
	for (cont = 0; cont < memoria.capaciadbloques; cont++)
	{
		ByteMemoria[init_memory + cont] = via.datos[cont];
	}
	   

}




/*
La funcion read block(unsigned int blocknum) debe leer el bloque
blocknum de memoria y guardarlo en el lugar que le corresponda en
la memoria cache.
*/

void read_block(unsigned int blocknum)
{
	unsigned int memoria_init = (blocknum << memoria.bitsoffset);
	unsigned int cont = 0;
	
	
	// CONJUNTO Y VIA QUE VAMOS A COPIAR
	unsigned int setnum = find_set(memoria_init);
	unsigned int earliest = find_earliest(setnum);
	
	
	// VA A COPIAR, SI NO HAY UN ULTIMO LO AGREGO EN EL PRIMERO
	if (earliest == EFES)
		earliest = 0;
    
	struct sVia via = memoria.Bloque[setnum].vias[earliest];
 
 
	if (via.is_dirty && via.cargada)
		write_block(earliest, setnum);

 
	// COPIA LOS DATOS
	for (cont = 0; cont < memoria.capaciadbloques; cont++)
	{
		memoria.Bloque[setnum].vias[earliest].datos[cont] = ByteMemoria[memoria_init + cont];
	}
	
	memoria.Bloque[setnum].vias[earliest].is_dirty = 0;
	memoria.Bloque[setnum].vias[earliest].tag = blocknum >> memoria.bitsindex;
	memoria.Bloque[setnum].vias[earliest].time_stamp = memoria.timestampgen++;
	memoria.Bloque[setnum].vias[earliest].cargada = 1;
}



/*
La funcion find_via(unsigned int address) busca la via en la que esta guardada address 
y devuelve 0xFFFFFFFF si no esta
*/
int find_via(unsigned int address) 
{
	unsigned int offset = find_offset(address);
	unsigned int set = find_set(address);
	unsigned int tag = find_tag(address);
	unsigned int encontradoenvia = EFES;

	unsigned int cont = 0;

	// Recorro las vias del conjunto
	for (cont = 0; cont < memoria.cantidadvias; cont++)
	{
		if (memoria.Bloque[set].vias[cont].cargada == 1)
			if (memoria.Bloque[set].vias[cont].tag == tag)
			{
				encontradoenvia = cont;
			}
	}
	
	
	return encontradoenvia;
	
}

/*
La funcion read byte(unsigned int address, char *hit) debe retornar
el valor correspondiente a la posicioon de memoria address,
buscnandolo primero en el cache, y escribir 1 en *hit si es un hit y 0 si
es un miss.
*/
char read_byte(unsigned int address, unsigned char *hit)
{
	unsigned int offset = find_offset(address);
	unsigned int set = find_set(address);
	unsigned int tag = find_tag(address);
	unsigned int block = find_block(address);
	unsigned int encontradoenvia = find_via(address);
	
	memoria.lecturas++;
	

	// SI NO LO ENCUENTRA LO VA A LEER
	if (encontradoenvia == EFES)
	{
		encontradoenvia = find_earliest(set);
		read_block(block);
		
		// MARCA SALIDA
		hit[0] = 0;		
		memoria.misses++;
	}
	else
	{
		
		// MARCA SALIDA
		hit[0] = 1;
	}
	return memoria.Bloque[set].vias[encontradoenvia].datos[offset];
}


/*
La funcion write byte(unsigned int address, char value) debe
escribir el valor value en la posicion correcta del bloque que corresponde
a address, si esta en el cache, y escribir 1 en *hit si es un hit
y 0 si es un miss.
*/
void write_byte(unsigned int address, char value, unsigned char *hit)
{
	unsigned int offset = find_offset(address);
	unsigned int set = find_set(address);
	unsigned int tag = find_tag(address);
	unsigned int block = find_block(address);
	unsigned int encontradoenvia = find_via(address);
	// SI NO LO ENCUENTRA LO VA A LEER
	
	
	memoria.lecturas++;
	if (encontradoenvia == EFES)
	{
		
		memoria.misses++;
		encontradoenvia = find_earliest(set);
		read_block(block);
		
		// MARCA SALIDA
		hit[0] = 0;
	}
	else
	{
		// MARCA SALIDA
		hit[0] = 1;
	}

	memoria.Bloque[set].vias[encontradoenvia].is_dirty = 1;
	memoria.Bloque[set].vias[encontradoenvia].datos[offset] = value;
	
}

/*
La funcion get miss rate() debe devolver el porcentaje de misses
desde que se inicializo el cache.
read byte() y write byte() solo deben interactuar con la memoria
a traves de las otras primitivas.
*/
char get_miss_rate()
{
	return (memoria.misses * 100) /  memoria.lecturas ;
	

}


void mostrarcache()
{
	int cont = 0;
	int contVias = 0;
	
	printf("ESTADO CACHE\n");
	for (cont = 0; cont < memoria.cantidadbloques; cont++)
	{
		printf("%d   ----   ", cont);
		
		for (contVias = 0; contVias < memoria.cantidadvias; contVias++)
		{
			printf("Via %d : ", contVias);
			if (memoria.Bloque[cont].vias[contVias].cargada == 1)
				printf("%d D: %d", memoria.Bloque[cont].vias[contVias].tag, memoria.Bloque[cont].vias[contVias].is_dirty);
			else
				printf("NL");
				
			printf("     ");
		}
	
		printf("\n");
	
	}
	
}


void print_help()
{
			fprintf (stdout, "Usage: \n tp2 -h \n tp2 -V \n tp2 options archivo \nOptions: \n -h, --help  ~Imprime ayuda.\n");
			fprintf (stdout, " -V, --version ~Version del programa.\n -o, --output ~Archivo de salida.\n");
			fprintf (stdout, " -w, --ways ~Cantidad de vias.\n -c, --cachesize ~Cantidad de kilobytes del cache.");
			fprintf (stdout, "\n -b, --blocksize ~Cantidad de bytes del bloque.\nExamples:\n tp2 -w 4 -cs 8 -bs 16 prueba1.mem\n");
	
}

int main(int argc, char *argv[]) {



	char* output_file_name = NULL;
    int ways = -1, block_size = -1, cache_size = -1;
    int i = 0;

	if (argc > 1)
	{
		if (strcmp(argv[1],"-h") == 0 || strcmp(argv[1] , "--help") == 0)
		{
			print_help();		
			return 0;
		}
	}

/*
Se debera incluir la salida que produzca el programa con los siguientes
archivos de prueba, para las siguientes caches: [4 KB, 4WSA, 32bytes] y
[16KB, una va, 128 bytes].
	tp2 -w 4 -c 4 -b 32 prueba5.mem
	tp2 -w 1 -c 16 -b 4 prueba5.mem

*/

    for (i = 1; i < argc-2; i++) 
	{
		if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i] , "--help") == 0) 
		{
			print_help();
			return 0;
		}
		else if (strcmp(argv[i] , "-V") == 0 || strcmp(argv[i] , "--version") == 0) {
			fprintf (stdout, "Version 1.0\n");
			return 0;
		}
		else if (strcmp(argv[i] , "-o") == 0 || strcmp(argv[i] , "--output") == 0) {
			i++;
			output_file_name = argv[i];
		}
		else if (strcmp(argv[i] , "-w") == 0 || strcmp(argv[i] , "--ways") == 0) {
			i++;
			ways = atoi(argv[i]);
		}
		else if (strcmp(argv[i] , "-c") == 0 || strcmp(argv[i] , "--cachesize") == 0) {
			i++;
			int argument = atoi(argv[i]);
			if (argument <= 0) {
				fprintf (stderr, "ERROR: bad argument input, cache size can not be equal or less than 0\n");
				return ERROR;
			}
			int bits = 10 + (log(argument)/log(2));
			cache_size = pow(2, bits);
		}
		else if (strcmp(argv[i] , "-b") == 0 || strcmp(argv[i] , "--blocksize") == 0) {
			i++;
			block_size = atoi(argv[i]);
		}
		else {
			fprintf (stdout, "Use -h or --help if needed.\n");
			return ERROR;
		}
	}

    if (ways <= 0 || cache_size <= 0 || block_size <= 0 ) {
        fprintf (stderr, "ERROR: bad arguments input\n");
        return ERROR;
    }

	memoria.capaciadbloques = block_size;
	memoria.cantidadvias = ways;
    int conjuntos = (cache_size / block_size) / ways;
	memoria.cantidadbloques = conjuntos;
	int bits_offset = log((double)block_size)/log((double)2);
	int bits_index = log((double)conjuntos)/log((double)2);
	memoria.bitsoffset = bits_offset; 
	memoria.bitsindex = bits_index;
    int bits_tag = BITS_DIR_MEM - bits_index - bits_offset;
	memoria.bitstag = bits_tag;
	
	//PRINTS PARA CHEQUEO - ELIMINAR
    fprintf (stdout, "cache de %d Bytes\n", cache_size);
    fprintf (stdout, "#conjuntos %d de %d Bytes cada bloque\n", conjuntos, block_size);
    fprintf (stdout, "#vias %d\n ", ways);
    fprintf (stdout, "%d bits de TAG, %d bits de INDEX, %d bits de OFFSET\n", bits_tag, bits_index, bits_offset);
    
    int BYTE_MEMORIA = pow(2, BITS_DIR_MEM);
	ByteMemoria = malloc(sizeof(char) *  BYTE_MEMORIA);

    char* input_file_name = argv[argc-1]; 
    FILE* input_file = fopen(input_file_name, "r");
    if (!input_file) {
        fprintf(stderr, "Error while opening input file %s\n", input_file_name);
        return 1;
    }

	if (output_file_name) {
		FILE* output_file = fopen(output_file_name, "w");
		if (!output_file) {
			fprintf(stderr, "Error while opening output file %s\n", output_file_name);
			return 1;
		}
		
		/*
		
		ESTO NO ME COMPILA
		stdout = output_file;
		stderr = output_file;
		*/
	}


	// INICIALIZO LA MEMORIA CACHE
    init();	


	char x[6];
	int memory_pos;
	unsigned char hit = 0;
	
	
    while (fscanf(input_file, "%5s", x)!= EOF) 
	{
        if (strcmp(x, "init") == 0)
		{
			// Borra la memoria y la vuelve a crear
			end();
			init();
		
		} 
		else if (strcmp(x, "R") == 0) 
		{
			fscanf(input_file, "%d", &memory_pos);
			//PRINTS PARA CHEQUEO - ELIMINAR
			fprintf(stdout, "R %d\n", memory_pos);
			char data = read_byte(memory_pos, &hit);
			fprintf (stdout, "data: %d", data);
			if (hit == 1) fprintf(stdout, " -HIT\n");
			else fprintf (stdout, " -MISS\n");
		}
		else if (strcmp(x, "W") == 0) 
		{
			fscanf(input_file, "%7s", x);
			char* number = strtok(x, ",");
			memory_pos = atoi(number);
			
			int value;
			fscanf(input_file, "%d", &value);
			//PRINTS PARA CHEQUEO - ELIMINAR
			fprintf(stdout, "W %d %d\n", memory_pos, value);
			
			write_byte(memory_pos, value, &hit);
			if (hit == 1) fprintf(stdout, "HIT\n");
			else fprintf (stdout, "MISS\n");
		}
		else if (strcmp(x, "MR") == 0) 
		{			
			char mr = get_miss_rate();
			fprintf(stdout, "MISS RATE: %d\n", mr);
		}
    }

    fclose(input_file);
	free(ByteMemoria);
	end();

	return 0;
}
