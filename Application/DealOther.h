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
	void ExitWin();//�½�һ���źŲ�

private:
	Ui::OtherWindow ui;
private slots:
	void StartRecord();
	void StopRecord();
};

