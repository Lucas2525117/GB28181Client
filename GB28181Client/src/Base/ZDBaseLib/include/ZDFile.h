#ifndef _ZD_FILE_H_
#define _ZD_FILE_H_

#if defined(_MSC_VER) && _MSC_VER>1000
#pragma once
#endif

#if defined(OS_WINDOWS)
	#include <Windows.h>
#endif

typedef wchar_t WCHAR;

// 文件是否存在
bool IsExistFile(const char* filename);

// 目录是否存在
bool IsExistPath(const char* path);
bool IsExistDir(const char* path);

// 获取文件大小
INT64 GetFileSize(const char* filename);

// 创建文件夹
bool CreateDirPath(const char* fullPath, INT_PTR len = -1);

// 创建文件
bool CreateFilePath(const char* fullPath, INT_PTR len = -1);

// 删除一个文件或者文件夹
bool RemoveFileOrDir(const char* pathname);

inline bool IsExistDir(const char* path)
	{ return IsExistPath(path); }

#endif // _ZD_FILE_H_

