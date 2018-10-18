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

bool Check_Valid_Format(unsigned char Format_Type)
{
    return Format_Type == 0 || Format_Type == 1;
}

uint16_t Get_First_Format_Size(uint8_t Count)
{
    return  sizeof(Count) + Count * 2;
}

uint16_t Get_First_Format_Bytes(unsigned char* Current_Position, unsigned char* File_Stop)
{
    uint8_t Count = *Current_Position;
    unsigned char* End_of_Line = Current_Position + Get_First_Format_Size(Count);

    return (End_of_Line > File_Stop) ? -1 : (End_of_Line - Current_Position);
}

uint32_t Get_INT32_String(unsigned char* Start_Position, unsigned char* End_Position)
{
    uint8_t Count_Characters = End_Position - Start_Position;
    char* INT32_String = malloc(Count_Characters + 1);
    INT32_String[Count_Characters] = '\0';
    memcpy(INT32_String, Start_Position, Count_Characters);
    uint32_t Number =  atoi(INT32_String);
    free(INT32_String);

    return Number;
}

bool Parse_Number(unsigned char* Start_Position, unsigned char* Current_Position)
{
    char c = *Current_Position;
    uint64_t Input_Bytes = Current_Position - Start_Position;

    return !isdigit(c) || Input_Bytes >= 5;
}

uint8_t Get_Second_Format_Size(unsigned char* Current_Position)
{
    unsigned char* Start_Position = Current_Position;
    while (!Parse_Number(Start_Position, Current_Position))
    { 
    	Current_Position++; 
    }

    return Current_Position - Start_Position;
}

bool Test_Line_End(unsigned char c)
{
    return Check_Valid_Format(c) || c == '\n';
}

bool Test_Number_End(unsigned char c)
{
    return c == ',';
}

uint16_t Get_Second_Format_Bytes(unsigned char* Current_Position, unsigned char* File_Stop)
{
    unsigned char* Line_Position = Current_Position;
    uint32_t Count = Get_INT32_String(Line_Position, Line_Position + 3);
    Line_Position += 3;

    for (int i = 0; i < Count; i++)
    {
        uint8_t Input_Bytes = Get_Second_Format_Size(Line_Position);
        Line_Position += Input_Bytes;

        if (Test_Line_End(*Line_Position))
        { 
        	break; 
        }

        if (!Test_Number_End(*Line_Position))
        { 
        	return -1; 
        }

        Line_Position++;
    }

    return Line_Position - Current_Position;
}

bool Test_Format(unsigned char* Current_Position, unsigned char* End_Position)
{
    while (Current_Position < End_Position) 
    {
        uint8_t Format_Type = *Current_Position++;

        if (!Check_Valid_Format(Format_Type)) 
        { 
        	return false; 
        }

        int16_t Input_Bytes = (Format_Type == 0) ?
                             Get_First_Format_Bytes(Current_Position, End_Position) :
                             Get_Second_Format_Bytes(Current_Position, End_Position) ;

        if (Input_Bytes == -1) 
        { 
        	return false; 
        }

        Current_Position += Input_Bytes;
    }

    return Current_Position == End_Position;
}

int Write_Socket(int File_Descriptor, void* Data , int Data_Size)
{
    while (Data_Size != 0)
    {
        int Written_Bytes = write(File_Descriptor, Data, Data_Size);

        if (Written_Bytes < 0)
        {
            if (errno == EINTR)
            {
                Written_Bytes  = 0;
            }

            perror(strerror(errno));
            exit(-1);
        }

        Data_Size -= Written_Bytes;
        Data += Written_Bytes;
    }
}

uint8_t Get_Number_Digits(uint8_t Count)
{
    uint8_t Digit_Count = 0;

    while (Count != 0)
    {
        Digit_Count++;
        Count /= 10;
    }

    return Digit_Count;
}

unsigned char* Convert_Three_Byte_String(uint8_t Count) 
{
    uint8_t Number_Digits = Get_Number_Digits(Count);
    char* String_Number = malloc(3 + 1);
    String_Number[3] = '\0';
    memset(String_Number, '0', 3);
    int Index = 3 - 1;

    for (int i = 0; i < Number_Digits; i++) 
    {
        String_Number[Index] = (Count % 10) + '0';
        Count /= 10;
    }

    return String_Number;
}

uint16_t Read_INT16(unsigned char** File)
{
    uint16_t Higher_Bits = *(*File)++;
    uint8_t Lower_Bits = *(*File)++;
    Higher_Bits <<= 8;
    uint16_t Read_Result = Higher_Bits | Lower_Bits;

    return Read_Result;
}

void Write_File_Unchanged(unsigned char* Current_Position, unsigned char* File_Stop, FILE* File)
{
    while (Current_Position < File_Stop) 
    {
        uint8_t Format_Type = *Current_Position++;

        if (Format_Type == 0) 
        {
            uint8_t Count = *Current_Position++;
            fprintf(File, "%s ", Convert_Three_Byte_String(Count));

            for (int i = 0; i < Count - 1; i++) 
            {
                fprintf(File, "%d ", Read_INT16(&Current_Position));
            }

            fprintf(File, "%d\n", Read_INT16(&Current_Position));
        }
        else if (Format_Type == 1) 
        {
            char Count[3 + 1];
            Count[3] = '\0';
            memcpy(Count, Current_Position, 3);
            fprintf(File, "%s", Count);
            Current_Position += 3;
            while (!Check_Valid_Format(*Current_Position)) 
            {
                fprintf(File, "%c", *Current_Position++);
            }

            fprintf(File, "%c", '\n');
        }
    }
}

