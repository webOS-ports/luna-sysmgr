import Qt 4.7

Image {
    property real uiScale: 1.0
    property int widthOffset: 56 * uiScale
    scale: uiScale/4
    width: (parent.width - widthOffset) / (uiScale/4)
    anchors.horizontalCenter: parent.horizontalCenter
    source: "/usr/palm/sysmgr/images/menu-divider.png"
}
