import QtQuick 6.5
import QtQuick.Window 6.5
import F3D 1.0

Window {
    id: root
    width: 800
    height: 600
    visible: true
    title: "libf3d + QML"
    color: "#202020"

    property string modelPath: ""
    Component.onCompleted: {
        if (typeof initialModelPath === "string") {
            modelPath = initialModelPath
        }
    }

    F3DView {
        id: view
        anchors.fill: parent
        focus: true

        modelPath: root.modelPath

        Keys.enabled: true
        Keys.onPressed: function (event) {
            view.keyPress(event.key, event.text, event.modifiers)
            event.accepted = true
        }

        Keys.onReleased: function (event) {
            view.keyRelease(event.key, event.modifiers)
            event.accepted = true
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton

            onPressed: function (mouse) {
                view.mousePress(mouse.x, mouse.y,
                    mouse.button, mouse.modifiers)
                mouse.accepted = true
            }

            onReleased: function (mouse) {
                view.mouseRelease(mouse.x, mouse.y,
                    mouse.button, mouse.modifiers)
                mouse.accepted = true
            }

            onPositionChanged: function (mouse) {
                view.mouseMove(mouse.x, mouse.y,
                    mouse.buttons, mouse.modifiers)
            }
        }


        WheelHandler {
            id: wheelHandler
            target: view
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad

            onWheel: function (event) {
                view.mouseWheel(event.angleDelta.x, event.angleDelta.y, event.modifiers)
                event.accepted = true
            }
        }
    }
}
