#ifndef _ZD_FILE_H_
#define _ZD_FILE_H_

#if defined(_MSC_VER) && _MSC_VER>1000
#pragma once
#endif

#if defined(OS_WINDOWS)
	#include <Windows.h>
#endif

typedef wchar_t WCHAR;

// �ļ��Ƿ����
bool IsExistFile(const char* filename);

// Ŀ¼�Ƿ����
bool IsExistPath(const char* path);
bool IsExistDir(const char* path);

// ��ȡ�ļ���С
INT64 GetFileSize(const char* filename);

// �����ļ���
bool CreateDirPath(const char* fullPath, INT_PTR len = -1);

// �����ļ�
bool CreateFilePath(const char* fullPath, INT_PTR len = -1);

// ɾ��һ���ļ������ļ���
bool RemoveFileOrDir(const char* pathname);

inline bool IsExistDir(const char* path)
	{ return IsExistPath(path); }

#endif // _ZD_FILE_H_

