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









float lerp(float a, float b, float c)
{
	return a + c * (b - a);
}

// wide char to multi byte:
std::string ws2s(const std::wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), 0, 0, 0, 0);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), int(wstr.length() + 1), &strTo[0], size_needed, 0, 0);
	return strTo;
}

std::wstring s2ws(const std::string & s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}


std::vector<std::string> SplitPath(const std::string& str, const std::set<char> delimiters)
{
	std::vector<std::string> result;

	char const* pch = str.c_str();
	char const* start = pch;
	for (; *pch; ++pch)
	{
		if (delimiters.find(*pch) != delimiters.end())
		{
			if (start != pch)
			{
				std::string str(start, pch);
				result.push_back(str);
			}
			else
			{
				result.push_back("");
			}
			start = pch + 1;
		}
	}
	result.push_back(start);

	return result;
}

std::vector<std::string> GetFileNamesFromFolder(std::wstring folderpath)
{
	std::vector<std::string> filenames;
	
	std::wstring pattern = folderpath;

	pattern.append(L"\\*");
	WIN32_FIND_DATA data;
	HANDLE hFind;
	int size = pattern.size();
	WCHAR patternW[250];

	if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
		do {
			filenames.push_back(ws2s(folderpath + L"\\" + data.cFileName));
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	else
	{
		std::string error = "Error cannot open file:" + std::to_string(GetLastError());
		MessageBox(NULL, s2ws(error).c_str(), L"Error", MB_OK);
	}

	return filenames;
}

#include <string>
#include <shlobj.h>
#include <iostream>
#include <sstream>

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		std::string tmp = (const char *)lpData;
		std::cout << "path: " << tmp << std::endl;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

std::wstring BrowseFolder(std::string saved_path)
{
	TCHAR path[MAX_PATH];

	const char * path_param = saved_path.c_str();

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = (L"Browse for folder...");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = (BFFCALLBACK)BrowseCallbackProc;
	bi.lParam = (LPARAM)path_param;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

		return path;
	}

	return L"";
}




OpenFileDialog::OpenFileDialog(void)
{
	this->DefaultExtension = 0;
	this->FileName = new TCHAR[MAX_PATH];
	this->Filter = 0;
	this->FilterIndex = 0;
	this->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	this->InitialDir = 0;
	this->Owner = 0;
	this->Title = 0;
}

bool OpenFileDialog::ShowDialog()
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->Owner;
	ofn.lpstrDefExt = this->DefaultExtension;
	ofn.lpstrFile = this->FileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = this->Filter;
	ofn.nFilterIndex = this->FilterIndex;
	ofn.lpstrInitialDir = this->InitialDir;
	ofn.lpstrTitle = this->Title;
	ofn.Flags = this->Flags;

	GetOpenFileName(&ofn);

	if (_tcslen(this->FileName) == 0) return false;

	return true;
}



void split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}


std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}


const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}
