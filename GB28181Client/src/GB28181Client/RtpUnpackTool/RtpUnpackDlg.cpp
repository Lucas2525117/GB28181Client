#include "RtpUnpackDlg.h"

static void* RtpPacketAlloc(void* param, int bytes)
{
	assert(param);
	RtpUnpackDlg* unpacker = (RtpUnpackDlg*)param;
	return unpacker->PacketAlloc();
}

static void RtpPacketFree(void* /*param*/, void* /*packet*/)
{
}

static int RtpDecodePacket(void* param, const void* packet, int bytes, uint32_t timestamp, int flags)
{
	assert(param);
	RtpUnpackDlg* unpacker = (RtpUnpackDlg*)param;
	return unpacker->DecodePacket(packet, bytes, timestamp, flags);
}

int DataReadThread(void* param)
{
	assert(param);
	RtpUnpackDlg* unpacker = (RtpUnpackDlg*)param;
	unpacker->RtpDataReadWorker();
	return 0;
}

RtpUnpackDlg::RtpUnpackDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("RTP解包工具"));
	resize(1000, 800);

	InitUI();
	InitAction();
}

RtpUnpackDlg::~RtpUnpackDlg()
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();
}

void RtpUnpackDlg::InitUI()
{
	m_playWidget = new(std::nothrow) PlayWidget();
	if (m_playWidget)
		m_playWidget->Init();
	ui.stackedWidget->insertWidget(0, m_playWidget);
	ui.stackedWidget->setCurrentIndex(0);
}

void RtpUnpackDlg::InitAction()
{
	connect(ui.pb_choose, &QPushButton::clicked, [=]() {
		ChooseFile();
		});

	connect(ui.pb_startTrans, &QPushButton::clicked, [=]() {
		StartRtpUnpack();
		});
}

void RtpUnpackDlg::closeEvent(QCloseEvent* event)
{
	m_running = false;
	if (m_thread.joinable())
		m_thread.join();
}

void RtpUnpackDlg::ChooseFile()
{
	QString filePath = QFileDialog::getOpenFileName(
		this,//指定父窗口
		QString::fromLocal8Bit("标准文件对话框"),
		"."
	);

	ui.le_filePath->setText(filePath);
}

void RtpUnpackDlg::StartRtpUnpack()
{
	if (m_thread.joinable())
		return;

	QString filePath = ui.le_filePath->text();
	if (filePath.isEmpty())
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("错误"), QString::fromLocal8Bit("文件路径为空"), QMessageBox::Ok);
		return;
	}

	m_rtpPayloadParam.payload = 100;
	m_rtpPayloadParam.encoding = "PS";
	m_rtpPayloadParam.frtp = fopen(/*filePath.toStdString().c_str()*/"C:\\rtp.tcp", "rb");
	if (!m_rtpPayloadParam.frtp)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("打开输入文件失败"), QMessageBox::Ok);
		return;
	}

	m_rtpPayloadParam.fout = fopen("C:\\out.h264", "wb");
	if (!m_rtpPayloadParam.fout)
	{
		QMessageBox::critical(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("打开输出文件失败"), QMessageBox::Ok);
		return;
	}

	struct rtp_payload_t handler;
	handler.alloc = RtpPacketAlloc;
	handler.free = RtpPacketFree;
	handler.packet = RtpDecodePacket;
	m_rtpPayloadParam.decoder = rtp_payload_decode_create(100, "PS", &handler, this);

	m_thread = std::thread(DataReadThread, this);
}

void* RtpUnpackDlg::PacketAlloc()
{
	static uint8_t buffer[2 * 1024 * 1024 + 4] = { 0, 0, 0, 1, };
	assert(bytes <= sizeof(buffer) - 4);
	return buffer + 4;
}

void RtpUnpackDlg::PacketFree()
{
}

int RtpUnpackDlg::DecodePacket(const void* packet, int bytes, uint32_t timestamp, int flags)
{
	static const unsigned char start_code[4] = { 0, 0, 0, 1 };
	static unsigned char buffer[2 * 1024 * 1024] = {0, };

	size_t size = 0;
	if (0 == strcmp("H264", m_rtpPayloadParam.encoding) 
		|| 0 == strcmp("H265", m_rtpPayloadParam.encoding)
		|| 0 == strcmp("PS", m_rtpPayloadParam.encoding))
	{
		memcpy(buffer, start_code, sizeof(start_code));
		size += sizeof(start_code);
	}

	memcpy(buffer + size, packet, bytes);
	size += bytes;

	fwrite(buffer, 1, size, m_rtpPayloadParam.fout);

	if (m_playWidget)
		m_playWidget->AddData(CODEC_VIDEO_H264, (void*)buffer, size);

	return 0;
}

void RtpUnpackDlg::RtpDataReadWorker()
{
	while (m_running)
	{
		unsigned char s2[2];
		if (2 != fread(s2, 1, 2, m_rtpPayloadParam.frtp))
			break;

		m_rtpPayloadParam.size = (s2[0] << 8) | s2[1];
		assert(ctx.size < sizeof(ctx.packet));
		if (m_rtpPayloadParam.size != (int)fread(m_rtpPayloadParam.packet, 1, m_rtpPayloadParam.size, m_rtpPayloadParam.frtp))
			break;

		if (m_rtpPayloadParam.packet[1] < RTCP_FIR || m_rtpPayloadParam.packet[1] > RTCP_LIMIT)
			rtp_payload_decode_input(m_rtpPayloadParam.decoder, m_rtpPayloadParam.packet, m_rtpPayloadParam.size);
	}

	fclose(m_rtpPayloadParam.frtp);
	fclose(m_rtpPayloadParam.fout);
}
