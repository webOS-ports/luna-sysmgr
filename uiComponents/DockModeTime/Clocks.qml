import Qt 4.7

Rectangle {
    id: container
    width: 720
    height: 1280
    color: "black"

    property real uiScale: 1.0
    property bool mainTimerRunning: false
    property int isLandscape: (runtime.orientation+1)%2

    function setWidth(w) {
        width = w;
    }

    function setHeight(h) {
        width = h;
    }

    VisualItemModel{
        id: clockList
        AnalogClock{width: container.width; height: container.height; uiScale: container.uiScale; uiScale: container.uiScale; smooth: true; glass: 1; timerRunning: mainTimerRunning}
        DigitalClock{width: container.width; height: container.height; uiScale: container.uiScale; uiScale: container.uiScale; smooth: true; timerRunning: mainTimerRunning}
        AnalogClock{width: container.width; height: container.height; uiScale: container.uiScale; uiScale: container.uiScale; smooth: true; glass: 0; timerRunning: mainTimerRunning}
    }

    ListView {
        id: flickable
        anchors.fill: parent
        focus: true
        highlightRangeMode: ListView.StrictlyEnforceRange
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        model: clockList
        boundsBehavior: Flickable.DragOverBounds
    }

    Row {
         spacing: 10 * uiScale
         anchors.centerIn: parent
         anchors.verticalCenterOffset: parent.height/2.5
         Image { id: clockdot1; scale: uiScale/4; smooth: true; source: "../../images/dockmode/time/indicator/"+(flickable.currentIndex==0 ? "on" : "off") + ".png" }
         Image { id: clockdot2; scale: uiScale/4; smooth: true; source: "../../images/dockmode/time/indicator/"+(flickable.currentIndex==1 ? "on" : "off") + ".png" }
         Image { id: clockdot3; scale: uiScale/4; smooth: true; source: "../../images/dockmode/time/indicator/"+(flickable.currentIndex==2 ? "on" : "off") + ".png" }
    }
}



