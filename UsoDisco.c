/*
	Universidad Simón Bolívar
	Departamento de Computación y Tecnología de la Información
	Sistemas de Operación I (CI3825)

	Profesores:				Emely Arráiz 		(teoría)
							Ángela Di Serio		(práctica)
							Carlos Gómez		(práctica)

	Segundo proyecto		Comando "UsoDisco", verifica la cantidad de bloques ocupados en disco
							por un directorio utilizando programación multithreads

	Autores:				Guillermo Betancourt,	carnet 11-10103
							Gabriel Giménez,		carnet 12-11006

	Última modificación:	27/03/2016 - 1:18am
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include "dir_queue.h"

/*
	Se define un tamaño máximo para los flags del comando ./UsoDisco, dos espacios para el flag
	y un espacio para el caracter nulo '\0'
*/
#define MAX_CMD 3

/*
	Se define un tamaño máximo el slash, un espacio para el caracter '\' y un espacio para el
	caracter nulo '\0'
*/
#define SLASH_SIZE 2

/*
	Estructura cmd_param: contiene los parámetros al ejecutar el comando ./UsoDisco
	
	Miembros:	- int multith:		Número de threads a utilizar (por default es 1)
				- char *dir:		Apuntador a un espacio de tamaño PATH_MAX bytes (ver header
									limits.h) donde se almacena el path del directorio inicial
									(por default es donde esté guardado el archivo UsoDisco.c)
				- char *outfile:	Apuntador a un espacio de tamaño PATH_MAX bytes (ver header
									limits.h) donde se almacena el nombre del archivo de salida
									(por default es /dev/stdout, es decir, la salida standard
									por terminal)

	Alias: cmd_param
*/
typedef struct cmd_param {
	int multith;
	char *dir;
	char *outfile;
} cmd_param;

/*
	Estructura thread_param: contiene los parámetros que necesita cada thread para ejecutar la
	revisión de cada directorio que se le asigne

	Miembros:	- short int busy_thread:		Entero de 2 bytes que indica si el thread está
												ocupado, si está libre, o si ya puede terminar
				- FILE *fd:						Apuntador a estructura FILE (ver header
												stdio.h) que almacena el descriptor del archivo
												de salida
				- char *dir:					Apuntador a un espacio de tamaño PATH_MAX (ver
												header limits.h) donde se almacena el path del
												directorio a ser examinado
				- dir_queue *return_queue:		Apuntador a estructura dir_queue (ver header
												dir_queue.h) donde se almacenarán los
												subdirectorios del directorio a examinar en
												caso de que este último los tenga. Esta
												cola se retorna al hilo padre para que sus
												elementos sean encolados en una cola principal
				- long long int used_blocks:	Entero de 8 bytes donde se retorna el número
												de bloques que ocupan todos los archivos
												inmediatos del directorio examinado
				- pthread_cond_t thread cond:	Estructura pthread_cond_t (ver header pthreads.h)
												donde se almacena la variable de condición de
												cada thread, manejada a través de un semáforo
												único para todo el código

	Alias: thread_param
*/
typedef struct thread_param {
	short int busy_thread;
	FILE *fd;
	char *dir;
	dir_queue *return_queue;
	long long int used_blocks;
	pthread_cond_t thread_cond;
} thread_param;

// Variable global char *slash: se utiliza para poder concatenar correctamente los nombres de los directorios
static char *slash;

/*
	Variable global pthread_mutex_t mutex:	Estructura pthread_mutex_t (ver header pthreads.h) que representa
											el semáforo global a utilizar por el proceso principal y los
											threads
*/
pthread_mutex_t mutex;

