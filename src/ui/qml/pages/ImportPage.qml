import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Page {
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20

            Item { Layout.preferredHeight: 24 }

            Label {
                text: "Import / Export"
                font.pixelSize: 24
                font.bold: true
                color: "#ffffff"
                Layout.leftMargin: 24
            }

            // Import section
            GroupBox {
                title: "Import Assets from CSV"
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Label {
                        text: "CSV format: ticker, name, sector, dividendYield, beta, price"
                        color: "#808080"
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        spacing: 12
                        Button {
                            text: "📂 Select CSV File"
                            onClicked: importDialog.open()
                        }
                        Label {
                            id: importStatus
                            text: ""
                            color: "#4ecdc4"
                        }
                    }
                }
            }

            // Export section
            GroupBox {
                title: "Export Portfolio to CSV"
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Label {
                        text: portfolioController.portfolioResult.holdings !== undefined
                            ? "Current portfolio has " + portfolioController.portfolioResult.holdings.length + " holdings."
                            : "Generate a portfolio first to export."
                        color: "#808080"
                    }

                    Button {
                        text: "💾 Export to CSV"
                        enabled: portfolioController.portfolioResult.holdings !== undefined
                        onClicked: exportDialog.open()
                    }

                    Label {
                        id: exportStatus
                        text: ""
                        color: "#4ecdc4"
                    }
                }
            }

            Item { Layout.preferredHeight: 24 }
        }
    }

    FileDialog {
        id: importDialog
        title: "Select CSV file to import"
        nameFilters: ["CSV files (*.csv)", "All files (*)"]
        onAccepted: {
            importExportController.importCsv(selectedFile.toString())
        }
    }

    FileDialog {
        id: exportDialog
        title: "Export portfolio as CSV"
        nameFilters: ["CSV files (*.csv)"]
        fileMode: FileDialog.SaveFile
        onAccepted: {
            importExportController.exportCsv(selectedFile.toString(), portfolioController.portfolioResult.holdings)
        }
    }

    Connections {
        target: importExportController
        function onImportCompleted(count) { importStatus.text = "✅ Imported " + count + " new assets" }
        function onExportCompleted(path) { exportStatus.text = "✅ Exported to " + path }
        function onErrorOccurred(msg) { importStatus.text = "❌ " + msg }
    }
}
