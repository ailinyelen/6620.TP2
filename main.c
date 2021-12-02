#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include "cache.h"



void print_help() {
	fprintf (stdout, "Usage: \n tp2 -h \n tp2 -V \n tp2 options archivo \nOptions: \n -h, --help  ~Imprime ayuda.\n");
	fprintf (stdout, " -V, --version ~Version del programa.\n -o, --output ~Archivo de salida.\n");
	fprintf (stdout, " -w, --ways ~Cantidad de vias.\n -c, --cachesize ~Cantidad de kilobytes del cache.");
	fprintf (stdout, "\n -b, --blocksize ~Cantidad de bytes del bloque.\nExamples:\n tp2 -w 4 -cs 8 -bs 16 prueba1.mem\n");
	
}

int main(int argc, char *argv[]) {

	char* output_file_name = NULL;
	FILE* output_file = NULL;
    int ways = -1, block_size = -1, cache_size = -1;

	while (1) {

		static struct option long_options[] =
		{
		{"help",     no_argument,       0, 'h'},
		{"version",     no_argument,       0, 'V'},
		{"output",  required_argument, 0, 'o'},
		{"ways",  required_argument, 0, 'w'},
		{"cachesize",  required_argument, 0, 'c'},
		{"blocksize",  required_argument, 0, 'b'},
		{0, 0, 0, 0}
		};

		int option_index = 0;

		int opt = getopt_long (argc, argv, "hVo:w:c:b:", long_options, &option_index);

		if (opt == -1) {
			break;
		}
	
		switch (opt){
			case 'h':
				print_help();
				return OK;
				break;

			case 'V':
				fprintf (stdout, "Version 1.0\n");
				return OK;
				break;

			case 'o': ;
				output_file_name = optarg;
				break;
			
			case 'w': 
				ways = atoi(optarg);
				break;

				case 'c': ;
				int argument = atoi(optarg);
				if (argument <= 0) {
					fprintf (stderr, "ERROR: bad argument input, cache size can not be equal or less than 0\n");
					return ERROR;
				}
				if (argument > MEMORY_SIZE) {
					fprintf (stderr, "ERROR: bad argument input, cache size can not be larger than memory size (64KB)\n");
					return ERROR;
				}

				int bits = 10 + (log(argument)/log(2));
				cache_size = pow(2, bits);
				break;

			case 'b': 
				block_size = atoi(optarg);
				break;
			
			default:
				fprintf (stderr, "Use -h or --help if needed.\n");
				return ERROR;
		}

	};

    if (ways <= 0 || cache_size <= 0 || block_size <= 0 ) {
        fprintf (stderr, "ERROR: missing arguments. Use -h for help.\n");
        return ERROR;
    }

	if (block_size > cache_size) {
		fprintf (stderr, "ERROR: bad argument input, block size can not be larger than cache size\n");
		return ERROR;
	}

	int blocks = cache_size / block_size;

	//chequeo que no me queden bloques fuera de las vias
	if ((blocks % ways != 0)) {
		fprintf (stderr, "ERROR: bad argument input, relation between block size and ways count can not be applied\n");
		return ERROR;
	}

	if (optind >= argc) {
		fprintf(stderr, "ERROR: missing input file\n");
        return ERROR;
	}

    char* input_file_name = argv[optind];
    FILE* input_file = fopen(input_file_name, "r");
    if (!input_file) {
        fprintf(stderr, "Error while opening input file %s\n", input_file_name);
        return ERROR;
    }

	if (output_file_name) {
		output_file = fopen(output_file_name, "w");
		if (!output_file) {
			fprintf(stderr, "Error while opening output file %s\n", output_file_name);
			fclose(input_file);
			return ERROR;
		}
		
		stdout = output_file;
		stderr = output_file;
	}

	int error = begin(block_size, ways, cache_size);
	if (error) {
		fprintf(stderr, "Memory Error\n");
		fclose(input_file);
		if (output_file) fclose(output_file);
		return ERROR;
	}


	char x[6];
	int memory_pos = 0;
	unsigned char hit = 0;
	
	
    while (fscanf(input_file, "%5s", x) != EOF) 
	{
        if (strcmp(x, "init") == 0) init();
		
		else if (strcmp(x, "R") == 0) 
		{
			fscanf(input_file, "%d", &memory_pos);
			if (memory_pos < MEMORY_SIZE) {

				char data = read_byte(memory_pos, &hit);

				fprintf (stdout, "READ address %d, data %d", memory_pos, data);
				
				if (hit == 1) fprintf(stdout, " - HIT\n");
				else fprintf (stdout, " - MISS\n");
			}
			else fprintf (stderr, "READ: error pos %d dont exists\n", memory_pos);
		}
		else if (strcmp(x, "W") == 0) 
		{
			fscanf(input_file, "%7s", x);
			char* number = strtok(x, ",");
			memory_pos = atoi(number);

			if (memory_pos < MEMORY_SIZE) {		

				int value;
				fscanf(input_file, "%d", &value);

				fprintf(stdout, "WRITE address %d data %d", memory_pos, value);
				
				write_byte(memory_pos, value, &hit);
				if (hit == 1) fprintf(stdout, " - HIT\n");
				else fprintf (stdout, " - MISS\n");
			}
			else fprintf (stderr, "WRITE: error pos %d dont exists\n", memory_pos);
		}
		else if (strcmp(x, "MR") == 0) 
		{			
			char mr = get_miss_rate();
			if (mr == NO_MISS_RATE) fprintf(stdout, "MISS RATE: NO DATA\n");
			else fprintf(stdout, "MISS RATE: %d\n", mr);
		}
    }

    fclose(input_file);
	if (output_file) fclose(output_file);

	end();

	return OK;
}
