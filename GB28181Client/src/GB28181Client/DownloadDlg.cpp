#include "DownloadDlg.h"
#include <qdir.h>

static int DownloadThread(void* param)
{
	DownloadDlg* p = (DownloadDlg*)param;
	if (p)
	{
		p->OnWorkThread();
	}
	return 0;
}

static void MyGBDownloadDataCB(int avtype, void* data, int dataLen, void* user)
{
	assert(user);
	DownloadDlg* cli = (DownloadDlg*)user;
	cli->GBDownloadDataCB(avtype, data, dataLen);
}

DownloadDlg::DownloadDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	InitUI();
	InitAction();
}

DownloadDlg::~DownloadDlg()
{
	Stop_();
}

void DownloadDlg::InitUI()
{

}

void DownloadDlg::InitAction()
{
	connect(ui.pb_startDownload, &QPushButton::clicked, [=]() {
		StartDownload();
		});

	connect(ui.pb_stopDownload, &QPushButton::clicked, [=]() {
		StopDownload();
		});
}

void DownloadDlg::Init(const std::string& devID, time_t startTime, time_t endTime)
{
	m_deviceID = devID;
	m_startTime = startTime;
	m_endTime = endTime;

	if (nullptr != m_file)
		fclose(m_file);
	m_file = nullptr;

	QString dirPath = QCoreApplication::applicationDirPath() + "/video/";
	QDir dir;
	if (!dir.exists(dirPath))
	{
		dir.mkpath(dirPath);
	}

	QString fileName = dirPath + QString::number(startTime) + ".data";
	if (!m_file)
	{
		m_file = fopen(fileName.toStdString().c_str(), "wb+");
	}
}

void DownloadDlg::closeEvent(QCloseEvent* event)
{
	Stop_();
}

void DownloadDlg::StartDownload()
{
	std::string requestUrl = "sip:34020000001320000001@192.168.0.111:5060";
	GB28181MediaContext mediaContext(requestUrl);
	mediaContext.SetRecvAddress("192.168.0.107");
	mediaContext.SetRecvPort(36000);
	mediaContext.SetDownloadTime(m_startTime, m_endTime);
	mediaContext.SetStreamType(StreamType_Download);
	mediaContext.SetDownloadSpeed(ui.cbx_speed->currentText().toInt());  // 倍速下载
	m_token = GB_Invite(mediaContext);
	if (m_token.empty())
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("开始录像下载失败"), QMessageBox::Ok);
		return;
	}

	m_thread = std::thread(DownloadThread, this);
}

void DownloadDlg::StopDownload()
{
	Stop_();
}

void DownloadDlg::Stop_()
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

	if (m_file)
		fclose(m_file);
	m_file = nullptr;
}

void DownloadDlg::OnWorkThread()
{
	std::string gburl = "gbudp://192.168.0.107:36000";
	m_receiver = GB_CreateStreamReceiver(gburl.c_str(), MyGBDownloadDataCB, this);
	m_receiver->Start();
}

void DownloadDlg::GBDownloadDataCB(int avtype, void* data, int dataLen)
{
	if (!m_file)
		return;

	fwrite(data, 1, dataLen, m_file);
}
