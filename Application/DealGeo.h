#pragma once
#include <qmainwindow.h>
#include <ui_dealGeo.h>
class DealGeo :
	public QMainWindow
{
	Q_OBJECT

public:
	DealGeo(QWidget *parent = Q_NULLPTR);
	void closeEvent(QCloseEvent *);
signals:
	void ExitWin();//新建一个信号槽

private:
	Ui::GeoWindow ui;
};

