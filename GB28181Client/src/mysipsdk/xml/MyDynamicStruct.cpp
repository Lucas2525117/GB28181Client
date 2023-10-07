#include "MyDynamicStruct.h"

CMyDynamicStruct::CMyDynamicStruct()
{
}

CMyDynamicStruct::~CMyDynamicStruct()
{
}

void CMyDynamicStruct::Set(std::shared_ptr<TiXmlDocument> xml)
{
	TiXmlNode* rootNode = xml->RootElement();
	m_option = StructAdd(rootNode);
}

void CMyDynamicStruct::Set(std::string key, std::string value)
{
	m_option[key] = value;
}

bool CMyDynamicStruct::Has(std::string key)
{
	return m_option.find(key) != m_option.end();
}

DynamicStruct CMyDynamicStruct::StructAdd(TiXmlNode* rootNode)
{
	DynamicStruct result;
	for (auto node = rootNode->FirstChild(); node != nullptr; node = node->NextSibling())
	{
		if (node->Type() != TiXmlNode::TINYXML_DOCUMENT)
		{
			std::string key = node->Value();
			if (isArray(node))
			{
				Json::Value records;
				if (0 == key.compare("DeviceList"))
				{
					DeviceListAdd(node, records);
				}
				else if (0 == key.compare("RecordList"))
				{
					DeviceListAdd(node, records);
				}
		
				if (!records.empty()) 
					result.insert(std::make_pair(key, records.toStyledString()));
			}
			else if (isTextNode(node))
			{
				// ·ÀÖ¹±ÀÀ£
				if (0 == key.compare("Info"))
					continue;

				result.insert(std::make_pair(key, GetNodeValue(node)));
			}
		}
	}

	return result;
}

void CMyDynamicStruct::DeviceListAdd(TiXmlNode* rootNode, Json::Value& records)
{
	for (auto node = rootNode->FirstChild(); node != nullptr; node = node->NextSibling())
	{
		if (node->Type() != TiXmlNode::TINYXML_DOCUMENT)
		{
			auto itemElement = node->ToElement();
			Json::Value record;
			for (auto element = itemElement->FirstChildElement();
				element;
				element = element->NextSiblingElement())
			{
				std::string value = element->GetText();
				record[element->Value()] = value.c_str();
			}

			records.append(record);
		}
	}
}
