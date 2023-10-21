#include "GB28181Server.h"

bool GB_Init(const char* concat, int loglevel)
{
	return CMySipMedia::GetInstance().Init(concat, loglevel);
}

bool GB_UnInit()
{
	return CMySipMedia::GetInstance().UnInit();
}

void GB_RegisterHandler(int handleType, DataCallback dataCB, void* user)
{
	CMySipMedia::GetInstance().RegisterHandler(handleType, dataCB, user);
}

int GB_PTZControl(const char* gbid, int controlType, int paramValue)
{
	return CMySipMedia::GetInstance().PTZControl(gbid, (PTZControlType)controlType, paramValue);
}

bool GB_Invite(const GB28181MediaContext& mediaContext, GB_TOKEN* token)
{
	return CMySipMedia::GetInstance().Invite(mediaContext, token);
}

bool GB_Subscribe(const GBSubscribeContext& subContext, GB_TOKEN* token)
{
	return CMySipMedia::GetInstance().Subscribe(subContext, token);
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