#ifndef _MY_SIP_MEDIA_H_
#define _MY_SIP_MEDIA_H_

#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
#include "MySipContext.h"
#include "MySipModule.h"

class MySipDialog
{
public:
	MySipDialog(pjsip_dialog* dialog)
		: m_dialog(dialog)
		, m_handle(nullptr)
	{
	}

	void OnMainReceive(pjsip_rx_data* rdata)
	{
		if (m_handle)
		{
			m_handle(rdata);
		}
	}

	pjsip_dialog* GetDialog()
	{
		return m_dialog;
	}

	void RegisterHandle(std::function<void(pjsip_rx_data*)> handle)
	{
		m_handle = handle;
	}

private:
	pjsip_dialog* m_dialog;
	std::function<void(pjsip_rx_data*)> m_handle;
};

class CMySipMedia
{
public:
	// 单例---c++11之后线程安全
	static CMySipMedia& GetInstance()
	{
		static CMySipMedia sipMedia;
		return sipMedia;
	}

	bool Init(const std::string& concat, int loglevel);

	void RegisterHandler(int handleType, DataCallback dataCB, void* user);

	std::string Invite(GB28181MediaContext mediaContext);

	bool Bye(const std::string& token);

	void ReceiveOk(pjsip_rx_data* rdata);

	void SendAck(pjsip_rx_data* rdata);

	void ConfirmReceiveOk(pjsip_rx_data* rdata) const;

	int QueryDeviceStatus(int type, const std::string& gbid);

	int QueryRecordInfo(const std::string& gbid, const GB28181MediaContext& mediaContext);

private:
	CMySipMedia();
	~CMySipMedia();

	static pj_bool_t OnReceive(pjsip_rx_data* rdata)
	{
		auto dialog = pjsip_rdata_get_dlg(rdata);
		for (auto& dlg : GetInstance().m_dialogs)
		{
			if (dialog == dlg.second->GetDialog())
			{
				dlg.second->OnMainReceive(rdata);
			}
		}
		return true;
	}

	static void OnStateChanged(pjsip_inv_session* inv, pjsip_event* e)
	{
		if (PJSIP_EVENT_RX_MSG == e->type)
		{
			if (PJSIP_ACK_METHOD == e->body.rx_msg.rdata->msg_info.cseq->method.id)
			{
				GetInstance().OnReceive(e->body.rx_msg.rdata);
			}
		}
	}

	static void OnNewSession(pjsip_inv_session* inv, pjsip_event* e)
	{
	}

	static void OnTsxStateChanged(pjsip_inv_session* inv, pjsip_transaction* tsx, pjsip_event* e)
	{
		if (e->body.tsx_state.type == PJSIP_EVENT_RX_MSG)
		{
			GetInstance().OnReceive(e->body.tsx_state.src.rdata);
		}
	}

	static void OnRxOffer(pjsip_inv_session* inv, const pjmedia_sdp_session* offer)
	{
	}

	static pj_status_t OnRxReinvite(pjsip_inv_session* inv, const pjmedia_sdp_session* offer, pjsip_rx_data* rdata)
	{
		return PJ_SUCCESS;
	}

	static void OnCreateOffer(pjsip_inv_session* inv, pjmedia_sdp_session** p_offer)
	{
	}

	static void OnSendAck(pjsip_inv_session* inv, pjsip_rx_data* rdata)
	{
	}

	static std::string CreateRealStreamSDP(GB28181MediaContext& mediaContext)
	{
		char str[500] = { 0 };
		pj_ansi_snprintf(str, 500,
			"v=0\n"
			"o=%s 0 0 IN IP4 %s\n"
			"s=Play\n"
			"c=IN IP4 %s\n"
			"t=0 0\n"
			"m=video %d RTP/AVP 96 98 97\n"
			"a=recvonly\n"
			"a=rtpmap:96 PS/90000\n"
			"a=rtpmap:98 H264/90000\n"
			"a=rtpmap:97 MPEG4/90000\n"
			"y=0100000001\n",
			mediaContext.GetDeviceId().c_str(),
			mediaContext.GetRecvAddress().c_str(),
			mediaContext.GetRecvAddress().c_str(),
			mediaContext.GetRecvPort()
		);
		return str;
	}

private:
	std::unordered_map<std::string, std::shared_ptr<MySipDialog>> m_dialogs;
	pjsip_module*     m_mainModule = nullptr;
	pjsip_module*     m_subModule  = nullptr;
	pj_thread_t*      m_workthread = nullptr;
};

#endif

