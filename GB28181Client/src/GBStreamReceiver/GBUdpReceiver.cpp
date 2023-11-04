#include "GBUdpReceiver.h"

#include "rtpsessionparams.h"
#include "rtpudpv4transmitter.h"
#include "rtppacket.h"
#include "rtpsources.h"
#include "rtpsourcedata.h"
#include "rtpaddress.h"
#include "rtpipv4address.h"

#define GB_UDP_RECV_BUF_SIZE (1024)

int VideoWorkerThread(void* param)
{
	assert(param);
	CGBUdpStreamReceiver* receiver = (CGBUdpStreamReceiver*)param;
	receiver->VideoDataWorker();
	return 0;
}

int AudioWorkerThread(void* param)
{
	assert(param);
	CGBUdpStreamReceiver* receiver = (CGBUdpStreamReceiver*)param;
	receiver->AudioDataWorker();
	return 0;
}

CGBUdpStreamReceiver::CGBUdpStreamReceiver(const char* gbUrl, GBDataCallBack func, void* userParam)
	: m_gbUrl(gbUrl)
	, m_func(func)
	, m_user(userParam)
{
	ParseUrl_(gbUrl);
	InitRtpSession_();
}

CGBUdpStreamReceiver::~CGBUdpStreamReceiver()
{
	Stop();

	if (m_parse)
	{
		delete m_parse;
		m_parse = nullptr;
	}

	if (m_G711AParse)
	{
		delete m_G711AParse;
		m_G711AParse = nullptr;
	}

	if (m_recvBuf)
		free(m_recvBuf);
	m_recvBuf = nullptr;

	for (int i = 0; i < ARRAY_SIZE; ++i)
	{
		if (m_packets[i])
			free(m_packets[i]);
		m_packets[i] = nullptr;
	}
}

void CGBUdpStreamReceiver::DeleteThis()
{
	delete this;
}

int CGBUdpStreamReceiver::Start(int streamType)
{
	if (!m_running)
		return -1;

	if(0 == streamType)
		m_thread = std::thread(VideoWorkerThread, this);
	else if(1 == streamType)
		m_thread = std::thread(AudioWorkerThread, this);
	m_streamType = streamType;
	return 0;
}

int CGBUdpStreamReceiver::Stop()
{
	m_running = false;
	if(m_thread.joinable())
		m_thread.join();
	return 0;
}

// gbudp://192.168.1.2:36000
int CGBUdpStreamReceiver::ParseUrl_(const std::string& gburl)
{
	size_t pos = gburl.find("gbudp://");
	if (std::string::npos == pos)
		return -1;

	std::string ipp = gburl.substr(pos+8, gburl.length());
	size_t pos_ipp = ipp.find(":");
	if (std::string::npos == pos_ipp)
		return -1;

	m_localIP = ipp.substr(0, pos_ipp);
	m_localPort = atoi(ipp.substr(pos_ipp + 1, ipp.length()).c_str());
	return 0;
}

int CGBUdpStreamReceiver::InitRtpSession_()
{
	RTPSessionParams sessionParams;
	sessionParams.SetMinimumRTCPTransmissionInterval(10);
	sessionParams.SetOwnTimestampUnit(1.0 / 90000.0);
	sessionParams.SetAcceptOwnPackets(true);

	RTPUDPv4TransmissionParams udpV4Params;
	udpV4Params.SetPortbase(m_localPort);
	udpV4Params.SetRTPReceiveBuffer(GB_UDP_RECV_BUF_SIZE* GB_UDP_RECV_BUF_SIZE*16);

	if (0 != Create(sessionParams, &udpV4Params))
		return -1;

	return 0;
}

int CGBUdpStreamReceiver::UdpConnect_()
{
	return 0;
}

int CGBUdpStreamReceiver::PackData_(struct rtp_packet* packet)
{
	if (m_dataLen + packet->len > m_bufSize)
	{
		unsigned char* ptr = (unsigned char*)realloc(m_recvBuf, m_bufSize + GB_UDP_RECV_BUF_SIZE * GB_UDP_RECV_BUF_SIZE);
		if (!ptr)
		{
			return -1;
		}

		m_recvBuf = ptr;
		m_bufSize += GB_UDP_RECV_BUF_SIZE * GB_UDP_RECV_BUF_SIZE;
	}

	if (packet->mark)
	{
		memcpy(m_recvBuf + m_dataLen, packet->data, packet->len);
		m_dataLen += packet->len;
		ProcessData_(packet->pts);
		m_dataLen = 0;
	}
	else if (packet->pts != m_lastPts && m_lastPts != 0)
	{
		if (m_dataLen > 0)
			ProcessData_(m_lastPts);

		memcpy(m_recvBuf, packet->data, packet->len);
		m_dataLen = packet->len;
	}
	else
	{
		memcpy(m_recvBuf + m_dataLen, packet->data, packet->len);
		m_dataLen += packet->len;
	}

	m_lastPts = packet->pts;
	m_lastSeq = packet->seq;

	return 0;
}

int CGBUdpStreamReceiver::ProcessData_(uint32_t pts)
{
	if (!m_parse && 0 != m_payload)
	{
		m_parse = new(std::nothrow) CPSParse(m_func, m_user); // 默认采用ps封装
		if (0 != m_baseTime)
			m_parse->SetBaseTime(m_baseTime);
	}

	// 有丢包，丢包标记只看同一帧数据包内是否有丢包
	if (m_lost)
	{
		m_needIframe = true;
		m_lost = false;
		return 0;
	}

	// 定位到下一个完整i帧
	if (m_needIframe)
	{
		int i = 0;
		int len = m_dataLen > 50 ? 50 : m_dataLen;
		while (i + 3 < len)
		{
			if (0x00 == m_recvBuf[i] && 0x00 == m_recvBuf[i + 1] && 0x01 == m_recvBuf[i + 2] && 0xbb == m_recvBuf[i + 3])
			{
				m_needIframe = false;

				break;
			}

			++i;
		}
	}

	if (m_parse && !m_needIframe)
	{
		m_parse->InputData(m_recvBuf, m_dataLen);
	}

	return 0;
}

