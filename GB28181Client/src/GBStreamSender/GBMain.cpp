#include "StreamSenderInterface.h"
#include "GBUdpSender.h"
#include "GBTcpServerSender.h"
#include "GBTcpClientSender.h"
#include <string>

IStreamSender* GB_CreateStreamSender(const char* gbUrl)
{
	if (!gbUrl)
		return nullptr;

	IStreamSender* sender = nullptr;
	std::string url = gbUrl;
	if (0 == url.find("gbudp"))         // udp
	{
		sender = new CGBUdpSender(gbUrl);
	}
	else if (0 == url.find("gbtcps"))
	{
		sender = new CGBTcpServerSender(gbUrl);
	}
	else if (0 == url.find("gbtcpc"))
	{
		sender = new CGBTcpClientSender(gbUrl);
	}

	return sender;
}