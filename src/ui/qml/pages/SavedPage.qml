import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20

            Item { Layout.preferredHeight: 24 }

            RowLayout {
                Layout.leftMargin: 24
                spacing: 16
                Label {
                    text: "Saved Portfolios"
                    font.pixelSize: 24
                    font.bold: true
                    color: "#ffffff"
                }
                Button {
                    text: "🔄 Refresh"
                    onClicked: savedController.refresh()
                }
            }

            Label {
                text: "No saved portfolios yet. Generate one first!"
                color: "#808080"
                visible: !savedController || savedController.portfolios.length === 0
                Layout.leftMargin: 24
            }

            Repeater {
                model: savedController ? (savedController.portfolios || []) : []

                delegate: Frame {
                    required property var modelData
                    required property int index
                    Layout.fillWidth: true
                    Layout.leftMargin: 24
                    Layout.rightMargin: 24

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Label {
                                text: modelData.name
                                font.pixelSize: 16
                                font.bold: true
                                color: "#ffffff"
                                Layout.fillWidth: true
                            }
                            Label {
                                text: modelData.createdAt
                                color: "#808080"
                                font.pixelSize: 12
                            }
                            Button {
                                text: "📋 Load"
                                onClicked: portfolioController.loadPortfolio(modelData)
                            }
                            Button {
                                text: "🗑️"
                                onClicked: savedController.deletePortfolio(modelData.id)
                            }
                        }

                        RowLayout {
                            spacing: 16
                            Label { text: "Target: " + (modelData.targetYield * 100).toFixed(2) + "%"; color: "#b0b0b0" }
                            Label { text: "Achieved: " + (modelData.achievedYield * 100).toFixed(2) + "%"; color: "#4ecdc4" }
                            Label { text: "Beta: " + Number(modelData.aggregateBeta).toFixed(3); color: "#ffe66d" }
                            Label { text: modelData.holdings.length + " holdings"; color: "#808080" }
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: 24 }
        }
    }
}
