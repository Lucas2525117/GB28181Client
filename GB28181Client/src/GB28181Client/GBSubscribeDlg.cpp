#include "GBSubscribeDlg.h"

GBSubscribeDlg::GBSubscribeDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_startAlarm, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString deviceIPP = ui.le_ipp->text();
		QString startTime = ui.dte_startTime->text();
		QString endTime = ui.dte_endTime->text();
		if (gbid.isEmpty() || deviceIPP.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("参数不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigSubscribe(gbid, deviceIPP, startTime, endTime, Alarm_Subscribe, 300);
		});

	connect(ui.pb_stopAlarm, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString deviceIPP = ui.le_ipp->text();
		QString startTime = ui.dte_startTime->text();
		QString endTime = ui.dte_endTime->text();
		if (gbid.isEmpty() || deviceIPP.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigSubscribe(gbid, deviceIPP, startTime, endTime, Alarm_Subscribe, 0);
		});

	connect(ui.pb_startDevice, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString deviceIPP = ui.le_ipp->text();
		QString startTime = ui.dte_startTime->text();
		QString endTime = ui.dte_endTime->text();
		if (gbid.isEmpty() || deviceIPP.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigSubscribe(gbid, deviceIPP, startTime, endTime, MobilePosition_Subscribe, 300);
		});

	connect(ui.pb_stopDevice, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString deviceIPP = ui.le_ipp->text();
		QString startTime = ui.dte_startTime->text();
		QString endTime = ui.dte_endTime->text();
		if (gbid.isEmpty() || deviceIPP.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigSubscribe(gbid, deviceIPP, startTime, endTime, MobilePosition_Subscribe, 0);
		});

	connect(ui.pb_startPTZ, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString deviceIPP = ui.le_ipp->text();
		QString startTime = ui.dte_startTime->text();
		QString endTime = ui.dte_endTime->text();
		if (gbid.isEmpty() || deviceIPP.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigSubscribe(gbid, deviceIPP, startTime, endTime, PTZPosition_Subscribe, 300);
		});

	connect(ui.pb_stopPTZ, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString deviceIPP = ui.le_ipp->text();
		QString startTime = ui.dte_startTime->text();
		QString endTime = ui.dte_endTime->text();
		if (gbid.isEmpty() || deviceIPP.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigSubscribe(gbid, deviceIPP, startTime, endTime, PTZPosition_Subscribe, 0);
		});

	connect(ui.pb_startCatalog, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString deviceIPP = ui.le_ipp->text();
		QString startTime = ui.dte_startTime->text();
		QString endTime = ui.dte_endTime->text();
		if (gbid.isEmpty() || deviceIPP.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigSubscribe(gbid, deviceIPP, startTime, endTime, Catalog_Subscribe, 300);
		});

	connect(ui.pb_stopCatalog, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString deviceIPP = ui.le_ipp->text();
		QString startTime = ui.dte_startTime->text();
		QString endTime = ui.dte_endTime->text();
		if (gbid.isEmpty() || deviceIPP.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigSubscribe(gbid, deviceIPP, startTime, endTime, Catalog_Subscribe, 0);
		});
}

GBSubscribeDlg::~GBSubscribeDlg()
{
}
