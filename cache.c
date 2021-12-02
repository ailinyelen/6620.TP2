#include <math.h>
#include "cache.h"
#include <stdlib.h>

int blocks_count (int block_size, int ways, int cache_size) {
    return (cache_size / block_size) / ways;
}

int bits_offset (int block_size) {
    return log((double)block_size)/log((double)2);
}

int bits_index (int blocks_count) {
    return log((double)blocks_count)/log((double)2);
}


int begin (int block_size, int ways, int cache_size) {

    cache.capaciadbloques = block_size;
	cache.cantidadvias = ways;
	cache.cantidadbloques = blocks_count(block_size, ways, cache_size);
	cache.bitsoffset = bits_offset(block_size); 
	cache.bitsindex = bits_index(cache.cantidadbloques);
    cache.bitstag = BITS_DIR_MEM - cache.bitsindex - cache.bitsoffset;
	cache.lecturas = 0;
    cache.misses = 0;

	Memoria_principal = malloc(sizeof(char) *  MEMORY_SIZE);
	if (Memoria_principal == NULL) return ERROR;
	
	cache.Bloque = malloc(sizeof(struct sBloque) * cache.cantidadbloques);
	if (cache.Bloque == NULL) {
		free(Memoria_principal);
		return ERROR;
	}

	int cont = 0;
	int contvias = 0;
	
	for (cont = 0; cont < cache.cantidadbloques; cont++)
	{
		cache.Bloque[cont].vias = malloc(sizeof(struct sVia) * cache.cantidadvias);
		if (cache.Bloque[cont].vias == NULL) {
			for (int i = 0; i < cont; i++) free(cache.Bloque[i].vias);
			free(cache.Bloque);
			free(Memoria_principal);
			return ERROR;
		}

		for (contvias = 0; contvias < cache.cantidadvias; contvias++)
		{
			cache.Bloque[cont].vias[contvias].is_dirty = 0;
			cache.Bloque[cont].vias[contvias].cargada = 0;
			cache.Bloque[cont].vias[contvias].tag = 0;
			cache.Bloque[cont].vias[contvias].time_stamp = -1;
			cache.Bloque[cont].vias[contvias].datos = malloc(sizeof(char) * cache.capaciadbloques);

			if (cache.Bloque[cont].vias[contvias].datos == NULL) {
				for (int i = 0; i < cont; i++) {
					for (int j = 0; j < contvias; j++) {
						free(cache.Bloque[i].vias[j].datos);
					}
					free(cache.Bloque[i].vias);
				}
				free(cache.Bloque);
				free(Memoria_principal);
				return ERROR;
			}
		}
	}
	return OK;
}


