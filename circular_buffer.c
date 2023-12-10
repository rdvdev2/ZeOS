#include <circular_buffer.h>

void init_buff(circular_buff *buff) {
  buff->head = 0;
  buff->tail = 0;
}

int add_item(circular_buff *buff, char item) {
  // If the buffer is full don't add the item and return an error
  if ((buff->tail + 1) % CIRCULAR_BUFFER_SIZE == buff->head)
    return -1;

  buff->content[buff->tail] = item;
  if (++buff->tail == CIRCULAR_BUFFER_SIZE)
    buff->tail = 0;
  return 0;
}

int remove_item(circular_buff *buff, char *item) {
  if (buff->tail == buff->head)
    return -1;

  *item = buff->content[buff->head];
  if (++buff->head == CIRCULAR_BUFFER_SIZE)
    buff->head = 0;
  return 0;
}
