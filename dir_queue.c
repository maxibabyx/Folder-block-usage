#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "dir_queue.h"

void push(dir_queue *q, char *push_path) {
	node *tmp = malloc(sizeof(node));
	tmp -> path = (char*)malloc(PATH_MAX);
	strcpy(tmp -> path, push_path);
	tmp -> next = NULL;
	tmp -> prev = NULL;

	if (q -> front == NULL) {
		q -> front = q -> back = tmp;
	}
	else {
		tmp -> next = q -> back;
		q -> back -> prev = tmp;
		q -> back = tmp;
	}
	q -> size++;
}

node * pop(dir_queue *q) {
	node *tmp = q -> front;
	if (queue_size(q) > 1) {
		q -> front = q -> front -> prev;
		q -> front -> next = NULL;
	}
	else if (queue_size(q) == 1) {
		tmp = q -> front;
		q -> front = q -> back = NULL;
	}
	else if(!queue_size(q)) {return NULL;}
	q -> size--;
	return tmp;
}

char * node_path(node *n) {
	return n -> path;
}

void print_queue(dir_queue *q) {
	int i;
	node *tmp = q -> front;
	if (queue_size(q)) {
		printf("\nFRONT\tNodo con path %s\n", node_path(tmp));
		tmp = tmp -> prev;
		for (i = 0; i < queue_size(q) - 1; i++) {
			if (i != queue_size(q) - 2) {
				printf("\tNodo con path %s\n", node_path(tmp));
				tmp = tmp -> prev;
			}
			else {
				printf("BACK\tNodo con path %s\n\n", node_path(tmp));

			}
		}
	}
	else if (!queue_size(q)) {printf("La cola está vacía\n");}
	tmp = NULL;
}

int queue_size (dir_queue *q) {
	return q -> size;
}

void clean_queue(dir_queue *q) {
	while (queue_size(q)) {
		delete_node(pop(q));
	}
}

void delete_node(node *nodo) {
	free(node_path(nodo));
	nodo -> next = NULL;
	nodo -> prev = NULL;
	free(nodo);
}

void move_nodes(dir_queue *dest_queue, dir_queue *old_queue) {
	node *tmp_node;
	while (queue_size(old_queue)) {
		tmp_node = pop(old_queue);
		push(dest_queue, tmp_node -> path);
		delete_node(tmp_node);
	}
}