#include "ZDStreamReceiverInterface.h"
#include "RtmpTcpStreamReceiver.h"
#include <string>

IZDStreamReceiver* ZD_CreateStreamReceiver(int transType, const char* streamUrl, StreamDataCallBack func, void* userParam)
{
	if (!streamUrl)
		return nullptr;

	IZDStreamReceiver* receiver = nullptr;
	std::string url = streamUrl;
	if (0 == url.find("rtmp://"))
	{
		if (0 == transType)              // tcp
		{
			receiver = new CRtmpTcpStreamReceiver(streamUrl, func, userParam);
		}
		else if (1 == transType)         // udp
		{
		}
	}

	return receiver;
}