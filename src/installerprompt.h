#ifndef INSTALLERPROMPT_H
#define INSTALLERPROMPT_H

#include <QMainWindow>
#include <QComboBox>
#include <QProcess>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>

namespace NetworkManager {
    class Device;
    class WirelessDevice;
    class WirelessNetwork;
}

namespace Ui { class InstallerPrompt; }

class InstallerPrompt : public QMainWindow {
    Q_OBJECT

public:
    explicit InstallerPrompt(QWidget *parent = nullptr);
    ~InstallerPrompt() override;

private slots:
    void refreshNetworkList();
    void onLanguageChanged(int index);
    void onConnectWifiClicked();
    void updateConnectionStatus();
    void handleWiFiConnectionChange(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason);
    void tryLubuntu();
    void installLubuntu();

private:
    Ui::InstallerPrompt *ui;
    QProcess *process;
    NetworkManager::WirelessDevice::Ptr wifiDevice;
    QMap<QString, NetworkManager::WirelessNetwork::Ptr> wifiNetworkMap;

    void handleWifiConnection(const QString &ssid, bool recoverFromWrongPassword = false);
    void initLanguageComboBox();
    QStringList getAvailableLanguages() const;
    void showWifiOptions();
    NetworkManager::Connection::Ptr findConnectionBySsid(const QString &ssid);
    QMap<QString, QVariant> createSettingsBySSID(const QString &ssid);
};

#endif // INSTALLERPROMPT_H
