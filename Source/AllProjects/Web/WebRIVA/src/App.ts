import * as RIVAProto from "./RIVAProto";
import * as BaseTypes from "./BaseTypes";
import * as ImgCache from "./ImgCache";
import * as GraphDraw from "./GraphDraw";

//
//  Our app is always one of these states:
//
//  Connecting - Waiting to get the web socket connected. In this case we put up a
//  transparent div that covers the whole screen and shows that we are trying to
//  connect. We use a web worker to manage the socket which posts us status messages.
//  When we see a connect we go to:
//
//  -----------
//  Init - Once we have connected, then we move to Init state. Here we basically
//  wait for a login result message. It will either indicate an error in which case we
//  display that error to the user and stop. Or it will indicate success and provide
//  us some info. Any errors at this point are fatal so we just give up and move to
//  failed state.
//
//  If successful, we move to WaitTmpl state.
//
//  -----------
//  WaitTmpl = Nowe need to wait for the initial template to be loaded. It will be the
//  default one configured for the user we logged in under. We need this to set up our
//  drawing surface. We will get one even if the template failed to load. The server
//  will send us a dummy one, so that we can be satisfied, and then it send us a msg
//  to show to the user. The template will be empty so it won't do anything. This tells
//  us how big the base template is, so we can size our canvas accordingly.
//
//  Once we get this, we move to Ready state and we get rid of the 'connecting' DIV.
//  And we should start seeing graphics commands for the new template.
//
//  ----------
//  Ready - At this point, we are in normal state where the user can interact with us.
//  The server will start sending us graphics commands, we'll start sending him user
//  input commands and periodic pings.
//
//
//
//  It is retardedly complicated to do what we need to do because of the totally async
//  nature of javascript. Even decoding an image that we already have is done async.
//  And we don't want to process graphics commands until we have the images required.
//
//  When we get a StartDraw, we start caching graphics commands until we get an end
//  draw and/or all sent images have been added to the image cache.
//
//  If we get any image data, because we don't have a required image in our image cache
//  or it's been changed since what we have, we will continue to save up graphics
//  commands, but we will start processing that image. Since the loading of even the
//  image data is async, while doing that more graphics commands and even more image
//  msgs can come in. We will queue them all up.
//
//  When we get the completion event of the current image, we see if we have another
//  image to load. If so, we start that one loading.
//
//  We will start drawing either when:
//
//      1. We get the last EndDraw and there are no images left to load
//      2. We get the completion event on the last image in the queue and we have
//         already seen the EndDraw for the last StartDraw.
//
//  At that point, we start processing  graphics commands in a loop. Since that is
//  synchronous we can finally just drain the graphics cache and not worry about having
//  to deal with anything else.
//
//  It is possible that another StartDraw begins while we are processing images, and we
//  don't want to assume that they will never be nested. So we have a startDraw counter.
//  If that counter is zero when we see an EndDraw, then we have seen a final one and
//  we know that we can start processing the graphics cache (as soon as any images
//  complete.) If it's not zero, we just bump up the counter and keep going. Eventually
//  we hit one of the two conditions above and start drawing.
//
//
//  We only have one persistent visual component, well two. We have a DIV that fills the
//  body, and in which we center the canvas on which we draw. The other two are transient,
//  the connecting DIV and our options menu. These are placed absolute at 0,0 and overlap
//  our content DIV while they are present. We use HTML templates to create them and then
//  destroy them when we are done. So most of the time just the canvas and its containing
//  DIV are present.
//
//
//  We handle both mouse and touch input. We funnel both to a common handlers which
//  sends messages to the server as appropriate. The server only cares about single
//  point press, release, and move. But we also need to be able to invoke a menu, so
//  the mouse and touch specific handlers will watch for those scenarios and invoke
//  the menu DIV. Sometimes we don't know it's a menu operation until after we have had
//  to send a press to the server, so we may have to send the server a cancel gesture
//  message if it later turns out to be something else, such as the menu invocation, or
//  just some random thing we don't handle.
//
enum EAppStates {
    Connecting,
    Init,
    WaitTmpl,
    Ready,
    Failed
}


//
//  A simple class we use to track download info for images. We have to queue up
//  images to download and process them until we have them all. Once each one is
//  fully received (it may take multiple msgs), we can add it to the image cache
//  and set the image data on it. That is also async, so the image cache sends us
//  an event to let us know its done. At which point we start another if it is
//  available.
//
class ImgQItem {
    // The image data may only be the initial chunk
    constructor(public path        : string,
                public serNum      : number,
                public imgRes      : BaseTypes.RIVASize,
                public imgData     : string,
                public isPNG       : boolean) {
        this.isPNG = isPNG;
        this.path = path;
        this.serNum = serNum;
        this.imgRes = imgRes;
        this.imgData = imgData
    }
}


// A simple class to track touch input points
class TouchPoint {
    constructor(xPos : number,
                yPos : number,
                id : number) {

        this.completed = false;
        this.id = id;
        this.startAt = new BaseTypes.RIVAPoint(null);
        this.startAt.set(xPos, yPos);
    }

    // Returns true if complete and the start and end points are within a few pixels
    withinMoveLimit() : boolean {
        // If not completed say no
        if (this.completed === false)
                return false;

        return (Math.abs(this.startAt.x - this.endAt.x) < 5) &&
                (Math.abs(this.startAt.y - this.endAt.y) < 5)
    }

    update(xPos : number, yPos : number, end : boolean) {
        this.endAt = new BaseTypes.RIVAPoint(null);
        this.endAt.set(xPos, yPos);
        this.completed = end;
    }

    completed : boolean;
    endAt : BaseTypes.RIVAPoint;
    startAt : BaseTypes.RIVAPoint;
    id : number;
}


//
//  We have some external scripts for supporting user extensions. So we have
//  to declare these. If not used, they will be default, do-nothing implementations.
//
declare function rivaWebCamCreate
(
    wdgId : string,
    x : number,
    y : number,
    cx : number,
    cy : number,
    parms : string[],
    extType : string
);
declare function rivaSetWebCamURL
(
    tarURL : string, wdgId : string, params : string[], extType : string
);
declare function rivaWebCamDestroy(wdgId : string, extType : string);
declare function rivaWebCamSetup(wdgId : string, params : string[], extType : string);
declare function rivaDoRIVACmd(pathCmd : string, p1 : string, p2 : string, p3 : string);



//
//  This is our main application which does the bulk of the work. It's fairly hefty though
//  we offload a good bit of it to the image cache and web socket classes.
//
export class WebRIVAApp {

    //
    //  Being in the background can cause us issues. CPU is heavy throttled on some
    //  systems when in the bacgkround. So, we try to sense when this happens. This
    //  flag is set, and we send a msg to the web worker. When in this state we will
    //  not get drawing commands. We just will exchange pings to keep the connection
    //  alive.
    //
    //  So when we sense background, we send a msg to the server to tell him we are
    //  in the bgn and we tell the websocket web worker to go into background mode.
    //
    //  When we sense back to the front, we tell the server about that, and we tell
    //  the websocket web worker to go back to normal mode. The server will force
    //  a full redraw to get us up to date.
    //
    //  We set a flag to remember our bgn/fgn state so that, worst case, if we don't
    //  get notified of coming back to the foreground, our user input handler can
    //  watch this flag and force us out. Obviously if the user is interacting we are
    //  not in the bgn.
    //
    //  Defaults to false in case we don't have visibility support, so we don't end
    //  up permanently throttled.
    //
    private inBgnTab : boolean = false;


    //  Some settable flags that affect us locally
    private logGraphCmds : boolean = false;
    private logImgProc : boolean = false;
    private logMisc : boolean = false;
    private logTouch : boolean = false;
    private logWebSock : boolean = false;
    private fullScreenMode : boolean = false;
    private reconnSpread : number = 0;

    //
    //  This one is just for debugging and let's us account for differences between the
    //  development setup and the deployed setup. It's set via &debugmode=1
    //
    private debugMode : boolean = false;

    //
    //  And some that affect the server, so these must be sent to the server
    //  when they are changed from the default.
    //
    private logSrvMsgs : boolean = false;
    private noCache : boolean = false;
    private logGUIEvents : boolean = false;

    //
    //  They can indicate on the URL that they want us to log to the server. WE don't do
    //  the graphics commands because there are a lot of messages, but the other stuff
    //  we do.
    //
    private logToServer : boolean = false;

    //
    //  This is our current application state. It's the fundamental driver of what we do
    //  and how we react to messages. See the enum comments at the top of this file. We
    //  start in connecting mode.
    //
    private curState : EAppStates;

    //
    //  The URL we will use to connect to the server. It's a well known URL,
    //  we just need to get the host:port part of the invocation URL and build
    //  it up.
    //
    private tarWSURL : string;

    //
    //  Our web worker that keeps the socket going even if we are not the active
    //  tab.
    //
    private ourSock : Worker;

    // The port we will invoke the websocket connection on
    private wssPort : number;

    // The ping message we send is always the same, so we pre-build it
    private pingMsg : string;

    //
    //  We store the user name and password for when we need to reconnect. These come
    //  from the original URL.
    //
    private userName : string;
    private password : string;

    //
    //  We don't process graphics commands until we get the EndDraw to indicate
    //  we have all the commands for a current redraw, and after we have processed
    //  any image loading operations that are pending.
    //
    private graphQ : Array<Object>;


