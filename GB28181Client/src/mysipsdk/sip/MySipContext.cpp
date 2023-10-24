#include "MySipContext.h"

CMySipContext::CMySipContext()
{
}

CMySipContext::~CMySipContext()
{
	//UnInit();
}

bool CMySipContext::Init(const std::string& concat, int logLevel)
{
	if (concat.empty() || logLevel < 0)
		return false;

	m_concat = concat;
	pj_log_set_level(logLevel);
	auto status = pj_init();

	status = pjlib_util_init();

	pj_caching_pool_init(&m_cachingPool, &pj_pool_factory_default_policy, 0);

	status = pjsip_endpt_create(&m_cachingPool.factory, nullptr, &m_endPoint);

	status = pjsip_tsx_layer_init_module(m_endPoint);

	status = pjsip_ua_init_module(m_endPoint, nullptr);
	
	//init subscribe
	status = pjsip_evsub_init_module(m_endPoint);
	status = pjsip_pres_init_module(m_endPoint, pjsip_evsub_instance());

	m_pool = pj_pool_create(&m_cachingPool.factory, "proxyapp", 4000, 4000, nullptr);
	auto pjStr = StrToPjstr(GetAddr());

	pj_sockaddr_in pjAddr;
	pjAddr.sin_family = pj_AF_INET();
	pj_inet_aton(&pjStr, &pjAddr.sin_addr);

	auto port = GetPort();
	pjAddr.sin_port = pj_htons(static_cast<pj_uint16_t>(GetPort()));
	status = pjsip_udp_transport_start(m_endPoint, &pjAddr, nullptr, 1, nullptr);
	if (status != PJ_SUCCESS) return status == PJ_SUCCESS;

	auto realm = StrToPjstr(GetLocalDomain());
	return pjsip_auth_srv_init(m_pool, &m_authentication, &realm, lookup, 0) == PJ_SUCCESS ? true : false;
}

bool CMySipContext::UnInit()
{
	pjsip_endpt_destroy(m_endPoint);
	pj_pool_release(m_pool);
	pj_caching_pool_destroy(&m_cachingPool);
	pj_shutdown();
	return true;
}

bool CMySipContext::RegisterCallback(pjsip_inv_callback* callback)
{
	return pjsip_inv_usage_init(m_endPoint, callback) == PJ_SUCCESS ? true : false;
}

bool CMySipContext::InitModule()
{
	return pjsip_100rel_init_module(m_endPoint) == PJ_SUCCESS ? true : false;
}

bool CMySipContext::RegisterModule(pjsip_module* module)
{
	if (!m_endPoint || !module)
		return false;

	return pjsip_endpt_register_module(m_endPoint, module) == PJ_SUCCESS ? true : false;
}

bool CMySipContext::UnRegisterModule(pjsip_module* module)
{
	if (!m_endPoint || !module)
		return false;

	return pjsip_endpt_unregister_module(m_endPoint, module) == PJ_SUCCESS ? true : false;
}

bool CMySipContext::CreateWorkThread(pj_thread_proc* proc, pj_thread_t* workthread, void* arg, const std::string& threadName)
{
	return pj_thread_create(m_pool, threadName.c_str(), proc, arg, 0, 0, &workthread) == PJ_SUCCESS ? true : false;
}

bool CMySipContext::DestroyThread(pj_thread_t* thread)
{
	return pj_thread_destroy(thread) == PJ_SUCCESS ? true : false;
}

void CMySipContext::PJThreadSleep(int msec)
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	//pj_thread_sleep(msec);
}

void CMySipContext::HandleEvent(pj_time_val* delay)
{
	pjsip_endpt_handle_events(m_endPoint, delay);
}

bool CMySipContext::Invite(pjsip_dialog* dlg, GB28181MediaContext mediaContext, std::string sdp)
{
	pjsip_inv_session* inv;
	if (PJ_SUCCESS != pjsip_inv_create_uac(dlg, nullptr, 0, &inv)) 
		return false;

	pjsip_tx_data* tdata = nullptr;
	if (PJ_SUCCESS != pjsip_inv_invite(inv, &tdata)) 
		return false;

	pjsip_media_type type;
	type.type = pj_str((char*)"application");
	type.subtype = pj_str((char*)"sdp");
	auto text = pj_str(const_cast<char*>(sdp.c_str()));
	tdata->msg->body = pjsip_msg_body_create(m_pool, &type.type, &type.subtype, &text);

	auto hName = pj_str((char*)"Subject");
	auto subjectUrl = mediaContext.GetDeviceId() + ":" + SiralNum + "," + GetCode() + ":" + SiralNum;
	auto hValue = pj_str(const_cast<char*>(subjectUrl.c_str()));
	auto hdr = pjsip_generic_string_hdr_create(m_pool, &hName, &hValue);
	pjsip_msg_add_hdr(tdata->msg, reinterpret_cast<pjsip_hdr*>(hdr));

	pjsip_inv_send_msg(inv, tdata);
	return true;
}

