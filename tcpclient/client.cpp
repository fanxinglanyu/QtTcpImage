#include "client.h"
#include "ui_client.h"

#include <QtNetwork>
#include <QFileDialog>
#include <QCompleter>

Client::Client(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);

    //地址和端口自动补全以及默认提示
    QStringList hostWordList, portWordList;
    hostWordList <<tr("127.0.0.1");
    portWordList << tr("6666");
    QCompleter* completerHost = new QCompleter(hostWordList, this);
    QCompleter* completerPort = new QCompleter(portWordList, this);

    ui->hostLineEdit->setCompleter(completerHost);
    ui->portLineEdit->setCompleter(completerPort);
    ui->hostLineEdit->setPlaceholderText(tr("127.0.0.1"));
    ui->portLineEdit->setPlaceholderText(tr("6666"));

    payloadSize = 64 * 1024; // 64KB
    totalBytes = 0;
//    bytesWritten = 0;
//    bytesToWrite = 0;
    isOk = false;
    
    ui->sendButton->setEnabled(false);

    tcpClient = new QTcpSocket(this);

    // 当连接服务器成功时，发出connected()信号，isOK置为true
    connect(tcpClient, SIGNAL(connected()), this, SLOT(tcpConnected()));
    //当点击发送按钮后(且isOK为true)，发出buildConnected()信号，开始传送数据
    connect(this, SIGNAL(buildConnected()), this, SLOT(startTransfer()));
    //当断开连接时，发出disconnected(),isOK置为false
    connect(tcpClient, SIGNAL(disconnected()), this, SLOT(tcpDisconnected()));
    //显示错误
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
}

Client::~Client()
{
    delete ui;
}

void Client::openFile()
{
    fileName = QFileDialog::getOpenFileName(this);
    
    if (!fileName.isEmpty()) {
        
        //获得实际文件名
        currentImageName = fileName.right(fileName.size()
                                                 - fileName.lastIndexOf('/')-1);

        ui->clientStatusLabel->setText(tr("打开 %1 文件成功！").arg(currentImageName));

        if(isOk == true){
            ui->sendButton->setEnabled(true);
        }
    }
}

void Client::send()
{
    if(!isOk){
        ui->clientStatusLabel->setText(tr("请先连接服务器"));
        return;
    }else{
        //发射信号
        emit buildConnected();
        qDebug() << "emit buildConnected()" << endl;
    }
}

void Client::connectServer()
{
    // 初始化已发送字节为0
//    bytesWritten = 0;
    ui->clientStatusLabel->setText(tr("连接中…"));

//    //连接到服务器
    tcpClient->connectToHost(ui->hostLineEdit->text(),
                             ui->portLineEdit->text().toInt());

    isOk = true;
    qDebug() << "connectServer: isOk is ok" << endl;
}


void Client::startTransfer()
{
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_6);

   //获得图片数据
    QImage image(fileName);
    QString imageData = getImageData(image);

    qDebug() << "fileName: " <<fileName << endl;
//    qDebug() << "imageData" << imageData << endl;
    
    // 保留总大小信息空间、图像大小信息空间，然后输入图像信息
    sendOut << qint64(0) << qint64(0) << imageData;

    // 这里的总大小是总大小信息、图像大小信息和实际图像信息的总和
    totalBytes += outBlock.size();
    sendOut.device()->seek(0);

    // 返回outBolock的开始，用实际的大小信息代替两个qint64(0)空间
    sendOut << totalBytes << qint64((outBlock.size() - sizeof(qint64)*2));

    //发出readyRead（）信号
    tcpClient->write(outBlock);

    qDebug() << "图片的内容大小: " << qint64((outBlock.size() - sizeof(qint64)*2)) << endl;
    qDebug() << "整个包的大小: " << totalBytes << endl;
//    qDebug() << "发送完文件头结构后剩余数据的大小(bytesToWrite): " << bytesToWrite <<endl;

    outBlock.resize(0);

    ui->clientStatusLabel->setText(tr("传送文件 %1 成功").arg(currentImageName));
    totalBytes = 0;
//    bytesToWrite = 0;
}

void Client::displayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpClient->errorString();
    tcpClient->close();

    ui->clientStatusLabel->setText(tr("客户端就绪"));
    ui->sendButton->setEnabled(true);
}

void Client::tcpConnected()
{
    isOk = true;
    ui->connectButton->setText(tr("断开"));

    ui->clientStatusLabel->setText(tr("已连接"));
}

void Client::tcpDisconnected()
{
    isOk = false;
    tcpClient->abort();
    ui->connectButton->setText(tr("连接"));

    ui->clientStatusLabel->setText(tr("连接已断开"));
}

QByteArray Client::getImageData(const QImage &image)
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    image.save(&buffer, "png");
    imageData = imageData.toBase64();
    
    return imageData;
}

// 打开按钮
void Client::on_openButton_clicked()
{
    ui->clientStatusLabel->setText(tr("状态：等待打开文件！"));
    openFile();

}

// 发送按钮
void Client::on_sendButton_clicked()
{
    send();
}

void Client::on_connectButton_clicked()
{
    if (ui->connectButton->text() == tr("连接")) {
        tcpClient->abort();
        connectServer();
    } else {
        tcpClient->abort();
    }
}