    //
    //  This is our image cache object. It contains a list of image cache items, each
    //  of which represents an image we got from the server.
    //
    private imgCache : ImgCache.ImgCache;

    //
    //  We also queue up image messages, so that we can load them up into images
    //  in our image cache. We also need to keep a ref to the one we are currently
    //  loading, and the one we are currently storing away if it's a multi-block
    //  image msg.
    //
    //  Note that we also push images into the queue if they are found in the
    //  persistent cache. They still need to be loaded up into images just as though
    //  they were downloaded.
    //
    private imgQ : Array<ImgQItem>;
    private curImageDownloading : ImgQItem = null;
    private curImageLoading : ImgQItem = null;

    //
    //  We have an issue that we build up our 'connecting' screen from a template. It
    //  contains an image element that shows our logo. But, if we lose connection, and
    //  put the connecting screen again, it will try to load the logo from the server
    //  again, but it won't because we lost the server. So, the first time we connect,
    //  we will grab the image data and on subsequent connections we'll manually set
    //  the image data.
    //
    private logoImgData : string = "";

    //
    //  A counter that we use to track how many StartDraw messages we have backed up.
    //  We can get another start while images are still downloading from the one
    //  before. So we don't want to start processing graphics commands upon completion
    //  of the last image if we have not seen the final end draw.
    //
    private startDrawCnt : number = 0;

    // An instance of our class that wraps the canvas and handles drawing commands
    private tarDraw : GraphDraw.GraphDraw = null;

    // We still need to access the canvas ourself for non-drawing type things
    private ourCanvas : HTMLCanvasElement = null;

    // The DIV that contains the canvas and any remote widgets
    private tmplDIV : HTMLDivElement = null;

    // The main DIV that stuff goes into
    private mainDIV : HTMLDivElement = null;

    //
    //  This is initialized to 1.0, and is the initial viewport scaling that we set in the
    //  HTML. It can be set via URL to something else, and we'll update the viewport meta
    //  tag to reflect that.
    //
    private viewportScale : number;

    //
    //  We remember if we are in a press state, so that we can discard bogus releases,
    //  and so we can only send move events when the user is actually tracking something,
    //  which is a huge efficiency win.
    //
    private inPressedState : Boolean;

    //  When doing mouse input, we remember when/where we were pressed. If they hold it
    //  for over three seconds and release without moving, we treat that as a menu
    //  invocation.
    //
    private mousePressedAt : BaseTypes.RIVAPoint;
    private mousePressedTime : number;

    //
    //  We can only send move/drag movements so quickly or we'll just overwhelm the
    //  server. So  we both use the lastMousePos value to limit movement to every X
    //  pixels, and we use pendingMousePos and a timer to send them out no quicker
    //  than a certain rate.
    //
    //  Press/release we do immediately, but moves we throttle. So we update
    //  lastMousePos every time we send one, or on each press/release. When we get
    //  a move, we store that in pendingMousePos. When the timer kicks in, if
    //  they are not the same, it sends the pending and sets it as the last.
    //
    //  We only do anything if we are in ready state.
    //
    private lastMousePos : BaseTypes.RIVAPoint;
    private pendingMousePos : BaseTypes.RIVAPoint;
    private inputTimer : number;

    //
    //  When touch input is being used, we have to deal with multiple touches. We
    //  have two scenarios.
    //
    //  1. One touch only, we send the usual press, drag, and release stuff. If another
    //     touch shows up, we tell the server to cancel the gesture.
    //  2. Two touches where are pressed and released without moving. This is used to
    //     invoke the menu in touch mode. If either moves more than a few pixels,
    //     we don't do anything. That includes if the first one has moved by the time
    //     the second one shows up.
    //
    //  So we may see a situation where we see the first touch, we send a press to the
    //  server, then we see a second and we just have to cancel it.
    //
    private touchPnts : Array<TouchPoint>;

    //
    //  This is passed through to the extension files to allow for support of multiple
    //  extension schemes. It's set via &exttype=xxx. The xxx part we store here, it
    //  will be empty if not specificied.
    //
    private extType : string = "";


    // ------------------------------------------------------------
    //  Our constructor, which does all of the setup, to get the incoming user
    //  provided values and set up the target websocket URL and to get the socket
    //  object created and then kick off the process by trying a connect.
    // ------------------------------------------------------------
    constructor() {

        // If we were  invoked via HTTPS default to WSS (and vice versa)
        var doSecure : boolean = (window.location.protocol === "https:");

        // But they can override
        var secureParm : string = this.getURLQPByName("dosecure", window.location.search);
        if (secureParm === "Yes")
            doSecure = true;

        //
        //  Set a default port. We will start with the port that they used to
        //  invoke the initial HTTP request. That's almost always going to
        //  be fight. Then we see if there is an override. The info about
        //  the port might not be available, so we start with the default
        //  port for the protocol.
        //
        this.wssPort = doSecure ? 443 : 80;
        if (window.location.port)
            this.wssPort = Number(window.location.port);
        var portOver : string = this.getURLQPByName("tarport", window.location.search);
        if (portOver)
            this.wssPort = parseInt(portOver);

        // Store the incoming name and password
        this.userName = this.getURLQPByName("user", window.location.search);
        this.password = this.getURLQPByName("pw", window.location.search);

        //
        //  See if they want to enable debugging via the URL. If so, set those flags.
        //
        if (this.getURLQPByName("debugMode", window.location.search))
            this.debugMode = true;

        if (this.getURLQPByName("logimgproc", window.location.search))
            this.logImgProc = true;

        if (this.getURLQPByName("loggraphcmds", window.location.search))
            this.logGraphCmds = true;

        if (this.getURLQPByName("logmisc", window.location.search))
            this.logMisc = true;

        if (this.getURLQPByName("logtouch", window.location.search))
            this.logTouch = true;

        if (this.getURLQPByName("logwebsock", window.location.search))
            this.logWebSock = true;

        var spreadVal : string = this.getURLQPByName("reconnspread", window.location.search);
        if (spreadVal)
            this.reconnSpread = parseInt(spreadVal);

        // They can also ask us to log to the server
        if (this.getURLQPByName("logtosrv", window.location.search))
            this.logToServer = true;


        // Store the extension type if any
        var extTypeCheck = this.getURLQPByName("exttype", window.location.search);
        if (extTypeCheck !== null)
            this.extType = extTypeCheck;


        // OK now set up the URL that we'll use to make the Websocket connection
        this.tarWSURL = (doSecure ? "wss://" : "ws://")
                      + window.location.host.split(":")[0]
                      + ":"
                      + this.wssPort
                      + "/Websock/CQSL/WebRIVA.html?";

        // Add the user name and password to it
        this.tarWSURL += "user=" + this.userName + "&pw=" + this.password;

        // They can provide a session name, which we'll pass on to the server
        var sessName : string = this.getURLQPByName("sessname", window.location.search);
        if (sessName)
            this.tarWSURL += "&sessname=" + sessName;

        // They can provide environmental variables for this session as well
        for (var envNum : number = 1; envNum <= 9; envNum++) {
            var envVal : string = this.getURLQPByName("env" + envNum, window.location.search);
            if (envVal)
                this.tarWSURL += "&env" + envNum + "=" + envVal;
        }

        // They can override our default viewport scaling
        this.viewportScale = 1.0;
        var vpScale = this.getURLQPByName("vpscale", window.location.search);
        if (vpScale)
            this.viewportScale = Number(vpScale) || 1.0;

        // Create our graphics command queue
        this.graphQ = new Array<Object>();

        // And our image loading queue
        this.imgQ  = new Array<ImgQItem>();
        this.curImageDownloading = null;
        this.curImageLoading = null;

        //
        //  Create our image cache, and load up any images that we currently have
        //  in the cache.
        //
        this.imgCache = new ImgCache.ImgCache();

        //
        //  Create our web worker that manages the web socket connection. We don't
        //  start it doing anything yet, we just laod it.
        //
        this.ourSock = new Worker("./src/Websocket.js");

        // Make sure we have initial state set to the correct initial state
        this.curState = EAppStates.Connecting;

        // Register our handler for image load complete events
        window.addEventListener(ImgCache.ImgCacheItem.CompleteEvName, this.imgLoadComplete);

        // Set our unload event handler
        // window.onload = this.loadHandler;
        window.onunload = this.unloadHandler;

        // Look up some elements we access a lot
        this.ourCanvas = <HTMLCanvasElement>document.getElementById("TemplateCont");
        this.tmplDIV = <HTMLDivElement>document.getElementById("TemplateDIV");
        this.mainDIV = <HTMLDivElement>document.getElementById("MainDIV");

        //
        //  Create some values we use to throttle input move/drag events sent to the server.
        //  Then set up our mouse and touch handlers. We handle both, assuming only one will
        //  be used at any given instant.
        //
        //  We have to catch mouse release events at the window level, because there's
        //  pretty much no way for an element to know if a click inside him is ever released
        //  if the user moves the mouse outside of the element. It just calls our own mouse
        //  handler same as for the canvas ones.
        //
        //  The server will ignore any ups that aren't matched to a down, so a click outside
        //  the canvas won't hurt anything.
        //
        this.lastMousePos = new BaseTypes.RIVAPoint(null);
        this.pendingMousePos = new BaseTypes.RIVAPoint(null);
        this.touchPnts = new Array<TouchPoint>();
        this.mousePressedAt = new BaseTypes.RIVAPoint(null);

        this.ourCanvas.onmousedown = this.canvasMouseHandler;
        this.ourCanvas.onmousemove = this.canvasMouseHandler;
        window.onmouseup = this.canvasMouseHandler;

        this.ourCanvas.ontouchcancel = this.canvasTouchHandler;
        this.ourCanvas.ontouchend = this.canvasTouchHandler;
        this.ourCanvas.ontouchstart = this.canvasTouchHandler;
        this.ourCanvas.ontouchmove = this.canvasTouchHandler;

        // Start a timer that we use to send deferred user input move/drag events
        this.inputTimer = setInterval(this.dragTimerHandler, 100);

        // Set these to defaults that will get overridden below if needed
        this.noCache = false;
        this.logSrvMsgs = false;
        this.logGUIEvents = false;

        //
        //  They can force them via URL values if they want. Any that aren't set to
        //  the known defaults will be sent to the server upon connection.
        //
        this.logGUIEvents = this.getURLQPByName("logguievents", window.location.search) !== null;
        this.logSrvMsgs = this.getURLQPByName("logsrvmsgs", window.location.search) !== null;
        this.noCache = this.getURLQPByName("nocache", window.location.search) !== null;

        // Set us up a handler for messages from the web worker
        this.ourSock.addEventListener("message", this.workerMsgHandler);

        // The ping message we send is always the same, so pre-build it
        this.pingMsg = "{ \"" + RIVAProto.kAttr_OpCode + "\" : \"" +
                            RIVAProto.OpCodes.Ping.toString() + "\" }";


        this.forceVPScale();

        // Let's initially show our login popup
        this.showLoginPopup();

        //
        //  And now start up the web worker that manages the socket. He'll start connecting
        //  now and send us messages to keep us aware of the connection status. We also
        //  give it our initial debug state.
        //
        this.ourSock.postMessage({ type : "setdebug", data : this.logWebSock }, []);
        this.ourSock.postMessage
        (
            {
                type : "connect" , data : { url : this.tarWSURL, spread : this.reconnSpread }
            }, []
        );

        // On a screen resize (mostly rotation) update our viewport scaling
        window.onresize = (evt) => {
            this.forceVPScale();
        }

        // See the comments for inBgnTab above
        this.setVisHandler();
    }

