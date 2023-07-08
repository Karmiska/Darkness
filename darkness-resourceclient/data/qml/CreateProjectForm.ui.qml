import QtQuick 2.4

Item {
    width: 400
    height: 240
    property alias browseLocationText: browseLocationText
    property alias rectangle4: rectangle4
    property alias rectangle5: rectangle5
    property alias createProjectButton: createProjectButton
    property alias projectLocationInput: projectLocationInput
    property alias projectNameInput: projectNameInput
    property alias cancelButton: cancelButton
    property alias browseLocationButton: browseLocationButton

    FontLoader { id: userFont; source: "fonts/Roboto-Thin.ttf" }

    Rectangle {
        id: rectangle1
        color: "#c6d0cb"
        anchors.fill: parent

        Text {
            id: text1
            text: qsTr("Project name")
            font.bold: true
            anchors.left: parent.left
            anchors.leftMargin: 25
            anchors.top: parent.top
            anchors.topMargin: 25
            font.pixelSize: 13
            font.family: userFont.name
        }

        Rectangle {
            id: rectangle2
            height: 30
            color: "#ffffff"
            anchors.right: parent.right
            anchors.rightMargin: 25
            anchors.left: parent.left
            anchors.leftMargin: 25
            anchors.top: parent.top
            anchors.topMargin: 50
        }

        TextInput {
            id: projectNameInput
            objectName: "projectNameInput"
            height: 30
            text: qsTr("Name")
            font.bold: true
            anchors.right: parent.right
            anchors.rightMargin: 25
            anchors.left: parent.left
            anchors.leftMargin: 37
            anchors.top: parent.top
            anchors.topMargin: 50
            font.pixelSize: 13
            font.family: userFont.name
            verticalAlignment: Text.AlignVCenter
            selectByMouse: true
            mouseSelectionMode: TextInput.SelectCharacters
        }

        Text {
            id: text2
            text: qsTr("Location")
            font.bold: true
            anchors.left: parent.left
            anchors.leftMargin: 25
            anchors.top: parent.top
            anchors.topMargin: 100
            font.pixelSize: 13
            font.family: userFont.name
        }

        Rectangle {
            id: rectangle3
            x: -7
            y: -9
            height: 30
            color: "#ffffff"
            anchors.topMargin: 125
            anchors.rightMargin: 25
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 25
        }

        TextInput {
            id: projectLocationInput
            objectName: "projectLocationInput"
            height: 30
            text: qsTr("Name")
            font.bold: true
            horizontalAlignment: Text.AlignLeft
            anchors.right: parent.right
            anchors.rightMargin: 25
            anchors.left: parent.left
            anchors.leftMargin: 37
            anchors.top: parent.top
            anchors.topMargin: 125
            font.pixelSize: 13
            font.family: userFont.name
            verticalAlignment: Text.AlignVCenter
            selectByMouse: true
            mouseSelectionMode: TextInput.SelectCharacters

            Text {
                id: browseLocationText
                x: 0
                text: qsTr(". . .")
                font.bold: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.right: parent.right
                anchors.rightMargin: 9
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.top: parent.top
                anchors.topMargin: 0
                font.pixelSize: 14
            }

            MouseArea {
                id: browseLocationButton
                x: 326
                width: 30
                anchors.top: parent.top
                anchors.topMargin: 0
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.right: parent.right
                anchors.rightMargin: 4
                hoverEnabled: true
            }

        }

        Rectangle {
            id: rectangle4
            x: 161
            width: 79
            height: 30
            color: "#666666"
            radius: 5
            anchors.top: parent.top
            anchors.topMargin: 180
            anchors.right: parent.right
            anchors.rightMargin: 160

            Text {
                id: text5
                x: 28
                y: 8
                color: "#ffffff"
                text: qsTr("Cancel")
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 14
                font.family: userFont.name
            }

            MouseArea {
                id: cancelButton
                anchors.fill: parent
                hoverEnabled: true
            }
        }

        Rectangle {
            id: rectangle5
            x: 259
            width: 116
            height: 30
            color: "#a9e8a5"
            radius: 5
            anchors.top: parent.top
            anchors.topMargin: 180
            anchors.right: parent.right
            anchors.rightMargin: 25

            Text {
                id: text4
                x: 28
                y: 8
                text: qsTr("Create project")
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 14
                font.family: userFont.name
            }

            MouseArea {
                id: createProjectButton
                anchors.fill: parent
                hoverEnabled: true
            }
        }


    }
}
