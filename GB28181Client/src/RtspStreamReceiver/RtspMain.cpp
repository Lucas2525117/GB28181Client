#include "RtspStreamReceiverInterface.h"
#include "RtspTcpStreamReceiver.h"
#include "RtspUdpStreamReceiver.h"

IRtspStreamReceiver* Rtsp_CreateStreamReceiver(int transType, const char* gbUrl, StreamDataCallBack func, void* userParam)
{
	if (!gbUrl)
		return nullptr;

	IRtspStreamReceiver* receiver = nullptr;
	std::string url = gbUrl;
	if (0 == url.find("rtsp://"))
	{
		if (0 == transType)              // tcp
		{
			receiver = new CRtspTcpStreamReceiver(gbUrl, func, userParam);
		}
		else if (1 == transType)         // udp
		{
			receiver = new CRtspUdpStreamReceiver(gbUrl, func, userParam);
		}
	}
	
	return receiver;
}