    // Handle our loading by setting up the viewport initially
    private loadHandler = () => {
        this.forceVPScale();
    }

    // Handle our unloading by shutting down the web socket
    private unloadHandler = () => {
        if (this.ourSock)
            this.ourSock.terminate();
    }



    // ------------------------------------------------------------
    //  This is our handler for the web worker that manages our socket connection,
    //  so all notifications related to our web socket come here, and some helpers
    //  that we call.
    // ------------------------------------------------------------
    workerMsgHandler = (msg : MessageEvent) => {
        switch(msg.data.type) {

            case "connected" :
                this.wsConnect();
                break;

            case "disconnected" :
                this.wsClose();
                break;

            case "msg" :
                // Pass it the text of the message
                this.wsMsg(msg.data.data);
                break;

            case "ping" :
                this.sendPing();
                break;

            default :
                break;
        }
    }

    wsConnect() {

        if (this.logWebSock)
            console.log("Connected to server");

        // Go to init state to wait for a login result
        this.curState = EAppStates.Init;

        // Reset everything else that is per-connection
        this.startDrawCnt = 0;
        this.inPressedState = false;
        this.tarDraw = new GraphDraw.GraphDraw(this.logGraphCmds);
        this.graphQ.length = 0;
        this.imgQ.length = 0;
        this.curImageDownloading = null;
        this.curImageLoading = null;

        //
        //  Build and send an image map message, which lets the server know what
        //  images, at what resolutions and with what serial numbers we already
        //  have. This will be empty on an initial connection, but after that, it can
        //  significantly reduce reconnect time since we'll already have all of
        //  the images.
        //
        //  The server won't start sending drawing commands until he get this
        //  msg. And we send our background tab status with this msg, so if we are
        //  in the bgn tab upon connection, we won't get any drawing until they
        //  bring us forward.
        //
        //  We also pass in the initial server flags so that they get there in time
        //  before content starts loading. We piggy back the current background tab
        //  state into the flags as well, to avoid another parameter.
        //
        var stateMsg : Object = new Object();
        stateMsg[RIVAProto.kAttr_OpCode] = RIVAProto.OpCodes.SessionState;
        this.imgCache.buildImgMapMsg(stateMsg);

        var srvFlags : number = 0;
        if (this.logGUIEvents)
            srvFlags |= RIVAProto.kSrvFlag_LogGUIEvents;
        if (this.logSrvMsgs)
            srvFlags |= RIVAProto.kSrvFlag_LogSrvMsgs;
        if (this.noCache)
            srvFlags |= RIVAProto.kSrvFlag_NoCache;
        if (this.inBgnTab)
            srvFlags |= RIVAProto.kSrvFlag_InBgnTab;
        stateMsg[RIVAProto.kAttr_ToSet] = srvFlags;
        stateMsg[RIVAProto.kAttr_Mask] = RIVAProto.kSrvFlags_AllBits;

        this.sendMsg(JSON.stringify(stateMsg));

        // Let the socket worker know our current bgn/fgn state
        this.ourSock.postMessage({ type : "setbgntab", data : this.inBgnTab }, []);
    }

    wsClose() {

        //
        //  If we lost connection, we may have web camera or web widget elements
        //  that will now be orphaned. So we need to delete all children of the
        //  template DIV object that aren't our canvas.
        //
        var childElems : HTMLCollection = this.tmplDIV.children;
        for (var index = 0; index < childElems.length; index++) {
            if (childElems[index].nodeName.toLowerCase() == "video") {
                try {
                    rivaWebCamDestroy(childElems[index].id, this.extType);
                }

                catch(e) {
                    this.logErr(e.toString());
                }
            }
        }

        // Go into connecting mode if not already
        if (this.curState !== EAppStates.Connecting) {
            this.curState = EAppStates.Connecting;

            if (this.logWebSock)
                console.log("Lost connection to server. Trying to reconnect...");

            // Show the connecting popup
            this.showLoginPopup();
        }
    }

