#include "MyInviteHandler.h"

static bool SdpDecode(const char* body, CSdp& sdp)
{
	if (nullptr == body)
	{
		sdp.m_videoPort = 65535; // 针对400等错误做特殊处理
		return true;
	}

	const char* pContent = body;
	const char* anchor = nullptr;
	std::string line;

	// 解析SDP字段
	while (pContent && *pContent != '\0')
	{
		char ch = *pContent++;
		if (*pContent++ != '=')
		{
			return false;
		}

		anchor = pContent;
		while (*pContent != '\0' && *pContent != '\r' && *pContent != '\n') pContent++;
		line.assign(anchor, pContent);

		if (ch == 'v')
		{
			//v=0
			sdp.m_version = line;
		}
		else if (ch == 'o')
		{
			//o=31010600002000000001 0 0 IN IP4 192.168.12.150
			std::istringstream stream(line);
			std::string owner, sid, ver, ip, tmp;
			stream >> owner >> sid >> ver >> tmp >> tmp >> ip;
			if (owner.empty() || sid.empty() || ver.empty() || ip.empty())
			{
				return false;
			}

			sdp.m_ownerId = owner;
			sdp.m_oSessionId = sid;
			sdp.m_oVer = ver;
			sdp.m_oIP = ip;
		}
		else if (ch == 's')
		{
			//s=Play
			sdp.m_playType = line;
		}
		else if (ch == 'u')
		{
			//u=31010602001310000001:1
			std::string::size_type p = line.find(':');
			if (p != std::string::npos)
			{
				sdp.m_uid = line.substr(0, p);
				sdp.m_param = line.substr(p + 1);
			}
			else
			{
				sdp.m_uid = line;
			}
		}
		else if (ch == 'c')
		{
			//c=IN IP4 192.168.12.150
			std::istringstream stream(line);
			std::string ip, tmp;
			stream >> tmp >> tmp >> ip;
			if (ip.empty())
			{
				return false;
			}

			sdp.m_cIP = ip;
		}
		else if (ch == 't')
		{
			//t=0 0
			std::istringstream stream(line);
			std::string start, end;
			stream >> start >> end;
			if (start.empty() || end.empty())
			{
				return false;
			}

			sdp.m_startTime = start;
			sdp.m_endTime = end;
		}
		else if (ch == 'm')
		{
			//m=video 57576 RTP/AVP 96 97 98
			//m=audio 57576 RTP/AVP 96 97 98
			std::istringstream stream(line);
			std::string avtype, transport;
			int port = 0;
			stream >> avtype >> port >> transport;
			if (0 == port) port = 40001;//假端口
			if (avtype.empty() || port == 0)
			{
				return false;
			}

			if (avtype == "video")
				sdp.m_videoPort = port;
			if (avtype == "audio")
				sdp.m_audioPort = port;
			sdp.m_transport = transport;
		}
		else if (ch == 'a')
		{
			//a=recvonly
			//a=rtpmap:96 PS/90000
			//a=rtpmap:97 MPEG4/90000
			//a=rtpmap:98 H264/90000

			if (line.find("rtpmap:") != std::string::npos)
			{
				line = line.substr(7);
				std::istringstream stream(line);
				std::string key, value;
				stream >> key >> value;
				if (key.empty() || value.empty())
				{
					return false;
				}

				sdp.m_rtpMap[key] = value;
			}
			else if (line.find("downloadspeed:") != std::string::npos)
			{
				sdp.m_downloadSpeed = line.substr(14);
			}
			else if (line.find("filesize:") != std::string::npos)
			{
				sdp.m_fileSize = line.substr(9);
			}
			else if (line.find("setup:") != std::string::npos)
			{
				sdp.m_setup = line.substr(6);
			}
			else if (line.find("connection:") != std::string::npos)
			{
				sdp.m_connection = line.substr(11);
			}
			else if (line.find("recvonly") != std::string::npos
				|| line.find("sendonly") != std::string::npos
				|| line.find("sendrecv") != std::string::npos)
			{
				sdp.m_transType = line;
			}
			else if (line.find("vkek:") != std::string::npos)
			{
				sdp.m_vkeks.push_back(line.substr(5));
			}
		}
		else if (ch == 'y')
		{
			sdp.m_ssrc = line;
		}
		else if (ch == 'f')
		{
			sdp.m_f = line;
		}

		// next line
		while (*pContent == '\r' || *pContent == '\n') pContent++;
	}

	return true;
}

CMyInviteHandler::CMyInviteHandler()
{
}

CMyInviteHandler::~CMyInviteHandler()
{
}

bool CMyInviteHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_INVITE_METHOD == rdata->msg_info.cseq->method.id)
	{
		std::string bodyData = CMySipContext::GetInstance().GetMessageBody(rdata);
		if (bodyData.empty())
			return false;

		CSdp recvSdp;
		if (!SdpDecode(bodyData.c_str(), recvSdp))
			return false;

		if (Type_VideoInvite == m_handleType)
		{
			CMyVideoInviteInfo inviteInfo;
			inviteInfo.deviceIP = recvSdp.m_cIP;
			inviteInfo.transport = recvSdp.m_videoPort;

			if (m_dataCB)
				m_dataCB(m_handleType, m_user, &inviteInfo);

			return true;
		}
		else if (Type_Invite == m_handleType)
		{
			if (m_dataCB)
				m_dataCB(m_handleType, m_user, nullptr);
		}

		char sendSdp[500] = { 0 };
		pj_ansi_snprintf(sendSdp, 500,
			"v=0\n"
			"o=%s 0 0 IN IP4 %s\n"
			"s=Play\n"
			"c=IN IP4 %s\n"
			"t=0 0\n"
			"m=audio 40000 RTP/AVP 8\n"	
			"a=rtpmap:8 PCMA/8000\n"
			"a=sendonly\n"
			"y=%s\n"
			, CMySipContext::GetInstance().GetSipCode().c_str()
			, CMySipContext::GetInstance().GetSipIP().c_str()
			, CMySipContext::GetInstance().GetSipIP().c_str()
			, recvSdp.m_ssrc.c_str()
		);

		Response(rdata, PJSIP_SC_OK, NoHead, sendSdp, SUBTYPE_SDP);
		return true;
	}
	else if (PJSIP_OTHER_METHOD == rdata->msg_info.cseq->method.id)
	{
		
	}

	return false;
}