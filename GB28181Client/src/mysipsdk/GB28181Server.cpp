#include "GB28181Server.h"
#include "GBDeviceControl.h"
#include "GBDeviceConfig.h"

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

int GB_DeviceControl(const char* gbid, const char* method, const char* request)
{
	return CGBDeviceControl::GetInstance().SetCommand(gbid, method, request);
}

int GB_DeviceConfig(const char* gbid, const char* method, const char* request)
{
	return CGBDeviceConfig::GetInstance().SetCommand(gbid, method, request);
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

int GB_VoiceBroadcast(const char* gbid, const char* sourceID, const char* targetID)
{
	return CMySipMedia::GetInstance().VoiceBroadcast(gbid, sourceID, targetID);
}

bool GB_Bye(const char* token)
{
	return CMySipMedia::GetInstance().Bye(token);
}