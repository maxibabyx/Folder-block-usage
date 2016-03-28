#ifndef directories_queue
#define directories_queue

/*
	Estructura node:			Nodo de uno lista enlazada, con el path de un directorio y
								apuntadores a sus nodos vecinos.

	Miembros:	- char *path:	Apuntador a cararteres, donde se guarda el path del directorio
				- node *next:	Apuntador a un a una estructura node, que indica el nodo vecino
								siguiente
				- char *prev:	Apuntador a un a una estructura node, que indica el nodo vecino
								previo

	Alias: node
*/
typedef struct node {
	char *path;
	struct node *next, *prev;
} node;


/*
	Estructura dir_queue		Lista doblemente enlazada con apuntadores al primer y al último
								elemento

	Miembros:	- node *front:	Apuntador a estructura node, que indica el elemento al frente de
								la cola
				- node *back:	Apuntador a estructura node, que indica el elemento al final de
								la cola
				- int size:		Entero que guarda la cantidad de elementos que posee la cola

	Alias: dir_queue
*/
typedef struct dir_queue {
	node *front, *back;
	int size;
} dir_queue;

/*
	Procedimiento push():			Al ejectuarse, crea un nuevo nodo, y hace una copia del path
									pasado como argumento y se lo asigna al nodo y agrega al nodo
									al final de la cola. Aumenta el tamaño de la cola en 1

	Parámetros:	- dir_queue *q:		Apuntador a una estructura dir_queue
				- char *push_path:	Apuntador a caracteres
*/
void push (dir_queue *q, char *push_path);

/*
	Función push():				Al ejecutarse, crea una referencia a la cabeza de la cola, actualiza
								la cabeza de la cola con el segundo elemento. Disminuye el tamaño de
								la cola en 1

	Parámetros:	- dir_queue *q:	Apuntador a una estructura dir_queue

	Retorna:	- node *tmp:	Apuntador a la antigua cabeza de la cola q
*/
node * pop (dir_queue *q);

/*
	Función push():						Al ejecutarse, devuelve el miembro 'path' de una estructura node

	Parámetros:	- node *n:				Apuntador a una estructura node

	Retorna:	- char *(tmp -> path):	Apuntador al path del nodo n
*/
char * node_path(node *n);

/*
	Función queue_size():			Al ejecutarse, devuelve el miembro 'size' de una estructura dir_queue

	Parámetros:	- dir_queue *q:		Apuntador a una estructura dir_queue

	Retorna:	- int (q- > size):	Entero con el tamaño de la cola q
*/
int queue_size (dir_queue *q);

/*
	Procedimiento delete_node():	Al ejecutarse, elimina toda la información de un nodo, y toda la memoria
									ocupada por el nodo es liberada

	Parámetros:	- node *n:			Apuntador a una estructura node
*/
void delete_node(node *n);

/*
	Procedimiento move_nodes():				Al ejecutarse, mueve los nodos de una estructura dir_queue origen
											a una estructura dir_queue destino

	Parámetros:	- dir_queue *dest_queue:	Apuntador a una estructura dir_queue
	Parámetros:	- dir_queue *orig_queue:	Apuntador a una estructura dir_queue
*/
void move_nodes(dir_queue *dest_queue, dir_queue *orig_queue);

#endif
