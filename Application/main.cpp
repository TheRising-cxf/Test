#include "DemoApplication.h"
#include <QtWidgets/QApplication>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
}

#undef main

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DemoApplication w;
    w.show();
    return a.exec();
	return 0;
}
