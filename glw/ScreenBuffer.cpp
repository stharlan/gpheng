#include "stdafx.h"
#include "ScreenBuffer.h"


ScreenBuffer::ScreenBuffer()
{
	this->buffer = (char*)malloc(80 * 20);
	memset(this->buffer, 0, 80 * 20);
	this->start = -1;
}


ScreenBuffer::~ScreenBuffer()
{
	free(this->buffer);
}

void ScreenBuffer::AddString(char* msg)
{
	this->start--;
	if (this->start < 0) this->start = 19;
	strcpy_s(this->buffer + (this->start * 80), 79, msg);
	*(this->buffer + 79) = 0x00;
}

void ScreenBuffer::AddString(const char* msg)
{
	this->start--;
	if (this->start < 0) this->start = 19;
	strcpy_s(this->buffer + (this->start * 80), 79, msg);
	*(this->buffer + 79) = 0x00;
}

const char* ScreenBuffer::GetString(int line)
{
	int realLocation = line + this->start;
	while (realLocation > 19) realLocation -= 20;
	return this->buffer + (80 * realLocation);
}
