#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QAbstractSocket>

class QTcpSocket;
class QFile;

namespace Ui {
class Client;
}

class Client : public QDialog
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = 0);
    ~Client();

private:
    Ui::Client *ui;

    QTcpSocket *tcpClient;
    QFile *localFile;     // 要发送的文件
    qint64 totalBytes;    // 发送数据的总大小
//    qint64 bytesWritten;  // 已经发送数据大小
//    qint64 bytesToWrite;  // 剩余数据大小
    qint64 payloadSize;   // 每次发送数据的大小(64k) 未用到
    QString fileName;     // 保存文件路径
    QByteArray outBlock;  // 数据缓冲区，即存放每次要发送的数据块

    QImage image;//图片
    QString currentImageName;//图片名

    volatile bool isOk;

private slots:
    void openFile();//打开文件
    void send();//发送
    void connectServer();//连接服务器
    void startTransfer();//发送图片数据
    void displayError(QAbstractSocket::SocketError);//处理错误函数
    void tcpConnected();//更新isOk的值，按钮以及label的显示
    void tcpDisconnected();//断开连接处理的事件

    //图片转base64字符串
    QByteArray getImageData(const QImage&);

    void on_openButton_clicked();//打开图片
    void on_sendButton_clicked();//发送图片
    void on_connectButton_clicked();//连接或断开服务器
signals:
    void buildConnected();//连接上服务器后，发出的信号
};


#endif // CLIENT_H
