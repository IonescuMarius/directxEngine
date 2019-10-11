#include "..\Includes\dxUtility.h"
#include "Dxerr.h"
#include <io.h>
#include <algorithm>

void CUtility::DebugString(const std::string &str)
{
	OutputDebugString(str.c_str());
}

bool CUtility::FailedHr(HRESULT hr)
{
	if (FAILED(hr))
	{
		/*DebugString("DirectX Reported Error: "+ToString(DXGetErrorString(hr))+" - " + 
			ToString(DXGetErrorDescription(hr))+"\n");*/
		DebugString("DirectX Reported an Error");

		return true;
	}

	return false;
}

char* CUtility::DuplicateCharString(const char* charString)
{
    if (!charString)
		return 0;

	size_t len=strlen(charString) + 1;
	char *newString = new char[len];
	memcpy(newString, charString, len*sizeof(char));

	return newString;
}

bool CUtility::FindFile(std::string *filename)
{
	if (!filename)
		return false;

	// Look for it first using the original filename and path
	if (DoesFileExist(*filename))
		return true;

	// Next try with stripping the path from it
	std::string pathOnly;
	std::string filenameOnly;
	SplitPath(*filename,&pathOnly,&filenameOnly);

	if (DoesFileExist(filenameOnly))
	{
		*filename=filenameOnly;
		return true;
	}

	// Could now try by looking in a specific data folder....
	return false;
}

bool CUtility::DoesFileExist(const std::string &filename)
{
	return (_access(filename.c_str(), 0) != -1);
}

void CUtility::SplitPath(const std::string& inputPath, std::string* pathOnly, std::string* filenameOnly)
{
	std::string fullPath(inputPath);
	std::replace(fullPath.begin(),fullPath.end(),'\\','/');

	std::string::size_type lastSlashPos=fullPath.find_last_of('/');

	// check for there being no path element in the input
	if (lastSlashPos==std::string::npos)
	{
		*pathOnly="";
		*filenameOnly=fullPath;
	}
	else // do the split
	{
		if (pathOnly)
			*pathOnly=fullPath.substr(0,lastSlashPos);

		if (filenameOnly)
			*filenameOnly=fullPath.substr(lastSlashPos+1,fullPath.size()-lastSlashPos-1);
	}
}

std::string CUtility::GetTheCurrentDirectory()
{
	int bufferSize=GetCurrentDirectory(0,NULL);
	char *buffer=new char[bufferSize];

	GetCurrentDirectory(bufferSize,buffer);
	std::string directory(buffer);
	delete []buffer;

	return directory;
}

float CUtility::VectorLength3D( D3DXVECTOR3 d3Vector )
{
	return sqrtf( (d3Vector.x * d3Vector.x) + (d3Vector.y * d3Vector.y) + (d3Vector.z * d3Vector.z));
}

float CUtility::random_number_float(float low, float high)
{
    return low + ((float)((float)rand() / (float)RAND_MAX) * (float)((high) - (low)));
}

int CUtility::random_number_int(int low, int high)
{
    return rand() % high + low;
}

int CUtility::StringLength(char* str)
{
	int cnt = 0;
	
	while( str[cnt] != '\0' )
		++cnt;
	return cnt;
}

int* CUtility::sortBubble(int* bubbleArray, int n)
{
	int temp,i,j;
	for(i = 0; i < n - 1; i++)
	{
		for(j = 0; j < n - i - 1; j++)
		{
			if(bubbleArray[j + 1] < bubbleArray[j])
			{
				temp = bubbleArray[j];
				bubbleArray[j] = bubbleArray[j + 1];
				bubbleArray[j + 1] = temp;
			}
		}
	}
	
	return bubbleArray; 
}

int CUtility::BinSearch(int data[], int numElements, int searchKey)
{
	int middle, left = 0, right = numElements - 1;
	while(left <= right)
	{
		middle = (left + right)/2;
		if(data[middle] < searchKey)
		{
			left = middle + 1;
		}		
		else if(data[middle] > searchKey)
		{
			right = middle -1;	
		}
		else
		{
			return middle;
		}
	}
	
	return -1;
}

int CUtility::size_str(char* str)
{
	int length;
	return length = strlen(str);
}

int CUtility::compare_str(const char *string1, const char *string2)
{
	int ret; 
	return ret = strcmp(string1, string2);
}

char* CUtility::copy_str(char *strDestination, const char *strSource)
{
	return strcpy(strDestination, strSource);
}

char* CUtility::concat_str(char *strDestination,const char *strSource)
{
	return strcat(strDestination, strSource);
}