#ifndef _SIP_HEADER_H_
#define _SIP_HEADER_H_

enum HandlerType
{
	Type_Register = 1,
	Type_KeepAlive,
	Type_RecvCatalog,
	Type_RecvStatus,
	Type_RecvRecordInfo,
	Type_RecvDeviceInfo,
	Type_RecvDeviceStatus,
	Type_Download,
	Type_Alarm,
};

enum StreamRequiredType
{
	StreamType_RealStream = 0,
	StreamType_Playback = 1,
	StreamType_Download = 2
};

enum SubscribeType
{
	Alarm_Subscribe = 0,          // 报警订阅
	MobilePosition_Subscribe,     // 移动设备订阅
	PTZPosition_Subscribe,        // PTZ精准位置变化订阅
	Catalog_Subscribe,            // 目录订阅
};

enum DeviceControlType
{
	ControlType_PTZCmd = 0,         // 云台控制
	ControlType_TeleBoot,           // 远程控制
	ControlType_RecordCmd,          // 录像控制
	ControlType_GuardCmd,           // 报警布防/撤防
	ControlType_AlarmCmd,           // 报警复位
	ControlType_IFrameCmd,          // 强制关键帧
	ControlType_DragZoomIn,         // 拉框放大
	ControlType_DragZoomOut,        // 拉框缩小
	ControlType_HomePosition,       // 看守位
	ControlType_PTZPreciseCtrl,     // PTZ精准控制
	ControlType_DeviceUpgrade,      // 设备软件升级
	ControlType_FormatSDCard,       // 存储卡格式化
	ControlType_TargetTrack,        // 目标跟踪
};

enum PTZControlType
{
	PTZ_CTRL_HALT = 0,			// 停止
	PTZ_CTRL_RIGHT,		        // 右转 值越大速度越快 0~255
	PTZ_CTRL_RIGHTUP,		    // 右上
	PTZ_CTRL_UP,		        // 上转
	PTZ_CTRL_LEFTUP,		    // 左上
	PTZ_CTRL_LEFT,		        // 左转
	PTZ_CTRL_LEFTDOWN,		    // 左下
	PTZ_CTRL_DOWN,		        // 下转
	PTZ_CTRL_RIGHTDOWN,		    // 右下
	PTZ_CTRL_ZOOM,              // 镜头放大/缩小 速度值 >0:放大 <0:缩小 0:停止 -255~255
	PTZ_CTRL_IRIS,              // 光圈放大/缩小 速度值 >0:放大 <0:缩小 0:停止 -255~255
	PTZ_CTRL_FOCUS,             // 镜头聚焦/放焦 速度值 >0:聚焦近 <0:聚焦远 0:停止 -255~255
};

class GB28181MediaContext
{
public:
	explicit GB28181MediaContext(const std::string& requestUrl)
		: m_requestUrl(requestUrl)
		, m_recvAddr("")
		, m_recvPort(-1)
		, m_senderAddr("")
		, m_deviceId("")
	{
		const size_t start = requestUrl.find_first_of(":");
		const size_t end = requestUrl.find_first_of("@");
		m_deviceId = requestUrl.substr(start + 1, end - start - 1);
	}

	std::string GetDeviceId() const { return m_deviceId; }

	void SetRecvAddress(std::string recvAddr) { m_recvAddr = recvAddr; }
	std::string GetRecvAddress() const { return m_recvAddr; }

	void SetRecvPort(int recvPort) { m_recvPort = recvPort; }
	int GetRecvPort() const { return m_recvPort; }

	std::string GetRequestUrl() const { return m_requestUrl; }
	std::string GetStartTime() const { return m_startTime; }
	std::string GetEndTime() const { return m_endTime; }

	void SetTime(std::string startTime, std::string endTime)
	{
		m_startTime = startTime;
		m_endTime = endTime;
	}

	int64_t GetPBStartTime() const { return m_pbStartTime; }
	int64_t GetPBEndTime() const { return m_pbEndTime; }

	void SetPBTime(int64_t startTime, int64_t endTime)
	{
		m_pbStartTime = startTime;
		m_pbEndTime = endTime;
	}

	int64_t GetDownloadStartTime() const { return m_downloadStartTime; }
	int64_t GetDownloadEndTime() const { return m_downloadEndTime; }

	void SetDownloadTime(int64_t startTime, int64_t endTime)
	{
		m_downloadStartTime = startTime;
		m_downloadEndTime = endTime;
	}

	void SetDeviceId(std::string deviceId) { m_deviceId = deviceId; }

	void SetStreamType(StreamRequiredType streamRequiredType) { m_streamRequiredType = streamRequiredType; }
	StreamRequiredType GetStreamType() const { return m_streamRequiredType; }

	void SetDownloadSpeed(int speed) { m_downloadSpeed = speed; }
	int GetDownloadSpeed() const { return m_downloadSpeed; }

private:
	std::string    m_requestUrl;
	std::string    m_recvAddr;
	std::string    m_senderAddr;
	std::string    m_deviceId;
	std::string    m_startTime;
	std::string    m_endTime;
	int64_t        m_pbStartTime;
	int64_t        m_pbEndTime;
	int64_t        m_downloadStartTime;
	int64_t        m_downloadEndTime;
	unsigned short m_recvPort;
	StreamRequiredType  m_streamRequiredType;
	int            m_downloadSpeed = 1;  // 默认1倍速
};

class GBSubscribeContext
{
public:
	explicit GBSubscribeContext() {}

	void SetDeviceId(const std::string& deviceId) { m_deviceId = deviceId; }
	std::string GetDeviceId() const { return m_deviceId; }

	void SetRequestUrl(const std::string& requestUrl) { m_requestUrl = requestUrl; }
	std::string GetRequestUrl() const { return m_requestUrl; }

	void SetSubscirbeType(SubscribeType type) { m_subscribeType = type; }
	SubscribeType GetSubscribeType() const { return m_subscribeType; }

	void SetExpires(int expires) { m_expires = expires; }
	int GetExpires() const { return m_expires; }

private:
	SubscribeType  m_subscribeType;
	std::string    m_requestUrl = "";
	std::string    m_deviceId = "";
	int            m_expires = 0;
};

#endif
