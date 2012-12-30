import Qt 4.7

Item {
    property bool   isPressed: false
    property bool   active: true
    property string caption: "Button"
    property bool   affirmative: false
    property bool   negative:    false
    property real   inactiveOpacity: 0.70
    property real uiScale : 1.0

    width:  200 * uiScale;
    height:  40 * uiScale;

    BorderImage {
        id: pressedBkg
        source: affirmative ? ( !isPressed ? "/usr/palm/sysmgr/images/pin/button-green.png" : "/usr/palm/sysmgr/images/pin/button-green-press.png") :
                 ( negative ? ( !isPressed ? "/usr/palm/sysmgr/images/pin/button-red.png"   : "/usr/palm/sysmgr/images/pin/button-red-press.png")   :
                              ( !isPressed ? "/usr/palm/sysmgr/images/pin/button-black.png" : "/usr/palm/sysmgr/images/pin/button-black-press.png")  )
        visible: true;
        width: parent.width / uiScale;
        height: parent.height / uiScale;
        transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
        smooth: true;
        border { left: 40; top: 40; right: 40; bottom: 40 }
        opacity: active ? 1.0 : inactiveOpacity
    }

    Text {
        id: buttonText
        text: caption
        anchors.centerIn: parent
        color: "#FFF";
        font.bold: true;
        font.pixelSize: 16 * uiScale
        font.family: "Prelude"
        opacity: active ? 1.0 : inactiveOpacity
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
            action()
        }
    }

    signal action()
}
