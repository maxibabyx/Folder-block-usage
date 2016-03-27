#ifndef directories_queue
#define directories_queue

// Estructura nodo, elementos en la cola de directorios
typedef struct node {
	char *path;
	struct node *next, *prev;
} node;


/* Cola de directorios, funciona como lista doblemente enlazada
   con apuntador a la cabeza y a la cola */
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