int CGBUdpStreamReceiver::ProcessCachePackets_(struct rtp_packet* packet)
{
	int count = -1;
	uint16_t seq = (m_lastSeq + 1) % ARRAY_SIZE;
	do
	{
		count++;
		while (m_packets[seq])
		{
			struct rtp_packet* p = m_packets[seq];
			m_packets[seq] = nullptr;
			PackData_(p);
			free(p);

			seq = (m_lastSeq + 1) % ARRAY_SIZE;
		}

		// 空包，完成
		if (!packet)
			break;
		// 重复包，不处理
		if (m_lastSeq == packet->seq)
			break;

		// 当前缓冲位置有数据，需要先处理掉
		if (m_packets[packet->seq % ARRAY_SIZE])
		{
			seq = (seq + 1) % ARRAY_SIZE;
			continue;
		}

		// 处理完缓存数据后，正好是下一个待处理的数据，直接处理
		if (m_lastSeq == packet->seq - 1)
		{
			PackData_(packet);
		}
		else // 否则加入缓存
		{
			size_t headLen = sizeof(rtp_packet);
			struct rtp_packet* p = (struct rtp_packet*)malloc(headLen + packet->len);
			if (p)
			{
				memcpy(p ,packet, headLen);
				memcpy((uint8_t*)p + headLen, packet->data, packet->len);
				p->data = (uint8_t*)p + headLen;

				m_packets[packet->seq % ARRAY_SIZE] = p;
			}
		}

		break;
	} while (1);

	return 0;
}

int CGBUdpStreamReceiver::PackAudioData_(void* data, int len)
{
	if (!m_G711AParse && 0 != m_payload)
	{
		m_G711AParse = new(std::nothrow) CG711AParse(m_func, m_user);
	}

	if (m_G711AParse)
	{
		m_G711AParse->InputData(data, len);
	}

	return 0;
}

SOCKET CGBUdpStreamReceiver::UDPCreate_()
{
	WSADATA  wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		// add log
		return INVALID_SOCKET;
	}

	return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

bool CGBUdpStreamReceiver::UDPBind_(SOCKET sock, int port)
{
	SOCKADDR_IN bindAddress;   //绑定地址
	bindAddress.sin_family = AF_INET;
	bindAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	bindAddress.sin_port = htons(port);
	auto ret = bind(sock, (sockaddr*)&bindAddress, sizeof(SOCKADDR));
	if (SOCKET_ERROR == ret)
		return false;

	return true;
}

void CGBUdpStreamReceiver::UDPClose_(SOCKET sock)
{
	if (INVALID_SOCKET != sock)
	{
		closesocket(sock);
		WSACleanup();
	}
}

void CGBUdpStreamReceiver::VideoDataWorker()
{
	uint8_t payload;
	while (m_running)
	{
		Poll();
		BeginDataAccess();

		if (GotoFirstSourceWithData())
		{
			do
			{
				RTPPacket* packet = nullptr;
				while (nullptr != (packet = GetNextPacket()))
				{
					payload = packet->GetPayloadType();
					if (0 == payload)
					{
						DeletePacket(packet);
						continue;
					}

					struct rtp_packet data;
					data.mark = packet->HasMarker();
					data.pts = packet->GetTimestamp();
					data.seq = packet->GetSequenceNumber();
					data.data = packet->GetPayloadData();
					data.len = packet->GetPayloadLength();

					m_payload = payload;

					if (m_lastSeq < 0)
					{
						m_lastSeq = data.seq - 1;
					}

					if (m_lastSeq = data.seq - 1)
					{
						PackData_(&data);
						ProcessCachePackets_(NULL);
					}
					else
					{
						ProcessCachePackets_(&data);
					}

					DeletePacket(packet);
				}

			} while (GotoNextSourceWithData());
		}

		EndDataAccess();
		Sleep(30);
	}

	Destroy();
}

void CGBUdpStreamReceiver::AudioDataWorker()
{
	uint8_t payload;
	while (m_running)
	{
		Poll();
		BeginDataAccess();

		if (GotoFirstSourceWithData())
		{
			do
			{
				RTPPacket* packet = nullptr;
				while (nullptr != (packet = GetNextPacket()))
				{
					// 8:PCMA
					payload = packet->GetPayloadType();
					if (0 == payload)
					{
						DeletePacket(packet);
						continue;
					}

					struct rtp_packet data;
					memset(&data, 0, sizeof(rtp_packet));
					data.mark = packet->HasMarker();
					data.pts = packet->GetTimestamp();
					data.seq = packet->GetSequenceNumber();
					data.data = packet->GetPayloadData();
					data.len = packet->GetPayloadLength();

					m_payload = payload;
					if (m_lastSeq < 0)
					{
						m_lastSeq = data.seq - 1;
					}

					if (m_lastSeq = data.seq - 1)
					{
						PackAudioData_(data.data, data.len);
					}

					DeletePacket(packet);
				}

			} while (GotoNextSourceWithData());
		}

		EndDataAccess();
		Sleep(30);
	}

	Destroy();
}
