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

    connect(ui->tryLubuntu, &QAbstractButton::clicked, this, &InstallerPrompt::tryLubuntu);
}

void InstallerPrompt::tryLubuntu()
{
    QApplication::quit();
}

InstallerPrompt::~InstallerPrompt()
{
    delete ui;
}

