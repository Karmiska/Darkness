import QtQuick 2.7

Page1Form {

    objectName: "mainPageForm"

    signal createProject()
    signal openProject()

    signal openProjectLocation(string name, string location)

    mouseArea1.onClicked: {
        console.log("mouse area clicked.");
        createProject();
    }

    mouseArea1.onEntered: {
        image1.visible = false;
        image2.visible = true;
    }

    mouseArea1.onExited: {
        image1.visible = true;
        image2.visible = false;
    }

    mouseArea2.onClicked: {
        console.log("mouse area clicked.");
        openProject();
    }

    mouseArea2.onEntered: {
        image3.visible = false;
        image4.visible = true;
    }

    mouseArea2.onExited: {
        image3.visible = true;
        image4.visible = false;
    }

    /*projectList.onLoadProject: {
        openProjectLocation(name, location);
    }*/
}
