import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1

Window {
    id: appWindowObjectName
    objectName: "appWindowObjectName"

    visible: true
    width: 800
    height: 600
    title: qsTr("Darkness Launcher")

    Page1 {
        anchors.fill: parent
    }

    CreateProjectMain {
        id: createProjectMainId
        objectName: "createProjectMainIdName"
        visible: false
        width: parent.width
        height: parent.height - 56
        x: 0
        y: visible ? 56 : parent.height

        Behavior on y {
            NumberAnimation {
                id: bouncebehavior
                easing {
                    type: Easing.OutQuad
                    amplitude: 1.0
                    period: 0.5
                }
                duration: 100
            }
        }

        onCloseCreateProject: {
            y = parent.height;
        }
    }
}
