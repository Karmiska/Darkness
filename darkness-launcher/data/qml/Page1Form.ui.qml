import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import Darkness 1.0

Item {
    id: projectList
    objectName: "projectListObjectName"
    property alias mouseArea1: mouseArea1
    property alias image1: image1
    property alias image2: image2
    property alias mouseArea2: mouseArea2
    property alias image3: image3
    property alias image4: image4
    property alias newButtonRectangle: newButtonRectangle
    property alias openButtonRectangle: openButtonRectangle
    property alias projectList: projectList

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
            anchors.topMargin: 56

            ListView {
                id: listView1
                /*anchors.right: parent.right
                anchors.rightMargin: 30
                anchors.left: parent.left
                anchors.leftMargin: 30
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 30
                anchors.top: parent.top
                anchors.topMargin: 30*/
                anchors.fill: parent
                anchors.margins: 30
                clip: true
                model: projectListModel

                spacing: 0
                delegate: Item {
                    x: 0
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    height: 60

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
                            text: projectName
                            font.bold: true
                            anchors.top: parent.top
                            anchors.topMargin: 10
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            font.pixelSize: 16
                            font.family: userFont.name
                        }

                        Text {
                            id: projectPathText
                            x: 3
                            y: 7
                            text: projectPath
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 10
                            anchors.left: parent.left
                            font.pixelSize: 12
                            anchors.leftMargin: 10
                            font.family: userFont.name
                            font.bold: true
                            color: "#666666"
                        }

                        Rectangle {
                            id: rectangle8
                            height: 1
                            visible: lineRole
                            color: "#333333aa"
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 1
                            anchors.right: parent.right
                            anchors.rightMargin: 0
                            anchors.left: parent.left
                            anchors.leftMargin: 0
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
                            projectNameText.font.underline = true;
                        }
                        onExited: {
                            projectNameText.font.underline = false;
                        }
                        onClicked:{
                            load = true;
                            //loadProject(projectName, projectPath);
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: rectangle2
        height: 56
        color: "#ffffff"
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: 0

        Text {
            id: text1
            x: 420
            y: 8
            color: "#545454"
            text: qsTr("Projects")
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 30
            font.family: userFont.name;
            font.bold: false
            font.pixelSize: 32
        }

        Rectangle {
            id: newButtonRectangle
            x: 265
            y: 34
            width: 100
            height: 40
            color: "#ffffff"
            anchors.right: parent.right
            anchors.rightMargin: 150
            anchors.verticalCenter: parent.verticalCenter

            OpacityAnimator {
                id: goDark
                objectName: "goDark"
                target: newButtonRectangle
                from: 1;
                to: 0;
                duration: 50
                running: false
            }

            OpacityAnimator {
                id: goLight
                objectName: "goLight"
                target: newButtonRectangle
                from: 0;
                to: 1;
                duration: 50
                running: false
            }

            MouseArea {
                id: mouseArea1
                objectName: "goDarkMouseArea"
                anchors.fill: parent
                hoverEnabled: true
            }

            Rectangle {
                id: rectangle3
                x: 0
                y: 0
                width: 100
                height: 40
                color: "#ffffff"

                Image {
                    id: image1
                    y: 12
                    width: 24
                    height: 24
                    opacity: 0.6
                    smooth: false
                    antialiasing: false
                    anchors.left: parent.left
                    anchors.leftMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    source: "images/project_add_24.png"
                }

                Image {
                    id: image2
                    x: 0
                    y: 11
                    width: 24
                    height: 24
                    smooth: false
                    antialiasing: false
                    visible: false
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    source: "images/project_add_light_24.png"
                    anchors.leftMargin: 15
                }

                Text {
                    id: text2
                    text: qsTr("NEW")
                    anchors.right: parent.right
                    anchors.rightMargin: 25
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 12
                    font.family: userFont.name;
                }

            }


        }

        Rectangle {
            id: openButtonRectangle
            x: 265
            y: 40
            width: 100
            height: 40
            color: "#ffffff"
            anchors.rightMargin: 40
            anchors.right: parent.right

            OpacityAnimator {
                id: goOpenDark
                objectName: "goOpenDark"
                target: openButtonRectangle
                from: 1;
                to: 0;
                duration: 50
                running: false
            }

            OpacityAnimator {
                id: goOpenLight
                objectName: "goOpenLight"
                target: openButtonRectangle
                from: 0;
                to: 1;
                duration: 50
                running: false
            }

            MouseArea {
                id: mouseArea2
                objectName: "goOpenDarkMouseArea"
                anchors.fill: parent
                hoverEnabled: true
            }

            Rectangle {
                id: rectangle6
                x: 0
                y: 0
                width: 100
                height: 40
                color: "#ffffff"
                Image {
                    id: image3
                    y: 12
                    width: 24
                    height: 24
                    opacity: 0.6
                    smooth: false
                    antialiasing: false
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    source: "images/project_open_24.png"
                    anchors.leftMargin: 15
                }

                Image {
                    id: image4
                    x: 0
                    y: 11
                    width: 24
                    height: 24
                    smooth: false
                    antialiasing: false
                    visible: false
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    source: "images/project_open_light_24.png"
                    anchors.leftMargin: 15
                }

                Text {
                    id: text3
                    text: qsTr("OPEN")
                    font.family: userFont.name
                    anchors.rightMargin: 25
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 12
                }
            }
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
