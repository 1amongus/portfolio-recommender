#pragma once

#include <QObject>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QWindow>

class TrayManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool minimizeToTray READ minimizeToTray WRITE setMinimizeToTray NOTIFY minimizeToTrayChanged)

public:
    explicit TrayManager(QObject* parent = nullptr);
    void setup(QWindow* window);

    bool minimizeToTray() const;
    void setMinimizeToTray(bool enabled);

    Q_INVOKABLE void showWindow();
    Q_INVOKABLE void hideToTray();

signals:
    void minimizeToTrayChanged();
    void generateRequested();
    void quitRequested();

private:
    void createTrayIcon();

    QSystemTrayIcon* m_trayIcon = nullptr;
    QMenu* m_trayMenu = nullptr;
    QWindow* m_window = nullptr;
    bool m_minimizeToTray = true;
};
