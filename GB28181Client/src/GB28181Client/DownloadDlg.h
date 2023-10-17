#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <thread>
#include "ui_DownloadDlg.h"
#include "MySipHeader.h"
#include "GB28181Server.h"
#include "StreamReceiverInterface.h"

class DownloadDlg : public QWidget
{
	Q_OBJECT

public:
	DownloadDlg(QWidget *parent = nullptr);
	~DownloadDlg();

	void InitUI();
	void InitAction();
	void Init(const std::string& devID, time_t startTime, time_t endTime);

	void OnWorkThread();
	void GBDownloadDataCB(int avtype, void* data, int dataLen);

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private:
	void StartDownload();
	void StopDownload();

	void Stop_();

private:
	Ui::DownloadDlgClass ui;

	std::string      m_deviceID = "";
	std::string      m_token = "";
	time_t           m_startTime = 0;
	time_t           m_endTime = 0;
	std::thread      m_thread;
	IStreamReceiver* m_receiver = nullptr;
	FILE*            m_file = nullptr;
};
