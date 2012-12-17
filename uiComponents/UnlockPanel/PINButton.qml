import Qt 4.7

Item {
    property real uiScale: 1.0
    property bool isPressed: false
    property string caption: ""
    property string imgSource: ""
    property bool   active: true

    BorderImage {
        id: pressedBkg
        source: "/usr/palm/sysmgr/images/pin/pin-key-highlight.png"
        visible: isPressed;
        width: parent.width / uiScale;
        height: parent.height / uiScale;
        transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
        smooth: true;
        border { left: 40; top: 40; right: 40; bottom: 40 }
    }

    Text {
        id: buttonText
        text: caption
        visible: caption != ""  && !buttonImg.visible;
        anchors.centerIn: parent
        color: "#FFF";
        font.bold: true;
        font.pixelSize: 30 * uiScale
        font.family: "Prelude"
        font.capitalization: Font.AllUppercase
    }

    Image {
        id: buttonImg
        source: imgSource
        visible: imgSource != "";
        scale: uiScale/4;
        smooth: true;
        anchors.centerIn: parent
    }

    MouseArea {
        id: mouseArea
        enabled: true;
        anchors.fill: parent
        onPressAndHold:  setPressed(true);
        onPressed: { mouse.accepted = true; setPressed(true); }
        onReleased: {setPressed(false);}
        onExited: {setPressed(false);}
        onCanceled: {setPressed(false);}
        onClicked: {
            actionPerformed()
         }
    }

    function setPressed (pressed) {
        if(active) {
            isPressed = pressed;
        }
    }

    function actionPerformed () {
        if(active) {
            action(caption)
        }
    }

    signal action(string text)
}
