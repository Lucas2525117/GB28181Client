#ifndef _MY_SIP_CONTEXT_H_
#define _MY_SIP_CONTEXT_H_

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iosfwd>
#include <sstream>
#include "MySip.h"
#include "MySipHeader.h"
#include "MyGBDevice.h"

#define NoHead     0
#define DateHead   1
#define AuthenHead 2

#define SiralNum  "015359105"

class CMySipContext
{
public:
	static CMySipContext& GetInstance()
	{
		static CMySipContext sipContext;
		return sipContext;
	}

	bool Init(const std::string& concat, int logLevel);

	bool RegisterCallback(pjsip_inv_callback* callback);

	bool InitModule();

	bool RegisterModule(pjsip_module* module);

	bool CreateWorkThread(pj_thread_proc* proc, pj_thread_t* workthread, void* arg, const std::string& threadName);

	void HandleEvent(pj_time_val* delay);

	bool Invite(pjsip_dialog* dlg, GB28181MediaContext mediaContext, std::string sdp);

	void Response(pjsip_rx_data* rdata, int st_code, int headType);

	pjsip_response_addr GetResponseAddr(pjsip_rx_data* rdata);

	void QueryDeviveInfo(CMyGBDevice* device, const std::string& dstIP, int dstPort, const std::string& scheme = "Catalog");

	std::string GetMessageBody(pjsip_rx_data* rdata);

	std::string GetConcat() { return m_concat; }

	pj_str_t StrToPjstr(const std::string& input);

private:
	CMySipContext();
	~CMySipContext();

	static pj_status_t lookup(pj_pool_t* pool,
		const pj_str_t* realm,
		const pj_str_t* accName,
		pjsip_cred_info* credInfo)
	{
		return PJSIP_SC_OK;
	}

private:
	std::string GetAddr();
	int GetPort();
	std::string GetLocalDomain();
	std::string GetCode();

private:
	std::string        m_concat;
	pj_pool_t*         m_pool = nullptr;
	pj_caching_pool    m_cachingPool;
	pjsip_endpoint*    m_endPoint = nullptr;
	pjsip_auth_srv     m_authentication;
};

#endif

