#pragma once

#include <QWidget>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "ui_GBCataLogDlg.h"
#include "MySipInfo.h"

class GBCataLogDlg : public QWidget
{
	Q_OBJECT

public:
	GBCataLogDlg(QWidget *parent = nullptr);
	~GBCataLogDlg();

	void AddCatalogData(const CMyCatalogInfo& catalog);

signals:
	void sigQueryCatalog(const QString& deviceID);

private:
	Ui::GBCataLogDlg ui;
};
