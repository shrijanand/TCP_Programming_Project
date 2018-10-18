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

// Main function
int main(int argc, char *argv[]) 
{
	return 0;
}