import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    property int pageIndex: 4

    header: ToolBar {
        contentHeight: 48

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8

            Label {
                text: "Saved Portfolios (" + savedController.count + ")"
                font.pixelSize: 22
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "Refresh"
                onClicked: savedController.refresh()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ListView {
            id: portfolioList
            Layout.fillHeight: true
            Layout.preferredWidth: 360
            clip: true
            model: savedController.portfolios

            delegate: Rectangle {
                required property int index
                required property var modelData

                width: portfolioList.width
                height: 80
                color: index === portfolioList.currentIndex ? "#2a3a4a" : (index % 2 === 0 ? "#252a33" : "#20242c")

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        portfolioList.currentIndex = index
                        savedController.selectPortfolio(index)
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 4

                    RowLayout {
                        Label {
                            text: modelData.name
                            font.bold: true
                            Layout.fillWidth: true
                        }

                        Button {
                            text: "\u2715"
                            flat: true
                            onClicked: savedController.deletePortfolio(index)
                        }
                    }

                    RowLayout {
                        Label {
                            text: "Yield: " + (modelData.achievedYield * 100).toFixed(2) + "%"
                        }

                        Label {
                            text: "Beta: " + Number(modelData.aggregateBeta).toFixed(3)
                        }

                        Label {
                            text: modelData.holdingsCount + " holdings"
                        }
                    }

                    Label {
                        text: modelData.createdAt
                        font.pixelSize: 11
                        opacity: 0.6
                    }
                }
            }
        }

        Rectangle {
            width: 1
            Layout.fillHeight: true
            color: "#444"
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 16
                padding: 24

                Label {
                    text: savedController.selectedPortfolio.name || "Select a portfolio"
                    font.pixelSize: 20
                    font.bold: true
                }

                RowLayout {
                    visible: savedController.selectedPortfolio.id !== undefined
                    spacing: 12

                    Button {
                        text: "Export CSV"
                        enabled: portfolioList.currentIndex >= 0
                        onClicked: exportController.exportPortfolioCsv(portfolioList.currentIndex)
                    }

                    Button {
                        text: "Export PDF"
                        enabled: portfolioList.currentIndex >= 0
                        onClicked: exportController.exportPortfolioPdf(portfolioList.currentIndex)
                    }
                }

                RowLayout {
                    visible: savedController.selectedPortfolio.name !== undefined
                    spacing: 24

                    Frame {
                        ColumnLayout {
                            Label {
                                text: "Target Yield"
                                font.pixelSize: 11
                                opacity: 0.7
                            }

                            Label {
                                text: ((savedController.selectedPortfolio.targetYield || 0) * 100).toFixed(2) + "%"
                                font.pixelSize: 18
                            }
                        }
                    }

                    Frame {
                        ColumnLayout {
                            Label {
                                text: "Achieved Yield"
                                font.pixelSize: 11
                                opacity: 0.7
                            }

                            Label {
                                text: ((savedController.selectedPortfolio.achievedYield || 0) * 100).toFixed(2) + "%"
                                font.pixelSize: 18
                            }
                        }
                    }

                    Frame {
                        ColumnLayout {
                            Label {
                                text: "Portfolio Beta"
                                font.pixelSize: 11
                                opacity: 0.7
                            }

                            Label {
                                text: Number(savedController.selectedPortfolio.aggregateBeta || 0).toFixed(3)
                                font.pixelSize: 18
                            }
                        }
                    }
                }

                RowLayout {
                    visible: savedController.selectedPortfolio.holdings !== undefined
                    Layout.fillWidth: true

                    Label {
                        text: "Ticker"
                        font.bold: true
                        Layout.preferredWidth: 100
                    }

                    Label {
                        text: "Weight"
                        font.bold: true
                        Layout.preferredWidth: 100
                    }

                    Label {
                        text: "Yield"
                        font.bold: true
                        Layout.preferredWidth: 100
                    }

                    Label {
                        text: "Beta"
                        font.bold: true
                        Layout.preferredWidth: 100
                    }
                }

                Repeater {
                    model: savedController.selectedPortfolio.holdings || []

                    RowLayout {
                        required property var modelData
                        Layout.fillWidth: true

                        Label {
                            text: modelData.ticker
                            Layout.preferredWidth: 100
                        }

                        Label {
                            text: (modelData.weight * 100).toFixed(2) + "%"
                            Layout.preferredWidth: 100
                        }

                        Label {
                            text: (modelData.yield * 100).toFixed(2) + "%"
                            Layout.preferredWidth: 100
                        }

                        Label {
                            text: Number(modelData.beta).toFixed(3)
                            Layout.preferredWidth: 100
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: savedController.refresh()
}