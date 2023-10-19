#include "PTZControlDlg.h"

PTZControlDlg::PTZControlDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_up, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_UP, speed.toInt());
		});

	connect(ui.pb_down, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_DOWN, speed.toInt());
		});

	connect(ui.pb_left, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_LEFT, speed.toInt());
		});

	connect(ui.pb_right, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_RIGHT, speed.toInt());
		});

	connect(ui.pb_leftup, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_LEFTUP, speed.toInt());
		});

	connect(ui.pb_leftdown, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_LEFTDOWN, speed.toInt());
		});

	connect(ui.pb_rightup, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_RIGHTUP, speed.toInt());
		});

	connect(ui.pb_rightdown, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_RIGHTDOWN, speed.toInt());
		});

	connect(ui.pb_zoomin, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_ZOOM, speed.toInt());
		});

	connect(ui.pb_zoomout, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_ZOOM, speed.toInt());
		});

	connect(ui.pb_irisin, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_IRIS, speed.toInt());
		});

	connect(ui.pb_irisout, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_IRIS, speed.toInt());
		});

	connect(ui.pb_focusner, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_FOCUS, speed.toInt());
		});

	connect(ui.pb_focusfar, &QPushButton::clicked, [=]() {
		QString gbid = ui.le_gbid->text();
		QString speed = ui.le_speed->text();

		if (gbid.isEmpty() || speed.isEmpty())
		{
			QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("国标id或速度不能为空"), QMessageBox::Ok);
			return;
		}

		emit sigPTZCmd(gbid, PTZ_CTRL_FOCUS, speed.toInt());
		});
}

PTZControlDlg::~PTZControlDlg()
{
}
