#include "RtspStreamReceiverInterface.h"
#include "RtspStreamReceiver.h"

IRtspStreamReceiver* Rtsp_CreateStreamReceiver(const char* gbUrl, StreamDataCallBack func, void* userParam)
{
	if (!gbUrl)
		return nullptr;

	IRtspStreamReceiver* receiver = nullptr;
	std::string url = gbUrl;
	if (0 == url.find("rtsp://"))    
	{
		receiver = new CRtspStreamReceiver(gbUrl, func, userParam);
	}

	return receiver;
}
