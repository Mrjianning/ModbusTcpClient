#ifndef ModbusTcpClient_H
#define ModbusTcpClient_H

#include <QMainWindow>
#include <QModbusDataUnit>
#include <QModbusClient>
#include <QModbusRtuSerialMaster>
#include <QSerialPort>
#include <QModbusDataUnit>
#include <QDebug>
#include <QUrl>
#include <QModbusTcpClient>


enum MyFunctionCode {
     one= QModbusRequest::WriteSingleCoil,
};

namespace Ui {
class ModbusTcpClient;
}

class ModbusTcpClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit ModbusTcpClient(QWidget *parent = 0);
    ~ModbusTcpClient();

    void cmd_write(QString data); // 发送数据
    void cmd_read(QString cmd);   // 读取数据

private slots:
    void on_sendButton_clicked();
    void on_connectButton_clicked();
    void onStateChanged(int state);
    void on_readButton_clicked();
    void readReady();

    void on_readButton_clear_clicked();
    void on_sendButton_clear_clicked();

private:
    Ui::ModbusTcpClient *ui;
    QModbusClient *modbusDevice;//QModbusClient被QModbusRtuSerialMaster和QModbusTcpClient继成
    QVector<quint16> m_holdingRegisters;

};

#endif // ModbusTcpClient_H
