import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "pages"

ApplicationWindow {
    id: root
    visible: true
    width: 1100
    height: 750
    title: "Portfolio Recommender"
    color: "#1a1d23"

    Material.theme: Material.Dark
    Material.accent: Material.Teal

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Sidebar
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 200
            color: "#14161a"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                Label {
                    text: "📊 Portfolio\nRecommender"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#e0e0e0"
                    padding: 12
                }

                Repeater {
                    model: ["Generate", "Backtest", "Sensitivity", "Import", "Saved", "Settings"]
                    delegate: Button {
                        Layout.fillWidth: true
                        text: modelData
                        flat: true
                        highlighted: stackView.currentIndex === index
                        onClicked: stackView.currentIndex = index
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // Main content
        StackLayout {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true

            GeneratePage {}
            BacktestPage {}
            SensitivityPage {}
            ImportPage {}
            SavedPage {}
            SettingsPage {}
        }
    }
}
