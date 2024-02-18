#include "languagechangedialog.h"
#include "ui_languagechangedialog.h"

LanguageChangeDialog::LanguageChangeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LanguageChangeDialog)
{
    ui->setupUi(this);
}

LanguageChangeDialog::~LanguageChangeDialog()
{
    delete ui;
}
