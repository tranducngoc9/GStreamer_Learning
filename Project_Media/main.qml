import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
    Column{
        spacing: 10
        Button {
            text: "Play"
            onClicked: {
                myCppClass.play_cb()
            }
        }
        Button {
            text: "Pause"
            onClicked: {
                myCppClass.pause_cb()
            }
        }
        Button {
            text: "Stop"
            onClicked: {
                myCppClass.stop_cb()
            }
        }
        Button {
            text: "Seek"
            onClicked: {
                myCppClass.seek_cb()
            }
        }
    }
}