    //
    //  This is called when we get a msg from the server. We look at the msg type and
    //  figure out how to process it. It's just a JSON object flattened to a string.
    //
    //  We either process the message or we get an error because the message is invalid
    //  or it isn't correct for our current state or it can't be processed for some
    //  state specific reason. Depending on the error, we may recycle the connection.
    //
    wsMsg(msgText : string) {

        // Parse the message
        var jsonData : Object = JSON.parse(msgText);

        // Get the opcode number out and convert to the enum value
        var opOrdinal : number = jsonData[RIVAProto.kAttr_OpCode];
        var opCode : RIVAProto.OpCodes = <RIVAProto.OpCodes>opOrdinal;

        //
        //  Check for graphics commands first, which are all below a specific value,
        //  then we can check for other specific msgs we handle.
        //
        if (opCode < RIVAProto.OpCodes.LastGraphics) {
            //
            //  It's a graphics command. We have to be in ready state or something
            //  is wrong. We just queue them up.
            //
            if (this.curState === EAppStates.Ready) {
                this.graphQ.push(jsonData);
            } else {
                // Log this? Could be a lot of them if something went wrong
            }

        } else if (opCode === RIVAProto.OpCodes.CreateRemWidget) {

            this.createRemWidget(jsonData);

        } else if (opCode === RIVAProto.OpCodes.DestroyRemWidget) {

            this.destroyRemWidget(jsonData);

        } else if (opCode === RIVAProto.OpCodes.ExitViewer) {

            // Doesn't appear to be any way to do this anymore

        } else if (opCode === RIVAProto.OpCodes.SetRemWidgetURL) {

            this.setRemWidgetURL(jsonData);

        } else if (opCode === RIVAProto.OpCodes.StartDraw) {

            // Bump the start draw counter
            this.startDrawCnt++;

            // We add these to the queue as well
            this.graphQ.push(jsonData);

            if (this.logImgProc)
                this.logMsg("Got start draw");

                if (this.logImgProc) {
                    if ((this.curImageDownloading !== null) ||
                        (this.curImageLoading !== null) ||
                        (this.imgQ.length != 0)) {
                            this.logMsg("Still have images loading from previous cycle");
                    }
                }

        } else if (opCode === RIVAProto.OpCodes.EndDraw) {

            // We add these to the queue as well
            this.graphQ.push(jsonData);

            // Decrement the start draw counter. Check for any underflow
            this.startDrawCnt--;
            if (this.startDrawCnt < 0) {
                this.startDrawCnt = 0;
                alert("Start Draw counter underflow");
            }

            // There should not be any image currently being downloaded now
            if ((this.curImageDownloading !== null) && (this.startDrawCnt === 0)) {
                this.logErr("No image should be downloading at end of final EndDraw");
            }

            //
            //  If there are any images that have not loaded, then do nothing, the
            //  finish of loading of the last outstanding image will start the
            //  processing.
            //
            if ((this.imgQ.length === 0) && (this.curImageLoading === null)) {
                if (this.logImgProc)
                    this.logMsg("Final end draw, no images, processing msgs");
                this.processGraphQ();
            } else {
                if (this.logImgProc)
                    this.logMsg("Got end draw but images outstanding");
            }

        } else if (opCode === RIVAProto.OpCodes.ImgDataFirst) {

            // We have to be in ready state
            if (this.curState === EAppStates.Ready) {
                this.startImgDownload(jsonData);
            } else {
                if (this.logImgProc)
                    this.logErr("Got image start when not in ready state");
            }

        } else if (opCode === RIVAProto.OpCodes.ImgDataNext) {

            // We have to be in ready state of this couldn't be valid
            if (this.curState === EAppStates.Ready) {
                this.nextImgBlock(jsonData);
            } else  {
                if (this.logImgProc)
                    this.logErr("Got image next  when not in ready state");
            }

        } else if (opCode === RIVAProto.OpCodes.LoginRes) {
            //
            //  We should be in Init state. If so, we move to WaitTmpl state.
            //
            if (this.curState === EAppStates.Init) {
                //
                //  If successful, move to wait template state. Else show the error
                //  and move to failed state. There are no recoverable errors at this
                //  point.
                //
                if (jsonData[RIVAProto.kAttr_Status]) {
                    this.curState = EAppStates.WaitTmpl;
                } else {
                    this.curState = EAppStates.Failed;
                    alert(jsonData[RIVAProto.kAttr_MsgText]);
                }

            } else {
                this.logErr("Got login results msg when not in login results mode");
            }

        } else if (opCode === RIVAProto.OpCodes.NewTemplate) {

            //
            //  This one we can see either in waittmpl state or ready state. In init
            //  state we are getting the initial default template for the user
            //  who we logged in as.
            //
            if ((this.curState === EAppStates.WaitTmpl) ||
                (this.curState === EAppStates.Ready)) {

                //
                //  If in init mode, we can move to ready mode now and show the canvas and
                //  hide the login layer. After thus th
                //
                if (this.curState === EAppStates.WaitTmpl) {
                    this.curState = EAppStates.Ready;

                    // Hide the connecting popup
                    this.destroyLoginPopup();
                }

                // And call our method that handles updating for a new base template
                this.handleNewTemplate(jsonData);

            } else {
            }

        } else if (opCode === RIVAProto.OpCodes.RIVACmd) {
            //
            //  If the path starts with our built in command prefix, we handle it. Else we
            //  pass it to the extension command.
            //
            var cmdPath : string = jsonData[RIVAProto.kAttr_Path];
            try {
                if (cmdPath.substring(0, RIVAProto.kIntRIVACmdPref.length) == RIVAProto.kIntRIVACmdPref) {
                    this.doRIVACmd
                    (
                        cmdPath,
                        jsonData[RIVAProto.kAttr_P1],
                        jsonData[RIVAProto.kAttr_P2],
                        jsonData[RIVAProto.kAttr_P3]
                    );
                } else {
                    rivaDoRIVACmd
                    (
                        cmdPath,
                        jsonData[RIVAProto.kAttr_P1],
                        jsonData[RIVAProto.kAttr_P2],
                        jsonData[RIVAProto.kAttr_P3]
                    );
                }
            }

            catch(e) {
                this.logErr(e.toString());
            }
        } else if (opCode === RIVAProto.OpCodes.SetTmplBorderClr) {

            // Set the color as the background on the DIV that contains our canvas
            this.mainDIV.style.backgroundColor = jsonData[RIVAProto.kAttr_ToSet];

            //
            //  The canvas has to be updated to make sure the upper/left boundary gets
            //  set to the same color as the background.
            //
            this.tarDraw.setLRBoundaryColor(jsonData[RIVAProto.kAttr_ToSet]);

        } else if (opCode === RIVAProto.OpCodes.ShowErrorMsg) {
            //
            //  Display the error message to the user. This doesn't necessarily
            //  mean we are doomed. It may be some action error. If it is a
            //  deadly error, we'll just get a subsequent close.
            //
        } else if (opCode === RIVAProto.OpCodes.ShowException) {
            //
            //  Display the exception info to the user.
            //
        } else {
            //
            //  It's one of the more general purpose messages, so handle them
            //  appropriately.
            //
            switch(opCode)
            {
                case RIVAProto.OpCodes.ShowMsg :

                    break;

                default :
                    break;
            };
        }
    }



    // ------------------------------------------------------------
    //  Private helpers
    // ------------------------------------------------------------

    //
    //  We force the viewport scaling since the user can override it.
    //
    forceVPScale() {

        if (this.viewportScale === 1.0)
            return;

        if (this.logMisc)
            this.logMsg("viewportScale=" + this.viewportScale);

        // See if we have added this yet, if not create it, else update it
        // width=device-width,
        var newVPContent = "initial-scale="
                            + this.viewportScale + ", minimal-ui, maximum-scale=4, minimum-scale=0.1";
        var viewportTag = document.querySelector('meta[name=viewport]');
        if (!viewportTag) {
            var newTag = document.createElement('meta');
            newTag.id = "viewport";
            newTag.name = "viewport";
            newTag.content = newVPContent;
            document.getElementsByTagName('head')[0].appendChild(newTag);
        } else {
            viewportTag.setAttribute('content', newVPContent);
        }
    }


    // Get a parameter from the passed search part of a URL
    private getURLQPByName(parmName : string, toSearch : string) : string {
        var match = RegExp('[?&]' + parmName + '=([^&]*)').exec(toSearch);
        if (match)
            return match[1];
        return null;
    }

    //
    //  When we get a message indicating a new base template is loading, we need to
    //  reset ourself in terms of graphics state, and update our drawing surface to
    //  the size required by the new template.
    //
    //  The canvas is set up to stay sized to our template DIV, so resizing it will
    //  cause the canvas to resize, though the view it contains still needs to be
    //  resized as well.
    //
    private handleNewTemplate(msgData : Object) {

        // Resize our template DIV to the size of the template
        var newSize : BaseTypes.RIVASize = new  BaseTypes.RIVASize(msgData[RIVAProto.kAttr_Size]);
        this.tmplDIV.style.width = newSize.width + "px";
        this.tmplDIV.style.height = newSize.height + "px";

        // Let the canvas know about it
        this.tarDraw.newTemplate(msgData);
    }


    //
    //  A helper to send a server flag setting messages to the server. The caller
    //  sends us the flag bit. We pass that as the mask, and then either zero or
    //  that value as the value to set.
    //
    sendServerFlag(srvFlag : number, newState : boolean) {

        var toSend : Object = { };
        toSend[RIVAProto.kAttr_OpCode] = RIVAProto.OpCodes.SetServerFlags;
        toSend[RIVAProto.kAttr_Mask] = srvFlag;
        if (newState)
            toSend[RIVAProto.kAttr_ToSet] = srvFlag;
        else
            toSend[RIVAProto.kAttr_ToSet] = 0;

        var msgText = JSON.stringify(toSend);
        if (this.logMisc)
            this.logMsg(msgText);

        this.sendMsg(msgText);
    }


    // Set a handler for the visibility API if available
    setVisState(newState : boolean) {

        //
        //  Tell the server about this change. He will stop sending drawing commands until
        //  we go back to the foreground. Note the reversed meaning. He wants to know if
        //  we are visible. Our flag is whether we are in a bgn tab (not visible) or not.
        //  So we flip it when we send.
        //
        var toSend : string = "{ \"" + RIVAProto.kAttr_OpCode + "\" : \"" +
        RIVAProto.OpCodes.SetVisState.toString() + "\", \"" + RIVAProto.kAttr_State +
        "\" : " + (this.inBgnTab ? "false" : "true") + "}";
        this.sendMsg(toSend);

        if (this.logMisc) {
            if (newState)
                this.logMsg("Going into background tab mode");
            else
                this.logMsg("Coming back to foreground tab mode");
        }

        // Let the socket worker know about this change
        this.ourSock.postMessage({ type : "setbgntab", data : this.inBgnTab }, []);
    }

    visChangeHandler = () => {

        var visState = null;
        if (typeof document["hidden"] != null)
            visState = "hidden";
        else if (typeof document["msHidden"] != null)
            visState = "msHidden";
        else if (typeof document["webkitHidden"] != null)
            visState = "webkitHidden";

        if ((visState !== null) &&  document[visState]) {
            this.inBgnTab = true;
        } else {
            this.inBgnTab = false;
        }
        this.setVisState(this.inBgnTab);
    }

    setVisHandler() {

        var visChangeEvId = null;
        if (typeof document["visibilitychange"] !== null)
            visChangeEvId = "visibilitychange";
        else if (typeof document["msvisibilitychange"] !== null)
            visChangeEvId = "msvisibilitychange";
        else if (typeof document["webkitvisibilitychange"] !== null)
            visChangeEvId = "webkitvisibilitychange";

        if (visChangeEvId !== null)
            document.addEventListener(visChangeEvId, this.visChangeHandler, false);
    }



    //
    //  To get around any inconsistent implementations, we provide our own tree dup method.
    //  This is used for importing HTML templates.
    //
    importChildNodes(srcNode) : any {

        var retNode = null;
        switch(srcNode.nodeType) {
            case document.DOCUMENT_NODE :
            case document.ELEMENT_NODE :
            case document.DOCUMENT_FRAGMENT_NODE :
                var name = srcNode.nodeName;
                if ((name.length > 0) && (name[0] === '#')) {
                    name = name.slice(1);
                }
                var newNode = document.createElement(name);
                if (srcNode.attributes && (srcNode.attributes.length > 0)) {
                for (var attrInd = 0; attrInd < srcNode.attributes.length; attrInd++) {
                        newNode.setAttribute
                        (
                            srcNode.attributes[attrInd].nodeName
                            , srcNode.getAttribute(srcNode.attributes[attrInd].nodeName)
                        );
                    }
                }

                // Recurse on children
                if (srcNode.childNodes && (srcNode.childNodes.length > 0)) {
                    for (var childInd = 0; childInd < srcNode.childNodes.length; childInd++) {
                        var newChild = this.importChildNodes(srcNode.childNodes[childInd]);
                        if (newChild != null)
                            newNode.appendChild(newChild);
                    }
                }
                retNode = newNode;
            break;

            case document.TEXT_NODE :
            case document.CDATA_SECTION_NODE :
                retNode = document.createTextNode(srcNode.nodeValue);
                break;
        }
        return retNode;
    }

