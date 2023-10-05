#ifndef _MY_DYNAMIC_STRUCT_H_
#define _MY_DYNAMIC_STRUCT_H_

#include "MyXmlParser.h"
#include "SipSDKPublic.h"
#include "json/json.h"

#define MAXDEEP 5

class CMyDynamicStruct
{
public:
	CMyDynamicStruct();
	~CMyDynamicStruct();

	void Set(std::shared_ptr<TiXmlDocument> xml);

	void Set(std::string key, std::string value);

	bool Has(std::string key);

	std::string Get(std::string key)
	{
		return m_option[key];
	}

private:
	DynamicStruct StructAdd(TiXmlNode* rootNode);

	void DeviceListAdd(TiXmlNode* rootNode, Json::Value& records);

	static bool isArray(TiXmlNode* node)
	{
		if (nullptr == node)
			return false;

		TiXmlElement* element = node->ToElement();
		if (nullptr == element)
			return false;

		int result = 0, val = -1;
		result = element->QueryIntAttribute("Num", &val);
		if (0 != result || val < 0)
			return false;

		return true;
	}

	static bool isTextNode(TiXmlNode* node)
	{
		std::string value = node->Value();
		return node->FirstChild() && (0 != value.compare("Item"));
	}

	static std::string GetNodeValue(TiXmlNode* node)
	{
		if (!node || !node->GetDocument())
			return "";

		return node->ToElement()->GetText();
	}

private:
	DynamicStruct m_option;
	int m_arrayDeep = 0;
	int m_structDeep = 0;
};

#endif

