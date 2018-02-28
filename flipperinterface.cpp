#include "flipperinterface.h"
#include <QDebug>

#define useDebug (1)

FlipperInterface::FlipperInterface(const QString &tcpAddr, const int &port, const int &flipperAddr, QObject *parent): QObject(parent), m_TCPaddress(tcpAddr), m_TCPport(port), m_FlipperAddress(flipperAddr), m_ChannelEnable(0x00)
{

#ifdef useDebug
    qDebug() << "Initialize Flipper interface";
#endif
    // set default update data interval

    m_CollectDataInterVal = m_Update_Interval_ms;
    // set default decimal value to all channels

    setDecimalValue(Channel1, 100);
    setDecimalValue(Channel2, 100);
    setDecimalValue(Channel3, 100);
    setDecimalValue(Channel4, 100);
    setDecimalValue(Channel5, 100);
    setDecimalValue(Channel6, 100);

    QObject::connect(this,SIGNAL(FlipperTcpSettingChanged()),this,SLOT(FlipperTcpSettingsChangedHandler()));
}

void FlipperInterface::start()
{

#ifdef useDebug
    qDebug() << "Flipper interface started";
#endif
        if(connectToFlipper()){
            initCollectDataTimer(m_CollectDataInterVal);
        }

}

void FlipperInterface::stop()
{
#ifdef useDebug
    qDebug() << "Flipper interface stopped";
#endif
    collectDataTimer->stop();
}

void FlipperInterface::setDecimalValue(const quint16 &channel, const quint16 &value)
{
#ifdef useDebug
    qDebug() << "Flipper interface: setDecimal value of channel" + QString::number(channel) + " to : " + QString::number(value);
#endif
    m_channelsDecimalPointsHash.insert(channel, value);
}

void FlipperInterface::setEnableChannels(const quint8 &value)
{
#ifdef useDebug
    qDebug() << "Flipper interface: setEnable channe"  + value;
#endif
    m_ChannelEnable = value;
}

void FlipperInterface::FlipperRespondHandler()
{

#ifdef useDebug
    qDebug() << "Flipper interface: enter Flipper Respond Handler";
#endif
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();

        qDebug() << "start address: " + QString::number(unit.startAddress());

        if(unit.startAddress() & Channel1
                || unit.startAddress() & Channel2
                || unit.startAddress() & Channel3
                || unit.startAddress() & Channel4
                || unit.startAddress() & Channel5
                || unit.startAddress() & Channel6)
        {

            qDebug() << "value count: " + unit.valueCount();
            qDebug() << unit.value(0);
            qDebug() << unit.value(1);

            double realValue = ((double) unit.value(0) / (double) m_channelsDecimalPointsHash.value(unit.startAddress()));

            qDebug() << "double value: " + QString::number(realValue);
           // emit RecordDewPointToLocalDB(unit.startAddress(), (double) (unit.value(0) / m_channelsDecimalPointsHash.value(unit.startAddress())));
        }

        else
        {
            // something else here
        }


    } else if (reply->error() == QModbusDevice::ProtocolError) {
        qDebug() << reply->errorString();
    } else {
        qDebug() <<reply->errorString();
    }

    reply->deleteLater();
}

void FlipperInterface::emitRequests()
{
#ifdef useDebug
    qDebug() << "Flipper interface: enter emitRequests";
#endif
    if(m_ChannelEnable & Channel1) m_requestList.append(QModbusDataUnit(QModbusDataUnit::InputRegisters,m_CH1MeasureReleativeAddress,m_ReadOutWordNumberMeasureCommand));
    if(m_ChannelEnable & Channel2) m_requestList.append(QModbusDataUnit(QModbusDataUnit::InputRegisters,m_CH2MeasureReleativeAddress,m_ReadOutWordNumberMeasureCommand));
    if(m_ChannelEnable & Channel3) m_requestList.append(QModbusDataUnit(QModbusDataUnit::InputRegisters,m_CH3MeasureReleativeAddress,m_ReadOutWordNumberMeasureCommand));
    if(m_ChannelEnable & Channel4) m_requestList.append(QModbusDataUnit(QModbusDataUnit::InputRegisters,m_CH4MeasureReleativeAddress,m_ReadOutWordNumberMeasureCommand));
    if(m_ChannelEnable & Channel5) m_requestList.append(QModbusDataUnit(QModbusDataUnit::InputRegisters,m_CH5MeasureReleativeAddress,m_ReadOutWordNumberMeasureCommand));
    if(m_ChannelEnable & Channel6) m_requestList.append(QModbusDataUnit(QModbusDataUnit::InputRegisters,m_CH6MeasureReleativeAddress,m_ReadOutWordNumberMeasureCommand));

    emitRequestsHandler();
}

