import Qt 4.7

MenuListEntry {
    id: muteElement
    property int    ident:         0
    property alias  muteText:      muteToggle.text
    property bool   mute:          false
    property bool   delayUpdate:   false
    property string newText:       ""
    property bool   newMuteStatus: false
    property real uiScale;

    property int iconSpacing : 16 * uiScale
    property int rightMarging: 12 * uiScale

    content:
        Item {
        width: muteElement.width

            Text {
            id: muteToggle
                x: ident;
                anchors.verticalCenter: parent.verticalCenter
                text: runtime.getLocalizedString("Mute Sound")
                color: "#FFF";
                font.bold: false;
                font.pixelSize: 18 * uiScale;
                font.family: "Prelude"
            }

            Image {
                id: muteIndicatorOn
                visible: !mute
                x: parent.width - (width / 2) - iconSpacing - rightMarging
                anchors.verticalCenter: parent.verticalCenter
                scale: uiScale/4
                smooth: true

                source: "/usr/palm/sysmgr/images/statusBar/icon-mute.png"
             }

            Image {
                id: muteIndicatorOff
                visible: mute
                x: parent.width - (width / 2) - iconSpacing - rightMarging
                anchors.verticalCenter: parent.verticalCenter
                scale: uiScale/4
                smooth: true

                source: "/usr/palm/sysmgr/images/statusBar/icon-mute-off.png"
             }
        }
}