bool CMySipContext::Subscribe(pjsip_dialog* dlg, pjsip_evsub_user* pres_user, const std::string& eventName, const std::string& xmlText, const GBSubscribeContext& subContext)
{
	pjsip_evsub* sub = nullptr;
	static const pj_str_t STR_PRESENCE = { (char*)"presence", 8 };
	pj_status_t status = pjsip_evsub_create_uac(dlg, pres_user, &STR_PRESENCE, 0, &sub);
	if (PJ_SUCCESS != status)
		return false;

	pjsip_tx_data* tdata = nullptr;
	status = pjsip_evsub_initiate(sub, nullptr, subContext.GetExpires(), &tdata);
	if (PJ_SUCCESS != status)
		return false;

	pjsip_media_type type;
	type.type = pj_str((char*)"application");
	type.subtype = pj_str((char*)"MANSCDP+xml");
	auto text = pj_str(const_cast<char*>(xmlText.c_str()));
	tdata->msg->body = pjsip_msg_body_create(m_pool, &type.type, &type.subtype, &text);

	auto hName = pj_str((char*)"Subject");
	auto subjectUrl = subContext.GetDeviceId() + ":" + SiralNum + "," + GetCode() + ":" + SiralNum;
	auto hValue = pj_str(const_cast<char*>(subjectUrl.c_str()));
	auto hdr = pjsip_generic_string_hdr_create(m_pool, &hName, &hValue);
	pjsip_msg_add_hdr(tdata->msg, reinterpret_cast<pjsip_hdr*>(hdr));

	status = pjsip_evsub_send_request(sub, tdata);
	if (PJ_SUCCESS != status)
		return false;

	return true;
}

void CMySipContext::Response(pjsip_rx_data* rdata, int st_code, int headType, const std::string& text)
{
	pjsip_tx_data* tdata;
	pj_status_t status = pjsip_endpt_create_response(m_endPoint, rdata, st_code, nullptr, &tdata);
	if (PJ_SUCCESS != status)
		return;

	if (!text.empty())
	{
		pjsip_media_type type;
		type.type = pj_str((char*)"application");
		type.subtype = pj_str((char*)"MANSCDP+xml");
		auto info = pj_str(const_cast<char*>(text.c_str()));
		tdata->msg->body = pjsip_msg_body_create(m_pool, &type.type, &type.subtype, &info);
	}
	
	std::string date;
	pj_str_t c;
	pj_str_t key;
	pjsip_hdr* hdr;
	std::stringstream ss;
	time_t t;
	switch (headType)
	{
	case DateHead:
		key = pj_str((char*)"Date");
		t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		ss << std::put_time(std::localtime(&t), "%FT%T");
		date = ss.str();
		hdr = reinterpret_cast<pjsip_hdr*>(pjsip_date_hdr_create(m_pool, &key, pj_cstr(&c, date.c_str())));
		pjsip_msg_add_hdr(tdata->msg, hdr);
		break;
	case AuthenHead:
		pjsip_auth_srv_challenge(&m_authentication, nullptr, nullptr, nullptr, PJ_FALSE, tdata);
		break;
	default:
		break;
	}

	pjsip_response_addr addr;
	status = pjsip_get_response_addr(m_pool, rdata, &addr);
	if (PJ_SUCCESS != status)
	{
		pjsip_tx_data_dec_ref(tdata);
		return;
	}

	status = pjsip_endpt_send_response(m_endPoint, &addr, tdata, nullptr, nullptr);
	if (PJ_SUCCESS != status)
	{
		pjsip_tx_data_dec_ref(tdata);
	}
	// pjsip_endpt_respond_stateless(endPoint, rdata, 200, NULL,  NULL, NULL);
}

pjsip_response_addr CMySipContext::GetResponseAddr(pjsip_rx_data* rdata)
{
	pjsip_response_addr addr;
	pjsip_get_response_addr(m_pool, rdata, &addr);
	return addr;
}

