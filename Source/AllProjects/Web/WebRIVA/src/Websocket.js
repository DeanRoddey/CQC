//
//  This class is run as a web worker. It keeps us connected and handles passing
//  us incoming messages and accepting messages to send. Since it has to be loaded
//  separately via the worker mechanism, we just make it straight javascript and
//  don't import anything.
//

// The URL we will connect to, which is sent to us in the Start
var tarURL = "";

// Our web socket that we use to communicate with the CQC web server
var ourSock = null;

// A timer we use to continue to try to auto-reconnect if we lose it
var reconnTimer = 0;

//
//  And this is the actual reconnection time, which is a minimum plus any offset
//  above. Minimim is five seconds, so this will be kept if no reconnect spread
//  is provided. We also have another for when we are in the background, which is
//  not randomized because all we do is connect and don't draw anything until we
//  become visible again.
//
var reconnTime = 5000;
var reconnTimeBgn = 15000;

// We save our context for use within web socket callbacks and such
var ourContext = this;

// The main class keeps up to date with the debug mode set by the user
var debugMode = false;

// The main app tells us when we between fgn/bgn modes
var inBgnTab = false;


// This is where we get posted messages from the main app
addEventListener("message", function(event) {

    switch(event.data.type)
    {
        case "connect" :
            //
            //  Save the URL, and do an initial connection attempt. That will kick
            //  start the process. We also get the reconnection time in seconds, so
            //  calculate a random time for our socket, adjusted up to millis.
            //
            ourContext.tarURL = event.data.data.url;
            var reconnOffset = 1000 * (Math.floor(Math.random() * event.data.data.spread) + 1);
            ourContext.reconnTime += reconnOffset;

            if (ourContext.debugMode) {
                console.log
                (
                    "Reconnect spread: " + event.data.data.spread +
                    ", fgn seconds: " + (ourContext.reconnTime / 1000) +
                    ", bgn seconds: " + (ourContext.reconnTimeBgn / 1000)
                );
            }


            //
            //  We shouldn't have a socket here, but if so kill it and try again with
            //  this URL.
            //
            if (ourContext.ourSock) {
                ourContext.ourSock.Close();
                ourContext.ourSock = null;
            }
            ourContext.connectToSrv();
            break;

        case "shutdown" :
            if (ourContext.debugMode)
                console.log("Received socket shutdown command");

            // Stop the reconnection timer if it's running
            if (ourContext.reconnTimer !== 0) {
                clearInterval(ourContext.reconnTimer);
                ourContext.reconnTimer = 0;
            }

            // Clean up our socket if we have it
            if (ourContext.ourSock) {
                ourContext.ourSock.close();
                ourContext.ourSock = null;
            }
            break;

        case "send" :
            // Just send the passed message if our socket is ready
            if (!ourContext.ourSock || (ourContext.ourSock.readyState != 1)) {
                if (ourContext.debugMode)
                    console.log("Not connected, can't send socket message");
            } else {

                // Try to send it
                try {
                    ourContext.ourSock.send(event.data.data);
                }

                catch(error) {
                    if (ourContext.debugMode)
                        console.error("WS send error: " + error);
                }
            }
            break;

        case "setbgntab" :
            ourContext.inBgnTab = event.data.data;
            break;

        case "setdebug" :
            if (ourContext.debugMode !== event.data.data)
                ourContext.debugMode = event.data.data;
            break;

        default :
            if (ourContext.debugMode)
                console.error("Unknown web socket worker message");
            break;
    };
});


// ----------------------------------------
// These are all just local helpers to keep the stuff above cleaner
// ----------------------------------------

//
//  This is called to get us to try to connect. We set up handlers and
//  react to success or failure.
//
function connectToSrv() {

    // Shouldn't get called here if we have a socket
    if (ourContext.ourSock !== null) {
        if (ourContext.debugMode)
            console.error("The websocket is already connected");

        // Just to keep things happy anyway, force it down
        if (ourContext.ourSock.readyState == 1)
            ourContext.ourSock.close();
        ourContext.ourSock = null;
    }

    //Try to create the new websocket and set up our internal handlers
    try {
        ourContext.ourSock = new WebSocket(ourContext.tarURL);

        // We just post the message to the application to handle
        ourContext.ourSock.addEventListener("message", function(evt)  {
            postMessage({ type : "msg", data : evt.data }, [] );
        });

        // We release our socket and tell the application we disconnected
        ourContext.ourSock.addEventListener("close", function(evt) {

            // If not all the way closed yet, then see if we need to do anything
            if (ourContext.ourSock) {

                if (ourContext.ourSock.readyState < 3) {

                    // It's closing but not closed yet
                    if (ourContext.debugMode)
                        console.log("Socket connection is closing");

                } else {

                    // It's now closed so we can clean up, so drop the socket reference
                    if (ourContext.debugMode)
                        console.log("Socket connection is closed");

                    ourContext.ourSock = null;

                    // Tell the main app that are disconnected
                    ourContext.postMessage({ type : "disconnected", data : "" }, []);

                    //
                    //  Restart the timer to use for reconnection, at an appropriate rate for
                    //  fgn/bgn state.
                    //
                    setReconnTimer();
                }
            }
        });

        ourContext.ourSock.addEventListener("open", function(evt) {

            // Once we get fully connected
            if (ourContext.ourSock.readyState === 0) {

                if (ourContext.debugMode)
                    console.log("Socket connection process has begun");

            } else if (ourContext.ourSock.readyState === 1) {

                if (ourContext.debugMode)
                    console.log("Socket connection has completed, telling app");

                // Tell the main application about it
                ourContext.postMessage({ type : "connected", data : "" }, []);

                // Reset the timer to use as a ping timer, every 30 seconds
                if (ourContext.reconnTimer !== 0)
                    clearInterval(ourContext.reconnTimer);
                ourContext.reconnTimer = setInterval(ourContext.reconnHandler, 30000);
            }
        });
    }

    catch(err) {
        if (ourContext.debugMode)
            console.error("Exception in WS handler: " + err);

        // Make sure our socket gets dropped if it got created
        if (ourContext.ourSock) {
            ourContext.ourSock = null;
            ourContext.lastConnState = false;
        }
    }
}


//
//  This is our timer handler. We use it for two things. If we are connected we
//  use it to send pings. If we are not connected, we use it to try to reconnect.
//  We don't actually send the ping, becasue we don't want to bring in any
//  Typescript files. So we just send the application a message to do it.
//
function reconnHandler() {
    if (ourContext.ourSock === null) {
        // The socket is null so we are trying to connect
        ourContext.connectToSrv();
    } else {
        // Else we are pinging, if the socket is fully connected
        if (ourContext.ourSock.readyState == 1)
            ourContext.postMessage({ type : "ping", data : "" }, []);
    }
}


//
//  Sets up our reconnection timer at an appropriate rate for our bgn/fgn state.
//
function setReconnTimer(evt) {

    if (ourContext.reconnTimer !== 0)
        clearInterval(ourContext.reconnTimer);

    // Decide the time to use, which is longer if in the background
    var connSecs = ourContext.reconnTime;
    if (ourContext.inBgnTab)
        connSecs = ourContext.reconnTimeBgn;

    ourContext.reconnTimer = setInterval(ourContext.reconnHandler, connSecs);
}
