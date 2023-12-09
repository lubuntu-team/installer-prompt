#ifndef INSTALLERPROMPT_H
#define INSTALLERPROMPT_H

#include <QMainWindow>
#include <QComboBox>
#include <QProcess>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QMutex>
#include <QLineEdit>
#include <QProcess>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>

namespace Ui { class InstallerPrompt; }

class InstallerPrompt : public QMainWindow {
    Q_OBJECT

public:
    explicit InstallerPrompt(QWidget *parent = nullptr);
    ~InstallerPrompt() override;

private slots:
    void refreshNetworkList();
    void onLanguageChanged(int index);
    void onLanguageConfirm();
    void onConnectWifiClicked();
    void updateConnectionStatus();
    void handleWiFiConnectionChange(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason);
    void tryLubuntu();
    void installLubuntu();
    void languageProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void languageProcessError(QProcess::ProcessError error);

private:
    Ui::InstallerPrompt *ui;
    QProcess *process;
    NetworkManager::WirelessDevice::Ptr wifiDevice;
    QString wifiSSID;
    QMutex wifiChangeMutex;
    NetworkManager::Connection::Ptr findConnectionBySsid(const QString &ssid);
    bool wifiWrongHandling = false;
    QLineEdit *passwordLineEdit;
    QMap<QString, QString> languageLocaleMap;
    QString selectedLanguage = "English (United States)";
    QString localeName = "en_US";

    void handleWifiConnection(const QString &ssid, bool recoverFromWrongPassword = false);
    QString promptForWifiPassword(const QString &ssid, bool isWrongPassword = false);
    void connectToWifi(const QString &ssid, const QString &password, bool recoverFromWrongPassword = false);
    void initLanguageComboBox();
    QStringList getAvailableLanguages();
    void showWifiOptions();
    NMVariantMapMap createSettingsBySSID(const QString &ssid);
};

#endif // INSTALLERPROMPT_H