void CMySipContext::QueryDeviceInfo(CMyGBDevice* device, const std::string& gbid, const std::string& scheme)
{
	char szQuerInfo[200] = { 0 };
	pj_ansi_snprintf(szQuerInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Query>\n"
		"<CmdType>%s</CmdType>\n"
		"<SN>17430</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"</Query>\n", scheme.c_str(), gbid.c_str()
	);

	pjsip_tx_data* tdata = nullptr;
	const pjsip_method method = { PJSIP_OTHER_METHOD,{ (char*)"MESSAGE", 7 } };
	auto text = StrToPjstr(std::string(szQuerInfo));
	auto target = StrToPjstr(device->GetSipIpUrl());
	auto from = StrToPjstr(m_concat);
	auto to = StrToPjstr(device->GetSipCodecUrl());
	auto contact = StrToPjstr(m_concat);
	pjsip_endpt_create_request(m_endPoint, &method, &target, &from, &to, &contact, nullptr, -1, &text, &tdata);

	tdata->msg->body->content_type.type = pj_str((char*)"Application");
	tdata->msg->body->content_type.subtype = pj_str((char*)"MANSCDP+xml");
	pjsip_endpt_send_request(m_endPoint, tdata, -1, nullptr, nullptr);
}

void CMySipContext::QueryRecordInfo(CMyGBDevice* device, const std::string& gbid, const std::string& startTime, const std::string& endTime, const std::string& scheme)
{
	char szRecordInfo[400] = { 0 };
	pj_ansi_snprintf(szRecordInfo, 400,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Query>\n"
		"<CmdType>%s</CmdType>\n"
		"<SN>17430</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<StartTime>%s</StartTime>\n"
		"<EndTime>%s</EndTime>\n"
		"<FilePath></FilePath>\n"
		"<Address></Address>\n"
		"<Secrecy>0</Secrecy>\n"
		"<Type>all</Type>\n"
		"<RecorderID>all</RecorderID>\n"
		"</Query>\n", scheme.c_str(), gbid.c_str(), startTime.c_str(), endTime.c_str()
	);

	pjsip_tx_data* tdata = nullptr;
	const pjsip_method method = { PJSIP_OTHER_METHOD,{ (char*)"MESSAGE", 7 } };
	auto text = StrToPjstr(std::string(szRecordInfo));
	auto target = StrToPjstr(device->GetSipIpUrl());
	auto from = StrToPjstr(m_concat);
	auto to = StrToPjstr(device->GetSipCodecUrl());
	auto contact = StrToPjstr(m_concat);
	pjsip_endpt_create_request(m_endPoint, &method, &target, &from, &to, &contact, nullptr, -1, &text, &tdata);

	tdata->msg->body->content_type.type = pj_str((char*)"Application");
	tdata->msg->body->content_type.subtype = pj_str((char*)"MANSCDP+xml");
	pjsip_endpt_send_request(m_endPoint, tdata, -1, nullptr, nullptr);
}

int CMySipContext::SendSipMessage(CMyGBDevice* device, const std::string& sipMsg)
{
	pjsip_tx_data* tdata = nullptr;
	const pjsip_method method = { PJSIP_OTHER_METHOD,{ (char*)"MESSAGE", 7 } };
	auto text = StrToPjstr(sipMsg);
	auto target = StrToPjstr(device->GetSipIpUrl());
	auto from = StrToPjstr(m_concat);
	auto to = StrToPjstr(device->GetSipCodecUrl());
	auto contact = StrToPjstr(m_concat);
	pjsip_endpt_create_request(m_endPoint, &method, &target, &from, &to, &contact, nullptr, -1, &text, &tdata);

	tdata->msg->body->content_type.type = pj_str((char*)"Application");
	tdata->msg->body->content_type.subtype = pj_str((char*)"MANSCDP+xml");
	return pjsip_endpt_send_request(m_endPoint, tdata, -1, nullptr, nullptr);
}

int CMySipContext::PTZControl(CMyGBDevice* device, const std::string& gbid, PTZControlType ptzType, int paramValue)
{
	std::string result = ParsePTZCmd(device, gbid, ptzType, paramValue);
	return SendSipMessage(device, result);
}

std::string CMySipContext::GetMessageBody(pjsip_rx_data* rdata)
{
	auto type = static_cast<pjsip_ctype_hdr*>(pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_CONTENT_TYPE, nullptr));
	if (type)
	{
		return std::string(static_cast<char*>(rdata->msg_info.msg->body->data), rdata->msg_info.msg->body->len);
	}
	return std::string();
}

pj_str_t CMySipContext::StrToPjstr(const std::string& input)
{
	pj_str_t pjStr;
	pj_strdup2_with_null(m_pool, &pjStr, const_cast<char*>(input.c_str()));
	return pjStr;
}

std::string CMySipContext::GetAddr()
{
	size_t start = m_concat.find_first_of("@");
	size_t end = m_concat.find_last_of(":");
	return m_concat.substr(start + 1, end - start - 1);
}

