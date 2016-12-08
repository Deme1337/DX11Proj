// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>
#include "Input.h"

// TODO: reference additional headers your program requires here

typedef uint32_t uint32;
typedef uint8_t uint8;


//error messagebox
#define MBERROR(MSGs, CAPT, WINDOW) \
(\
	(MessageBox(WINDOW, MSGs, CAPT, MB_OK))\
)

/*
float XMLength(XMFLOAT3 v);
XMFLOAT3 Normalize(XMFLOAT3 xf);
*/

enum ShaderCompilation
{
	VertexShader,
	PixelShader
};


float lerp(float a, float b, float c);

namespace Keys
{
	int key(int iKey);
	int onekey(int iKey);
	extern char kp[256];
}



// wide char to multi byte:
std::string ws2s(const std::wstring& wstr);


//Safe release from http://www.3dgep.com/introduction-to-directx-11/#DXGI
// Safely release a COM object.
template<typename T>
inline void SafeRelease(T& ptr)
{
	if (ptr != NULL)
	{
		ptr->Release();
		ptr = NULL;
	}
}

template <class T>
class TInitZero
{
public:
	TInitZero()
		: Value((T)0)
	{
	}

	TInitZero(const T& InValue)
		: Value(InValue)
	{
	}

	/*
	// assignment operator
	TInitZero<T> &operator=(const T& in)
	{
	Value = in;
	return *this;
	}
	*/

	operator const T() const
	{
		return Value;
	}

	/*
	const T& T::operator++()
	{
	++itsVal;
	return *this;
	}

	const T T::operator++(int)
	{
	T temp(*this);
	++itsVal;
	return temp;
	}
	*/


	T operator ->() const
	{
		return Value;
	}

	// dangerous to expose? needed for ++Value
	operator T& ()
	{
		return Value;
	}

	T& GetRef()
	{
		return Value;
	}

	// dangerous? Maybe better we go though Get()
	T* operator&()
	{
		return &Value;
	}


private:
	T Value;
};


//chrono stuff testing http://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point TimeVar;

#define duration(a) std::chrono::duration_cast<std::chrono::nanoseconds>(a).count()
#define timeNow() std::chrono::high_resolution_clock::now()

template<typename F, typename... Args>
double funcTime(F func, Args&&... args) {
	TimeVar t1 = timeNow();
	func(std::forward<Args>(args)...);
	return duration(timeNow() - t1);
}




/*EXAMPLE*/

/*
#include <iostream>
#include <algorithm>

typedef std::string String;

//first test function doing something
int countCharInString(String s, char delim){
    int count=0;
    String::size_type pos = s.find_first_of(delim);
    while ((pos = s.find_first_of(delim, pos)) != String::npos){
        count++;pos++;
    }
    return count;
}

//second test function doing the same thing in different way
int countWithAlgorithm(String s, char delim){
    return std::count(s.begin(),s.end(),delim);
}


int main(){
    std::cout<<"norm: "<<funcTime(countCharInString,"precision=10",'=')<<"\n";
    std::cout<<"algo: "<<funcTime(countWithAlgorithm,"precision=10",'=');
    return 0;
	}
	
*/



#ifndef FILE_DLG
#define FILE_DLG
#include <Windows.h>
#include <Commdlg.h>
#include <tchar.h>

class OpenFileDialog
{
public:
	OpenFileDialog(void);

	TCHAR*DefaultExtension;
	TCHAR*FileName;
	TCHAR*Filter;
	int FilterIndex;
	int Flags;
	TCHAR*InitialDir;
	HWND Owner;
	TCHAR*Title;

	bool ShowDialog();
};

#endif

#include <string>
#include <sstream>
#include <vector>


void split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

const std::string currentDateTime();