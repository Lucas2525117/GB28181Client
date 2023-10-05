#include "MyXmlParser.h"

CMyXmlParser::CMyXmlParser(std::string url)
{
	std::string str;
	str = std::string(GB2312ToUnicode(url.c_str(), url.size()));
	str = StrReplace(str, "GB2312", "UTF-8");
	str = StrReplace(str, "gb2312", "UTF-8");

	m_xml = std::make_shared<TiXmlDocument>();
	m_xml->Parse(str.c_str());
}

CMyXmlParser::~CMyXmlParser()
{
}

char* CMyXmlParser::UnicodeToGB2312(const char* src)
{
	auto length = MultiByteToWideChar(CP_UTF8, 0, src, -1, nullptr, 0);
	auto wstr = new wchar_t[length + 1];
	memset(wstr, 0, length + 1);
	auto dst = new char[length + 1];
	memset(dst, 0, length + 1);

	MultiByteToWideChar(CP_UTF8, 0, src, -1, wstr, length);
	length = WideCharToMultiByte(CP_ACP, 0, wstr, -1, nullptr, 0, nullptr, nullptr);

	WideCharToMultiByte(CP_ACP, 0, wstr, -1, dst, length, nullptr, nullptr);
	if (wstr)
	{
		delete[] wstr;
	}

	return dst;
}

char* CMyXmlParser::GB2312ToUnicode(const char* src, int len)
{
	int lengthWide = MultiByteToWideChar(CP_ACP, 0, src, -1, nullptr, 0);
	auto wstr = new wchar_t[lengthWide * Times];
	memset(wstr, 0, lengthWide * Times);
	int lengthMulti = MultiByteToWideChar(CP_ACP, 0, src, len, wstr, lengthWide * Times);
	auto dst = new char[lengthMulti * Times];
	memset(dst, 0, lengthMulti * Times);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, dst, lengthMulti * Times, nullptr, nullptr);
	if (wstr)
	{
		delete[] wstr;
	}

	return dst;
}

std::string CMyXmlParser::StrReplace(std::string str, const std::string& from, const std::string& to)
{
	std::stringstream ss;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) 
	{
		ss << str.substr(0, start_pos) << to;
		start_pos += from.length();
		str = str.substr(start_pos);
	}
	ss << str;
	return ss.str();
}