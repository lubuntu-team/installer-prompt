#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set the background image, and let it change with the release
    QPixmap bg("../img/background.png");
    bg = bg.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Window, bg);
    this->setPalette(palette);

    // Set the button colors
    ui->tryLubuntu->setStyleSheet("background-color: rgba(0, 104, 200, 100);");
    ui->installLubuntu->setStyleSheet("background-color: rgba(0, 104, 200, 100);");
}

MainWindow::~MainWindow()
{
    delete ui;
}

