import Qt 4.7

Item {
    property real uiScale: 1.0
    property real textScale: 1.0
    property real layoutScale: 1.0
    property bool isPIN: false;
    property int maxPINLength:  30
    property int maxPassLength: 30
    property alias enteredText: inputField.text

    signal textFieldClicked();

    width: 320 * layoutScale;
    height: isPIN ? inputField.height + 12 * layoutScale: 50 * layoutScale;

    function keyInput(keyText, isNumber) {
        if(inputField.text.length < (isPIN ? maxPINLength : maxPassLength)) {
            if(!isPIN || (isNumber)) {
                inputField.text = inputField.text.concat(keyText);
            }
        }
    }

    function clearAll() {
        inputField.text = "";
    }

    function deleteOne() {
        if(inputField.text.length > 0) {
            inputField.text = inputField.text.slice(0, inputField.text.length-1);
        }
    }

    function setHintText(hint) {
        hintText.text = hint;
    }

    BorderImage {
        visible: !isPIN;
        source: "/usr/palm/sysmgr/images/pin/password-lock-field.png"
        width: parent.width / uiScale;
        height: parent.height / uiScale;
        transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
        border { left: 120; top: 40; right: 120; bottom: 40 }
    }

    TextInput {
        id: inputField;
        width: parent.width - (16 * layoutScale);
        anchors.verticalCenter: parent.verticalCenter;
        anchors.horizontalCenter: parent.horizontalCenter;
        echoMode: TextInput.PasswordEchoOnEdit;
        passwordCharacter: "•"
        cursorVisible: !isPIN;
        cursorPosition: text.length;
        activeFocusOnPress: false;
        focus: false;
        horizontalAlignment: isPIN ? TextInput.AlignHCenter : TextInput.AlignLeft;
        color: isPIN ? "#FFF" : "#000";
        font.bold: true;
        font.pixelSize: 18 * textScale
        font.letterSpacing: 2 * layoutScale
        font.family: "Prelude"

        MouseArea {
            anchors.fill: parent
            onClicked: {
                textFieldClicked();
            }
        }
    }

    Text {
        id: hintText
        visible: inputField.text.length == 0;
        color: "#9C9C9C";
        font.pixelSize: 17 * textScale
        font.family: "Prelude"
        width: parent.width - (20 * layoutScale);
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: isPIN ? Text.AlignHCenter : Text.AlignLeft;

        text: isPIN ? "Enter PIN" : " Enter Password"; // Localize this
    }
}
