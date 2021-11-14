#pragma once
#include <qmainwindow.h>
#include <ui_dealOther.h>
class DealOther :
	public QMainWindow
{
	Q_OBJECT

public:
	DealOther(QWidget *parent = Q_NULLPTR);
	void closeEvent(QCloseEvent *);
signals:
	void ExitWin();//新建一个信号槽

private:
	Ui::OtherWindow ui;
private slots:
	void StartRecord();
	void StopRecord();
};

