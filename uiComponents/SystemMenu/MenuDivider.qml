import Qt 4.7

Image {
    property int widthOffset: 7
    property real layoutScale: 1.0
    width: (parent.width - widthOffset) * layoutScale
    anchors.horizontalCenter: parent.horizontalCenter
    source: "/usr/palm/sysmgr/images/menu-divider.png"
}
