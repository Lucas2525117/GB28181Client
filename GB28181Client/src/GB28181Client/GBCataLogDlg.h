#pragma once

#include <QWidget>
#include <qpushbutton.h>
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
	void sigQueryCatalog();

private:
	Ui::GBCataLogDlg ui;
};
