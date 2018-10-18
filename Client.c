// Including necessary libraries
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/Client_Socket.h>
#include <sys/stat.h>
#include <unistd.h>

// Main function
int main(int argc, char const *argv[])
{
  if (argc < 6) 
  {
    fprintf(stderr, "Error: Kindly check the number of arguments.\n");
    exit(1);
  }

  uint8_t Format = atoi(argv[4]);

  if (Format < 0 || Format > 3) 
  {
    fprintf(stderr, "Error: Kindly check that Format is not out of range.\n");
    exit(1);
  }

  return 0;
}