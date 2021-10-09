//
//  The callbacks below allow you to customize WebRIVA's use of web camera widgets.
//
//  rivaWebCamCreate() is called first. If you want to create the element yourself,
//  you can do so here. Else return null and a standard video element will be created.
//
//  If the web cam widget has an initially defined URL, or as soon as one gets set,
//  you will get a call to rivaSetWebCamURL to set the URL for the video stream.
//
//  On teardown, clean up the connection so that the video element can be destroyed
//  cleanly.
//
//  The Params value is a raw JSON object that has has key : value entries for
//  each parameter configured on the web cam widget. Access them as:
//
//      p1 = params["somekey"];
//

rivaWebCamCreate = function(wdgId, x, y, cx, cy, parms, extType)
{
    console.log
    (
        "rivaWebCamCreate - wdgId: " + wdgId +
        ", area: " + x + "/" + y + "/" + cx + "/" + cy +
        ", parms: " + parms +
        ", extType: " + extType
    );
    return null;
}


rivaSetWebCamURL = function(wdgId, tarURL, params, extType) {
    console.log
    (
        "rivaSetWebCamURL - tarURL: " + tarURL +
        ", wdgId: " + wdgId  +
        ", params: " + params +
        ", extType: " + extType
    );
}


// Only called if you don't create your own above
rivaWebCamSetup = function(wdgId, params, extType) {
    console.log
    (
        "rivaWebCamSetup - wdgId: " + wdgId  +
        ", params: " + params +
        ", extType: " + extType
    );
}


rivaWebCamTeardown = function(wdgId, extType) {
    console.log("TearDown() - wdgId: " + wdgId + ", extType: " + extType);
}
