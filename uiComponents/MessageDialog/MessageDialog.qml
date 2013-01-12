import Qt 4.7
import "../ActionButton"

Item {
    property real uiScale: 1.0
    property int  edgeOffset: 11 * uiScale
    property int  margin: 6 * uiScale
    property int  topOffset: 4 * uiScale

    property string dialogTitle: "Title"
    property string dialogMessage: "Message Body."
    property int    numberOfButtons: 3 // valid between 0 and 3

    property alias actionButton1: button1;
    property alias actionButton2: button2;
    property alias actionButton3: button3;

    signal button1Pressed();
    signal button2Pressed();
    signal button3Pressed();

    width: (320 + 2 * edgeOffset) * uiScale
    height: titleText.height + msgText.height + ((numberOfButtons > 0) ? (button1.height + button2.height + button3.height) : edgeOffset) + 2*edgeOffset + 4*margin + topOffset;

    id: dialog;

    function setupDialog(title, message, numButtons) {
         dialogTitle     = title;
         dialogMessage   = message;
         numberOfButtons = numButtons;
     }

    function setButton1(message, type) {
        setupButton(button1, message, type);
    }

    function setButton2(message, type) {
        setupButton(button2, message, type);
    }

    function setButton3(message, type) {
        setupButton(button3, message, type);
    }

    function setupButton (button, message, type) {
        button.caption = message;
        button.affirmative = (type == "affirmative");
        button.negative = (type == "negative");
        button.visible = (type != "disabled");
    }

    function fade(fadeIn, fadeDuration) {
        fadeAnim.duration = fadeDuration;

        if(fadeIn) {
            opacity = 1.0;
        } else {
            opacity = 0.0;
        }
    }

    Behavior on opacity {
        NumberAnimation{ id: fadeAnim; duration: 300; }
    }

    onOpacityChanged: {
        if(opacity == 0.0) {
            visible = false;
         } else {
            visible = true;
        }
    }

    BorderImage {
        source: "/usr/palm/sysmgr/images/popup-bg.png"
        width: parent.width * 4;
        height: parent.height * 4;
        transform: Scale { origin.x: 0; origin.y: 0; xScale: 0.25; yScale: 0.25;}
        border { left: 140; top: 160; right: 140; bottom: 160 }
    }

    Text {
        id: titleText;
        width: dialog.width - 2 * (edgeOffset + margin);
        font.family: "Prelude"
        font.pixelSize: 18
        font.bold: true;
        wrapMode: Text.Wrap;
        color: "#FFF";
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignLeft;
        y: edgeOffset + margin + topOffset;

        text: dialogTitle;
    }

    Text {
        id: msgText;
        width: dialog.width - 2 * (edgeOffset + margin);
        font.family: "Prelude"
        font.pixelSize: 14
        font.bold: true;
        wrapMode: Text.Wrap;
        color: "#FFF";
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignLeft;
        y: titleText.y + titleText.height + margin;

        text: dialogMessage;
    }


    ActionButton {
        id: button1;
        caption: "Button 1";
        width: dialog.width - 2 * (edgeOffset + margin) - 1;
        height: visible ? 52 : 0;
        x: edgeOffset + margin + 1;
        y: msgText.y + msgText.height + margin;
        visible: numberOfButtons > 0;
        onAction: button1Pressed();
    }

    ActionButton {
        id: button2;
        caption: "Button 2";
        width: dialog.width - 2 * (edgeOffset + margin) - 1;
        height: visible ? 52 : 0;
        x: edgeOffset + margin + 1;
        y: button1.y + button1.height
        visible: numberOfButtons > 1;
        onAction: button2Pressed();
    }

    ActionButton {
        id: button3;
        caption: "Button 3";
        width: dialog.width - 2 * (edgeOffset + margin) - 1;
        height: visible ? 52 : 0;
        x: edgeOffset + margin + 1;
        y: button2.y + button2.height
        visible: numberOfButtons > 2;
        onAction: button3Pressed();
    }

}
