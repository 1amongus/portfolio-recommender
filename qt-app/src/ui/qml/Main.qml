import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import PortfolioRecommender 1.0

ApplicationWindow {
    id: window
    width: 1280
    height: 800
    minimumWidth: 1024
    minimumHeight: 768
    visible: true
    title: "Portfolio Recommender"

    onClosing: function(close) {
        if (trayManager && trayManager.minimizeToTray) {
            close.accepted = false
            trayManager.hideToTray()
        }
    }

    Material.theme: Material.Dark
    Material.accent: Material.Teal

    RowLayout {
        anchors.fill: parent
        spacing: 0

        NavigationBar {
            id: navigationBar
            Layout.fillHeight: true
            Layout.preferredWidth: 240
            currentIndex: stackView.currentItem && stackView.currentItem.pageIndex !== undefined ? stackView.currentItem.pageIndex : 0
            onPageRequested: function(index) {
                stackView.replace(navigationBar.pageComponent(index))
            }
        }

        StackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: navigationBar.pageComponent(0)
        }
    }
}