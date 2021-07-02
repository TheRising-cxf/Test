#include    "DealAudio.h"
DealAuido::DealAuido(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setCentralWidget(ui.splitter);

	//������ʾͼ��
	QChart *chart = new QChart;
	chart->setTitle("��Ƶ����ԭʼ�ź�");
	ui.chartView->setChart(chart);

	lineSeries = new QLineSeries(); //����
	chart->addSeries(lineSeries);

	QValueAxis *axisX = new QValueAxis;  //������
	axisX->setRange(0, displayPointsCount); //chart��ʾ4000������������
	axisX->setLabelFormat("%g");
	axisX->setTitleText("Samples");

	QValueAxis *axisY = new QValueAxis;  //������
	axisY->setRange(0, 256); // UnSingedInt���������ݷ�Χ0-255
//    axisY->setRange(-1, 1);
	axisY->setTitleText("Audio level");

	chart->setAxisX(axisX, lineSeries);
	chart->setAxisY(axisY, lineSeries);
	chart->legend()->hide();

	//
	ui.comboDevices->clear();
	deviceList = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);//�����豸�б�
	for (int i = 0; i < deviceList.count(); i++)
	{
		QAudioDeviceInfo device = deviceList.at(i);
		ui.comboDevices->addItem(device.deviceName());
	}

	if (deviceList.size() > 0)
	{
		ui.comboDevices->setCurrentIndex(0); //����comboDevices���ź�currentIndexChanged()
		curDevice = deviceList.at(0);//
	}
	else
	{
		ui.actStart->setEnabled(false);
		ui.actDeviceTest->setEnabled(false);
		ui.actSave->setEnabled(false);
		ui.groupBoxDevice->setTitle("֧�ֵ���Ƶ��������(���豸)");
	}
}
void DealAuido::closeEvent(QCloseEvent *)
{
	emit ExitWin();
}
QString DealAuido::SampleTypeString(QAudioFormat::SampleType sampleType)
{//��QAudioFormat::SampleType����ת��Ϊ�ַ���
	QString result("Unknown");
	switch (sampleType) {
	case QAudioFormat::SignedInt:
		result = "SignedInt";
		break;
	case QAudioFormat::UnSignedInt:
		result = "UnSignedInt";
		break;
	case QAudioFormat::Float:
		result = "Float";
		break;
	case QAudioFormat::Unknown:
		result = "Unknown";
	}
	return result;
}

QString DealAuido::ByteOrderString(QAudioFormat::Endian endian)
{ //��QAudioFormat::Endian  ת��Ϊ�ַ���
	if (endian == QAudioFormat::LittleEndian)
		return "LittleEndian";
	else if (endian == QAudioFormat::BigEndian)
		return "BigEndian";
	else
		return "Unknown";
}

void DealAuido::on_IODevice_UpdateBlockSize(qint64 blockSize)
{//��ʾ��������С�����ݿ��С
	ui.LabBufferSize->setText(QString::asprintf("QAudioInput::bufferSize()=%d",
		audioInput->bufferSize()));

	ui.LabBlockSize->setText(
		QString("IODevice���ݿ��ֽ���=%1").arg(blockSize));
}

void DealAuido::on_comboDevices_currentIndexChanged(int index)
{//ѡ����Ƶ�����豸
	curDevice = deviceList.at(index);//��ǰ��Ƶ�豸

	ui.comboCodec->clear(); //֧�ֵ���Ƶ����
	QStringList codecs = curDevice.supportedCodecs();
	for (int i = 0; i < codecs.size(); ++i)
		ui.comboCodec->addItem(codecs.at(i));

	ui.comboSampleRate->clear(); //֧�ֵĲ�����
	QList<int> sampleRate = curDevice.supportedSampleRates();
	for (int i = 0; i < sampleRate.size(); ++i)
		ui.comboSampleRate->addItem(QString("%1").arg(sampleRate.at(i)));

	ui.comboChannels->clear();//֧�ֵ�ͨ����
	QList<int> Channels = curDevice.supportedChannelCounts();
	for (int i = 0; i < Channels.size(); ++i)
		ui.comboChannels->addItem(QString("%1").arg(Channels.at(i)));

	ui.comboSampleTypes->clear(); //֧�ֵĲ���������
	QList<QAudioFormat::SampleType> sampleTypes = curDevice.supportedSampleTypes();
	for (int i = 0; i < sampleTypes.size(); ++i)
		ui.comboSampleTypes->addItem(SampleTypeString(sampleTypes.at(i)),
			QVariant(sampleTypes.at(i)));

	ui.comboSampleSizes->clear();//�������С
	QList<int> sampleSizes = curDevice.supportedSampleSizes();
	for (int i = 0; i < sampleSizes.size(); ++i)
		ui.comboSampleSizes->addItem(QString("%1").arg(sampleSizes.at(i)));

	ui.comboByteOrder->clear();//�ֽ���
	QList<QAudioFormat::Endian> endians = curDevice.supportedByteOrders();
	for (int i = 0; i < endians.size(); ++i)
		ui.comboByteOrder->addItem(ByteOrderString(endians.at(i)));
}

