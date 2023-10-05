#ifndef _mysipsdk_public_h_
#define _mysipsdk_public_h_

#include <map>
#include <vector>

typedef void(*DataCallback)(int type, void* user, void* data);

typedef std::map<std::string, std::string> DynamicStruct;
typedef std::vector<DynamicStruct> DynamicVector;



#endif
