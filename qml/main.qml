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
    title: qsTr("Qt Capture Video")

    property string cameraDisplayName: qsTr("Camera")
    property int cameraIndex: 0
    property var frameStart: Date.now()
    property int frameCount: 0
    property string fps: ""
    property bool landscape: width >= height

    Page {
        anchors.fill: parent

        background: Rectangle {
            color: "black"
        }

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            width: landscape ? parent.width / 2 : parent.width
            height: landscape ? parent.height : parent.height / 2
            source: camera
            filters: [ captureVideoFilter ]
            autoOrientation: true

            Frame {
                width: parent.width

                background: Rectangle {
                    color: "black"
                    opacity: 0.5
                }

                Text {
                    width: parent.width
                    text: ""
                    + qsTr("%1 (%2 of %3)")
                        .arg(cameraDisplayName)
                        .arg(cameraIndex + 1)
                        .arg(QtMultimedia.availableCameras.length) + "\n"
                    + qsTr("Orientation: %1").arg(videoOutput.orientation) + "\n"
                    + prettify(captureVideoFilter.imageInfo) + "\n"
                    + fps
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    color: "yellow"
                    font.pointSize: 10
                }
            }
        }

        footer: Frame {
            background: Rectangle {
                color: "#808080"
            }

            RowLayout {
                width: parent.width

                Item { Layout.fillWidth: true }

                Button {
                    icon.source: "images/camera-switch-front-back-32.svg"
                    onClicked: changeCamera()
                }

                Item { Layout.fillWidth: true }

                Button {
                    icon.source: "images/function-32.svg"
                    onClicked: changeMethod()
                }

                Item { Layout.fillWidth: true }
            }
        }
    }

    Camera {
        id: camera
    }

    CaptureVideoFilter {
        id: captureVideoFilter

        onImageChanged: {
            frameCount++;
            let elapsed = (Date.now() - frameStart) / 1000.0;
            fps = (frameCount / elapsed).toFixed(0) + " FPS";
        }

        onFrame: {
            frameCount++;
            let elapsed = (Date.now() - frameStart) / 1000.0;
            fps = (frameCount / elapsed).toFixed(0) + " FPS";
        }
    }

    Component.onCompleted: {
        console.log(JSON.stringify(QtMultimedia.availableCameras, undefined, 2));
        Qt.callLater(selectCamera, 0);
    }

    function selectCamera(index) {
        if (index > QtMultimedia.availableCameras.length) {
            return;
        }
        cameraIndex = index;
        camera.stop();
        camera.deviceId = QtMultimedia.availableCameras[cameraIndex].deviceId;
        camera.start();
        cameraDisplayName = QtMultimedia.availableCameras[cameraIndex].displayName;
        resetFpsInfo();
    }

    function changeCamera() {
        let nextCameraIndex = (cameraIndex + 1) % QtMultimedia.availableCameras.length;
        selectCamera(nextCameraIndex);
    }

    function changeMethod() {
        switch (captureVideoFilter.conversionMethod)
        {
        case CaptureVideoFilter.ConversionMethodNone:
            captureVideoFilter.conversionMethod = CaptureVideoFilter.ConversionMethodQt;
            break;
        case CaptureVideoFilter.ConversionMethodQt:
            captureVideoFilter.conversionMethod = CaptureVideoFilter.ConversionMethodMap;
            break;
        case CaptureVideoFilter.ConversionMethodMap:
            captureVideoFilter.conversionMethod =
                    Qt.platform.os === "android"
            ? CaptureVideoFilter.ConversionMethodOpenGL
            : CaptureVideoFilter.ConversionMethodNone
            break;
        default:
            captureVideoFilter.conversionMethod = CaptureVideoFilter.ConversionMethodNone;
            break;
        }
        resetFpsInfo();
    }

    function resetFpsInfo() {
        frameStart = Date.now();
        frameCount = 0;
        fps = "0 FPS";
    }

    function prettify(obj) {
        return Object.entries(obj).map( ([a,b]) => a + ": " + b).join("\n");
    }
}