    // ----------------------------------------------------
    //  Image handling
    // ----------------------------------------------------

    //
    //  We got an image data start message. It may be a whole image, or it may be the
    //  chunk of a multi-chunk image transfer. If multi-we create our temp image cache
    //  item that we use to accumualte the data. One way or the other, when we've got
    //  the image data, we call processImg() below to get it into the image cache.
    //
    //  The data is the base64 encoded PNG image.
    //
    private startImgDownload(msgData : Object) {


        // Init our temp info to store the data till we can store it
        var imgPath : string = msgData[RIVAProto.kAttr_Path];
        var imgData : string = msgData[RIVAProto.kAttr_DataBytes];
        var imgRes : BaseTypes.RIVASize = new BaseTypes.RIVASize(msgData[RIVAProto.kAttr_Size]);
        var serialNum : number = msgData[RIVAProto.kAttr_SerialNum];

        if (this.logImgProc)
            this.logMsg("Start image download (SN=" + serialNum + ") : " + imgPath);

        // If there's only one, then we have it, so add it to the queue
        if (msgData[RIVAProto.kAttr_Last]) {

            if (this.logImgProc)
                this.logMsg("Single block image, so download complete");

            this.imgQ.push
            (
                new ImgQItem
                (
                    imgPath,
                    serialNum,
                    imgRes,
                    imgData,
                    msgData[RIVAProto.kAttr_Flag]
                )
            );

            // If there is no image currently loading, then start one loading.
            if (this.curImageLoading === null) {
                if (this.logImgProc)
                    this.logMsg("No images currently loading, starting a new one");
                this.processImg(this.imgQ.pop());
            }

        } else {

            if (this.logImgProc)
                this.logMsg("Multi block image, queuing for completion");

            //
            //  Store this one as the current downloading image. We can't put it on the
            //  queue until it's done.
            //
            this.curImageDownloading = new ImgQItem
            (
                imgPath,
                msgData[RIVAProto.kAttr_SerialNum],
                imgRes,
                imgData,
                msgData[RIVAProto.kAttr_Flag]
            );
        }
   }

    private nextImgBlock(msgData : Object) {

        // Make sure it's for the image we should be seeing
        if (this.curImageDownloading.path !== msgData[RIVAProto.kAttr_Path]) {
            alert
            (
                "Expected img data for '" +
                this.curImageDownloading.path +
                "' but got '" +
                msgData[RIVAProto.kAttr_Path] +
                "'"
            );
        }

        // Add the data to our accumulator
        this.curImageDownloading.imgData += msgData[RIVAProto.kAttr_DataBytes];

        //
        //  If this is the last block, then push our guy onto the image queue to
        //  be processed and null out the downloading member.
        //
        if (msgData[RIVAProto.kAttr_Last] ) {

            if (this.logImgProc)
                this.logMsg("Got last image block: " + this.curImageDownloading.path);

            this.imgQ.push(this.curImageDownloading);
            this.curImageDownloading = null;

            // If there is no image currently loading, then start one loading.
            if (this.curImageLoading !== null) {
                if (this.logImgProc)
                    this.logMsg("No images currently loading, starting a new one");
                this.processImg(this.imgQ.pop());
            }
        }
    }

    //
    //  This is called when we get a completed image. Either because it fit in a single
    //  block or we got the last block. We get it into the cache or find it's existing
    //  entry. Then we do a set on it, which will cause it to start loading up from the
    //  base64 format to the actual image object.
    //
    //  When it completes, our imgLoadComplete event handler will be called.
    //
    private processImg(toProc : ImgQItem) {

        if (this.logImgProc)
            this.logMsg("Processing image: " + toProc.path);

        // Store this one as the one we are processing
        this.curImageLoading = toProc;

        // if it's in the cache, use that one, else a new one
        var newItem : ImgCache.ImgCacheItem = this.imgCache.findItem(toProc.path);

        if (newItem === null) {
            newItem = new ImgCache.ImgCacheItem(toProc.path, toProc.serNum, toProc.imgRes);
            this.imgCache.addImage(newItem);
        }

        //
        //  And set the new data on it. This will continue async and post us an event
        //  when it is done. We'll point those events to imgLoadComplete below.
        //
        newItem.setImageData(toProc.imgData, toProc.serNum, toProc.isPNG);
    }

    //
    //  Our handler for the custom event we register for image load completes. If there
    //  are more images to process, we start the next one processing.
    //
    //  We also get this one updated in the persistent image cache. We need to do this
    //  wile we still have the base64 data.
    //
    private imgLoadComplete = (state) => {

        if (this.logImgProc)
            this.logMsg("Image load complete: " + this.curImageLoading.path);

        // Null out the loading pointer
        this.curImageLoading = null;

        //
        //  See if there is another image available to start loading. If so, start
        //  it up. If there are not, and there's not one currently downloading, and
        //  we have seen the last EndDraw, then let's start processing graphics
        //  commands.
        //
        var nextImg : ImgQItem = this.imgQ.pop();
        if (nextImg) {
            if (this.logImgProc)
                this.logMsg("More images to download, starting next one");
            this.processImg(nextImg);
        } else if ((this.startDrawCnt === 0) && !this.curImageDownloading) {
            if (this.logImgProc)
                this.logMsg("No more images to process, processing msgs");
            this.processGraphQ();
        }
    }


    // ----------------------------------------------------
    //  Drawing command processing
    // ----------------------------------------------------

    //
    //  This is called when we have all of the graphics commands and images for
    //  the current update cycle from the server. We just loop through the queued
    //  commands and process them.
    //
    processGraphQ() {

        if (this.logGraphCmds)
            console.log("Processing queued graphics cmds");

        var opCode : RIVAProto.OpCodes;
        while (this.graphQ.length) {
            try{
                var curMsg : Object = this.graphQ.shift();
                opCode = <RIVAProto.OpCodes>curMsg[RIVAProto.kAttr_OpCode];
                if (this.logGraphCmds)
                    this.logGraphicsCmd(curMsg);
                if (!this.tarDraw.handleGraphicsCmd(opCode, curMsg, this.imgCache)) {
                    if (this.logGraphCmds)
                        console.log("Unhandled opcode: " + opCode);
                }
            }

            catch(e) {
                if (this.logGraphCmds)
                    console.error("Got an exception in graphics command: " + opCode);
            }
        }
    }


    // ----------------------------------------------------
    //  User input handling
    // ----------------------------------------------------

    // A timer we use to send deferred move/drag messages
    dragTimerHandler = () => {

        // If the pending and last mouse move points aren't the same send one
        if (!this.pendingMousePos.samePoint(this.lastMousePos)) {

            // Store the pending as the last
            this.lastMousePos.setFrom(this.pendingMousePos);

            // We only actually need to send it if in ready state
            if (this.curState === EAppStates.Ready) {

                var toSend : string = "{ \"" + RIVAProto.kAttr_OpCode + "\" : \"" +
                              RIVAProto.OpCodes.Move.toString() + "\", \"" + RIVAProto.kAttr_At +
                              "\" : \"" + this.lastMousePos.x + "," + this.lastMousePos.y + "\""
                              + "}";

                this.sendMsg(toSend);
            }
        }
    }