/*
	Procedimiento usoDiscoHelp():	Al ejecutarse, despliga la ayuda para la utilización comando ./UsoDisco
*/
void usoDiscoHelp() {
	printf("UsoDisco: ./UsoDisco [-h] | [-n hilos] [-o salida] [-d directorio]\
		\n\n\tCalcula de forma concurrente el espacio en disco utilizado por los\
		\n\tarchivos regulares a partir de un directorio usando hilos\
		\n\n\tOpciones:\
		\n\t\t-h\tmuestra la presente ayuda para utilizar el comando\
		\n\t\t-n\tnúmero de hilos a utilizar para el cálculo del\
		\n\t\t\tespacio ocupado\
		\n\t\t-o\tnombre del archivo de salida con la información\
		\n\t\t\tcalculada por el comando. Si no se especifica un\
		\n\t\t\tarchivo de salida, se imprime por terminal\
		\n\t\t-d\tdirectorio inicial para calcular el espacio de\
		\n\t\t\tuso del disco. Si no se especifica un directorio\
		\n\t\t\tse toma como directorio predeterminado donde esté\
		\n\t\t\tel archivo UsoDisco.c\n");
}

/*
	Procedimiento invalid_args():	Al ejecutarse, imprime un mensaje que indica los flags del comando
									./UsoDisco son incorrectos y finaliza la ejecución
*/
void invalid_args() {
	printf("Argumentos inválidos. El programa abortará ahora.\n");
	exit(1);
}

/*
	Función check_dir():				Función que examina un directorio. El path del directorio es
										abierto, y se itera sobre cada uno de sus archivos, se verifica
										qué tipo de archivo es, cuántos bloques ocupa y su path completo.
										Finalmente, se cierra el directorio

	Parámetros:	- char *top_dir:		Apuntador a espacio de tamaño PATH_MAX bytes(ver header
										limits.h) donde se almacena el path del directorio a ser
										examinado
				- FILE *fd:				Apuntado a estructura FILE (ver header stdio.h) donde se
										almacena el descriptor del archivo de salida
				- dir_queue *ret_queue:	Apuntador a estructura dir_queue (ver header dir_queue.h)
										donde se almacenarán los subdirectorios de top_dir en
										caso de que este últmo los tenga
				- char* outfile:		Apuntador a espacio de tamaño PATH_MAX bytes(ver header
										limits.h) donde se almacena el nombre del archivo de salida.
										Este parámetro funciona únicamente para evitar la impresión
										del archivo de salida al momento de examinar el directorio
										principal, ya que dicho archivo está siendo modificado y no
										se sabrá la cantidad de bloques que ocupa antes de finalizar
										la ejecución del programa

	Retorna:	Un entero de 8 bytes con los bloques que ocupan todos los archivos inmediatos de top_dir

*/
long long int check_dir(char *top_dir, FILE *fd, dir_queue *ret_queue, char* outfile) {
	long long int total_blocks_checked = 0;
	DIR *head_dir;
	struct dirent *dirent_ptr;
	struct stat *inode = malloc(sizeof(struct stat));
	char *real_path = (char*)malloc(PATH_MAX);
	char *tmp_path = (char*)malloc(PATH_MAX);
	
	memset(real_path, 0, PATH_MAX);
	memset(tmp_path, 0, PATH_MAX);
	if ((head_dir = opendir(top_dir)) == NULL) {
		printf("Error al abrir el directorio %s\n", top_dir);
		exit(1);
	};
	while ((dirent_ptr = readdir(head_dir)) != NULL) {
		if (!strcmp(dirent_ptr -> d_name, ".") || !strcmp(dirent_ptr -> d_name, "..") || (outfile != NULL && !strcmp(outfile, dirent_ptr -> d_name))) {
			continue;
		};
		strcpy(real_path, top_dir);
		if (real_path[strlen(real_path) - 1] != '/') {
			strcat(real_path, slash);
		}
		strcat(real_path, dirent_ptr -> d_name);
		if (stat(real_path, inode)) {
			printf("Error al verificar el archivo %s en el directorio %s\n", dirent_ptr -> d_name, top_dir);
			if (fclose(fd)) {
				printf("Error al cerrar el archivo de salida\n");
			}
			exit(1);
		}
		// pthread_mutex_lock(&mutex);
		switch ((inode -> st_mode) & S_IFMT) {
			case S_IFBLK: 
				fputs("(BLK)\t\t", fd);
				break;
			case S_IFCHR:
				fputs("(CHR)\t\t", fd);
				break;
			case S_IFDIR:
				fputs("(DIR)\t\t", fd);
				strcpy(tmp_path, real_path);
				strcat(tmp_path, slash);
				push(ret_queue, tmp_path);
				break;
			case S_IFIFO:
				fputs("(FIFO)\t\t", fd);
				break;
			case S_IFLNK:
				fputs("(LNK)\t\t", fd);
				break;
			case S_IFREG:
				fputs("(REG)\t\t", fd);
				break;
			case S_IFSOCK:
				fputs("(SOCK)\t\t", fd);
				break;
			default:
				fputs("(UKW)\t\t", fd);
				break;
		}
		if (S_ISBLK(inode -> st_mode) || S_ISCHR(inode -> st_mode)) {
			fprintf(fd, "(nul)\t\t");
		}
		else if (S_ISDIR(inode -> st_mode)) {
			fprintf(fd, "%lld (inodo)\t", (long long)(inode -> st_blocks));
			total_blocks_checked += inode -> st_blocks;
		}
		else {
			fprintf(fd, "%lld\t\t", (long long)(inode -> st_blocks));
			total_blocks_checked += inode -> st_blocks;
		}
		fprintf(fd, "%s\n", real_path);
		// pthread_mutex_unlock(&mutex);
	}
	if (closedir(head_dir)) {
		printf("Error al cerrar el directorio %s\n", top_dir);
		exit(1);
	}
	// Comentar este IF si se activan utilizan los semáforos en check_dir(), puesto que el orden no está garantizado
	if (outfile == NULL) {
		fprintf(fd, "%lld bloques ocupados en el directorio %s\n\n", total_blocks_checked, top_dir);
	}
	dirent_ptr = NULL;
	free(tmp_path);
	free(real_path);
	free(inode);
	return total_blocks_checked;
}

