import QtQuick 2.5
import QtQuick.Dialogs 1.2

FileDialog {
    id: fileDialog
    //title: "Please choose a file"
    folder: shortcuts.home

    selectMultiple: true
}
