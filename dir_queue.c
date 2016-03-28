/*
	Las especificaciones de cada función en este archivo están comentadas
	en el header dir_queue.h
*/
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
	(q -> size)++;
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
	(q -> size)--;
	return tmp;
}

char * node_path(node *n) {
	return n -> path;
}

int queue_size (dir_queue *q) {
	return q -> size;
}

void delete_node(node *n) {
	free(node_path(n));
	n -> next = NULL;
	n -> prev = NULL;
	free(n);
}

void move_nodes(dir_queue *dest_queue, dir_queue *orig_queue) {
	node *tmp_node;
	while (queue_size(orig_queue)) {
		tmp_node = pop(orig_queue);
		push(dest_queue, tmp_node -> path);
		delete_node(tmp_node);
	}
}