void FlipperInterface::emitRequestsHandler()
{
#ifdef useDebug
    qDebug() << "Flipper interface: enter emitRequestHandler";
    qDebug() << "number of requests : " + m_requestList.count();
#endif
    for (int i = 0; i < m_requestList.count(); i++)
    {
        if(auto *reply = modbusDevice->sendReadRequest(m_requestList.at(i),m_FlipperAddress))
        {
            if (!reply->isFinished())
                QObject::connect(reply, &QModbusReply::finished, this, &FlipperInterface::FlipperRespondHandler);
            else
                delete reply; // broadcast replies return immediately
        }
        else
        {

        }
    }
    m_requestList.clear();
}

bool FlipperInterface::connectToFlipper()
{
#ifdef useDebug
    qDebug() << "Flipper interface: enter connectToFlipper()";
#endif
    modbusDevice = new QModbusTcpClient(this);
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, m_TCPaddress);
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, m_TCPport);
    modbusDevice->setTimeout(1000);
    modbusDevice->setNumberOfRetries(3);
    QObject::connect(modbusDevice, SIGNAL(errorOccurred(QModbusDevice::Error)),this, SLOT(ModbusDeviceErrorHandler(QModbusDevice::Error)));

    if(modbusDevice->connectDevice())
    {
        qDebug() << modbusDevice->state();
        QObject::connect(modbusDevice, &QModbusTcpClient::stateChanged, this, &FlipperInterface::onStateChanged);

        return true;

    }
    return false;
}


void FlipperInterface::initCollectDataTimer(const int &interval)
{

#ifdef useDebug
    qDebug() << "Flipper interface: enter initCollectDataTimer()";
#endif
    collectDataTimer = new QTimer(this);
    collectDataTimer->setInterval(interval);
    QObject::connect(collectDataTimer, SIGNAL(timeout()), this,SLOT(emitRequests()));

}

void FlipperInterface::setCollectDataInterval(const int &interval)
{
#ifdef useDebug
    qDebug() << "Flipper interface: enter setCollectDataTimer()";
#endif
    m_CollectDataInterVal = interval;
}

void FlipperInterface::ModbusDeviceErrorHandler(QModbusDevice::Error error)
{
#ifdef useDebug
    qDebug() << "Flipper interface: enter ModbusDeviceErrorHandler()";
#endif
    //    static int connectAttempCounter = 0;
    // print error to console

    // this is a pure error State and error handler distribution

    //    // disconnect
    //        modbusDevice->disconnectDevice();

    //    // emit Reconnect to Device signal
    //        if(modbusDevice->connectDevice())
    //        {
    //            connectAttempCounter = 0;
    //        }
    //        else{
    //            connectAttempCounter ++;
    //        }

}

void FlipperInterface::onStateChanged(int state)
{
#ifdef useDebug
    qDebug() << "Flipper interface: enter onStateChanged()";
#endif
    qDebug() << modbusDevice->state();
    static int attemptCounter = 0;
    if(state  == QModbusDevice::ConnectedState)
    {
        collectDataTimer->start();
        emitRequests();
    }
    else if(state == QModbusDevice::UnconnectedState)
    {

        modbusDevice->connectDevice();
        attemptCounter++;

        if(attemptCounter == m_ConnectToDevice_Attempts_Max)
        {
            attemptCounter = 0;
            // emit FATAL ERROR HERE  and STOP THE WHOLE PROGRAM
        }
    }

}

void FlipperInterface::setFlipperTcpSettings(const QString &tcpAddr, const int &port, const int &flipperAddr)
{
#ifdef useDebug
    qDebug() << "Flipper interface: enter setFlipperTcpSettings()";
#endif
    m_TCPaddress = tcpAddr;
    m_TCPport = port;
    m_FlipperAddress = flipperAddr;
    emit FlipperTcpSettingChanged();
}


void FlipperInterface::FlipperTcpSettingsChangedHandler()
{
#ifdef useDebug
    qDebug() << "Flipper interface: enter FlipperTcpSettingsChangedHandler()";
#endif
    if(collectDataTimer->isActive())
    {
        collectDataTimer->stop();
        if(connectToFlipper()) collectDataTimer->start();
    }
    else
    {
        connectToFlipper();
    }
}



