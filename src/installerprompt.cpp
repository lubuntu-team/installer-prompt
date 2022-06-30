#include "installerprompt.h"
#include "./ui_installerprompt.h"

InstallerPrompt::InstallerPrompt(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::InstallerPrompt)
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

InstallerPrompt::~InstallerPrompt()
{
    delete ui;
}

