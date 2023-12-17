#include "ZDStreamReceiverInterface.h"
#include "RtspTcpStreamReceiver.h"
#include "RtspUdpStreamReceiver.h"

IZDStreamReceiver* ZD_CreateStreamReceiver(int transType, const char* streamUrl, StreamDataCallBack func, void* userParam)
{
	if (!streamUrl)
		return nullptr;

	IZDStreamReceiver* receiver = nullptr;
	std::string url = streamUrl;
	if (0 == url.find("rtsp://"))
	{
		if (0 == transType)              // tcp
		{
			receiver = new CRtspTcpStreamReceiver(streamUrl, func, userParam);
		}
		else if (1 == transType)         // udp
		{
			receiver = new CRtspUdpStreamReceiver(streamUrl, func, userParam);
		}
	}
	
	return receiver;
}
