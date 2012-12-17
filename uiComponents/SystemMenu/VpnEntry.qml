import Qt 4.7

Item {
    property string name
    property bool   connected:      false
    property string connStatus:     ""
    property string status:         ((connStatus == "connecting") ? runtime.getLocalizedString("Connecting...") : ((connStatus == "connectfailed") ? runtime.getLocalizedString("Unable to connect") : ""))
    property string vpnProfileInfo: ""
    
    property real uiScale: 1.0

    property int iconSpacing : 16 * uiScale
    property int rightMarging: 12 * uiScale

    Item {
        anchors.fill: parent
        Text {
            id: mainText
            anchors.verticalCenter: parent.verticalCenter
            text: name;
            horizontalAlignment: Text.AlignLeft
            width: parent.width - (check.width * (uiScale/4)) - rightMarging - iconSpacing - 5
            elide: Text.ElideRight;
            color: "#FFF";
            font.bold: false;
            font.pixelSize: 16 * uiScale
            font.family: "Prelude"
        }

        Text {
            id: statusText
            visible: status != ""
            y: mainText.y + mainText.baselineOffset + 1
            text: status;
            color: "#AAA";
            font.pixelSize: 10 * uiScale
            font.family: "Prelude"
            font.capitalization: Font.AllUppercase
        }
    }

    Image {
        id: check
        x: parent.width - (width / 2) - iconSpacing - rightMarging
        anchors.verticalCenter: parent.verticalCenter
        visible: connected
        scale: uiScale/4
        source: "/usr/palm/sysmgr/images/statusBar/system-menu-popup-item-checkmark.png"
    }
}

