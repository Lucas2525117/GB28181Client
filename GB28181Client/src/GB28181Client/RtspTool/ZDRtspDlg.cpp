#include "ZDRtspDlg.h"

static void RtspDataCB(int avtype, void* data, int dataLen, void* user)
{
	assert(user);
	ZDRtspDlg* cli = (ZDRtspDlg*)user;
	cli->InputRtspData(avtype, data, dataLen);
}

ZDRtspDlg::ZDRtspDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("RTSP流测试工具"));
	resize(1000, 800);

	InitUI();
	InitAction();
}

ZDRtspDlg::~ZDRtspDlg()
{
	StopRtspPlay();
}

void ZDRtspDlg::InitUI()
{
	m_playWidget = new(std::nothrow) PlayWidget();
	if (m_playWidget)
		m_playWidget->Init();
	ui.stackedWidget->insertWidget(0, m_playWidget);
	ui.stackedWidget->setCurrentIndex(0);
}

void ZDRtspDlg::InitAction()
{
	connect(ui.pb_play, &QPushButton::clicked, [=]() {
		StartRtspPlay();
		});

	connect(ui.pb_stop, &QPushButton::clicked, [=]() {
		StopRtspPlay();
		});
}

void ZDRtspDlg::closeEvent(QCloseEvent* event)
{
	StopRtspPlay();
}

void ZDRtspDlg::StartRtspPlay()
{
	QString strUrl = ui.le_filePath->text();
	if (strUrl.isEmpty())
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("取流url为空"), QMessageBox::Ok);
		return;
	}

	int streamType = ui.cbx_streamType->currentIndex();
	m_receiver = ZD_CreateStreamReceiver(streamType, strUrl.toStdString().c_str(), RtspDataCB, this);
	if (m_receiver)
		m_receiver->Start(0);
}

void ZDRtspDlg::StopRtspPlay()
{
	if (m_receiver)
	{
		m_receiver->Stop();
		m_receiver->DeleteThis();
		m_receiver = nullptr;
	}
}

void ZDRtspDlg::InputRtspData(int avtype, void* data, int dataLen)
{
	if (m_playWidget)
		m_playWidget->AddData(avtype, data, dataLen);
}
