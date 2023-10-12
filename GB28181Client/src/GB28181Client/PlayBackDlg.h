#pragma once

#include <QWidget>
#include <qmessagebox.h>
#include <thread>
#include "ui_PlayBackDlg.h"
#include "MySipHeader.h"
#include "GB28181Server.h"
#include "StreamReceiverInterface.h"
#include "public.h"
#include "PlayBackWidget.h"

class PlayBackDlg : public QWidget
{
	Q_OBJECT

public:
	PlayBackDlg(QWidget *parent = nullptr);
	~PlayBackDlg();

	void InitAction();

	void Start(const std::string& devID, time_t startTime, time_t endTime);
	void Stop();
	void OnWorkThread();
	void GBPBDataCB(int avtype, void* data, int dataLen);

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private:
	Ui::PlayBackDlgClass ui;

	std::string  m_token = "";
	std::thread  m_thread;
	IStreamReceiver* m_receiver = nullptr;
	PlayBackWidget*  m_playBackWidget = nullptr;
};
