#include "mainwindow.h"
#include "ui_mainwindow.h"

mainwindow::mainwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mainwindow)
{
    ui->setupUi(this);
    hostBtn = ui->hostBtn;
    clientBtn = ui->clientBtn;
    hostBack = ui->hostBack;
    clientStart = ui->clientStart;
    clientBack = ui->clientBack;
    ipEdit = ui->ipEdit;
    portEdit = ui->portEdit;
    message = ui->message;

    startBox = ui->startBox;
    hostBox = ui->hostBox;
    clientBox = ui->clientBox;
    sessionBox = ui->sessionBox;
    status = ui->status;

    clientBox->hide();
    hostBox->hide();
    sessionBox->hide();

    tcpServer = new QTcpServer(this);
    receiveSocket = new QTcpSocket(this);

    connect(hostBtn, SIGNAL(clicked()), this, SLOT(hostPart()));
    connect(clientBtn, SIGNAL(clicked()), this, SLOT(clientPart()));
}

mainwindow::~mainwindow()
{
    delete ui;
}

void mainwindow::hostPart()
{

    tcpServer->listen();
    QList<QHostAddress> ipList = QNetworkInterface::allAddresses();
    status->setText("IP : \n");

    foreach(QHostAddress i, ipList){
        if(i.toIPv4Address())  status->setText(status->text() + i.toString() + '\n');
    }

    status->setText(status->text() + "\nport : " + QString::number(tcpServer->serverPort()) + '\n');



    startBox->hide();
    hostBox->show();
    connect(hostBack, SIGNAL(clicked()), this, SLOT(backToMenu()));
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(startSession()));
    character = "host";
}

void mainwindow::clientPart()
{
    startBox->hide();
    clientBox->show();
    connect(clientBack, SIGNAL(clicked()), this, SLOT(backToMenu()));
    connect(clientStart, SIGNAL(clicked()), this, SLOT(prepareStart()));
    character = "client";
}

void mainwindow::startSession()
{
    qDebug()<<"sending....";
    if(character == "host"){

        hostBox->hide();
        sessionBox->show();
        QTcpSocket *clientConnection = tcpServer->nextPendingConnection();

        //datas
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_5_7);
        stream<<"hello world!\n";

        connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
        clientConnection->write(data);
        connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readMessage()));
        clientConnection->disconnectFromHost();
    }
    else if(character == "client"){

        receiveSocket->connectToHost(host, port);
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setDevice(receiveSocket);
        stream.setVersion(QDataStream::Qt_5_7);

        stream<<"hi_world";

        receiveSocket->write(data);
        connect(receiveSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
        receiveSocket->abort();
    }
}

void mainwindow::backToMenu()
{
    startBox->show();
    clientBox->hide();
    hostBox->hide();
    tcpServer->close();
}

void mainwindow::prepareStart()
{
    if(!ipEdit->text().isEmpty() && !portEdit->text().isEmpty()){
        sessionBox->show();
    }
    else return;

    host = ipEdit->text();
    port = portEdit->text().toInt();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(startSession()));
    timer->start(50);
}

void mainwindow::readMessage()
{
    qDebug()<<"new message came in!";
    QDataStream in;
    in.startTransaction();
    QString data;
    in>>data;

    message->setText(message->toPlainText() + '\n' + data);
}
