#ifndef _RTMP_COMMAND_H_
#define _RTMP_COMMAND_H_

#include "StreamPublic.h"
#include "StreamDefine.h"
#include "rtmp-client.h"
#include "flv-demuxer.h"
#include "flv-proto.h"
#include "ZDTcpClient.h"
#include "XXXInpack.h"
#include "H264Inpack.h"
#include "H265Inpack.h"
#include <string>
#include <assert.h>

class CRtmpCommand
{
public:
	CRtmpCommand(ZDTcpClientPtr tcpClient, StreamDataCallBack func, void* userParam);
	~CRtmpCommand();

	int Create(const std::string& appName, const std::string& playPath, const std::string& tcurl);

	void Destroy();

	int Start(int publish);

	int InputData(void* data, int len);

public:
	int Send(const void* header, int headerLen, const void* body, int bodyLen);

	int PlayVideo(const void* data, size_t bytes, uint32_t timestamp);

	int PlayAudio(const void* data, size_t bytes, uint32_t timestamp);

	int PlayScript(const void* data, size_t bytes, uint32_t timestamp);

	int Package(int codec, const void* data, size_t bytes, uint32_t pts, uint32_t dts, int flags);

private:
	ZDTcpClientPtr         m_tcpClient;
	StreamDataCallBack     m_func;
	void*                  m_user;

	flv_demuxer_t* m_flvDemuxer = nullptr;
	rtmp_client_t* m_rtmp = nullptr;
	std::shared_ptr<XXX2InPack> m_xxx2Inpack;
};

typedef std::shared_ptr<CRtmpCommand> RtmpCommandPtr;

#endif