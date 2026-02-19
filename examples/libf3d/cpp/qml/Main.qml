import QtQuick
import F3D

Window
{
    id: root
    width: 800
    height: 600
    visible: true
    title: "libf3d + QML (C++)"
    color: "#202020"

    F3DView
    {
        id: view
        anchors.fill: parent
        focus: true

        enableMouse: true
        enableKeyboard: true
        // Note: Do not use url (file:///c:/your/file.qsb) but ordinary file paths (c:/your/file.qsb)
        modelPath: fileArgument

        Component.onCompleted:
        {
            setOption("render.show_edges", false);
            setOption("ui.axis", true);
            setOption("ui.fps", true);
            setOption("ui.filename", true);
        }
    }
}
