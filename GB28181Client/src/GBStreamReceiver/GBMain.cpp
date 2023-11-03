#include "StreamReceiverInterface.h"
#include "GBUdpReceiver.h"
#include "GBTcpClientReceiver.h"
#include <string>

IStreamReceiver* GB_CreateStreamReceiver(const char* gbUrl, GBDataCallBack func, void* userParam)
{
	if (!gbUrl)
		return nullptr;

	IStreamReceiver* receiver = nullptr;
	std::string url = gbUrl;
	if (0 == url.find("gbudp"))
	{
		receiver = new CGBUdpStreamReceiver(gbUrl, func, userParam);
	}
	else if (0 == url.find("gbtcpc"))   // tcp±»¶¯
	{
		receiver = new CGBTcpClientStreamReceiver(gbUrl, func, userParam);
	}
	else if (0 == url.find("gbtcps"))
	{
	}

	return receiver;
}