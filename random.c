// Implementation source:
// https://wiki.osdev.org/Random_Number_Generator#The_Standard.27s_Example

unsigned long next = 1;

int rand(void) {
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) { next = seed; }
