#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <QRegExpValidator>
#include <qmessagebox.h>
#include "ui_GBRegisterDlg.h"

class GBRegisterDlg : public QWidget
{
	Q_OBJECT

public:
	GBRegisterDlg(QWidget *parent = nullptr);
	~GBRegisterDlg();

	void SetRegisterResult(const QString& result);

private:
	void InitUi();
	void InitAction();

signals:
	void sigSipConnect(const std::string& gbid, const std::string& ip, int sipport, int transType);
	void sigSipDisConnect();

private:
	Ui::GBRegisterDlg ui;
};