void Write_File_First_Second(unsigned char* Current_Position, unsigned char* File_Stop, FILE* File)
{
    while (Current_Position < File_Stop)
    {
        uint8_t Format_Type = *Current_Position++;

        if (Format_Type == 0)
        {
            uint8_t Count = *Current_Position++;
            fprintf(File, "%s ", Convert_Three_Byte_String(Count));

            for (int i = 0; i < Count - 1; i++)
            {
                fprintf(File, "%d,", Read_INT16(&Current_Position));
            }

            fprintf(File, "%d\n", Read_INT16(&Current_Position));
        }
        else if (Format_Type == 1)
        {
            char Count[3 + 1];
            Count[3] = '\0';
            memcpy(Count, Current_Position, 3);
            fprintf(File, "%s ", Count);
            Current_Position += 3;

            while (!Check_Valid_Format(*Current_Position))
            {
                fprintf(File, "%c", *Current_Position++);
            }

            fprintf(File, "%c", '\n');
        }
    }
}

void Write_File_Second_First(unsigned char* Current_Position, unsigned char* File_Stop, FILE* File)
{
    while (Current_Position < File_Stop)
    {
        uint8_t Format_Type = *Current_Position++;

        if (Format_Type == 0)
        {
            uint8_t Count = *Current_Position++;
            fprintf(File, "%s ", Convert_Three_Byte_String(Count));

            for (int i = 0; i < Count - 1; i++)
            {
                fprintf(File, "%d ", Read_INT16(&Current_Position));
            }

            fprintf(File, "%d\n", Read_INT16(&Current_Position));
        }
        else if (Format_Type == 1)
        {
            char Count[3 + 1];
            Count[3] = '\0';
            memcpy(Count, Current_Position, 3);
            fprintf(File, "%s ", Count);
            Current_Position += 3;

            while (!Check_Valid_Format(*Current_Position))
            {
                char c = *Current_Position++;

                if (c == ',')
                {
                    fprintf(File, "%c", ' ');
                }
                else
                {
                    fprintf(File, "%c", c);
                }
            }

            fprintf(File, "%c", '\n');
        }
    }
}

void Write_File_Swap(unsigned char* Current_Position, unsigned char* File_Stop, FILE* File)
{
    while (Current_Position < File_Stop)
    {
        uint8_t Format_Type = *Current_Position++;

        if (Format_Type == 0)
        {
            uint8_t Count = *Current_Position++;
            fprintf(File, "%s ", Convert_Three_Byte_String(Count));

            for (int i = 0; i < Count - 1; i++)
            {
                fprintf(File, "%d,", Read_INT16(&Current_Position));
            }

            fprintf(File, "%d\n", Read_INT16(&Current_Position));
        }
        else if (Format_Type == 1)
        {
            char Count[3 + 1];
            Count[3] = '\0';
            memcpy(Count, Current_Position, 3);
            printf("Amount: %s\n", Count);
            printf("3\n");
            fprintf(File, "%s ", Count);
            printf("4\n");
            Current_Position += 3;

            while (!Check_Valid_Format(*Current_Position))
            {
                char c = *Current_Position++;

                if (c == ',')
                {
                    fprintf(File, "%c", ' ');
                }
                else
                {
                    fprintf(File, "%c", c);
                }
            }

            fprintf(File, "%c", '\n');
        }
    }
}

void Write_File(uint8_t Translation_Type, unsigned char* File, uint64_t Size_of_File, unsigned char* Name_of_File)
{

    FILE* Output = fopen(Name_of_File, "w");

    if (!Output)
    {
        printf("Error: Cannot open file. \n");
        return;
    }

    unsigned char* File_Stop = File + Size_of_File;

    if (Translation_Type == 0)
    {
        Write_File_Unchanged(File, File_Stop, Output);
    }

    else if (Translation_Type == 1)
    {
        Write_File_First_Second(File, File_Stop, Output);
    }

    else if (Translation_Type == 2)
    {
        Write_File_Second_First(File, File_Stop, Output);
    }

    else if (Translation_Type == 3)
    {
        Write_File_Swap(File, File_Stop, Output);
    }

    fclose(Output);
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
            bool Is_Valid = Test_Format(File_Start, File_Stop);
            if (Is_Valid)
            {
                Write_Socket(Client_Init, "Success!", sizeof("Success!"));
                Write_File(Message.Transmission_Type, Message.File, Message.Size_of_File, Message.Name_of_File);
            }
            else
            {
                Write_Socket(Client_Init, "Error: Invalid Format", sizeof("Error: Invalid Format"));
            }
        
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

	if (close(Server_Socket) < 0)
	{
		printf("Error: Server could not be closed. \n");
	}
}