import Qt 4.7
import "../ActionButton"


Item {
    property real uiScale : 1.0
    property int  margin: 6 * uiScale
    property int  topOffset: 4 * uiScale

    property string dialogTitle: runtime.getLocalizedString("Dismissing Cards");
    property string dialogMessage: runtime.getLocalizedString("You can close an application by using your finger to flick it up and off screen while in Card View.");

    property alias actionButton1: okButton;

    signal okButtonPressed();

    width: 320 * uiScale;
    height: 170 * uiScale;

    id: dialog;

    function setWidth(inWidth) {
        width = inWidth;
    }

    function setUiScale(scale) {
        uiScale = scale;
    }

    Text {
        id: titleText;
        width: dialog.width - 2 * margin;
        font.family: "Prelude"
        font.pixelSize: 18 * uiScale
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
        font.pixelSize: 14 * uiScale
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
        width: dialog.width - 2 * margin - 1;
        height: visible ? 52 * uiScale : 0;
        x: margin + 1;
        onAction: dialog.okButtonPressed();
		anchors.bottom: dialog.bottom;
    }
}
//*/
