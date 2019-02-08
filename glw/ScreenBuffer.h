#pragma once
class ScreenBuffer
{
public:
	ScreenBuffer();
	~ScreenBuffer();
	void AddString(char* msg);
	void AddString(const char* msg);
	const char* GetString(int line);
private:
	char* buffer;
	int start;
};

