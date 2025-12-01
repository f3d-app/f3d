import QtQuick
import QtQuick.Window
import F3D 1.0

Window {
    id: root
    width: 800
    height: 600
    visible: true
    title: "libf3d + QML (C++)"
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

        transform: Scale {
            yScale: -1
            origin.y: view.height / 2
        }

        Keys.enabled: true
        Keys.onPressed: function (event) {
            view.KeyPress(event.key, event.text, event.modifiers)
            event.accepted = true
        }

        Keys.onReleased: function (event) {
            view.KeyRelease(event.key, event.modifiers)
            event.accepted = true
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton

            onPressed: function (mouse) {
                view.MousePress(mouse.x, mouse.y,
                    mouse.button, mouse.modifiers)
                mouse.accepted = true
            }

            onReleased: function (mouse) {
                view.MouseRelease(mouse.x, mouse.y,
                    mouse.button, mouse.modifiers)
                mouse.accepted = true
            }

            onPositionChanged: function (mouse) {
                view.MouseMove(mouse.x, mouse.y,
                    mouse.buttons, mouse.modifiers)
            }
        }

        WheelHandler {
            id: wheelHandler
            target: view
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad

            onWheel: function (event) {
                view.MouseWheel(event.angleDelta.x, event.angleDelta.y, event.modifiers)
                event.accepted = true
            }
        }
    }
}
