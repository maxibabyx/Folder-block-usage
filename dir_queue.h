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

// Inserta un elemento en la cola, retorna el tamaño de la cola
void push (dir_queue *q, char *push_path);

// Elimina la cabeza de la cola, retorna un apuntador al elemento eliminado
node * pop (dir_queue *q);

// Devuelve el atributo "path" de la estructura nodo
char * node_path(node *n);

// DEBUGGING - Imprime el path de cada uno de los elementos de la cola
void print_queue(dir_queue *q);

// DEBUGGING - Imprime el path de un nodo
void print_node(node *n);

// Tamaño de la cola, retorna el número de elementos en la cola
int queue_size (dir_queue *q);

// DEBUGGING - Vacía la cola
void clean_queue(dir_queue *q);

// Mueve los nodos de una cola a otra
void move_nodes(dir_queue *new_queue, dir_queue *old_queue);

// Destructor de un nodo
void delete_node(node *nodo);

#endif
