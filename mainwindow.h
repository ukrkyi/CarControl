/* (c) 2020 ukrkyi */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

	QTcpServer server;
	QTcpSocket * connection;

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_startButton_clicked();
	void on_stopButton_clicked();
	void connection_created();
	void disconnected();
	void data_received();

private:
	Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
