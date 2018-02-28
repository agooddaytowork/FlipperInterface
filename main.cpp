#include <QCoreApplication>
#include "flipperinterface.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FlipperInterface aFlipperinterface("192.168.1.11", 502,1);
    aFlipperinterface.setEnableChannels(FlipperInterface::Channel1);

    aFlipperinterface.setCollectDataInterval(5000);
    aFlipperinterface.start();

    return a.exec();
}
