#include "PlayBackDlg.h"

static int PlayBackThread(void* param)
{
	PlayBackDlg* p = (PlayBackDlg*)param;
	if (p)
	{
		p->OnWorkThread();
	}
	return 0;
}

static void MyGBPBDataCB(int avtype, void* data, int dataLen, void* user)
{
	assert(user);
	PlayBackDlg* cli = (PlayBackDlg*)user;
	cli->GBPBDataCB(avtype, data, dataLen);
}

PlayBackDlg::PlayBackDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	resize(800, 600);

	InitUI();
	InitAction();
}

PlayBackDlg::~PlayBackDlg()
{
	Stop();
}

void PlayBackDlg::InitUI()
{
	if (!m_playBackWidget)
	{
		m_playBackWidget = new(std::nothrow) PlayBackWidget();
		if (m_playBackWidget)
			m_playBackWidget->Init();
	}
	ui.horizontalLayout->addWidget(m_playBackWidget);
}

void PlayBackDlg::InitAction()
{

}

void PlayBackDlg::closeEvent(QCloseEvent* event)
{
	Stop();
}

void PlayBackDlg::Start(const std::string& devID, time_t startTime, time_t endTime)
{
	std::string requestUrl = "sip:34020000001320000001@192.168.0.111:5060";
	GB28181MediaContext mediaContext(requestUrl);
	mediaContext.SetRecvAddress("192.168.0.107");
	mediaContext.SetRecvPort(36000);
	mediaContext.SetPBTime(startTime, endTime);
	mediaContext.SetStreamType(StreamType_Playback);
	m_token = GB_Invite(mediaContext);
	if (m_token.empty())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("开始视频点播失败"), QMessageBox::Ok);
		return;
	}	

	m_thread = std::thread(PlayBackThread, this);
}

void PlayBackDlg::Stop()
{
	if (m_thread.joinable())
		m_thread.join();

	if (m_receiver)
	{
		m_receiver->Stop();
		m_receiver->DeleteThis();
		m_receiver = nullptr;
	}
	
	if (!m_token.empty())
		GB_Bye(m_token.c_str());
}

void PlayBackDlg::OnWorkThread()
{
	std::string gburl = "gbudp://192.168.0.107:36000";
	m_receiver = GB_CreateStreamReceiver(gburl.c_str(), MyGBPBDataCB, this);
	m_receiver->Start();
}

void PlayBackDlg::GBPBDataCB(int avtype, void* data, int dataLen)
{
	if(m_playBackWidget)
		m_playBackWidget->AddData(CODEC_VIDEO_H264, data, dataLen);
}
