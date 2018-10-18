// Including necessary libraries
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

extern int errno;

typedef struct Message_Struct 
{
    uint8_t Transmission_Type;
    uint16_t Length_of_Name;
    uint64_t Size_of_File;
    unsigned char* File;
    unsigned char* Name_of_File;
}Message_Struct;

uint8_t Get_Transmission_Type(int Socket_File_Descriptor)
{
    uint8_t Transmission_Type;

    for (int i = 0; i < sizeof(Transmission_Type); i++)
    {
        int Input_Bytes = read(Socket_File_Descriptor, &Transmission_Type, 1);

        if (Input_Bytes < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror(strerror(errno));
            exit(-1);
        }
    }

    return Transmission_Type;
}

uint64_t Get_File_Size(int Socket_File_Descriptor)
{
    uint64_t Size_of_File;
    int Total_Bytes = sizeof(Size_of_File);
    int Input_Bytes = 0;
    uint64_t* Buffer = &Size_of_File;

    while (Input_Bytes < Total_Bytes) 
    {
        int Get_Byte  = read(Socket_File_Descriptor, Buffer + Input_Bytes, Total_Bytes - Input_Bytes);
        Input_Bytes += Get_Byte;
    }
    
    return ntohl(Size_of_File);
}

unsigned char* Get_File(int Socket_File_Descriptor, uint64_t Size_of_File)
{
    unsigned char* File = malloc(Size_of_File + 1);
    File[Size_of_File] = '\0';
    int Input_Bytes = 0;
    unsigned char* Current = File;

    while (Input_Bytes < Size_of_File) 
    {
        int Get_Byte  = read(Socket_File_Descriptor, Current + Input_Bytes, Size_of_File - Input_Bytes);
        Input_Bytes += Get_Byte;
    }

    return File;
}

uint16_t Get_Length_of_Name(int Socket_File_Descriptor)
{
    uint16_t Length_of_Name;
    int Input_Bytes = 0;
    int Total_Bytes = sizeof(Length_of_Name);
    uint16_t* Buffer = &Length_of_Name;

    while (Input_Bytes < Total_Bytes) 
    {
        int Get_Byte  = read(Socket_File_Descriptor, Buffer + Input_Bytes, Total_Bytes - Input_Bytes);
        Input_Bytes += Get_Byte;
    }

    return ntohs(Length_of_Name);
}

char* Get_Name_of_File(int Socket_File_Descriptor, uint16_t Length_of_Name)
{
    char* Name_of_File = malloc(Length_of_Name + 1);
    Name_of_File[Length_of_Name] = '\0';
    int Input_Bytes = 0;
   	unsigned char* Current = Name_of_File;

    while (Input_Bytes < Length_of_Name) 
    {
        int Get_Byte  = read(Socket_File_Descriptor, Current + Input_Bytes, Length_of_Name - Input_Bytes);
        Input_Bytes += Get_Byte;
    }

    return Name_of_File;
}

Message_Struct Get_Message(int File_Descriptor)
{
    Message_Struct Message;
    Message.Transmission_Type = Get_Transmission_Type(File_Descriptor);
    Message.Size_of_File = Get_File_Size(File_Descriptor);
    Message.File = Get_File(File_Descriptor, Message.Size_of_File);
    Message.Length_of_Name = Get_Length_of_Name(File_Descriptor);
    Message.Name_of_File  = Get_Name_of_File(File_Descriptor, Message.Length_of_Name);
    return Message;
}

int Create_Server(uint16_t Port_Number)
{
    int Server_Init = 0;

    if ((Server_Init = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
         char * Error_Text = strerror(errno);
         perror(Error_Text);
         exit(-1);
    }

    struct sockaddr_in Server_Address;
    Server_Address.sin_family = AF_INET;
    Server_Address.sin_addr.s_addr = htonl(INADDR_ANY);
    Server_Address.sin_port = htons(Port_Number);


    if (bind(Server_Init, (struct sockaddr *) &Server_Address, sizeof(Server_Address)) < 0) 
    {
        char * Error_Text = strerror(errno);
        perror(Error_Text);
        exit(-1);
    }

    return Server_Init;
}

void Start_Server(int Server_Socket) 
{
    if (listen(Server_Socket, 3) < 0) 
    {
        char * Error_Text = strerror(errno);
        perror(Error_Text);
        exit(-1);
    }

    struct sockaddr_in Client_Address;
    int Size_of_Address = sizeof(Client_Address);
    int Client_Init = 0;

    while (1) 
    {
        if ((Client_Init = accept(Server_Socket, (struct sockaddr*) &Client_Address, &Size_of_Address)) < 0) 
        {
            char * Error_Text = strerror(errno);
            perror(Error_Text);
            exit(-1);
        }

            Message_Struct Message = Get_Message(Client_Init);
            unsigned char* File_Start = Message.File;
            unsigned char* File_Stop = Message.File + Message.Size_of_File;
        
        if (close(Client_Init) < 0) 
        {
            char * Error_Text = strerror(errno);
            perror(Error_Text);
            exit(-1);
        }
    }
}

// Main function
int main(int argc, char *argv[]) 
{
	if (argc < 2)
	{
		fprintf(stderr, "Error: Kindly check that a port is provided. \n");
		exit(1);
	}

	int Server_Socket = Create_Server(atoi(argv[1]));

	Start_Server(Server_Socket);

	return 0;
}