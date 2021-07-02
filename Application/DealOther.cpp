#include "DealOther.h"
DealOther::DealOther(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}
void DealOther::closeEvent(QCloseEvent *)
{
	emit ExitWin();
}