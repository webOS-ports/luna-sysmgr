import Qt 4.7
import "../ActionButton"
import CustomComponents 1.0


InputItem {
    id: unlockPanel
    property real uiScale: 1.0
    property int  edgeOffset: 11 * uiScale
    property int  margin: 6 * uiScale
    property int  topOffset: 4 * uiScale
    property bool isPINEntry: true
    property int  minPassLength: 4
    property bool enforceMinLength: false
    property string queuedTitle: ""
    property string queuedHint: ""

    signal entryCanceled();

    signal requestFocusChange(bool focusRequest);

    signal passwordSubmitted(string password, bool isPIN);

    function setUiScale(scale) {
        uiScale = scale;
    }

    function setupDialog(isPIN, title, hintMessage, enforceLength, minLen) {
        isPINEntry = isPIN;
        titleText.text = title;
        enforceMinLength = enforceLength;
        minPassLength = minLen;
        passwordField.clearAll();
        passwordField.setHintText(hintMessage);
     }

    function queueUpTitle(newTitle, newHint) {
        queuedTitle = newTitle;
        queuedHint = newHint;
    }

    function fade(fadeIn, fadeDuration) {
        fadeAnim.duration = fadeDuration;

        if(fadeIn) {
            opacity = 1.0;
        } else {
            opacity = 0.0;
        }
    }

    width: (320 * uiScale) + 2 * edgeOffset
    height: buttonGrid.y + buttonGrid.height + edgeOffset + margin;
    focus: true;

    BorderImage {
        source: "/usr/palm/sysmgr/images/popup-bg.png"
        width: parent.width / uiScale;
        height: parent.height / uiScale;
        transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
        smooth: true;
        border { left: 140; top: 160; right: 140; bottom: 160 }
    }

    Text {
        id: titleText;
        font.family: "Prelude"
        font.pixelSize: 18 * uiScale
        font.bold: true;
        color: "#FFF";
        anchors.horizontalCenter: parent.horizontalCenter
        y: edgeOffset + margin + topOffset;

        text: "Device Locked";
    }

    PasswordField {
        id: passwordField;
        isPIN: isPINEntry;
        width: (320 - 4) * uiScale;
        x: edgeOffset + (3 * uiScale)
        y: titleText.y + titleText.height + (isPINEntry ? 0 : 6 * uiScale)
        uiScale: unlockPanel.uiScale
        uiScale: unlockPanel.uiScale
        uiScale: unlockPanel.uiScale

        onTextFieldClicked: {
            if(!isPINEntry) {
                requestFocusChange(true);
            } else {
                focus = true;
            }
        }
    }

    PINPad {
        id: keyPad
        visible: isPINEntry
        x: edgeOffset
        anchors.top: passwordField.bottom
        uiScale: unlockPanel.uiScale
        uiScale: unlockPanel.uiScale
        uiScale: unlockPanel.uiScale

        onKeyAction: {
            if(keyText == "\b") {
                // backspace key pressed
                passwordField.deleteOne();
            } else {
                passwordField.keyInput(keyText, true);
                if(queuedTitle != "") {
                    titleText.text = queuedTitle;
                    queuedTitle = "";
                }
                if(queuedHint != "") {
                    passwordField.setHintText(queuedHint);
                    queuedHint = "";
                }
            }
        }
    }

    Grid {
        id: buttonGrid
        width: (320 * uiScale) - 2 * margin
        x: edgeOffset + margin
        anchors.top: isPINEntry ? keyPad.bottom : passwordField.bottom;

        columns: 2
        rows: 1
        spacing: margin + 1

        ActionButton {
            caption: runtime.getLocalizedString("Cancel");
            width: buttonGrid.width/buttonGrid.columns - margin / 2
            height: 52 * uiScale
            uiScale: unlockPanel.uiScale
            uiScale: unlockPanel.uiScale
            uiScale: unlockPanel.uiScale
            onAction: entryCanceled();
        }

        ActionButton {
            caption: runtime.getLocalizedString("Done");
            affirmative: true
            width: buttonGrid.width/buttonGrid.columns - margin / 2
            height: 52 * uiScale
            uiScale: unlockPanel.uiScale
            uiScale: unlockPanel.uiScale
            uiScale: unlockPanel.uiScale
            active: passwordField.enteredText.length >= (enforceMinLength ? minPassLength : 1);
            onAction: {
                if(passwordField.enteredText.length > 0) {
                    passwordSubmitted(passwordField.enteredText, isPINEntry);
                }
            }
        }
    }

    function isValidKey(keyCode) {
        if(((keyCode >= Qt.Key_Escape) && (keyCode <= Qt.Key_Direction_R)) ||
           ((keyCode >= Qt.Key_Back) && (keyCode <= Qt.Key_unknown))     ) {
            // filter keys here
            return false;
        }

        return true;
    }

    function isNumber(keyCode) {
        if((keyCode >= Qt.Key_0) && (keyCode <= Qt.Key_9)) {
            return true;
        }

        return false;
    }

    Behavior on opacity {
        NumberAnimation{ id: fadeAnim;
                         duration: 300;
                         onStarted: {
                               if(opacity == 0.0) {
                               }  else {
                               }
                            }
                        }
    }

    onOpacityChanged: {
        if(opacity == 0.0) {
            if(!isPINEntry) {
                // faded away, clear focus
                requestFocusChange(false);
            } else {
                focus = false;
            }
            visible = false;
        } else if(opacity == 1.0) {
            if(!isPINEntry) {
                // faded in, request focus
                requestFocusChange(true);
            } else {
                focus = true;
            }
            visible = true;
        } else {
            visible = true;
        }
    }

    Keys.onPressed: {
        event.accepted = true;

        if(isValidKey(event.key)) {
             passwordField.keyInput(event.text, isNumber(event.key));
             if(queuedTitle != "") {
                 titleText.text = queuedTitle;
                 queuedTitle = "";
             }
             if(queuedHint != "") {
                 passwordField.setHintText(queuedHint);
                 queuedHint = "";
             }
         } else if(event.key == Qt.Key_Backspace) {
             passwordField.deleteOne();
         }
    }

    Keys.onReleased: {
        event.accepted = true;
    }

    Keys.onDeletePressed: {
        passwordField.deleteOne();
        event.accepted = true;
    }

    Keys.onEnterPressed: {
        event.accepted = true;
        if(passwordField.enteredText.length > 0) {
            passwordSubmitted(passwordField.enteredText, isPINEntry);
        }
    }

    Keys.onReturnPressed: {
        event.accepted = true;
        if(passwordField.enteredText.length > 0) {
            passwordSubmitted(passwordField.enteredText, isPINEntry);
        }
    }
}
