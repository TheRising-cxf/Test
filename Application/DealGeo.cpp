#include "DealGeo.h"
DealGeo::DealGeo(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}
void DealGeo::closeEvent(QCloseEvent *)
{
	emit ExitWin();
}