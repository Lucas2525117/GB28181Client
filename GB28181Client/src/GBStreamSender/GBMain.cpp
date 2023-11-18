#include "StreamSenderInterface.h"
#include "GBUdpSender.h"
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

	return sender;
}