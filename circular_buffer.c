#include <circular_buffer.h>

void init_buff(circular_buff* buff) {
  content = {};
  head = 0;
  tail = 0;
}

int add_item(circular_buff* buff, char item) {
  #If the buffer is full don't add the item and return an error
  if((buff->tail+1) % BUFF_SIZE == buff->head) return -1;
  
  content[buff->tail] = item;
  if(++buff->tail == BUFF_SIZE) buff->tail = 0;
  return 0; 
}

int remove_item(circular_buff* buff, char* item) {
  if(buff->tail == buff->head) return -1;
  
  *item = buff->head;
  buff->head++;
  return 0;
}
