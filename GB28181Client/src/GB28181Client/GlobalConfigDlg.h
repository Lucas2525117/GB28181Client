#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include "public.h"
#include "ui_GlobalConfigDlg.h"

class GlobalConfigDlg : public QWidget
{
	Q_OBJECT

public:
	GlobalConfigDlg(QWidget *parent = nullptr);
	~GlobalConfigDlg();

signals:
	void sigGlobalConfig(GlobalConfigParam param);

private:
	Ui::GlobalConfigDlgClass ui;
};