/*
funcion init() debe inicializar los bloques de la cache como invalidos,
la memoria simulada en 0 y la tasa de misses a 0.
*/
void init() {

	for (int i = 0; i < MEMORY_SIZE; i++) {
		Memoria_principal[i] = 0;
	}

	for (int cont = 0; cont < cache.cantidadbloques; cont++)
	{
		for (int contvias = 0; contvias < cache.cantidadvias; contvias++)
		{
			cache.Bloque[cont].vias[contvias].is_dirty = 0;
			cache.Bloque[cont].vias[contvias].cargada = 0;
			cache.Bloque[cont].vias[contvias].tag = 0;
			cache.Bloque[cont].vias[contvias].time_stamp = -1;
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
	
	for (cont = 0; cont < cache.cantidadbloques; cont++)
	{
		
		for (contvias = 0; contvias < cache.cantidadvias; contvias++)
		{
			
			free(cache.Bloque[cont].vias[contvias].datos);
		}
		
		free(cache.Bloque[cont].vias);
		
	}
	free(cache.Bloque);
    free(Memoria_principal);
}

void free_memory () {
	
	end();
	free(Memoria_principal);
	return;
}

/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
*/
unsigned int find_offset(unsigned int address)
{
	
	unsigned int efes = EFES;
	efes = (efes >> (32 - cache.bitsoffset));
	return address & efes;	
}




/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
*/
unsigned int find_set(unsigned int address)
{
	unsigned int sinoffset = (address >> cache.bitsoffset);
	unsigned int efes = EFES;
	efes = (efes >> (32 - cache.bitsindex));
	unsigned int ret = efes & sinoffset;
	return ret;
}


/*
La funcion find set(unsigned int address) debe devolver el conjunto
de cache al que mapea la direccion address.
*/
unsigned int find_tag(unsigned int address)
{	
	return (address >> (cache.bitsoffset + cache.bitsindex));	
}



/*
La funcion find block(unsigned int address) debe devolver el bloque
de memoria correspondiente a la direccion address.
*/

unsigned int find_block (unsigned int address)
{
	/// EL INDICE DE BLOQUE EN MEMORIA FISICA? QUE DIFERENCIA HAY CON find_set
	unsigned int sinoffset = (address >> cache.bitsoffset);
	return sinoffset;
}



/*
La funcion find earliest(unsigned int setnum) debe devolver el
bloque mas 'antiguo' dentro de un conjunto, utilizando el campo correspondiente
de los metadatos de los bloques del conjunto.
*/
unsigned int find_earliest(unsigned int setnum)
{
	struct sBloque bloque = cache.Bloque[setnum];
	int cont = 0;
	struct sVia viaearliest;

	// LE ASIGNA EL MAYOR NUMERO POSIBLE
	unsigned int ret = EFES;
	
	
	for (cont = 0; cont < cache.cantidadvias; cont++)
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
	return cache.Bloque[setnum].vias[way].is_dirty;
}


/*
La funcion write block(unsigned int way, unsigned int
setnum) debe escribir en las posiciones correspondientes de memoria
el contenido del bloque setnum de la via way.
*/
void write_block(unsigned int way, unsigned int setnum)
{
	unsigned int cont; 
	
    struct sVia via = cache.Bloque[setnum].vias[way];
 
 
	// Desplazo los BITS de TAG y le sumo el numero de bloque
	unsigned int init_memory = via.tag << cache.bitsindex;
	init_memory += setnum;
	
	// Desplazo los BITS de OFFSET
	init_memory = init_memory << cache.bitsoffset;
	
	// COPIA LOS DATOS
	for (cont = 0; cont < cache.capaciadbloques; cont++)
	{
		Memoria_principal[init_memory + cont] = via.datos[cont];
	}
	   

}




/*
La funcion read block(unsigned int blocknum) debe leer el bloque
blocknum de memoria y guardarlo en el lugar que le corresponda en
la memoria cache.
*/

void read_block(unsigned int blocknum)
{
	unsigned int memoria_init = (blocknum << cache.bitsoffset);
	unsigned int cont = 0;
	
	
	// CONJUNTO Y VIA QUE VAMOS A COPIAR
	unsigned int setnum = find_set(memoria_init);
	unsigned int earliest = find_earliest(setnum);
	
	
	// VA A COPIAR, SI NO HAY UN ULTIMO LO AGREGO EN EL PRIMERO
	if (earliest == EFES)
		earliest = 0;
    
	struct sVia via = cache.Bloque[setnum].vias[earliest];
 
 
	if (via.is_dirty && via.cargada)
		write_block(earliest, setnum);

 
	// COPIA LOS DATOS
	for (cont = 0; cont < cache.capaciadbloques; cont++)
	{
		cache.Bloque[setnum].vias[earliest].datos[cont] = Memoria_principal[memoria_init + cont];
	}
	
	cache.Bloque[setnum].vias[earliest].is_dirty = 0;
	cache.Bloque[setnum].vias[earliest].tag = blocknum >> cache.bitsindex;
	cache.Bloque[setnum].vias[earliest].time_stamp = cache.timestampgen++;
	cache.Bloque[setnum].vias[earliest].cargada = 1;
}



/*
La funcion find_via(unsigned int address) busca la via en la que esta guardada address 
y devuelve 0xFFFFFFFF si no esta
*/
int find_via(unsigned int address) 
{
	unsigned int set = find_set(address);
	unsigned int tag = find_tag(address);
	unsigned int encontradoenvia = EFES;

	unsigned int cont = 0;

	// Recorro las vias del conjunto
	for (cont = 0; cont < cache.cantidadvias; cont++)
	{
		if (cache.Bloque[set].vias[cont].cargada == 1)
			if (cache.Bloque[set].vias[cont].tag == tag)
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
	unsigned int block = find_block(address);
	unsigned int encontradoenvia = find_via(address);
	
	cache.lecturas++;
	

	// SI NO LO ENCUENTRA LO VA A LEER
	if (encontradoenvia == EFES)
	{
		encontradoenvia = find_earliest(set);
		read_block(block);
		
		// MARCA SALIDA
		hit[0] = 0;		
		cache.misses++;
	}
	else
	{
		
		// MARCA SALIDA
		hit[0] = 1;
	}
	return cache.Bloque[set].vias[encontradoenvia].datos[offset];
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
	unsigned int block = find_block(address);
	unsigned int encontradoenvia = find_via(address);
	// SI NO LO ENCUENTRA LO VA A LEER
	
	
	cache.lecturas++;
	if (encontradoenvia == EFES)
	{
		
		cache.misses++;
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

	cache.Bloque[set].vias[encontradoenvia].is_dirty = 1;
	cache.Bloque[set].vias[encontradoenvia].datos[offset] = value;
	
}

/*
La funcion get miss rate() debe devolver el porcentaje de misses
desde que se inicializo el cache.
read byte() y write byte() solo deben interactuar con la memoria
a traves de las otras primitivas.
*/
char get_miss_rate()
{
	if (cache.lecturas == 0) return NO_MISS_RATE;
	return (cache.misses * 100) /  cache.lecturas ;
}