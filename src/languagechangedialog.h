#ifndef LANGUAGECHANGEDIALOG_H
#define LANGUAGECHANGEDIALOG_H

#include <QDialog>

namespace Ui {
class LanguageChangeDialog;
}

class LanguageChangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LanguageChangeDialog(QWidget *parent = nullptr);
    ~LanguageChangeDialog();

private:
    Ui::LanguageChangeDialog *ui;
};

#endif // LANGUAGECHANGEDIALOG_H
