/****************************************************************************
*                                                                           *
* dxUtility.h -- Utils function                                             *
*                                                                           *
* Copyright (c) Ionescu Marius. All rights reserved(2009).                  *
*                                                                           *
****************************************************************************/

#pragma once

#include <string>
#include <sstream>
#include <d3d9.h>
#include <d3dx9.h>
#include <assert.h>

template <class T>
std::string ToString(const T & t)
{
	std::ostringstream oss;
	oss.clear();
	oss << t;
	return oss.str();
}

class CUtility
{
	private:
		~CUtility(void){};
	public:
		CUtility(void){};
		
		static bool FailedHr(HRESULT hr);
		static void DebugString(const std::string &str);
		static char* DuplicateCharString(const char* c_str);
		static bool FindFile(std::string *filename);
		static bool DoesFileExist(const std::string &filename);
		static void SplitPath(const std::string& inputPath, std::string* pathOnly, std::string* filenameOnly);
		static std::string GetTheCurrentDirectory();
		float VectorLength3D( D3DXVECTOR3 d3Vector );
		int random_number_int(int low, int high);
		float random_number_float(float low, float high);
		int StringLength(char* str);
		int size_str(char* str);
		int* sortBubble(int* bubbleArray, int n);
		int BinSearch(int data[], int numElements, int searchKey);
		int compare_str(const char *string1, const char *string2);
		char* copy_str(char *strDestination, const char *strSource);
		char* concat_str(char *strDestination,const char *strSource);
};
