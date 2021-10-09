rivaWebCamCreate = function(wdgId, x, y, cx, cy, parms, extType)
{
    console.log
    (
        "rivaWebCamCreate - wdgId: " + wdgId +
        ", area: " + x + "/" + y + "/" + cx + "/" + cy +
        ", parms: " + parms +
        ", extType: " + extType
    );

    var newCam = document.createElement('iframe');
    if (newCam) {

        newCam.id = wdgId;

        newCam.style.left = x.toString() + "px";
        newCam.style.top = y.toString() + "px";
        newCam.style.width = cx.toString() + "px";
        newCam.style.height = cy.toString() + "px";
        newCam.style.visibility = "visible";
        newCam.style.position = "relative";

        newCam.style.ZIndex = "1000";
        newCam["autoplay"] = true;
    }
    return newCam;
}


rivaSetWebCamURL = function(wdgId, tarURL, params, extType) {
    console.log
    (
        "rivaSetWebCamURL - tarURL: " + tarURL +
        ", wdgId: " + wdgId  +
        ", params: " + params +
        ", extType: " + extType
    );

    // Get parameters out like this
    var p1 = params["Key1"];

    var tarElem = document.getElementById(wdgId);
    tarElem.setAttribute("src", tarURL);
}


// Only called if you don't create your own above
rivaWebCamSetup = function(wdgId, params, extType) {
    console.log
    (
        "rivaWebCamSet - wdgId: " + wdgId  +
        ", params: " + params +
        ", extType: " + extType
    );
}


rivaWebCamTeardown = function(wdgId, extType) {
    console.log("TearDown() - wdgId: " + wdgId + ", extType: " + extType);
}


