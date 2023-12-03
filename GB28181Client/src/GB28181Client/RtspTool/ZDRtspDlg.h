#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "PlayWidget.h"
#include "RtspStreamReceiverInterface.h"
#include "ui_ZDRtspDlg.h"

class ZDRtspDlg : public QWidget
{
	Q_OBJECT

public:
	ZDRtspDlg(QWidget *parent = nullptr);
	~ZDRtspDlg();

	void InputRtspData(int avtype, void* data, int dataLen);

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private:
	void InitUI();
	void InitAction();

	void StartRtspPlay();
	void StopRtspPlay();

private:
	Ui::ZDRtspDlgClass ui;

	PlayWidget* m_playWidget = nullptr;
	IRtspStreamReceiver* m_receiver = nullptr;
};
