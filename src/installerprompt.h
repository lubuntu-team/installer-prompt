#ifndef INSTALLERPROMPT_H
#define INSTALLERPROMPT_H

#include <QMainWindow>
#include <QComboBox>
#include <QProcess>
#include <QPushButton>
#include <QLabel>
#include <QDialog>

namespace NetworkManager {
    class Device;
    class WirelessDevice;
    class WirelessNetwork;
}

namespace Ui { class InstallerPrompt; }

class InstallerPrompt : public QMainWindow
{
    Q_OBJECT

public:
    explicit InstallerPrompt(QWidget *parent = nullptr);
    ~InstallerPrompt() override;

private slots:
    void refreshNetworkList(); // Slot to handle network list refreshes
    void onLanguageChanged(int index);
    void onConnectWifiClicked();
    void tryLubuntu();
    void installLubuntu();

private:
    Ui::InstallerPrompt *ui;
    QProcess *process;
    QPushButton *connectWifiButton;
    QLabel *connectionStatusLabel;

    void initLanguageComboBox();
    QStringList getAvailableLanguages() const;
    bool checkInternetConnection();
    void showWifiOptions();
    void updateConnectionStatus(bool online);
};

#endif // INSTALLERPROMPT_H
