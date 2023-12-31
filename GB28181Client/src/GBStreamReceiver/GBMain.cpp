#include "StreamReceiverInterface.h"
#include "GBUdpReceiver.h"
#include "GBTcpClientReceiver.h"
#include "GBTcpServerReceiver.h"
#include <string>

IStreamReceiver* GB_CreateStreamReceiver(const char* gbUrl, StreamDataCallBack func, void* userParam)
{
	if (!gbUrl)
		return nullptr;

	IStreamReceiver* receiver = nullptr;
	std::string url = gbUrl;
	if (0 == url.find("gbudp"))         // udp
	{
		receiver = new CGBUdpStreamReceiver(gbUrl, func, userParam);
	}
	else if (0 == url.find("gbtcps"))   // tcp����
	{
		receiver = new CGBTcpServerStreamReceiver(gbUrl, func, userParam);
	}
	else if (0 == url.find("gbtcpc"))   // tcp����
	{
		receiver = new CGBTcpClientStreamReceiver(gbUrl, func, userParam);
	}

	return receiver;
}