int CMySipContext::GetPort()
{
	size_t start = m_concat.find_last_of(":");
	size_t end = m_concat.find_first_of(">");
	return std::stoi(m_concat.substr(start + 1, end - start - 1));
}

std::string CMySipContext::GetLocalDomain()
{
	size_t start = m_concat.find_first_of(":");
	int length = 10;
	return m_concat.substr(start + 1, length);
}

std::string CMySipContext::GetCode()
{
	size_t start = m_concat.find_first_of(":");
	size_t end = m_concat.find_first_of("@");
	return m_concat.substr(start + 1, end - start - 1);
}

std::string CMySipContext::ParsePTZCmd(CMyGBDevice* device, const std::string& gbid, PTZControlType ptzType, int paramValue)
{
	unsigned char ptzCmdStr[8] = { 0xA5, 0x0F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };
	PTZControlType type = ptzType;
	if (0 == paramValue)
		type = PTZ_CTRL_HALT;

	switch (type)
	{
	case PTZ_CTRL_HALT:
		break;
	case PTZ_CTRL_RIGHT:     // 右
		ptzCmdStr[3] = 0x01;
		ptzCmdStr[4] = paramValue & 0xFF;
		break;
	case PTZ_CTRL_RIGHTUP:   // 右上
		ptzCmdStr[3] = 0x09;
		ptzCmdStr[4] = paramValue & 0xFF;
		ptzCmdStr[5] = paramValue & 0xFF;
		break;
	case PTZ_CTRL_UP:        // 上
		ptzCmdStr[3] = 0x08;
		ptzCmdStr[5] = paramValue & 0xFF;
		break;
	case PTZ_CTRL_LEFTUP:    // 左上
		ptzCmdStr[3] = 0x0A;
		ptzCmdStr[4] = paramValue & 0xFF;
		ptzCmdStr[5] = paramValue & 0xFF;
		break;
	case PTZ_CTRL_LEFT:      // 左
		ptzCmdStr[3] = 0x02;
		ptzCmdStr[4] = paramValue & 0xFF;
		break;
	case PTZ_CTRL_LEFTDOWN:  // 左下
		ptzCmdStr[3] = 0x06;
		ptzCmdStr[4] = paramValue & 0xFF;
		ptzCmdStr[5] = paramValue & 0xFF;
		break;
	case PTZ_CTRL_DOWN:      // 下
		ptzCmdStr[3] = 0x04;
		ptzCmdStr[5] = paramValue & 0xFF;
		break;
	case PTZ_CTRL_RIGHTDOWN: // 右下
		ptzCmdStr[3] = 0x05;
		ptzCmdStr[4] = paramValue & 0xFF;
		ptzCmdStr[5] = paramValue & 0xFF;
		break;
	case PTZ_CTRL_ZOOM:
		if (paramValue > 0)
		{
			ptzCmdStr[3] = 0x10;
			ptzCmdStr[6] = (paramValue & 0x0F) << 4;
		}
		else if (paramValue < 0)
		{
			ptzCmdStr[3] = 0x20;
			ptzCmdStr[6] = ((-paramValue) & 0x0F) << 4;
		}
		break;
	case PTZ_CTRL_IRIS:
		if (paramValue > 0)
		{
			ptzCmdStr[3] = 0x44;
			ptzCmdStr[5] = paramValue & 0xFF;
		}
		else if (paramValue < 0)
		{
			ptzCmdStr[3] = 0x48;
			ptzCmdStr[5] = (-paramValue) & 0xFF;
		}
		break;
	case PTZ_CTRL_FOCUS:
		if (paramValue > 0)
		{
			ptzCmdStr[3] = 0x41;
			ptzCmdStr[4] = paramValue & 0xFF;
		}
		else if (paramValue < 0)
		{
			ptzCmdStr[3] = 0x42;
			ptzCmdStr[4] = (-paramValue) & 0xFF;
		}
		break;
	default:
		break;
	}

	std::string cmdstr;
	char tmp[8] = { 0, };
	for (int i = 0; i < 7; i++)
	{
		ptzCmdStr[7] += ptzCmdStr[i];
		sprintf(tmp, "%02X", ptzCmdStr[i]);
		cmdstr += tmp;
	}
	sprintf(tmp, "%02X", ptzCmdStr[7]);
	cmdstr += tmp;

	char szPTZInfo[200] = { 0 };
	pj_ansi_snprintf(szPTZInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Control>\n"
		"<CmdType>DeviceControl</CmdType>\n"
		"<SN>17430</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"<PTZCmd>%s</PTZCmd>\n"
		"</Control>\n", gbid.c_str(), cmdstr.c_str()
	);

	return szPTZInfo;
}
