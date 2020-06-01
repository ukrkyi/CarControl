/* (c) 2020 ukrkyi */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpSocket>

#define TEXT_START	"<html><head/><body><p align=\"center\"><span style=\" font-size:14pt; font-weight:600;\">"
#define TEXT_END	"</span></p></body></html>"

#define LABEL(X)	TEXT_START X TEXT_END

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), server(this), connection(NULL)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	server.setMaxPendingConnections(1);
	connect(&server, &QTcpServer::newConnection, this, &MainWindow::connection_created);
	ui->label->setAutoFillBackground(true);
	ui->label->setStyleSheet("QLabel { background-color : OrangeRed;}");
	ui->label->setText(LABEL("DISCONNECTED"));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_startButton_clicked()
{
	server.listen(QHostAddress::Any, 1488);
	ui->label->setStyleSheet("QLabel { background-color : DeepSkyBlue;}");
	ui->label->setText(LABEL("WAITING"));
	ui->startButton->setText("Stop");
	disconnect(ui->startButton, &QAbstractButton::clicked, this, &MainWindow::on_startButton_clicked);
	connect(ui->startButton, &QAbstractButton::clicked, this,  &MainWindow::on_stopButton_clicked);
}

void MainWindow::on_stopButton_clicked()
{
	if (connection != NULL) {
		disconnected();
	} else {
		server.close();
		ui->label->setStyleSheet("QLabel { background-color : OrangeRed;}");
		ui->label->setText(LABEL("DISCONNECTED"));
		ui->startButton->setText("Start");
		disconnect(ui->startButton, &QAbstractButton::clicked, this, &MainWindow::on_stopButton_clicked);
		connect(ui->startButton, &QAbstractButton::clicked, this,  &MainWindow::on_startButton_clicked);
	}
}

void MainWindow::connection_created()
{
	connection = server.nextPendingConnection();
	server.pauseAccepting();
	ui->area->clear();
	ui->log->clear();
	connect(connection, &QTcpSocket::disconnected, this, &MainWindow::disconnected);
	connect(connection, &QTcpSocket::readyRead, this, &MainWindow::data_received);
	ui->label->setStyleSheet("QLabel { background-color : SpringGreen;}");
	ui->label->setText(LABEL("CONNECTED"));
}

void MainWindow::disconnected()
{
	connection->close();
	connection->deleteLater();
	connection = NULL;
	server.resumeAccepting();
	ui->label->setStyleSheet("QLabel { background-color : DeepSkyBlue;}");
	ui->label->setText(LABEL("WAITING"));
}

void MainWindow::data_received()
{
	QByteArray data;
	while ((data = connection->readLine()).length() != 0) {
		QString line = data;
		if (line.startsWith("POS:")) {
			QStringList data = line.remove("POS:").split(',');
			ui->area->updatePosition(data[0].toFloat(), data[1].toFloat(), data[2].toFloat());
		} else if (line.startsWith("DST:")) {
			ui->area->newRange(line.remove("DST:").toFloat());
		} else
			ui->log->append(data.chopped(1)); // Log error
	}
}