/*
	Procedimiento thread_work():	Al ejecutarse, el thread se alterna entre ocupado y libre, hasta que es
									llamado para finalizar. Si el thread está libre, espera a que su variable
									de condición sea activada por el proceso principal. Si se activa la variable
									de condición significa que hay un nuevo directorio a ser examinado, o que el
									thread está siendo llamado para finalizar.

	Parámetros:	- void *th_param:	Apuntador de tipo vacío a una dirección de memoria donde comienza una
									estructura de tipo thread_param (ver definición de la estructura
									thread_param al inicio del código). Se debe convertir (cast) dicho apuntador
									al tipo thread_param
*/
void *thread_work(void *th_param) {
	while (((thread_param*)th_param) -> busy_thread != -1) {
		pthread_mutex_lock(&mutex);
		if (((thread_param*)th_param) -> busy_thread == 0) {
			pthread_cond_wait(&(((thread_param*)th_param) -> thread_cond), &mutex);
		}
		pthread_mutex_unlock(&mutex);
		if (((thread_param*)th_param) -> busy_thread == 1) {
			pthread_mutex_lock(&mutex); // Comentar este lock si se usan los semáforos dentro de check_dir()
			((thread_param*)th_param) -> used_blocks = check_dir(((thread_param*)th_param) -> dir, ((thread_param*)th_param) -> fd, ((thread_param*)th_param) -> return_queue, NULL);
			((thread_param*)th_param) -> busy_thread = 0;
			pthread_mutex_unlock(&mutex); // Comentar este unlock si se usan los semáforos dentro de check_dir()
		}
	}
	return(NULL);
}

