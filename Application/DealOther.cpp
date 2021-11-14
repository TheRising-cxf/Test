#include "DealOther.h"
DealOther::DealOther(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.startR, &QPushButton::clicked, this, &DealOther::StartRecord);
	connect(ui.stopR, &QPushButton::clicked, this, &DealOther::StopRecord);
	ui.stopR->setVisible(false);
}
void DealOther::closeEvent(QCloseEvent *)
{
	emit ExitWin();
}

void DealOther::StopRecord()
{
}
void DealOther::StartRecord()
{
	ui.stopR->setVisible(true);
}
