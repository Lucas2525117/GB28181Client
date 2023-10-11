#include "GB28181Server.h"

bool GB_Init(const char* concat, int loglevel)
{
	return CMySipMedia::GetInstance().Init(concat, loglevel);
}

void GB_RegisterHandler(int handleType, DataCallback dataCB, void* user)
{
	CMySipMedia::GetInstance().RegisterHandler(handleType, dataCB, user);
}

const char* GB_Invite(GB28181MediaContext mediaContext)
{
	return CMySipMedia::GetInstance().Invite(mediaContext).c_str();
}

int GB_QueryNetDeviceInfo(int type, const char* gbid)
{
	return CMySipMedia::GetInstance().QueryDeviceStatus(type, gbid);
}

void GB_QueryRecordInfo(const char* gbid, const GB28181MediaContext& mediaContext)
{
	CMySipMedia::GetInstance().QueryRecordInfo(gbid, mediaContext);
}

bool GB_Bye(const char* token)
{
	return CMySipMedia::GetInstance().Bye(token);
}