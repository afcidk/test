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
    tcpSocket = new QTcpSocket(this);

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
    ipEdit->setFocus();
    connect(clientBack, SIGNAL(clicked()), this, SLOT(backToMenu()));
    connect(clientStart, SIGNAL(clicked()), this, SLOT(prepareStart()));
    character = "client";
}

void mainwindow::startSession()
{
    if(character == "host"){
        qDebug()<<"host sending....";
        hostBox->hide();
        sessionBox->show();
        clientConnection = tcpServer->nextPendingConnection();
        connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readMessage()));

        //datas
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_5_7);
        stream<<QString("hello world!\n");

        connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));
        qDebug()<<"write : "<<clientConnection->write(data);
    }
    else if(character == "client"){
        qDebug()<<"client sending....";

        tcpSocket->abort();
        tcpSocket->connectToHost(host, port);
        connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));

        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_5_7);
        stream<<QString("hi world");

        qDebug()<<"write: "<<tcpSocket->write(data);
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
    timer->start(100);
}

void mainwindow::readMessage()
{
    qDebug()<<"new message came in!";
    QDataStream in;

    if(character == "host") in.setDevice(clientConnection);
    else in.setDevice(tcpSocket);

    in.startTransaction();
    QString data;
    in>>data;
    qDebug()<<data;

    message->setText(message->toPlainText() + '\n' + data);

    if(character == "host"){
       disconnect(clientConnection, SIGNAL(readyRead()), this, SLOT(readMessage()));
       clientConnection->disconnectFromHost();
    }
    else{
        tcpSocket->abort();
        disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
    }
}