    //
    //  A common method to process incoming mouse and touch events. We have separate event
    //  handlers for those, which massage the values as needed and pass them on to this
    //  method for generic handling. They give us the raw positions so that they don't
    //  have to redundandtly translate them, and so that we can log msgs that match the
    //  original touch positions.
    //
    processInput(opCode : RIVAProto.OpCodes, xRaw : number, yRaw : number, minMove : number) {

        //
        //  Absolute worse case, if we missed the fact that we came back to the foreground,
        //  we catch it here and force us back to foreground mode.
        //
        if (this.inBgnTab) {
            this.inBgnTab = false;
            this.setVisState(this.inBgnTab);

            if (this.logMisc)
                this.logMsg("User input forcing us back to foreground mode");
        }

        // Adjust the raw positions to account for window scrolling
        var canvasRect : ClientRect = this.ourCanvas.getBoundingClientRect();
        var xPos =  Math.floor(xRaw - (canvasRect.left + window.scrollX));
        var yPos =  Math.floor(yRaw - (canvasRect.top + window.scrollY));

        //
        //  If we aren't up and going, don't do anything. But we do keep the two mouse
        //  positions updated.
        //
        if (this.curState !== EAppStates.Ready)
        {
            // Make absolutely sure we get back to unpressed state
            this.inPressedState = false;

            this.lastMousePos.set(xPos, yPos);
            this.pendingMousePos.set(xPos, yPos);

            if (this.logTouch)
                this.logMsg("Ignoring input, not in ready state");
            return false;
        }

        // We need to do it, so let's get set up
        if (opCode === RIVAProto.OpCodes.Press) {

            // Set the last and pending mouse positions to this
            this.lastMousePos.set(xPos, yPos);
            this.pendingMousePos.set(xPos, yPos);

            if (this.logTouch)
                this.logMsg("Got press at " + xRaw + "," + yRaw);

        } else if (opCode === RIVAProto.OpCodes.Release) {

            // Set the last and pending mouse positions to this
            this.lastMousePos.set(xPos, yPos);
            this.pendingMousePos.set(xPos, yPos);

            if (this.logTouch)
                this.logMsg("Got release at " + xRaw + "," + yRaw);

        } else if (opCode === RIVAProto.OpCodes.Move) {

            //
            //  If we aren't in press state, then just update both values. If we
            //  are, then queue it up as a pending move if it has changed more than
            //  the minimum move amount passed.
            //
            if (this.inPressedState === false) {

                this.lastMousePos.set(xPos, yPos);
                this.pendingMousePos.set(xPos, yPos);

            } else {

                //
                //  If we've moved more than the indicated minimum change in either
                //  direction, queue it up. This cuts down on lots of small events
                //  when moving slowly.
                //
                if ((Math.abs(xPos - this.lastMousePos.x) > minMove)
                ||  (Math.abs(yPos - this.lastMousePos.y) > minMove))
                {
                    // Store a pending mouse move
                    this.pendingMousePos.set(xPos, yPos);
                }
            }
            return false;
        }

        //
        //  If we are not current in pressed state, ignore any release. Do this after
        //  updating the mouse position above, since it is still obviously a legitimate
        //  mouse event.
        //
        if (opCode === RIVAProto.OpCodes.Release) {
            if (this.inPressedState === false) {
                if (this.logTouch)
                    this.logMsg("Ignoring release, not in pressed state");
                return false;
            }

            if (this.logTouch)
                this.logMsg("Exiting pressed state");
            this.inPressedState = false;
        } else if (opCode === RIVAProto.OpCodes.Press) {
            this.inPressedState = true;
            if (this.logTouch)
                this.logMsg("Entering pressed state");
        }

        // Format a message and send it
        var toSend : string = "{ \"" + RIVAProto.kAttr_OpCode + "\" : \"" +
                            opCode.toString() + "\", \"" + RIVAProto.kAttr_At +
                            "\" : \"" + xPos + "," + yPos + "\""
                            + "}\n";

        if (this.logTouch) {
            this.logMsg
            (
                "Sending " +
                RIVAProto.OpCodes[opCode] +
                " msg, pos=" +
                xRaw + "/" +
                yRaw
            );
        }

        this.sendMsg(toSend);
    }


    //
    //  Handle mouse and touch input. We do the mouse or touch event specific stuff then
    //  pass the results on to the common input processing method above. He passes it
    //  on to the server if appropriate.
    //
    canvasMouseHandler = (e : MouseEvent) => {

        // Prevent default browser response
        e.preventDefault();

        // If the main button, then we process it as user input. Others we ignore
        if (e.button === 0) {

            // Convert the event into a RIVA opcode
            var opCode : RIVAProto.OpCodes = RIVAProto.OpCodes.OpCode_None;
            if (e.type === "mousedown") {

                opCode = RIVAProto.OpCodes.Press;

                // Remember where and when
                this.mousePressedAt.set(e.pageX, e.pageY);
                this.mousePressedTime = Date.now();

            } else if (e.type === "mouseup") {

                //
                //  If held over three seconds and not moved more than a few pixels,
                //  then cancel the gesture and treat it as a menu invocation.
                //
                var Test : number = Date.now() - this.mousePressedTime;
                if (this.inPressedState
                &&  (((Date.now() - this.mousePressedTime) / 1000) >= 2.5)
                &&  (Math.abs(this.mousePressedAt.x - e.pageX) < 3)
                &&  (Math.abs(this.mousePressedAt.y - e.pageY) < 3)) {

                    this.cancelTouch(true);
                    this.showMenuPopup();

                } else {

                    // Else process it as a release
                    opCode = RIVAProto.OpCodes.Release;
                }

            } else if (e.type === "mousemove") {

                opCode = RIVAProto.OpCodes.Move;

            }

            // Call the common handler, passing the raw coordinates if not eaten
            if (opCode !== RIVAProto.OpCodes.OpCode_None)
                this.processInput(opCode, e.pageX, e.pageY, 1);

        }
        return false;
    }

    //
    //  A helper to cancel an input operation. This is only used really for
    //  touch screen input. Mouse input never cancels, though they may ultimately
    //  move outside the initial widget and cause the release to be ignored.
    //
    cancelTouch(clearPntList : boolean) {
        //
        //  Clear any tracking related stuff, but not touch points since we still
        //  may be tracking a menu invocation gesture.
        //
        this.inPressedState = false;
        if (clearPntList)
            this.touchPnts.length = 0;

        var toSend : string = "{ \"" + RIVAProto.kAttr_OpCode + "\" : \"" +
                        RIVAProto.OpCodes.CancelInput.toString() + "\"}";

        if (this.logTouch)
            this.logMsg("Cancelling touch input");

        this.sendMsg(toSend);
        return false;

    }

    canvasTouchHandler = (e : TouchEvent) => {

        // Prevent it from propagating
        e.preventDefault();

        if (this.logTouch) {
            this.logMsg("Touch event. Type=" + e.type + ", Count=" + e.changedTouches.length);
            for (var ind = 0; ind < e.changedTouches.length; ind++) {
                var curPnt : Touch = e.changedTouches[ind];
                this.logMsg("   Id=" + curPnt.identifier + ", At=" + curPnt.pageX + "/" + curPnt.pageY);
            }
        }

        var opCode : RIVAProto.OpCodes = RIVAProto.OpCodes.OpCode_None;
        var xPos : number;
        var yPos : number;
        if (e.type === "touchstart") {

            //
            //  If there is only one touch point down and it's a start, then this has to
            //  be an initial touch, so clear our list.
            //
            if (e.touches.length === 1) {
                this.touchPnts.length = 0;
                if (this.logTouch)
                    this.logMsg("Saw initial touch, starting gesture");
            }


            //
            //  We know that, if the incoming new points plus the count we have is
            //  more than 2, it cannot be valid no matter what and this simplifies
            //  the rest of the logic.
            //
            //  It doesn't matter if some have come and gone. All we care about is
            //  how many have happened. We can only have one or two, total.
            //
            var combinedPnts : number = this.touchPnts.length + e.changedTouches.length;
            if (combinedPnts > 2) {
                if (this.logTouch)
                    this.logMsg("More than 2 touch points, cancelling");

                // Clear our touch list as well
                this.cancelTouch(true);
                return false;
            }

            // Add them to our list with id and start pos
            for (var ind = 0; ind < e.changedTouches.length; ind++) {
                var curPnt : Touch = e.changedTouches[ind];
                this.touchPnts.push(new TouchPoint(curPnt.pageX, curPnt.pageY, curPnt.identifier));
            }

            //
            //  If it's now a single touch, then a provisional gesture so send the press
            //  event. If we now have more than one, then cancel the gesture, though we
            //  may still see a menu invocation later.
            //
            if (combinedPnts == 1) {
                opCode = RIVAProto.OpCodes.Press;
                xPos = e.changedTouches[0].pageX;
                yPos = e.changedTouches[0].pageY;
            } else {
                if (this.logTouch)
                    this.logMsg("More than one touch point, cancelling gesture");

                // But don't clear touch points
                this.cancelTouch(false);
                return false;
            }

        } else if (e.type === "touchend") {

            //  End these touch points in our list.
            for (var ind = 0; ind < e.changedTouches.length; ind++) {
                var curPnt : Touch = e.changedTouches[ind];
                this.updateTouchPnt(curPnt.pageX, curPnt.pageY, curPnt.identifier, true);
            }

            // If there are no more active touch points we need to deal with it.
            if (e.touches.length === 0) {

                //  If we only ever saw a single point, then this has to be the one
                //  that is ending, so this is a completed gesture and we need to send
                //  a release if we are still tracking.
                //
                //  If there are two, we see if they have not moved too much. If not,
                //  then we invoke the menu.
                //
                if (this.touchPnts.length == 1) {
                    if (this.logTouch && (e.changedTouches[0].identifier !== this.touchPnts[0].id))
                        this.logErr("Released touch point is not the one we remembered");

                    if (this.inPressedState) {
                        opCode = RIVAProto.OpCodes.Release;
                        xPos = e.changedTouches[0].pageX;
                        yPos = e.changedTouches[0].pageY;
                    }
                } else if (this.touchPnts.length === 2) {

                    if (this.touchPnts[0].withinMoveLimit() &&
                        this.touchPnts[1].withinMoveLimit())
                    {
                        this.touchPnts.length = 0;
                        if (this.logTouch)
                            this.logMsg("Got valid touch menu invocation gesture");
                        this.showMenuPopup();
                    }
                }

                // We can clear our list now
                this.touchPnts.length = 0;
            }
        } else if (e.type === "touchmove") {

            // Update these touch points if in our list
            for (var ind = 0; ind < e.changedTouches.length; ind++) {
                var curPnt : Touch = e.changedTouches[ind];
                this.updateTouchPnt(curPnt.pageX, curPnt.pageY, curPnt.identifier, false);
            }

            // If we still only have only seen one touch point, it's a move
            if (this.touchPnts.length === 1) {
                opCode = RIVAProto.OpCodes.Move;
                xPos = e.changedTouches[0].pageX;
                yPos = e.changedTouches[0].pageY
            }

        } else if (e.type === "touchcancel") {

            //  Doesn't matter what point it is, we give up.
            this.touchPnts.length = 0;
            opCode = RIVAProto.OpCodes.CancelInput;
            xPos = e.changedTouches[0].pageX;
            yPos = e.changedTouches[0].pageY;
        }

        if (opCode !== RIVAProto.OpCodes.OpCode_None)
            this.processInput(opCode, xPos, yPos, 2);

        return false;
    }


