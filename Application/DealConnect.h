#pragma once
#include <qmainwindow.h>
#include <ui_dealConnect.h>
class DealConnect :
	public QMainWindow
{
	Q_OBJECT
signals:
	void ExitWin();//新建一个信号槽
public:
	DealConnect(QWidget *parent = Q_NULLPTR);
	~DealConnect();
	void closeEvent(QCloseEvent *);
	void StartConnect();
private:
	Ui::DealConnectWindow ui;
};

