import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtMultimedia 5.12
import QtCaptureVideoApp 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Android Camera App")

    property bool landscape: width >= height
    property var intervals: {
        "Slow": 10000,
        "Medium": 1000,
        "Fast" : 100
    }

    Page {
        anchors.fill: parent

        background: Rectangle {
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

        footer: Frame {
            background: Rectangle {
                color: "#808080"
            }

            RowLayout {
                width: parent.width

                Button {
                    text: qsTr("Slow")
                    checked:  captureVideoFilter.interval === intervals.Slow
                    onClicked: captureVideoFilter.interval = intervals.Slow;
                }

                Button {
                    text: qsTr("Medium")
                    checked:  captureVideoFilter.interval === intervals.Medium
                    onClicked: captureVideoFilter.interval = intervals.Medium;
                }

                Button {
                    text: qsTr("Fast")
                    checked:  captureVideoFilter.interval === intervals.Fast
                    onClicked: captureVideoFilter.interval = intervals.Fast;
                }

            }
        }
    }

    Camera {
        id: camera
    }

    CaptureVideoFilter {
        id: captureVideoFilter
        interval: intervals.Medium
    }
}
