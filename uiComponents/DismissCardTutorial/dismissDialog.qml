import Qt 4.7
import "../ActionButton"


Item {
    property real uiScale : 1.0
    property real textScale : 1.0
    property real layoutScale : 1.0
    property int  margin: 6 * layoutScale
    property int  topOffset: 4 * layoutScale

    property string dialogTitle: runtime.getLocalizedString("Dismissing Cards");
    property string dialogMessage: runtime.getLocalizedString("You can close an application by using your finger to flick it up and off screen while in Card View.");

    property alias actionButton1: okButton;

    signal okButtonPressed();

    width: 320 * layoutScale;
    height: 170 * layoutScale;

    id: dialog;

    function setWidth(inWidth) {
        width = inWidth;
    }

    function setUiScale(scale) {
        uiScale = scale;
    }

    function setTextScale(scale) {
        textScale = scale;
    }

    function setLayoutScale(scale) {
        layoutScale = scale;
    }

    Text {
        id: titleText;
        width: dialog.width - 2 * margin;
        font.family: "Prelude"
        font.pixelSize: 18 * textScale
        font.bold: true;
        wrapMode: Text.Wrap;
        color: "#FFF";
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignLeft;
        y: margin + topOffset;

        text: dialogTitle;
    }

    Text {
        id: msgText;
        width: dialog.width - 2 * margin;
        font.family: "Prelude"
        font.pixelSize: 14 * textScale
        font.bold: true;
        wrapMode: Text.Wrap;
        color: "#FFF";
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignLeft;
        y: titleText.y + titleText.height + margin;

        text: dialogMessage;
    }

 	ActionButton {
        id: okButton;
        caption: runtime.getLocalizedString("OK");
	uiScale : dialog.uiScale
	textScale : dialog.textScale
	layoutScale : dialog.layoutScale
        width: dialog.width - 2 * margin - 1;
        height: visible ? 52 * layoutScale : 0;
        x: margin + 1;
        onAction: dialog.okButtonPressed();
		anchors.bottom: dialog.bottom;
    }
}
//*/
