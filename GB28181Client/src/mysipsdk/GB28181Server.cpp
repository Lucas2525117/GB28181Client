#include "GB28181Server.h"

bool GB_Init(const std::string& concat, int loglevel)
{
	return CMySipMedia::GetInstance().Init(concat, loglevel);
}

void GB_RegisterHandler(int handleType, DataCallback dataCB, void* user)
{
	CMySipMedia::GetInstance().RegisterHandler(handleType, dataCB, user);
}

std::string GB_Invite(GB28181MediaContext mediaContext)
{
	return CMySipMedia::GetInstance().Invite(mediaContext);
}

int GB_QueryNetDeviceInfo(int type, const std::string& gbid)
{
	return CMySipMedia::GetInstance().QueryDeviceStatus(type, gbid);
}

void GB_QueryRecordInfo(const std::string& gbid, const GB28181MediaContext& mediaContext)
{
	CMySipMedia::GetInstance().QueryRecordInfo(gbid, mediaContext);
}

bool GB_Bye(const std::string& token)
{
	return CMySipMedia::GetInstance().Bye(token);
}