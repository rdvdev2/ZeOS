#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#define CIRCULAR_BUFFER_SIZE 32

typedef struct{
  char content[CIRCULAR_BUFFER_SIZE];
  int head;
  int tail;
}circular_buff;

//Inicializa el buffer a 0 y setea los punteros de cabeza y cola
void init_buffer(circular_buff* buff);

/*
  Adds an element to the tail of the buffer.
  It returns -1 if the buffer is full and 0 otherwise
*/
int add_item(circular_buff* buff, char item);

/*
  Quita un elemento de la cola y lo pone en item.
  Devuelve -1 en caso de error y 0 en caso de acierto
*/
int remove_item(circular_buff* buff, char* item);

#endif
