import Qt 4.7

Item {
    id: menuconatiner
    clip: true;

    property real  uiScale: 1.0
    property int  maxHeight: 410 * uiScale
    property int  headerIdent:   14 * uiScale
    property int  edgeOffset: 11 * uiScale
    property alias mainMenuItem: mainMenu;
    property alias scrollable: flickableArea.interactive;


    width: mainMenu.width + clipRect.anchors.leftMargin + clipRect.anchors.rightMargin;
    height: maxHeight;

    function setUiScale(scale) {
        uiScale = scale;
    }

    function setMaximumHeight(h) {
        maxHeight = h + clipRect.anchors.bottomMargin + clipRect.anchors.topMargin;
    }

    function setWidth(w) {
        width = w;
    }

    function setContent(menuContent) {
        mainMenu.children = [content]
    }

    signal menuScrollStarted();


    // ------------------------------------------------------------


    BorderImage {
        id: menuBorder
        source: "/usr/palm/sysmgr/images/menu-dropdown-bg.png"
        width: parent.width / uiScale;
        height: Math.max(border.top + border.bottom, Math.min(menuconatiner.height,  (mainMenu.height + clipRect.anchors.topMargin + clipRect.anchors.bottomMargin))) /uiScale;
        transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
        smooth: true;
        border { left: 120; top: 40; right: 120; bottom: 120 }
    }

    Rectangle { // clipping rect inside the menu border
        id: clipRect
        anchors.fill: parent
        color: "transparent"
        clip: true
        anchors.leftMargin: 11 * uiScale
        anchors.topMargin: 0
        anchors.bottomMargin:15 * uiScale
        anchors.rightMargin: 11 * uiScale

        Flickable {
            id: flickableArea
            width: mainMenu.width;
            height: Math.min(menuconatiner.height - clipRect.anchors.topMargin - clipRect.anchors.bottomMargin, mainMenu.height);
            contentWidth: mainMenu.width; contentHeight: mainMenu.height

            onMovementStarted: {
                menuScrollStarted();
            }

            Item {
                id: mainMenu
                width: 320 * uiScale
                height:  700 * uiScale
            }
        }
    }

    Item {
        id: maskTop
        z:10
        width: parent.width - (22 * uiScale)
        anchors.horizontalCenter: parent.horizontalCenter
        y: 0
        opacity: !flickableArea.atYBeginning ? 1.0 : 0.0

        BorderImage {
	    width: parent.width / uiScale;
	    transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
	    smooth: true;
            source: "/usr/palm/sysmgr/images/menu-dropdown-scrollfade-top.png"
            border { left: 80; top: 0; right: 80; bottom: 0 }
        }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y:0
	    transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
	    smooth: true;
            source: "/usr/palm/sysmgr/images/menu-arrow-up.png"
        }

        Behavior on opacity { NumberAnimation{ duration: 70} }
    }

    Item {
        id: maskBottom
        z:10
        width: parent.width - (22 * uiScale)
        anchors.horizontalCenter: parent.horizontalCenter
        y: flickableArea.height - (28 * uiScale)
        opacity: !flickableArea.atYEnd ? 1.0 : 0.0

        BorderImage {
            width: parent.width / uiScale
	    transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
	    smooth: true;
            source: "/usr/palm/sysmgr/images/menu-dropdown-scrollfade-bottom.png"
            border { left: 80; top: 0; right: 80; bottom: 0 }
        }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y:10 * uiScale
	    transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
	    smooth: true;
            source: "/usr/palm/sysmgr/images/menu-arrow-down.png"
        }

        Behavior on opacity { NumberAnimation{ duration: 70} }
    }


}
