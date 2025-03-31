#include "mainwindow.h"
#include <QApplication>
#include <QLabel>
 
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Hello World");
    QLabel *label = new QLabel("Hello World", this);
    label->setGeometry(50, 50, 200, 50); // 位置和大小可以根据需要调整
}
 
MainWindow::~MainWindow()
{
    // 析构函数
}
