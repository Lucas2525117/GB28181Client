#ifndef _RTSP_TCPDATA_RECEIVER_H_
#define _RTSP_TCPDATA_RECEIVER_H_

#include "StreamDefine.h"
#include "StreamPublic.h"
#include "RtspDefine.h"
#include "rtp-profile.h"
#include "rtp-demuxer.h"
#include "rtp-payload.h"
#include "rtcp-header.h"
#include "XXXInpack.h"
#include "H264Inpack.h"
#include "H265Inpack.h"
#include <memory>
#include <string>
#include <assert.h>

class CRtspTcpDataReceiver
{
public:
	CRtspTcpDataReceiver(bool isVideo, StreamDataCallBack func, void* userParam);
	~CRtspTcpDataReceiver();

	int Start(int interleave1, int interleave2, int payload, const std::string& encoding);

	bool Stop();

	void InputRtpData(const void* data, int len);
	int Package(const void* packet, int bytes, uint32_t timestamp, int flags);

private:
	int CreatePacker_(int payload, const std::string& encoding);

private:
	bool m_isVideo = false;
	StreamDataCallBack m_func;
	void* m_user;

	rtp_demuxer_t* m_demuxer = nullptr;
	std::shared_ptr<XXX2InPack> m_xxx2Inpack;
};

typedef std::shared_ptr<CRtspTcpDataReceiver> RtspTcpDataReceiverPtr;

#endif

