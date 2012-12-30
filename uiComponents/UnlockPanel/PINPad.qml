import Qt 4.7

Item {
    id: pinPad;
    property real uiScale: 1.0

    width: 320 * uiScale
    height: gridDividers.height * uiScale

    Image {
        id: gridDividers
        source: "/usr/palm/sysmgr/images/pin/pin-grid.png"
        transform: Scale { origin.x: 0; origin.y: 0; xScale: uiScale; yScale: uiScale;}
        smooth: true;
        property int topOffset: 4 * uiScale
        property int bottomOffset: 6 * uiScale
    }

    Grid {
        id: buttonGrid
        width: gridDividers.width * uiScale
        height: (gridDividers.height * uiScale) - gridDividers.topOffset - gridDividers.bottomOffset
        y: gridDividers.topOffset
        x:0
        columns: 3
        rows: 4
        spacing: 0


        PINButton {caption: "1"; width: buttonGrid.width/buttonGrid.columns; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        PINButton {caption: "2"; width: buttonGrid.width/buttonGrid.columns+1; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        PINButton {caption: "3"; width: buttonGrid.width/buttonGrid.columns+1; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        PINButton {caption: "4"; width: buttonGrid.width/buttonGrid.columns; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        PINButton {caption: "5"; width: buttonGrid.width/buttonGrid.columns+1; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        PINButton {caption: "6"; width: buttonGrid.width/buttonGrid.columns+1; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        PINButton {caption: "7"; width: buttonGrid.width/buttonGrid.columns; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        PINButton {caption: "8"; width: buttonGrid.width/buttonGrid.columns+1; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        PINButton {caption: "9"; width: buttonGrid.width/buttonGrid.columns+1; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        Rectangle {color: "transparent"; width: buttonGrid.width/buttonGrid.columns; height:buttonGrid.height/buttonGrid.rows;}
        PINButton {caption: "0"; width: buttonGrid.width/buttonGrid.columns+1; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
        PINButton {imgSource: "/usr/palm/sysmgr/images/pin/icon-delete.png"; caption: "\b"; width: buttonGrid.width/buttonGrid.columns+1; height:buttonGrid.height/buttonGrid.rows; uiScale: pinPad.uiScale; onAction: keyAction(text);}
    }

    signal keyAction(string keyText);
}
