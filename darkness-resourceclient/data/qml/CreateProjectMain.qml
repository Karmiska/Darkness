import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1

Item {
    id: createProjectName
    objectName: "createProjectName"
    visible: true
    signal closeCreateProject()

    CreateProject {
        id: createProjectId
        anchors.fill: parent

        onDestroyWindow: {
            closeCreateProject();
        }
    }

}
