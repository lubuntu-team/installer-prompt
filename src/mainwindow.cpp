#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set the background image, and let it change with the release
    QPixmap bg("/usr/share/lubuntu/wallpapers/lubuntu-default-wallpaper.png");
    bg = bg.scaled(this->size(), Qt::IgnoreAspectRatio);
    QPalette palette;
    palette.setBrush(QPalette::Window, bg);
    this->setPalette(palette);

    // Set the top label as the Lubuntu Logo
    QPixmap logo("../img/logo.png");
    int w = ui->logolabel->width();
    int h = ui->logolabel->height();
    ui->logolabel->setPixmap(logo.scaled (w,h,Qt::KeepAspectRatio));
}

MainWindow::~MainWindow()
{
    delete ui;
}