/*
	Procedimiento disk_check():		Al ejecutarse, el proceso principal examina el directorio principal, encola
									sus subdirectorios en caso de que los tenga, e inicializa los parámetros de
									cada thread con un directorio de la cola. Luego crea todos los threads a
									utilizar y verifica siempre si algún thread retornó nuevo directorios para
									ser encolados, le asigna un directorio nuevo y sigue monitoreando todos los
									thread. El proceso finaliza cuando todos los threads están libres, no tienen
									directorios nuevos para ser encolados y la cola principal está vacía

	Parámetros:	- cmd_param *init:	Apuntador a estructura cmd_param (ver definición de a estructura cmd_param al
									inicio del código) donde se almacenan los flags con los que fue ejecutado el
									comando  ./UsoDisco
*/
void disk_check(cmd_param *init) {
	FILE *fd;
	dir_queue cola = {.size = 0, .front = NULL, .back = NULL};
	thread_param *th_param[init -> multith];
	node *tmp_node;
	short int done_check = 0;
	pthread_t threads[init -> multith];
	char *outfile_path = (char*)malloc(PATH_MAX);
	int i, pth;
	long long int total_blocks = 0;
	void *status;
	clock_t start = clock();
	
	pthread_mutex_init(&mutex, NULL);
	th_param[0] = (thread_param*)malloc(sizeof(thread_param));
	th_param[0] -> return_queue = (dir_queue*)malloc(sizeof(dir_queue));
	th_param[0] -> dir = (char*)malloc(PATH_MAX);
	if (!strcmp(init -> outfile, "/dev/stdout")) {
		strcpy(outfile_path, "/dev/stdout");
	}
	else {
		strcpy(outfile_path, init -> dir);
		strcat(outfile_path, init -> outfile);
	}
	if ((fd = fopen(outfile_path, "w+")) == NULL) {
		printf("Error al abrir el archivo\n");
		exit(1);
	}
	fprintf(fd, "\nDIRECTORIO PRINCIPAL: %s\n\n", init -> dir);
	fputs("TIPO\t\tBLOQUES\t\tPATH\n\n", fd);
	total_blocks += check_dir(init -> dir, fd, th_param[0] -> return_queue, init -> outfile);
	if (queue_size(th_param[0] -> return_queue)) {
		fputs("\n---------------------------------------------------------------------------------------------------------\n\n", fd);
		fprintf(fd, "Subdirectorios de %s:\n\n\n", init -> dir);
		move_nodes(&cola, th_param[0] -> return_queue);
	}
	else {
		fputs("El directorio principal no tiene subdirectorios a examinar\n\n", fd);
	}
	free(th_param[0] -> dir);
	free(th_param[0] -> return_queue);
	free(th_param[0]);
	for (i = 0; i < init -> multith; i++) {
		th_param[i] = (thread_param*)malloc(sizeof(thread_param));
		th_param[i] -> busy_thread = 0;
		th_param[i] -> used_blocks = 0;
		th_param[i] -> fd = fd;
		th_param[i] -> dir = (char*)malloc(PATH_MAX);
		th_param[i] -> return_queue = (dir_queue*)malloc(sizeof(dir_queue));
		th_param[i] -> return_queue -> size = 0;
		th_param[i] -> return_queue -> front = NULL;
		th_param[i] -> return_queue -> back = NULL;
		pthread_cond_init(&(th_param[i] -> thread_cond), NULL);
	}
	for (i = 0; i < init -> multith; i++) {
		if (pth = pthread_create(&threads[i], NULL, &thread_work, th_param[i])) {
			printf("Error número %d al ejecutar pthread_create()\n", pth);
			if (fclose(fd)) {
				printf("Error al cerrar el archivo de salida\n");
			}
			exit(1);
		}
	}
	while(!done_check) {
		for (i = 0; i < init -> multith; i++) {
			if (!(th_param[i] -> busy_thread)) {
				pthread_mutex_lock(&mutex);
				move_nodes(&cola, th_param[i] -> return_queue);
				total_blocks += th_param[i] -> used_blocks;
				th_param[i] -> used_blocks = 0;
				if (queue_size(&cola)) {
					tmp_node = pop(&cola);
					strcpy(th_param[i] -> dir, node_path(tmp_node));;
					delete_node(tmp_node);
					th_param[i] -> busy_thread = 1;
					pthread_cond_signal(&(th_param[i] -> thread_cond));
				}
				pthread_mutex_unlock(&mutex);
			}
		}
		pthread_mutex_lock(&mutex);
		done_check = 1;
		for (i = 0; i < init -> multith; i++) {
			done_check = done_check & (th_param[i] -> busy_thread == 0) & (queue_size(th_param[i]-> return_queue) == 0);
		}
		if (done_check && queue_size(&cola)) {
			done_check = 0;
		}
		else if (done_check && !queue_size(&cola)) {
			done_check = 1;
		}
		pthread_mutex_unlock(&mutex);
	}
	for (i = 0; i < init -> multith; i++) {
		th_param[i] -> busy_thread = -1;
		pthread_cond_signal(&(th_param[i] -> thread_cond));
		if (pth = pthread_join(threads[i], NULL)) {
			printf("Error número %d al ejecutar pthread_join()\n", pth);
			if (fclose(fd)) {
				printf("Error al cerrar el archivo de salida\n");
			}
			exit(1);
		}
		total_blocks += th_param[i] -> used_blocks;
	}
	fprintf(fd, "%lld bloques ocupados desde el directorio %s\n", total_blocks, init -> dir);
	fprintf(fd, "%f segundos utilizados para la exploración\n\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	if (strcmp(init -> outfile, "/dev/stdout")) {
		printf("\nEl archivo de salida \"%s\" ha sigo guardado exitosamente en el directorio %s\n\n", init -> outfile, init -> dir);
	}
	if (fclose(fd)) {
		printf("Error al cerrar el archivo\n");
		exit(1);
	}
}