    //
    //  We find the passed touch point in our list and mark it complete and store its
    //  final position.
    //
    updateTouchPnt(xPos : number, yPos : number, id : number, end : boolean) {

        for (var ind = 0; ind < this.touchPnts.length; ind++) {
            var curPnt : TouchPoint = this.touchPnts[ind];
            if (curPnt.id == id) {
                curPnt.update(xPos, yPos, end);
                return;
            }
        }

        // We never found it
        if (this.logTouch)
            this.logMsg("Changed touch point was not in our list");
    }


    // ----------------------------------------------------
    //  Login popup menu stuff
    // ----------------------------------------------------

    //
    //  These methods show and destroy the login 'popup'. It is created on the fly
    //  from an HTML template. We add it to the main body, which puts it on top of
    //  the canvas.
    //
    showLoginPopup() {

        // Load our login popup template and get a copy of the nodes
        var toLoad : HTMLTemplateElement = <HTMLTemplateElement>document.getElementById("LoginPopupTmpl");
        var tmplCopy = this.importChildNodes(toLoad.content);

        // Get main DIV and load the imported nodes into it
        var mainDIV : HTMLDivElement = this.mainDIV;
        mainDIV.appendChild(tmplCopy);

        //
        //  If we have stored the logo image data, set it directly. Else set the path on
        //  it so it will load it.
        //
        var logoImg : HTMLImageElement = <HTMLImageElement>document.getElementById("CQSLLogo");
        if (this.logoImgData)
            logoImg.src = this.logoImgData;
        else
            logoImg.src = "/CQCImg/System/Logos/CQS_Small";
    }

    destroyLoginPopup() {
        //
        //  Before we destroy it, if we have not already, grab the image data from the logo
        //  image.
        //
        if (!this.logoImgData) {
            var logoImg : HTMLImageElement = <HTMLImageElement>document.getElementById("CQSLLogo");

            // We need to create a temp canvas and draw the image into it
            var tmpCanvas : HTMLCanvasElement = document.createElement("canvas");
            tmpCanvas.height = logoImg.height;
            tmpCanvas.width = logoImg.width;
            tmpCanvas.style.height = logoImg.height.toString();
            tmpCanvas.style.width = logoImg.width.toString();
            var tmpCtx : CanvasRenderingContext2D = tmpCanvas.getContext("2d");

            try {
                tmpCtx.drawImage(logoImg, 0, 0);

                // Now we can get it to give us the base64 image URL
                this.logoImgData = tmpCanvas.toDataURL();
            }

            catch(e)
            {
            }
        }

        document.getElementById("LoginPopup").remove();
    }



    // ----------------------------------------------------
    //  Options menu stuff
    // ----------------------------------------------------

    //
    //  These methods show and destroy the options menu 'popup'. It is created on
    //  the fly from an HTML template. We add it to the main div which puts it on top
    //  of the canvas.
    //
    showMenuPopup() {

        // Load our login popup template and get a copy of the nodes
        var toLoad : HTMLTemplateElement = <HTMLTemplateElement>document.getElementById("PopupMenuTmpl");
        var tmplCopy = this.importChildNodes(toLoad.content);

        // Get main DIV and load the imported nodes into it
        var mainDIV : HTMLDivElement = this.mainDIV;
        mainDIV.appendChild(tmplCopy);

        // Set up the click handlers now
        document.getElementById("FullScreen").onclick = this.menuClickHandler;
        document.getElementById("LogGraphCmds").onclick = this.menuClickHandler;
        document.getElementById("LogImgProc").onclick = this.menuClickHandler;
        document.getElementById("LogMisc").onclick = this.menuClickHandler;
        document.getElementById("LogTouch").onclick = this.menuClickHandler;
        document.getElementById("LogWebSock").onclick = this.menuClickHandler;

        document.getElementById("LogGUIEvents").onclick = this.menuClickHandler;
        document.getElementById("LogSrvMsgs").onclick = this.menuClickHandler;
        document.getElementById("NoCache").onclick = this.menuClickHandler;

        // Update the menu to make sure it reflects current settings
        this.updateMenuItem("LogGraphCmds", this.logGraphCmds);
        this.updateMenuItem("LogImgProc", this.logImgProc);
        this.updateMenuItem("LogMisc", this.logMisc);
        this.updateMenuItem("LogTouch", this.logTouch);
        this.updateMenuItem("LogWebSock", this.logWebSock);

        this.updateMenuItem("LogGUIEvents", this.logGUIEvents);
        this.updateMenuItem("LogSrvMsgs", this.logSrvMsgs);
        this.updateMenuItem("NoCache", this.noCache);

        this.updateMenuItem("FullScreen", this.fullScreenMode);
        document.getElementById("MenuClose").onclick = this.menuClickHandler;
    }

    destroyMenuPopup() {
        document.getElementById("PopupMenu").remove();
    }


    //
    //  A helper to update a context menu item. We hide/show the check mark image based
    //  the state of the item.
    //
    updateMenuItem(itemId : string, state : boolean) {
        var tarImg : HTMLImageElement = (<HTMLImageElement>document.getElementById(itemId + "Check"));
        if (state)
            tarImg.style.visibility = "visible";
        else
            tarImg.style.visibility = "hidden";
    }

    // A click handler for our menu items and the menu close button
    menuClickHandler = (srcevent : Event) => {

        var event = srcevent.srcElement as HTMLElement;

        srcevent.preventDefault();

        // Handle the special case of closing the menu. This simplifies the stuff below
        if (event.id === "MenuClose") {

            this.destroyMenuPopup();

            // This can mess up our input stuff, so be extra careful
            this.inPressedState = false;
            this.touchPnts.length = 0;

            return false;
        }

        // Has to be one of the checked options
        var newState : boolean = null;
        var srvFlag : number = RIVAProto.kSrvFlag_None;

        // Local stuff, where we just toggle our local flag
        if (event.id === "LogGraphCmds") {
            this.logGraphCmds = !this.logGraphCmds;
            newState = this.logGraphCmds;
        } else if (event.id === "LogImgProc") {
            this.logImgProc = !this.logImgProc;
            newState = this.logImgProc;
        } else if (event.id === "LogMisc") {
            this.logMisc = !this.logMisc;
            newState = this.logMisc;
        } else if (event.id === "LogTouch") {
            this.logTouch = !this.logTouch;
            newState = this.logTouch;
        } if (event.id === "LogWebSock") {
            this.logWebSock = !this.logWebSock;
            newState = this.logWebSock;

            // Update our web worker
            this.ourSock.postMessage({ type : "setdebug", data : newState }, []);

        // Server related flags. We toggle a local flag but need to tell the server also
        } else if (event.id === "LogGUIEvents") {
            this.logGUIEvents = !this.logGUIEvents;
            srvFlag = RIVAProto.kSrvFlag_LogGUIEvents;
            newState = this.logGUIEvents;
        } else if (event.id === "LogSrvMsgs") {
            this.logSrvMsgs = !this.logSrvMsgs;
            srvFlag = RIVAProto.kSrvFlag_LogSrvMsgs
            newState = this.logSrvMsgs;
        } else if (event.id === "NoCache") {
            this.noCache = !this.noCache;
            srvFlag = RIVAProto.kSrvFlag_NoCache
            newState = this.noCache;

        // Enter or exit full screen
        } else if (event.id === "FullScreen") {
            if (this.fullScreenMode) {
                this.fullScreenMode = false;
                this.exitFullScreen();
            } else {
                this.fullScreenMode = true;
                this.enterFullScreen();
            }
        }


        // Update the menu item's check mark
        this.updateMenuItem(event.id, newState);

        // If it's one of the ones related to the server, we need to pass it on
        if (srvFlag !== RIVAProto.kSrvFlag_None)
            this.sendServerFlag(srvFlag, newState);

        return false;
    }


    // ----------------------------------------------------
    //  We use a web worker to manage the socket, so we provide this helper to hide
    //  the details of how messages get sent.
    // ----------------------------------------------------
    sendMsg(toSend : string) {
        //
        //  We just post the message to our worker, with the approriate info to tell him
        //  it's a message to send.
        //
        this.ourSock.postMessage( { type : "send", data : toSend } );
    }


    // Sends out a ping message, which we do from a couple places
    sendPing() {

        // Just log this to the local console
        if (this.logMisc)
            console.log("Sending ping");

        this.sendMsg(this.pingMsg);
    }


    // ----------------------------------------------------
    //  Internally handled RIVACmd operations are passed here for processing. Others are passed
    //  to the extension file.
    // ----------------------------------------------------
    doRIVACmd(pathCmd : string, p1 : string, p2 : string, p3 : string) {

        if (pathCmd === RIVAProto.kIntRIVACmd_LoadURLTab) {

            //
            //  Just invoke a tab. p1 is the URL, and p2 is the tab name
            //
            var newTab = window.open(p1, p2);
            newTab.focus();

        } else {
            if (this.logMisc)
                console.log("Unknown internal RIVA cmd: " + pathCmd);
        }
    }


