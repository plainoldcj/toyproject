#pragma once

struct StrSplit
{
	int data[6];
};

void		StrSplit_Init(struct StrSplit* strSplit, const char* str, const char* sep);
int			StrSplit_Next(struct StrSplit* strSplit);

const char*	StrSplit_String(struct StrSplit* strSplit);
int			StrSplit_Size(struct StrSplit* strSplit);
