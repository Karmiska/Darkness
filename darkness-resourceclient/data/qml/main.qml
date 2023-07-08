import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1

Window {
    id: appWindowObjectName
    objectName: "appWindowObjectName"

    visible: true
    width: 400
    height: 300
    title: qsTr("Darkness Resource Encoding Server")

    Page1 {
        anchors.fill: parent
    }
}
