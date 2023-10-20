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

enum DeviceControlType
{
	ControlType_PTZCmd = 0,         // ��̨����
	ControlType_TeleBoot,           // Զ�̿���
	ControlType_RecordCmd,          // ¼�����
	ControlType_GuardCmd,           // ��������/����
	ControlType_AlarmCmd,           // ������λ
	ControlType_IFrameCmd,          // ǿ�ƹؼ�֡
	ControlType_DragZoomIn,         // ����Ŵ�
	ControlType_DragZoomOut,        // ������С
	ControlType_HomePosition,       // ����λ
	ControlType_PTZPreciseCtrl,     // PTZ��׼����
	ControlType_DeviceUpgrade,      // �豸�������
	ControlType_FormatSDCard,       // �洢����ʽ��
	ControlType_TargetTrack,        // Ŀ�����
};

enum PTZControlType
{
	PTZ_CTRL_HALT = 0,			// ֹͣ
	PTZ_CTRL_RIGHT,		        // ��ת ֵԽ���ٶ�Խ�� 0~255
	PTZ_CTRL_RIGHTUP,		    // ����
	PTZ_CTRL_UP,		        // ��ת
	PTZ_CTRL_LEFTUP,		    // ����
	PTZ_CTRL_LEFT,		        // ��ת
	PTZ_CTRL_LEFTDOWN,		    // ����
	PTZ_CTRL_DOWN,		        // ��ת
	PTZ_CTRL_RIGHTDOWN,		    // ����
	PTZ_CTRL_ZOOM,              // ��ͷ�Ŵ�/��С �ٶ�ֵ >0:�Ŵ� <0:��С 0:ֹͣ -255~255
	PTZ_CTRL_IRIS,              // ��Ȧ�Ŵ�/��С �ٶ�ֵ >0:�Ŵ� <0:��С 0:ֹͣ -255~255
	PTZ_CTRL_FOCUS,             // ��ͷ�۽�/�Ž� �ٶ�ֵ >0:�۽��� <0:�۽�Զ 0:ֹͣ -255~255
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
	StreamRequiredType  m_streamRequiredType = StreamType_RealStream;
	int            m_downloadSpeed = 1;  // Ĭ��1����
};

#endif
