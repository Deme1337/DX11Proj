// stdafx.cpp : source file that includes just the standard includes
// D3D_Template.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


char Keys::kp[256] = { 0 };
int Keys::key(int iKey)
{
	return (GetAsyncKeyState(iKey) >> 15) & 1;
}

int Keys::onekey(int iKey)
{
	if (key(iKey) && !kp[iKey]) { kp[iKey] = 1; return 1; }
	if (!key(iKey))kp[iKey] = 0;
	return 0;
}