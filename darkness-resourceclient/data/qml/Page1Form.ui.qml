import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import Darkness 1.0

Item {
    id: projectList
    objectName: "logListObjectName"

    FontLoader { id: userFont; source: "fonts/Roboto-Thin.ttf" }

    Rectangle {
        id: rectangle1
        color: "#c6d0cb"
        anchors.fill: parent

        ScrollView {
            id: scrollView
            anchors.right: parent.right
            anchors.rightMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 0
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.top: parent.top
            anchors.topMargin: 0

            ListView {
                id: listView1
                anchors.fill: parent
                anchors.margins: 2
                clip: true
                model: logListModel

                spacing: 0
                delegate: Item {
                    x: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    height: 20

                    Rectangle {
                        id: rectangle7
                        height: parent.height
                        color: "#c6d0cb"
                        anchors.top: parent.top
                        anchors.topMargin: 0
                        anchors.right: parent.right
                        anchors.rightMargin: 0
                        anchors.left: parent.left
                        anchors.leftMargin: 0

                        Text {
                            id: projectNameText
                            text: entry
                            font.bold: true
                            anchors.top: parent.top
                            anchors.topMargin: 2
                            anchors.left: parent.left
                            anchors.leftMargin: 3
                            font.pixelSize: 12
                            font.family: userFont.name
                        }

                        MouseArea {
                            id: testiArea
                            anchors.fill: parent

                        }

                    }
                    MouseArea {
                        id: itemMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onEntered: {
                            //projectNameText.font.underline = true;
                        }
                        onExited: {
                            //projectNameText.font.underline = false;
                        }
                        onClicked:{
                            //load = true;
                        }
                    }
                }
            }
        }
    }
}
