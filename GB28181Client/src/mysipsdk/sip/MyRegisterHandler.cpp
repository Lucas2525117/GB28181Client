#include "MyRegisterHandler.h"

static int QueryStatusWorker(void* param)
{
	CMyRegisterHandler* handler = static_cast<CMyRegisterHandler*>(param);
	handler->OnWorker();
	return 0;
}

CMyRegisterHandler::CMyRegisterHandler()
{
}

CMyRegisterHandler::~CMyRegisterHandler()
{
	StopUpdateDeviceStatus_();
}

bool CMyRegisterHandler::OnReceive(pjsip_rx_data* rdata)
{
	if (PJSIP_REGISTER_METHOD == rdata->msg_info.cseq->method.id)
	{
		auto expires = static_cast<pjsip_expires_hdr*>(pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_EXPIRES, nullptr));
		auto authHdr = static_cast<pjsip_authorization_hdr*>(pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_AUTHORIZATION, nullptr));
		if (expires && expires->ivalue > 0)
		{
			if(authHdr)
			{
				if (m_dataCB)
					m_dataCB(m_handleType, m_user, (void*)"register ok");

				Response(rdata, PJSIP_SC_OK, DateHead);
				QureryDeviceInfo(rdata);
			}
			else
			{
				Response(rdata, PJSIP_SC_UNAUTHORIZED, AuthenHead);
			}

			return true;
		}
		else if (expires && 0 == expires->ivalue)
		{
			// 增加设备注销处理(未处理认证流程,部分设备不支持)
			if (m_dataCB)
				m_dataCB(m_handleType, m_user, (void*)"unregister ok");

			Response(rdata, PJSIP_SC_OK, DateHead);
			StopUpdateDeviceStatus_();
			return true;
		}
	}
	return false;
}

void CMyRegisterHandler::OnWorker()
{
	while (m_running)
	{
		CMySipContext::GetInstance().PJThreadSleep(m_queryStatusInterval);
	}
}

void CMyRegisterHandler::QureryDeviceInfo(pjsip_rx_data* rdata)
{
	if (nullptr == m_device)
	{
		auto contactHdr = static_cast<pjsip_contact_hdr*>(pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_CONTACT, nullptr));
		auto pjUrl = static_cast<pjsip_sip_uri*>(pjsip_uri_get_uri(contactHdr->uri));
		m_device = CMyGBDeviceManager::GetInstance().Add(std::string(pjUrl->user.ptr, pjUrl->user.slen), std::string(pjUrl->host.ptr, pjUrl->host.slen), pjUrl->port);
		auto responceAddr = CMySipContext::GetInstance().GetResponseAddr(rdata);
		m_dstIp = pj_inet_ntoa(responceAddr.addr.ipv4.sin_addr);
		m_dstPort = responceAddr.addr.ipv4.sin_port;
		m_dstPort = ntohs(m_dstPort);
		m_device->SetNetAddr(m_dstIp, m_dstPort);
		//CMySipContext::GetInstance().QueryDeviveInfo(m_device.get(), m_dstIp, m_dstPort);
		//CMySipContext::GetInstance().QueryDeviveInfo(m_device.get(), m_dstIp, m_dstPort, "DeviceInfo");
		//CMySipContext::GetInstance().CreateWorkThread(QueryStatusWorker, m_queryStatusThread, this, "querydeviceInfo");
	}
}

void CMyRegisterHandler::UpdateDeviceStatus_()
{
	auto devicemaps = CMyGBDeviceManager::GetInstance().GetAllDevice();
	for (const auto& it : devicemaps)
	{
		CMySipContext::GetInstance().QueryDeviceInfo(it.second.get(), "");
	}
}

void CMyRegisterHandler::StopUpdateDeviceStatus_()
{
	m_running = false;
	if (m_queryStatusThread)
		CMySipContext::GetInstance().DestroyThread(m_queryStatusThread);
	m_queryStatusThread = nullptr;
}
