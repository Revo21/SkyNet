#pragma once

#include <string>
#include <memory>
#include <Awesomium/awesomium_capi.h>

using namespace std;


static int scancodeToAscii(DWORD scancode, unsigned short* result)
{
   static HKL layout=GetKeyboardLayout(0);
   static unsigned char State[256];
   if (GetKeyboardState(State)==FALSE) return 0;
   UINT vk=MapVirtualKeyEx(scancode,1,layout);
   return ToAsciiEx(vk,scancode,State,result,0,layout);
}

char* convertInt(int number)
{	
	char buffer [30];
	itoa (number,buffer,10);
	return buffer;
}

std::string& awe_string_to_std( const awe_string* aweString, std::string& outString ) 
{
	// get the length of the awe_string
	int length = awe_string_to_wide( aweString, NULL, 0 );

	// create the wide and ascii buffers to hold the data
	wchar_t* wbuff = new wchar_t[ length + 1 ];
	char* buff = new char[ length + 1 ];

	// set the data to zero
	memset( buff, 0, length + 1 );
	memset( wbuff, 0, sizeof( wchar_t ) * (length + 1) );

	// get the string contents from the awe_string to the wide buffer
	awe_string_to_wide( aweString, wbuff, length );

	// convert the wide char set to ascii char set
	size_t outCount;
	wcstombs_s( &outCount, buff, length + 1, wbuff, _TRUNCATE );

	// set the now ascii string to the std::string
	outString = buff;

	// clean up the buffer
	delete[] wbuff;
	delete[] buff;

	// return the out string
	return outString;
}

// Log methods
void clear_log()
{
	HANDLE filehandle;	
	filehandle = CreateFile("DX_Injection_Log.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);	
	CloseHandle(filehandle);
}
void add_log(char* string)
{
	HANDLE filehandle;
	DWORD dwReadBytes;
	char buffer[2048];
	filehandle = CreateFile("DX_Injection_Log.txt", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
	SetFilePointer(filehandle, 0, 0, FILE_END);
	sprintf_s(buffer, 1024, "Log: %s\r\n", string);
	WriteFile(filehandle, buffer, strlen(buffer), &dwReadBytes, 0);
	CloseHandle(filehandle);
}