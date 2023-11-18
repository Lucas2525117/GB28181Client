#ifndef _GB_SENDERPUBLIC_H_
#define _GB_SENDERPUBLIC_H_

enum SenderSteamStatus
{
	Status_OK = 0,
	Status_NetWork_Broken,
	Status_Serve_Exiting,
};

typedef void(*PSDataCallBack)(int64_t pts, void* data, int dataLen, void* userParam);

#endif
