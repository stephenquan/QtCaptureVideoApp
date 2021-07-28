import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtMultimedia 5.12
import VideoCaptureApp 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Android Camera App")

    property bool landscape: width >= height

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    VideoOutput {
        width: landscape ? parent.width / 2 : parent.width
        height: landscape ? parent.height : parent.height / 2

        source: camera
        filters: [ captureVideoFilter ]
        autoOrientation: true

        Frame {
            width: parent.width

            background: Rectangle {
                color: "#808080"
                opacity: 0.5
            }

            Text {
                width: parent.width
                text: qsTr("Camera")
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                color: "yellow"
            }
        }
    }

    Image {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: landscape ? parent.width / 2 : parent.width
        height: landscape ? parent.height : parent.height / 2
        source: captureVideoFilter.image
        fillMode: Image.PreserveAspectFit

        Frame {
            width: parent.width

            background: Rectangle {
                color: "#808080"
                opacity: 0.5
            }

            Text {
                width: parent.width
                text: captureVideoFilter.image
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                color: "yellow"
            }
        }
    }

    Camera {
        id: camera
    }

    CaptureVideoFilter {
        id: captureVideoFilter
        interval: 50
    }
}
