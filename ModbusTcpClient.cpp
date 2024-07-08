#include "ModbusTcpClient.h"
#include "ui_ModbusTcpClient.h"


ModbusTcpClient::ModbusTcpClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ModbusTcpClient),m_holdingRegisters(20)
{
    ui->setupUi(this);

    //QModbusTcpClient用于TCP通信中的client
    modbusDevice = new QModbusTcpClient(this);

    if (ui->lineEdit->text().isEmpty()){
        ui->lineEdit->setText(QLatin1Literal("192.168.0.7:8234"));
    }

    connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        statusBar()->showMessage(modbusDevice->errorString(), 5000);
    });

    //连接状态发生改变时改变connect按钮上的显示文字（connect or discennect
    connect(modbusDevice, &QModbusClient::stateChanged,this, &ModbusTcpClient::onStateChanged);

}

ModbusTcpClient::~ModbusTcpClient()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;
    delete ui;
}

// 发送数据
void ModbusTcpClient::cmd_write(QString data_str)
{
    // 清除状态栏显示的信息
    statusBar()->clearMessage();

    // 按十六进制编码接入文本
    QByteArray data = QByteArray::fromHex(data_str.toLatin1());
    qDebug()<< "发送的数据："<<  data;

    //  功能码：WriteSingleCoil = 0x05, 00 03 通道， FF00-开，0000-关
    QModbusRequest request(QModbusRequest::WriteSingleCoil, data);   //  00 03 00 00

    // 发送写数据 1 是 server address
    QModbusReply *reply = modbusDevice->sendRawRequest(request, 1);
    if (reply) {
        connect(reply, &QModbusReply::finished, this, [=]{
            readReady();
            reply->deleteLater();
        });

    } else if (reply != nullptr) {
        reply->deleteLater();
    }
}

// 读取线圈数据
void ModbusTcpClient::cmd_read(QString cmd)
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();

    QByteArray data = QByteArray::fromHex (cmd.toLatin1());

    QModbusRequest request(QModbusRequest::ReadCoils,data);

    // 发送读取请求
    if (auto *reply = modbusDevice->sendRawRequest(request, 1)) {
        if (!reply->isFinished()){
            //            connect(reply, &QModbusReply::finished, this, &MainWindow:: readReady);
        }else{
            delete reply;
        }
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

// 发送按钮点击
void ModbusTcpClient::on_sendButton_clicked()
{
    if (!modbusDevice)//如果设备没有被创建就返回
        return;

    // 获取输入文本
    QString str1 = ui->textEdit_write->toPlainText();
    cmd_write(str1);

}

// 连接按钮点击
void ModbusTcpClient::on_connectButton_clicked()
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();

    //如果处在非连接状态，进行连接
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        //获取IP和端口号
        const QUrl url = QUrl::fromUserInput(ui->lineEdit->text());
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());

        modbusDevice->setTimeout(1000);         //连接超时1S
        modbusDevice->setNumberOfRetries(3);    //连接失败重试三次连接
        if (!modbusDevice->connectDevice()) {
            statusBar()->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        }
    }else{//处在连接状态进行断开连接的操作
        modbusDevice->disconnectDevice();
    }
}

void ModbusTcpClient::onStateChanged(int state)                 //更新connect按钮的显示状态
{

    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));
}

// 读取按钮点击
void ModbusTcpClient::on_readButton_clicked()
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();
    QString str1 = ui->textEdit_read->toPlainText();
    cmd_read(str1);

}

// 接收命令返回结果
void ModbusTcpClient::readReady()
{
    //QModbusReply这个类存储了来自client的数据,sender()返回发送信号的对象的指针
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    // 数据从QModbusReply这个类的resuil方法中获取,也就是本程序中的reply->result()
    if (reply->error() == QModbusDevice::NoError) {

        const QModbusDataUnit unit = reply->result();
        const QByteArray data = reply->rawResult().data();
        qDebug()<< "readReady:"  <<data;

    } else if (reply->error() == QModbusDevice::ProtocolError) {
        statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                 arg(reply->errorString()).
                                 arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                 arg(reply->errorString()).
                                 arg(reply->error(), -1, 16), 5000);
    }

    reply->deleteLater();
}

void ModbusTcpClient::on_readButton_clear_clicked()
{
    ui->textEdit_read->clear();
}

void ModbusTcpClient::on_sendButton_clear_clicked()
{
    ui->textEdit_write->clear();
}
