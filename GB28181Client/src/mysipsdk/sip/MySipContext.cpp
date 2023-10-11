#include "MySipContext.h"

CMySipContext::CMySipContext()
{
}

CMySipContext::~CMySipContext()
{
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
	return  pjsip_endpt_register_module(m_endPoint, module) == PJ_SUCCESS ? true : false;
}

bool CMySipContext::CreateWorkThread(pj_thread_proc* proc, pj_thread_t* workthread, void* arg, const std::string& threadName)
{
	return pj_thread_create(m_pool, threadName.c_str(), proc, arg, 0, 0, &workthread) == PJ_SUCCESS ? true : false;
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

void CMySipContext::Response(pjsip_rx_data* rdata, int st_code, int headType)
{
	pjsip_tx_data* tdata;
	pj_status_t status = pjsip_endpt_create_response(m_endPoint, rdata, st_code, nullptr, &tdata);
	if (PJ_SUCCESS != status)
		return;
	
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
		ss << std::put_time(std::localtime(&t), "%F%T");
		date = ss.str();
		//date = DateTimeFormatter::format(LocalDateTime(), "%Y-%m-%dT%H:%M:%S");
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

void CMySipContext::QueryDeviceInfo(CMyGBDevice* device, const std::string& dstIP, int dstPort, const std::string& scheme)
{
	char szQuerInfo[200] = { 0 };
	pj_ansi_snprintf(szQuerInfo, 200,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<Query>\n"
		"<CmdType>%s</CmdType>\n"
		"<SN>17430</SN>\n"
		"<DeviceID>%s</DeviceID>\n"
		"</Query>\n", scheme.c_str(), device->GetUser()
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
