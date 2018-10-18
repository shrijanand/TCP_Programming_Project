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

bool Valid_File_Path(const char* File_Path)
{
    FILE* File = fopen(File_Path, "r");
    bool Valid_File_Path = File != NULL;
    fclose(File);

    return Valid_File_Path;
}

uint64_t Get_File_Size(const char* File_name)
{
    struct stat st;
    stat(File_name, &st);

    return st.st_size;
}

bool File_Empty(const char* File_Path)
{
    return Get_File_Size(File_Path) == 0;
}

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

  const char* File_Path = argv[3];

  if (!Valid_File_Path(File_Path)) 
  {
      fprintf(stderr, "Error: Kindly check that the file exists.\n");
      exit(1);
  }

  if (File_Empty(File_Path))
  {
      fprintf(stderr, "ERROR, FILE IS EMPTY\n");
      exit(1);
  }

  return 0;
}