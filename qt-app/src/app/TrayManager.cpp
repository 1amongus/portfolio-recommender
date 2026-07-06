#include "TrayManager.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QPolygon>

TrayManager::TrayManager(QObject* parent)
    : QObject(parent)
{
}

void TrayManager::setup(QWindow* window)
{
    m_window = window;

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        setMinimizeToTray(false);
        return;
    }

    createTrayIcon();
    m_trayIcon->show();
}

bool TrayManager::minimizeToTray() const
{
    return m_minimizeToTray;
}

void TrayManager::setMinimizeToTray(bool enabled)
{
    if (m_minimizeToTray == enabled) {
        return;
    }

    m_minimizeToTray = enabled;
    emit minimizeToTrayChanged();
}

void TrayManager::showWindow()
{
    if (!m_window) {
        return;
    }

    m_window->show();
    m_window->raise();
    m_window->requestActivate();
}

void TrayManager::hideToTray()
{
    if (!m_window) {
        return;
    }

    m_window->hide();
    if (m_trayIcon) {
        m_trayIcon->showMessage(
            tr("Portfolio Recommender"),
            tr("Portfolio Recommender is still running."),
            QSystemTrayIcon::Information,
            3000);
    }
}

void TrayManager::createTrayIcon()
{
    if (m_trayIcon) {
        return;
    }

    auto pixmap = QPixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(20, 28, 38));
    painter.drawRoundedRect(QRectF(1, 1, 30, 30), 7, 7);

    QPen gridPen(QColor(62, 74, 89));
    gridPen.setWidth(1);
    painter.setPen(gridPen);
    painter.drawLine(7, 24, 25, 24);
    painter.drawLine(7, 24, 7, 8);

    QPen greenPen(QColor(0, 200, 140));
    greenPen.setWidth(3);
    greenPen.setCapStyle(Qt::RoundCap);
    greenPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(greenPen);
    painter.drawPolyline(QPolygon({
        QPoint(8, 22),
        QPoint(13, 18),
        QPoint(17, 20),
        QPoint(23, 11)
    }));

    QPen bluePen(QColor(64, 169, 255));
    bluePen.setWidth(2);
    bluePen.setCapStyle(Qt::RoundCap);
    painter.setPen(bluePen);
    painter.drawPolyline(QPolygon({
        QPoint(8, 19),
        QPoint(12, 15),
        QPoint(16, 16),
        QPoint(23, 9)
    }));
    painter.end();

    m_trayMenu = new QMenu();
    auto* showAction = new QAction(tr("Show"), m_trayMenu);
    auto* generateAction = new QAction(tr("Generate Portfolio"), m_trayMenu);
    auto* quitAction = new QAction(tr("Quit"), m_trayMenu);

    m_trayMenu->addAction(showAction);
    m_trayMenu->addAction(generateAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(quitAction);

    m_trayIcon = new QSystemTrayIcon(QIcon(pixmap), this);
    m_trayIcon->setToolTip(tr("Portfolio Recommender"));
    m_trayIcon->setContextMenu(m_trayMenu);

    connect(showAction, &QAction::triggered, this, &TrayManager::showWindow);
    connect(generateAction, &QAction::triggered, this, &TrayManager::generateRequested);
    connect(quitAction, &QAction::triggered, this, &TrayManager::quitRequested);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) {
            if (m_window && m_window->isVisible()) {
                hideToTray();
            } else {
                showWindow();
            }
        }
    });
}
