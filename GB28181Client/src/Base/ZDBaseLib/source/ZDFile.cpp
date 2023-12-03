#include "ZDFile.h"

#if defined(OS_WINDOWS)
	static inline HANDLE CreateFile_Mpl_(const char* filename, DWORD access, DWORD mode, LPSECURITY_ATTRIBUTES attr, DWORD disp, DWORD flags, HANDLE h)
		{  return CreateFileA(filename, access, mode, attr, disp, flags, h);  }
	static inline DWORD GetFileAttributes_Mpl_(const char* filename)
		{  return GetFileAttributesA(filename);  }
#else
	static inline int stat64_Mpl_(const char* filename, struct stat64* fileinfo)
		{  return ::stat64(filename, fileinfo);  }
#endif

bool IsExistFile(const char* filename)
{
	if (NULL == filename)
		return false;

#if defined(OS_WINDOWS)
	DWORD attri = GetFileAttributes_Mpl_(filename);
	if (INVALID_FILE_ATTRIBUTES == attri)
		return false;
	return 0 == (FILE_ATTRIBUTE_DIRECTORY & attri);
#elif defined(OS_LINUX_)
	struct stat64 fileinfo;
	int r = stat64_Mpl_(filename, &fileinfo);
	return (0 == r && S_IFREG == (fileinfo.st_mode & S_IFREG));
#endif
}

bool IsExistPath(const char* path)
{
	if (NULL == path)
		return false;

#if defined(OS_WINDOWS)
	DWORD attri = GetFileAttributes_Mpl_(path);
	if (INVALID_FILE_ATTRIBUTES == attri)
		return false;
	return FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & attri);
#elif defined(EP_IS_LINUX_)
	struct stat64 fileinfo;
	int r = stat64_Mpl_(path, &fileinfo);
	return (0 == r && S_IFDIR == (fileinfo.st_mode & S_IFDIR));
#endif
}

INT64 GetFileSize(const char* filename)
{
	if (NULL == filename)
		return false;

#if defined(OS_WINDOWS)
	HANDLE hFile = CreateFile_Mpl_(
		filename,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		return -1;

	DWORD dwLSize, dwHSize;
	dwLSize = GetFileSize(hFile, &dwHSize);
	if (INVALID_FILE_SIZE == dwLSize && NO_ERROR != GetLastError())
	{
		CloseHandle(hFile);
		return -1;
	}

	CloseHandle(hFile);
	return (INT64(dwHSize) << 32) + INT64(dwLSize);
#else
	struct stat64 fileinfo;
	if (0 == stat64_Mpl_(filename, &fileinfo))
		return fileinfo.st_size;
	else
		return -1;
#endif
}

inline BOOL CreateDirPath_Mpl_(const char* fullPath, INT_PTR len)
{
#if defined(OS_WINDOWS)
	return CreateDirectoryA(fullPath, NULL);
#else
	return 0 == mkdir(dir, 0777);
#endif
}

bool CreateDirPath(const char* fullPath, INT_PTR len)
	{	return CreateDirPath_Mpl_(fullPath, len);	}

bool CreateFilePath(const char* fullPath, INT_PTR len)
	{   return true; }

bool RemoveFileOrDir(const char* pathname)
{
	return true;
}