void DealAuido::on_actStart_triggered()
{//��ʼ��Ƶ����
	fopen_s(&outputFile, "test.raw", "wb");
	QAudioFormat defaultAudioFormat; //ȱʡ��ʽ
	defaultAudioFormat.setSampleRate(44000);//�����ٶ�
	defaultAudioFormat.setChannelCount(1);//����ͨ��
	defaultAudioFormat.setSampleSize(8);
	defaultAudioFormat.setCodec("audio/pcm");//��������
	defaultAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
	defaultAudioFormat.setSampleType(QAudioFormat::UnSignedInt);

	//    curDevice = QAudioDeviceInfo::defaultInputDevice(); // ѡ��ȱʡ�豸
	if (!curDevice.isFormatSupported(defaultAudioFormat))
	{
		QMessageBox::critical(this, "��Ƶ�������ò���", "����ʧ�ܣ������豸��֧�ִ�����");
		return;
	}

	audioInput = new QAudioInput(curDevice, defaultAudioFormat, this);
	audioInput->setBufferSize(displayPointsCount);

	// ������Ƶ�������ݵ����豸
	displayDevice = new QmyDisplayDevice(lineSeries, displayPointsCount, outputFile,this);

	connect(displayDevice, SIGNAL(updateBlockSize(qint64)),
		this, SLOT(on_IODevice_UpdateBlockSize(qint64)));

	displayDevice->open(QIODevice::WriteOnly); //������д��ʽ��

	audioInput->start(displayDevice); //�����豸��Ϊ��������ʼ¼����Ƶ��������


	ui.actStart->setEnabled(false);
	ui.actStop->setEnabled(true);
	ui.actSave->setEnabled(false);
}

void DealAuido::on_actStop_triggered()
{
	audioInput->stop();
	audioInput->deleteLater();
	displayDevice->close();
	fclose(outputFile);
	disconnect(displayDevice, SIGNAL(updateBlockSize(qint64)),
		this, SLOT(on_IODevice_UpdateBlockSize(qint64)));
	displayDevice->deleteLater();

	ui.actStart->setEnabled(true);
	ui.actStop->setEnabled(false);
	ui.actSave->setEnabled(true);
}
void DealAuido::on_actSave_triggered()
{
	QString filename1 = QFileDialog::getSaveFileName(this, tr("Save Audio"), "", tr("Images (*.png *.bmp *.jpg)")); //ѡ��·��
}

void DealAuido::on_actDeviceTest_triggered()
{//������Ƶ�����豸�Ƿ�֧��ѡ�������
	QAudioFormat settings;

	settings.setCodec(ui.comboCodec->currentText());
	settings.setSampleRate(ui.comboSampleRate->currentText().toInt());
	settings.setChannelCount(ui.comboChannels->currentText().toInt());

	settings.setSampleType(QAudioFormat::SampleType(ui.comboSampleTypes->currentData().toInt()));

	settings.setSampleSize(ui.comboSampleSizes->currentText().toInt());

	//  ���ܲ����������䣬   QAudioFormat::Endian��ȡֵ�� QSysInfo::Endian��Ӧ�������෴
	//    testSettings.setByteOrder(QAudioFormat::Endian(ui.comboByteOrder->currentData().toInt()));
	if (ui.comboByteOrder->currentText() == "LittleEndian")
		settings.setByteOrder(QAudioFormat::LittleEndian);
	else
		settings.setByteOrder(QAudioFormat::BigEndian);

	if (curDevice.isFormatSupported(settings))
		QMessageBox::information(this, "��Ƶ�������ò���", "���Գɹ��������豸֧�ִ�����");
	else
		QMessageBox::critical(this, "��Ƶ�������ò���", "����ʧ�ܣ������豸��֧�ִ�����");
}
