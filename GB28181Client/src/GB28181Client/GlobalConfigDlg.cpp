#include "GlobalConfigDlg.h"

GlobalConfigDlg::GlobalConfigDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pb_ok, &QPushButton::clicked, [=]() {
		GlobalConfigParam param;
		param.streamTransMode = ui.cbb_streamTransMode->currentIndex();

		emit sigGlobalConfig(param);
		hide();
		});
}

GlobalConfigDlg::~GlobalConfigDlg()
{
}