    // ----------------------------------------------------
    //  Methods related to the special 'remote widgets' where we have to
    //  create a visual element ourself.
    // ----------------------------------------------------

    createRemWidget(msgData : Object) {
        // Figure out which type
        var ordVal : number = msgData[RIVAProto.kAttr_Type];
        var wdgType : RIVAProto.WdgTypes = <RIVAProto.WdgTypes>(ordVal);

        // And now create the appropriate thing
        if (wdgType === RIVAProto.WdgTypes.WebBrowser) {

            // Not dealt with yet

        } else if (wdgType === RIVAProto.WdgTypes.WebCamera) {

            // Get the parameters. This is just the raw JSON
            var params = msgData[RIVAProto.kAttr_Params];

            //
            //  This is the position relative to the overall template, which means
            //  the canvas in our case, and therefore the template DIV. So we set
            //  it absolute relative to the template DIV>
            //
            var atPos : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(msgData[RIVAProto.kAttr_At]);

            // Create the unique id for this widget
            var wdgId : string = RIVAProto.WdgTypes[wdgType] + msgData[RIVAProto.kAttr_Id];

            //
            //  Create a new element at the indicated area. We let them do it if they
            //  want. If they don't, then we will create a standard video element.
            //
            var newCam : HTMLVideoElement;
            newCam = rivaWebCamCreate
            (
                wdgId, atPos.x, atPos.y, atPos.width, atPos.height, params, this.extType
            );
            if (newCam === null) {
                newCam = document.createElement('video');
                if  (newCam) {
                    newCam.id = wdgId;
                    newCam.style.left = atPos.x.toString() + "px";
                    newCam.style.top = atPos.y.toString() + "px";
                    newCam.style.width = atPos.width.toString() + "px";
                    newCam.style.height = atPos.height.toString() + "px";
                    newCam.style.visibility = "visible";
                    newCam.style.position = "absolute";
                    newCam.style.zIndex = "1000";

                    // Typescript is incorrect on this one so we have to do it this way
                    newCam["autoplay"] = true;

                    // Since we created it, let them do post setup
                    rivaWebCamSetup(wdgId, params, this.extType);
                }
            }
            else {
                this.logMsg("Cam widget created via extension");

                // Just to be sure, make sure the widget id is set
                newCam.id = wdgId;
            }

            // If it got created, then add it to the DOM
            if (newCam !== null)
                this.tmplDIV.appendChild(newCam);
        }
    }

    destroyRemWidget(msgData : Object) {

        // Figure out which type
        var ordVal : number = msgData[RIVAProto.kAttr_Type];
        var wdgType : RIVAProto.WdgTypes = <RIVAProto.WdgTypes>(ordVal);

        // Build the unique id and try to remove this element
        var wdgId : string = RIVAProto.WdgTypes[wdgType] + msgData[RIVAProto.kAttr_Id];

        // Find the video widget and destroy it
        var toDestroy : HTMLElement = document.getElementById(wdgId);
        if (toDestroy) {

            // Let the client's code do any required cleanup first
            try {
                if (wdgType === RIVAProto.WdgTypes.WebCamera) {
                    this.logMsg("Destroying web cam widget: " + wdgId);
                    rivaWebCamDestroy(wdgId, this.extType);
                }
            }

            catch(e) {
                this.logErr(e.toString());
            }
        } else {
            this.logMsg("Video widget to destroy was not found");
        }
    }


    //
    //  This is called to update the URL of a remote widget.
    //
    setRemWidgetURL(msgData : Object) {
        // Figure out which type
        var ordVal : number = msgData[RIVAProto.kAttr_Type];
        var wdgType : RIVAProto.WdgTypes = <RIVAProto.WdgTypes>(ordVal);

        // Build up the unique widget id and use that to look up the element
        var wdgId : string = RIVAProto.WdgTypes[wdgType] + msgData[RIVAProto.kAttr_Id];

        var tarElem : HTMLElement = document.getElementById(wdgId);
        if (tarElem) {
            var tarURL = msgData[RIVAProto.kAttr_ToLoad];
            var optParms = msgData[RIVAProto.kAttr_Params];

            // Let the client's code do any required cleanup first
            try {
                if (wdgType === RIVAProto.WdgTypes.WebCamera) {
                    this.logMsg("Setting web cam URL: " + wdgId);
                    rivaSetWebCamURL(wdgId, tarURL, optParms, this.extType);
                }
            }

            catch(e) {
                this.logErr(e.toString());
            }
        }
    }


    // ----------------------------------------------------
    //  Helpers for full screen API support
    // ----------------------------------------------------
    fullScreenPossible() : boolean {
        var retVal : boolean = false;

        if (document.fullscreenEnabled) {
            retVal = document.fullscreenEnabled;
        } else if (document['msFullscreenEnabled']) {
            retVal = document['msFullscreenEnabled'];
        } else if (document['webkitFullscreenEnabled']) {
            retVal = document['webkitFullscreenEnabled'];
        } else if (document['mozFullscreenEnabled']) {
            retVal = document['mozFullscreenEnabled'];
        }

        return retVal;
    }

    enterFullScreen() {
        if (this.fullScreenPossible()) {
            var tarElem : HTMLElement = document.documentElement;

            if (tarElem.requestFullscreen) {
                tarElem.requestFullscreen();
            } else if (tarElem['msRequestFullscreen']) {
                tarElem['msRequestFullscreen']();
            } else if (tarElem['webkitRequestFullscreen']) {
                tarElem['webkitRequestFullscreen']();
            } else if (tarElem['mozRequestFullscreen']) {
                tarElem['mozRequestFullscreen']();
            }

        } else {
            alert("Full screen access is not available on this browser");
        }
    }

    exitFullScreen() {
        if (document.exitFullscreen) {
            document.exitFullscreen();
        } else if (document['msExitFullscreen']) {
            document['msExitFullscreen']();
        } else if (document['webkitExitFullscreen']) {
            document['webkitExitFullscreen']();
        } else if (document['mozExitFullscreen']) {
            document['mozExitFullscreen']();
        }
    }


    // ----------------------------------------------------
    //  A helper to log graphics commands to the console, in a readable way, since
    //  they are heavily compressed by using numeric ids for lots of stuff.
    // ----------------------------------------------------
    logGraphicsCmd(toLog : Object) {
        var msgText : string = "GMSG: ";

        // Get the opcode out and to a string
        var opNum : number = toLog[RIVAProto.kAttr_OpCode];
        msgText += RIVAProto.OpCodes[opNum];

        // And now loop through all the values
        var valInd : number = 0;
        Object.keys(toLog).forEach((key : string) => {

            // Eat this one. We did it explicitly above to make sure it's first
            if (key !== RIVAProto.kAttr_OpCode) {

                if (valInd === 0)
                    msgText + " - ";
                    msgText += ", ";

                // Translate some of them
                switch(key) {

                    case RIVAProto.kAttr_ClipMode :
                        msgText += "ClipMode=";
                        var mode : RIVAProto.ClipModes = <RIVAProto.ClipModes>Number(toLog[key]);
                        msgText += RIVAProto.ClipModes[mode];
                        break;

                    case RIVAProto.kAttr_ClipArea :
                        msgText += "ClipArea=" + toLog[key];
                        break;

                    case RIVAProto.kAttr_SrcArea :
                        msgText += "SrcArea=" + toLog[key];
                        break;

                    case RIVAProto.kAttr_TarArea :
                        msgText += "TarArea=" + toLog[key];
                        break;

                    case RIVAProto.kAttr_ToDraw :
                        msgText += "ToDraw=" + toLog[key];
                        break;

                    default :
                        msgText += key + "=" + toLog[key];
                        break;
                }
                valInd++;
            }
        });

        // These we always log to the console, even if server logging is enabled
        console.log(msgText);
    }


    // ----------------------------------------------------
    //  This is our single message logging method. Everyone (except the graphics cmds
    //  logging) has to call this so that we can redirect to the web server if asked
    //  to do so. For mobile devices this is a very useful option. Else we log to the
    //  local console, which is best used where available.
    // ----------------------------------------------------
    private logErr(toLog : string) {
        if (this.logToServer) {
            var toSend : string = "{ \"" + RIVAProto.kAttr_OpCode + "\" : " +
                                    "\"" + RIVAProto.OpCodes.LogMsg.toString() + "\", " +
                                    "\"" + RIVAProto.kAttr_MsgText + "\" : \"" + toLog + "\", " +
                                    "\"" + RIVAProto.kAttr_Flag + "\" : true" +
                                  "}";
            this.sendMsg(toSend);
        } else {
            console.log(toLog);
        }
    }

    private logMsg(toLog : string) {
        if (this.logToServer) {
            var toSend : string = "{ \"" + RIVAProto.kAttr_OpCode + "\" : " +
                                    "\"" + RIVAProto.OpCodes.LogMsg.toString() + "\", " +
                                    "\"" + RIVAProto.kAttr_MsgText + "\" : \"" + toLog + "\", " +
                                    "\"" + RIVAProto.kAttr_Flag + "\" : false" +
                                  "}";
            this.sendMsg(toSend);
        } else {
            console.log(toLog);
        }
    }
}


// Create our one global object
var ourApp : WebRIVAApp = new WebRIVAApp();
