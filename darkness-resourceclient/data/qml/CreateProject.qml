import QtQuick 2.4

CreateProjectForm {

    objectName: "createProjectForm"

    signal nameChanged(string location)
    signal locationChanged(string location)
    signal browseClicked()
    signal cancelClicked()
    signal createClicked()
    signal destroyWindow()

    projectNameInput.onAccepted: {
        nameChanged(projectNameInput.text);
    }

    projectLocationInput.onAccepted: {
        locationChanged(projectLocationInput.text);
    }

    browseLocationButton.onClicked: {
        browseClicked();
    }

    browseLocationButton.onEntered: {
        browseLocationText.color = "#44ff44"
    }
    browseLocationButton.onExited: {
        browseLocationText.color = "#000000"
    }

    cancelButton.onClicked: {
        cancelClicked();
        destroyWindow();
    }

    createProjectButton.onClicked: {
        if(projectNameInput.text != "" && projectLocationInput.text != "")
        {
            createClicked();
            destroyWindow();
        }
    }

    createProjectButton.onEntered: {
        rectangle5.color = "#d3f8d0"
    }
    createProjectButton.onExited: {
        rectangle5.color = "#a9e8a5"
    }

    cancelButton.onEntered: {
        rectangle4.color = "#888888"
    }
    cancelButton.onExited: {
        rectangle4.color = "#666666"
    }
}
