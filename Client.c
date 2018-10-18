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

int Create_Socket(const char *Server_IP, const char *Server_Port)
{
    int Socket_File_Descriptor = 0;

    if ((Socket_File_Descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        char * Error_Text = strerror(errno);
        perror(Error_Text);
        exit(-1);
    }

    struct sockaddr_in Server_Address;
    Server_Address.sin_family = AF_INET;
    Server_Address.sin_port = htons(atoi(Server_Port));

    if (inet_aton(Server_IP, &Server_Address.sin_addr) <= 0)
    {
        char * Error_Text = strerror(errno);
        perror(Error_Text);
        exit(-1);
    }

    if ((connect( Socket_File_Descriptor, (struct sockaddr*) &Server_Address, sizeof(Server_Address))) < 0)
    {
        char * Error_Text = strerror(errno);
        perror(Error_Text);
        exit(-1);
    }

    return Socket_File_Descriptor;
}

uint64_t Get_Message_Size(const char* File_Path, const char* File_Name, uint8_t Format)
{
    uint64_t File_size = Get_File_Size(File_Path);
    uint16_t File_Name_Size = strlen(File_Name);

    return sizeof(Format) + sizeof(File_Name_Size) + File_Name_Size
    + sizeof(File_size) + File_size;
}

unsigned char* Create_Message(const char* File_Path, const char* File_Name, uint8_t Format)
{
    uint64_t Message_Size = Get_Message_Size(File_Path, File_Name, Format);

    unsigned char* Message = malloc(Message_Size);
    unsigned char* Current_Position = Message;

    memcpy(Current_Position++, &Format, sizeof(Format));

    uint64_t File_size = Get_File_Size(File_Path);
    File_size = htonl(File_size);

    memcpy(Current_Position, &File_size, sizeof(File_size));

    File_size = ntohl(File_size);
    Current_Position += sizeof(File_size);
    FILE* File = fopen(File_Path, "rb");
    Current_Position += fread(Current_Position, sizeof(char), File_size, File);
    fclose(File);

    uint16_t to_name_size = strlen(File_Name);
    to_name_size = htons(to_name_size);

    memcpy(Current_Position, &to_name_size, sizeof(to_name_size));

    to_name_size = ntohs(to_name_size);
    Current_Position += sizeof(to_name_size);

    memcpy(Current_Position, File_Name, to_name_size);

    return Message;
}

int Write_Socket(int File_Descriptor, void* Data , int Data_Size)
{
    while (Data_Size != 0)
    {
        int Write_Bytes = write(File_Descriptor, Data, Data_Size);

        if (Write_Bytes < 0)
        {

            if (errno == EINTR)
            {
                Write_Bytes  = 0;
            }

            perror(strerror(errno));
            exit(-1);
        }

        Data_Size -= Write_Bytes;
        Data += Write_Bytes;
    }
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
      fprintf(stderr, "Error: Kindly check that the file is not empty.\n");
      exit(1);
  }

  int Client_Socket = Create_Socket(argv[1], argv[2]);

  uint64_t Message_Size = Get_Message_Size(File_Path, argv[5], Format);

  unsigned char* Message = Create_Message(File_Path, argv[5], Format);

  Write_Socket(Client_Socket, Message, Message_Size);

  return 0;
}