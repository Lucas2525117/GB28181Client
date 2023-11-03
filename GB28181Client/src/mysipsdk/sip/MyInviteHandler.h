#ifndef _MY_INVITE_HANDLER_H_
#define _MY_INVITE_HANDLER_H_

#include <list>
#include <map>
#include "MyEventHandler.h"
#include "MyXmlParser.h"
#include "MyDynamicStruct.h"
#include "MySipContext.h"
#include "MySipInfo.h"

struct CSdp {
	std::string							m_version;			// v
	std::string							m_ownerId;			// o_owner
	std::string							m_oSessionId;		// o_sessionid
	std::string							m_oVer;				// o_version
	std::string							m_oIP;				// o_ip
	std::string							m_playType;			// s
	std::string							m_uid;				// u_id
	std::string							m_param;			// u_param
	std::string							m_cIP;				// c_ip
	std::string							m_startTime;		// t start time
	std::string							m_endTime;			// t end time
	int									m_videoPort;		// m video port
	int									m_audioPort;		// m audio port
	std::string							m_transport;		// m transport
	std::map< std::string, std::string > m_rtpMap;			// a rtpmap
	std::string							m_transType;		// a sendonly/recvonly/sendrecv
	std::string							m_downloadSpeed;	// a download speed
	std::string							m_fileSize;			// a file size
	std::string							m_setup;			// a setup active/passive
	std::string							m_connection;		// a connection:new
	std::list< std::string >            m_vkeks;            // a vkek:<version> <encryptedVKEK>
	std::string							m_ssrc;				// y ssrc
	std::string							m_f;				// f video and audio param

	CSdp()
		:/*m_version("0"),m_oSessionId("0"),m_oVer("0"),*/
		m_startTime("0"), m_endTime("0"), m_videoPort(0), m_audioPort(0),
		m_transport("RTP/AVP"), m_transType("recvonly")
	{};
};

class CMyInviteHandler : public CMyEventHandler
{
public:
	CMyInviteHandler();
	virtual ~CMyInviteHandler();

	virtual bool OnReceive(pjsip_rx_data* rdata) override;
};

#endif

