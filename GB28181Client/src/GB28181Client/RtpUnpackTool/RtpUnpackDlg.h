#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <thread>
#include "PlayWidget.h"
#include "RtpUnpackDefine.h"
#include "rtp-payload.h"
#include "rtcp-header.h"
#include "ui_RtpUnpackDlg.h"

class RtpUnpackDlg : public QWidget
{
	Q_OBJECT

public:
	RtpUnpackDlg(QWidget *parent = nullptr);
	~RtpUnpackDlg();

	void* PacketAlloc();
	void PacketFree();
	int DecodePacket(const void* packet, int bytes, uint32_t timestamp, int flags);
	void RtpDataReadWorker();

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private:
	void InitUI();
	void InitAction();

	void ChooseFile();
	void StartRtpUnpack();

private:
	Ui::RtpUnpackDlgClass ui;

	PlayWidget* m_playWidget = nullptr;
	RtpPayloadParam m_rtpPayloadParam;
	std::thread m_thread;
	bool        m_running = true;
};

