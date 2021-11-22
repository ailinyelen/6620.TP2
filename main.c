#include <stdio.h>
#include <stdlib.h>
#define BYTE_MEMORIA 1024


/*
	Instalador Word to Telax
	
	https://www.grindeq.com/index.php?p=download&lang=en
	
	
	
	https://www.youtube.com/watch?v=nMjpFHBg_1Q
	https://getintopc.com/softwares/utilities/grindeq-math-utilities-2020-free-download/
*/
// PAGINA SALVADORA,
// https://products.aspose.app/pdf/es/conversion/docx-to-tex


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
de cache al que mapea la direccion address.
*/
unsigned int find_offset(unsigned int address)
{
	
	unsigned int efes = EFES;
	efes = (efes >> (32 - memoria.bitsoffset));
	return address & efes;	
}




/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
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
de cache al que mapea la direccion address.
*/
unsigned int find_tag(unsigned int address)
{	
	return (address >> (memoria.bitsoffset + memoria.bitsindex));	
}



/*
La funcion find block(unsigned int address) debe devolver el bloque
de memoria correspondiente a la direccion address.
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
escribir el valor value en la posicion correcta del bloque que corresponde
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
		hit[0] = 1;
	}
	else
	{
		// MARCA SALIDA
		hit[0] = 0;
	}

	memoria.Bloque[set].vias[encontradoenvia].is_dirty = 1;
	memoria.Bloque[set].vias[encontradoenvia].datos[offset] = value;
	
}

/*
La funcion get miss rate() debe devolver el porcentaje de misses
desde que se inicializo el cache.
read byte() y write byte() solo deben interactuar con la memoria
a traves de las otras primitivas.
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



int main(int argc, char *argv[]) 
{

	char miss = 0;
	memoria.bitsoffset = 4;
	memoria.capaciadbloques = 16;
	memoria.bitsindex = 4;
	memoria.cantidadbloques = 16;
	memoria.cantidadvias = 2;
	memoria.bitstag = 24;
	
	
	ByteMemoria = malloc(sizeof(char) * BYTE_MEMORIA);

	init();
	
	write_byte(256, 1, &miss);
	//read_byte(256, &miss);
	read_byte(512, &miss);
	read_byte(1024, &miss);
	
	mostrarcache();
	
	
	/*
	mostrarcache();
	read_byte(0, &miss);
	
	read_byte(256, &miss);
	mostrarcache();
	
	read_byte(512, &miss);
	mostrarcache();
	read_byte(768, &miss);
	mostrarcache();
	
	read_byte(1024, &miss);
	mostrarcache();
	*/
	/*

	UN MISS Y UN HIT
	---------------
	
	read_byte(0, &miss);
	read_byte(1, &miss);
	end();
	*/
	
	free(ByteMemoria);
	/*
	*/
	
	



	return 0;
}

