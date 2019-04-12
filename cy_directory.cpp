
//#include "stdafx.h"
#include <windows.h>
#pragma warning(disable : 4995)
#include <Shlwapi.h>
#pragma warning(default : 4995)
#include<string>
#include<tchar.h>
#include<vector>
#include "cy_directory.h"
#include "cy_path.h"

namespace cy{
namespace io{
/**
*@note ��Ҫʹ��SHFileOperation�� �ú������ȶ�
*/ 
	bool KDirectory::Copy(const std::wstring& srcDirName, const std::wstring& destDirName)
{
    if (srcDirName.empty() || destDirName.empty())
    {
        return false;
    }

	std::wstring tempSrcDirPath;
	std::wstring tempDestDirPath;
    bool bResult = true;

    if (!::PathFileExistsW(srcDirName.c_str()))
    {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return false;
    }

    if (!::PathFileExists(destDirName.c_str()))
    {
        ::CreateDirectoryW(destDirName.c_str(), NULL);
    }

    // Remove last '\'
    if (L'\\' == srcDirName[srcDirName.length() - 1])
    {
        tempSrcDirPath = srcDirName.substr(0, srcDirName.length() - 1);
    }

    if (L'\\' == destDirName[destDirName.length() - 1])
    {
        tempDestDirPath = destDirName.substr(0, destDirName.length() - 1);
    }


    WIN32_FIND_DATAW FindFileData = {0};
    tempSrcDirPath = srcDirName + L"\\*.*";

    HANDLE hFind = ::FindFirstFile(tempSrcDirPath.c_str(), &FindFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do 
        {
            tempSrcDirPath = srcDirName+L"\\";
            tempSrcDirPath.append(FindFileData.cFileName);
            tempDestDirPath = destDirName + L"\\";
            tempDestDirPath.append(FindFileData.cFileName);

			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			{// ���ӣ����������Ŀ¼���򲻿���
				continue;
			}

            if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                && wcscmp(FindFileData.cFileName, L".") && wcscmp(FindFileData.cFileName, L".."))
            {
                if(!Copy(tempSrcDirPath, tempDestDirPath))
                {
                    bResult = false;
                }
            }

            if(!((FindFileData.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY))
            {
                if(!CopyFileW(tempSrcDirPath.c_str(), tempDestDirPath.c_str(), FALSE))
                {
                    bResult = false;
                }
            }
        } while(FindNextFile(hFind, &FindFileData)!=0);

        ::FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    return bResult;
}

bool KDirectory::Delete( const std::wstring& dirPath, bool deleteRoot /*= true*/)
{	
	if (dirPath.empty())
	{
		return false;
	}

	if (!::PathFileExists(dirPath.c_str()))
	{
		if (ERROR_FILE_NOT_FOUND == ::GetLastError())
		{
			::SetLastError(S_OK);
		}

		return true;
	}

	std::wstring deleteDirPath = dirPath;

    // Remove last backslash('\')
	if ('\\' == deleteDirPath[deleteDirPath.length() - 1])
	{
		deleteDirPath = deleteDirPath.substr(0, deleteDirPath.length() - 1);
	}

	bool result = true;

	WIN32_FIND_DATAW FindFileData = { 0 };

	if (deleteRoot) 
	{
		::SetFileAttributesW(deleteDirPath.c_str(),FILE_ATTRIBUTE_NORMAL);
		if (::RemoveDirectory(deleteDirPath.c_str()))
		{
			return true;
		}		
	}

	std::wstring tempPath = deleteDirPath + L"\\*.*";

	HANDLE hFind = ::FindFirstFile(tempPath.c_str(), &FindFileData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		do 
		{
			tempPath = deleteDirPath + L"\\";
			tempPath.append(FindFileData.cFileName);

			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				&& _tcscmp(FindFileData.cFileName, _T(".")) && _tcscmp(FindFileData.cFileName, _T("..")))
			{
				if (!Delete(tempPath.c_str(), true))
				{
					result = false;
				}
			}

			if(!((FindFileData.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY))
			{
				DWORD dwFileAttr = GetFileAttributesW(tempPath.c_str());
				if (dwFileAttr & FILE_ATTRIBUTE_READONLY) 
				{ 
					SetFileAttributesW(tempPath.c_str(), FILE_ATTRIBUTE_NORMAL); 
				}

				if (!::DeleteFileW(tempPath.c_str()))
				{
					result = false;
				}
			}

		} while(FindNextFile(hFind, &FindFileData)!=0);

		::FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}

	if (deleteRoot)
	{
		::SetFileAttributesW(deleteDirPath.c_str(),FILE_ATTRIBUTE_NORMAL);
		if (!::RemoveDirectory(deleteDirPath.c_str()))
		{
			result = false;
		}		
	}

	return result;
}

bool KDirectory::Exists( const std::wstring& dirPath )
{
    if (dirPath.empty())
    {
        return false;
    }

	WIN32_FIND_DATA wfd;
	bool isExists = false;
	std::wstring realDirPath = dirPath;
	if (L'\\' == realDirPath[realDirPath.length() - 1])
	{
		realDirPath = realDirPath.substr(0, realDirPath.length() - 1);
	}
	HANDLE hFind = ::FindFirstFile(realDirPath.c_str(), &wfd);
	if ((hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		isExists = true;   
	}
	::FindClose(hFind);
	
	return isExists;
}

std::wstring KDirectory::GetCurrentDirectory()
{
	wchar_t buffer[MAX_PATH] = {0};
	DWORD retSize;
	
	retSize = ::GetCurrentDirectory(MAX_PATH, buffer);

	if (retSize == 0 || retSize > MAX_PATH)
	{
		return L"";
	}

	return buffer;
}

bool KDirectory::SetCurrentDirectory( const std::wstring& path )
{
    if (path.empty())
    {
        return false;
    }

	wstring fullPath = KPath::GetFullPath(path);
	
	return ( TRUE == ::SetCurrentDirectory(fullPath.c_str()));
}

vector<std::wstring> KDirectory::GetFiles( const std::wstring& path,
									 SearchOption option /*= TopDirectoryOnly*/,
									 std::wstring fileNamePattern /*= *.* */)
{
    std::vector<std::wstring> filesList;
    
    if (path.empty())
    {
        return filesList;
    }

    WIN32_FIND_DATAW FindFileData = {0};
    std::wstring patchDirPattern = KPath::Combine(path, fileNamePattern);

    HANDLE hFind = ::FindFirstFile(patchDirPattern.c_str(), &FindFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do 
        {
            if (0 == wcscmp(FindFileData.cFileName, L".") || 0 == wcscmp(FindFileData.cFileName, L".."))
            {
                continue;
            }

            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                AllDirectories == option)
            {
                std::wstring subDirPath = KPath::Combine(path, FindFileData.cFileName);
                vector<std::wstring> subFileList = GetFiles(subDirPath, AllDirectories);

                std::vector<std::wstring>::iterator it;
                for (it = subFileList.begin(); it < subFileList.end(); it++)
                {
                    filesList.push_back(*it);
                }
            }

            if(!((FindFileData.dwFileAttributes) & FILE_ATTRIBUTE_DIRECTORY))
            {
                std::wstring fileName = KPath::Combine(path, FindFileData.cFileName);;
                filesList.push_back(fileName);
            }
        } while(FindNextFile(hFind, &FindFileData)!=0);

        ::FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    return filesList;
}

std::vector<std::wstring> KDirectory::GetDirectories( const std::wstring& path, SearchOption option /*= TopDirectoryOnly*/ )
{
    std::vector<std::wstring> filesList;
    if (path.empty())
    {
        return filesList;
    }

    WIN32_FIND_DATAW FindFileData = {0};
    std::wstring patchDirPattern = KPath::Combine(path, L"*.*");

    HANDLE hFind = ::FindFirstFile(patchDirPattern.c_str(), &FindFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do 
        {
            if (0 == wcscmp(FindFileData.cFileName, L".") || 0 == wcscmp(FindFileData.cFileName, L".."))
            {
                continue;
            }

            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                std::wstring subDirPath = KPath::Combine(path, FindFileData.cFileName);
                filesList.push_back(subDirPath);

                if (AllDirectories == option)
                {
                    std::vector<std::wstring> subDirList = GetDirectories(subDirPath, AllDirectories);
                    std::vector<std::wstring>::iterator it;
                    for (it = subDirList.begin(); it < subDirList.end(); it++)
                    {
                        filesList.push_back(*it);
                    }
                }
            }
        } while(FindNextFile(hFind, &FindFileData)!=0);

        ::FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    return filesList;
}

bool KDirectory::CreateDirectory( const std::wstring& path )
{
    if (path.empty())
    {
        return false;
    }

    std::wstring dirName = path;

    // ����һ��б�ܣ��������ӵ������Ŀ¼�����С�
    if(dirName[dirName.length() - 1] !=  '\\' )
    {
        dirName.append(1, '\\');
    }
    
    std::vector<std::wstring> dirsList;
    std::wstring tempDir;

    for( UINT i = 0; i < dirName.length(); i++ )
    {
        if( dirName[i] != '\\')
        {
            tempDir.append(1, dirName[i] );
        }
        else
        {
            dirsList.push_back( tempDir );
            tempDir.append(1, '\\' );
        }
    }

    std::vector<std::wstring>::iterator iter;
    for( iter = dirsList.begin();iter != dirsList.end(); iter++ )
    {
        if (!KPath::Exists(*iter))
        {
            if(!::CreateDirectory((*iter).c_str(), NULL))
            {
                return false;
            }
        }
    }
    
    return KPath::Exists(path);
}

bool KDirectory::Rename( const std::wstring& oldPath, const std::wstring& newPath, bool deleteWhenNewPathExist /*= false*/ )
{
    if (oldPath.empty() || newPath.empty())
    {
        return false;
    }

    std::wstring fullOldPath = KPath::GetFullPath(oldPath);
    std::wstring fullNewPath = KPath::GetFullPath(newPath);

    if (!KPath::Exists(fullOldPath))
    {
        return false;
    }

    if (KPath::Exists(fullNewPath) && deleteWhenNewPathExist)
    {
        if(!KDirectory::Delete(fullNewPath.c_str(), true))
        {
            return false;
        }
    }

    if (::MoveFile(fullOldPath.c_str(), fullNewPath.c_str()))
    {
        return true;
    }
    else
    {
        return false;
    }

}
}
}