/*
	Procedimiento main():			Código que se ejecuta el iniciar el programa. Verifica los flags
									con los que se quiere ejecutar el comando ./UsoDisco, si los flags
									son válidos, se procede a ejecutar las funciones asociadas al comando.
									En caso contrario, se ejecuta invalid_args()

	Parámetros:	- int argc:			Cantidad de argumentos o flags con los que se desea ejecutar
									el comando ./UsoDisco
				- char* argv[]:		Arreglo de apuntadores a caracteres, donde se almacenan los
									argumentos o flags
*/

void main (int argc, char *argv[]) {
	char cmd[MAX_CMD];
	memset(cmd, 0, sizeof(cmd));
	slash = (char*)malloc(SLASH_SIZE);
	memset(slash, 0, 2);
	slash[0] = '/';
	cmd_param init = {.multith = -1, .outfile = NULL, .dir = NULL};

	if (argc == 2) {
		strcpy(cmd, argv[1]);
		if (!strcmp(cmd,"-h")) {
			usoDiscoHelp();
			exit(0);
		}
		else if (strcmp(cmd,"-h")) {
			invalid_args();
		}
	}
	else if (argc >= 1) {
		int i;
		for (i = 1; i < argc; i = i+2) {
			strcpy(cmd, argv[i]);
			if (!strcmp(cmd, "-n") && init.multith == -1 && atoi(argv[i+1]) > 0) {
				init.multith = atoi(argv[i+1]);
			}
			else if (!strcmp(cmd, "-o") && init.outfile == NULL) {
				init.outfile = argv[i+1];
			}
			else if (!strcmp(cmd, "-d") && init.dir == NULL) {
				init.dir = argv[i+1];
			}
			else {
				invalid_args();
			}
		}
		if (init.dir == NULL) {;
			init.dir = (char*)malloc(PATH_MAX);
			getcwd(init.dir, PATH_MAX);
			strcat(init.dir, slash);
		}
		if (init.outfile == NULL) {
			init.outfile = (char*)malloc(PATH_MAX);
			strcpy(init.outfile, "/dev/stdout");
		}
		if (init.multith == -1) {
			init.multith = 1;
		}
	}
	else {
		invalid_args();
	}
	disk_check(&init);	
}
