import QtQuick
import F3D

Window {
    id: root
    width: 800
    height: 600
    visible: true
    title: "libf3d + QML (C++)"
    color: "#202020"

    F3DView {
        id: view
        anchors.fill: parent
        focus: true

        enableMouse: true
        enableKeyboard: true
        modelPath: "C:/Path/to/Your/File/BrainStem.glb"

        Component.onCompleted: {
            setOption("render.show_edges", false);
            setOption("ui.axis", true);
            setOption("ui.fps", true);
            setOption("ui.filename", true);
        }
    }





}
