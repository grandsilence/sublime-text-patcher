#include "stdafx.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void panic(const char *format, ...);
char* read_file(const char *name, unsigned long *contentLength);
void backup_file(const char *name);
void save_file(const char* name, char* content, int contentLength);

char* pattern_scan(char* base, unsigned int size, const char* pattern, const char* mask)
{
	unsigned int patternLength = strlen(mask);

	for (unsigned int i = 0; i < size - patternLength; i++)
	{
		bool found = true;
		for (unsigned int j = 0; j < patternLength; j++)
		{
			if (mask[j] != '?' && pattern[j] != *(base + i + j))
			{
				found = false;
				break;
			}
		}
		if (found) {
			return (char*)(base + i);
		}
	}
	return nullptr;
}

void patch(char *content, int contentLength) 
{
	printf("Patching license...\n");
	// Patch Licensing
	char *addr1 = pattern_scan(content, contentLength, "\x40\x53\x48\x83\xEC\x20\x80\x39\x00\x75\x76\xE8", "xxxxxxxxxxxx");
	if (!addr1) {
		panic("First pattern not found. Or file already patched.");
	}	
	*(addr1 + 9) = 0xEB;

	char *addr2 = pattern_scan(content, contentLength, "\x45\x33\xC0\x8A\xD3\x48\x8D\x4D\xE7\xE8\x00\x00\x00\x00\x41\xF6\xC7\x02", "xxxxxxxxxx????xxxx");
	if (!addr2) {
		panic("Second pattern not found");
	}
	*((short*)(addr2 + 14)) = 0x6AEB;

	// Patch fonts
	const char *fontFace = "Segoe UI";
	const char *newFontFace = "Tahoma";

	printf("Patch font %s to %s ? For disabled DirectWrite. (y/N) ", fontFace, newFontFace);
	char answer = getchar();
	if (answer == 'Y' || answer == 'y') 
	{
		printf("Patching fonts...\n");
		char *addrFont = pattern_scan(content, contentLength, fontFace, "xxxxxxxx");
		if (!addrFont) {
			printf("Unable to find font offset \"%s\". Skipped font patch", fontFace);
			return;
		}
		strcpy(addrFont, newFontFace);
	}
}


int main(int argc, char* argv[])
{
	// Check the number of parameters
    if (argc < 2) {
		panic("Please provide argument path to sublime text executable");
	}

	const char *fileName = argv[1];
	unsigned long contentLength;

	char *content = read_file(fileName, &contentLength);
	patch(content, contentLength);

	backup_file(fileName);
	save_file(fileName, content, contentLength);

	printf("\n=========\nPatch DONE! Press any key...");
	getchar();

    return 0;
}

void panic(const char *format, ...)
{
	va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);

	getchar();
	exit(1);
}

char* read_file(const char *name, unsigned long *contentLength) 
{
	//Open file
	FILE *file = fopen(name, "rb");
	if (!file) 
		panic("Unable to open file %s", name);
	
	//Get file length and save it
	fseek(file, 0, SEEK_END);
	*contentLength = ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	char *buffer = (char *)malloc(*contentLength + 1);
	if (!buffer)
	{
		fclose(file);
		panic("Allocate memory error!");
	}

	//Read file contents into buffer
	fread(buffer, *contentLength, 1, file);
	fclose(file);

	return buffer;
}

void backup_file(const char *name)
{
	printf("Making backup of \"%s\"", name);
	unsigned long contentLength;

	char* content = read_file(name, &contentLength);

	char destPath[512];
	strcpy(destPath, name);
	strcat(destPath, ".bak");

	save_file(destPath, content, contentLength);
}

void save_file(const char *name, char *content, int contentLength) 
{
	FILE *file = fopen(name, "wb");
	if (!file)
		panic("Unable to save %s file", name);

	fwrite(content, contentLength, 1, file);
	
	fclose(file);
	free(content);
}