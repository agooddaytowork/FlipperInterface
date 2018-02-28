#include <QCoreApplication>
#include "flipperinterface.h"
#include "QThread"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FlipperInterface aFlipperinterface("192.168.1.11", 502,1);
    aFlipperinterface.setEnableChannels(FlipperInterface::Channel1 | FlipperInterface::Channel2);
    aFlipperinterface.setCollectDataInterval(5000);
    QThread aThread;

    aFlipperinterface.moveToThread(&aThread);
    QObject::connect(&aThread,SIGNAL(started()),&aFlipperinterface, SLOT(start()));

    aThread.start();




    return a.exec();
}
