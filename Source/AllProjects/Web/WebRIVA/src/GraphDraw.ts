import * as BaseTypes from "./BaseTypes";
import * as RIVAProto from "./RIVAProto";
import * as ImgCache from "./ImgCache";

//
//  This class contains our canvas and the handlers for all of the graphics output code.
//

//
//  A simple graphics context. The canvas' context scheme is useless pretty much,
//  so we do our own.
//
class GraphCtx {

    constructor(toCopy : GraphCtx = null) {
        if (toCopy) {
            this.bgnColor = toCopy.bgnColor;
            this.bmpMode = toCopy.bmpMode;
            this.textColor = toCopy.textColor;
            this.fontStackSize = toCopy.fontStackSize;
            this.fontStackSize = toCopy.fontStackSize;

            this.shadowBlur = toCopy.shadowBlur;
            this.shadowOffsetX = toCopy.shadowOffsetX;
            this.shadowOffsetY = toCopy.shadowOffsetY;
            this.shadowColor = toCopy.shadowColor;
        } else {
            this.reset();
        }
    }

    // Applies our stored settings to the passed context
    applyTo(tarCtx : CanvasRenderingContext2D) {

        // And set those values on our context object where applicable
        tarCtx.globalCompositeOperation = this.bmpMode;
        tarCtx.fillStyle = this.bgnColor;
        tarCtx.shadowBlur = this.shadowBlur;
        tarCtx.shadowOffsetX = this.shadowOffsetX;
        tarCtx.shadowOffsetY = this.shadowOffsetY;
        tarCtx.shadowColor = this.shadowColor;
    }


    reset() {
        this.bgnColor = "#FFFFFF";
        this.bmpMode = "source-over";
        this.fontStackSize = 0;
        this.clipStackSize = 0;
        this.textColor = "#000000";

        this.shadowBlur = 0;
        this.shadowOffsetX = 0;
        this.shadowOffsetY = 0;
        this.shadowColor = "#000000";
    }

    bgnColor        : string;
    bmpMode         : string;
    textColor       : string;

    // Drop shadow related stuff
    shadowBlur      : number;
    shadowOffsetX   : number;
    shadowOffsetY   : number;
    shadowColor     : string;

    // Internal debugging stuff
    clipStackSize   : number;
    fontStackSize   : number;
}

//
//  Font info that we push/pop on our font stack. We pre-format out the info to
//  a string that can be set on the canvas to set up this particular font.
//
class FontInfo {
    constructor(fontMsg : Object) {
        if (fontMsg) {
            // Get out the raw values
            this.height = fontMsg[RIVAProto.kAttr_FontH];
            this.faceName = fontMsg[RIVAProto.kAttr_FontFace];
            this.isBold = (fontMsg[RIVAProto.kAttr_Flags] & RIVAProto.kFontFlag_Bold) !== 0;
            this.isItalic = (fontMsg[RIVAProto.kAttr_Flags] & RIVAProto.kFontFlag_Italic) !== 0;
        } else {
            this.height = 12;
            this.isBold = false;
            this.isItalic = false;
            this.faceName = "Verdana";
        }

        // Format it the way we need to set on the canvas
        this.canvasFmt = "";
        if (this.isItalic)
            this.canvasFmt += "italic ";
        if (this.isBold)
            this.canvasFmt += "bold ";
        this.canvasFmt += (this.height - 0.5) + "px ";
        this.canvasFmt += this.faceName;
    }

    faceName : string;
    height : number;
    isBold : boolean;
    isItalic : boolean;

    canvasFmt : string;
}


//
//  This is our drawing class. The main application passes all graphics commands to
//  us here to be processed.
//
export class GraphDraw {

    // Our canvas that we display on and the context object for it
    private showSurface : HTMLCanvasElement;
    private showCtx : CanvasRenderingContext2D;

    //
    //  A secondary, offscreen canvas for double buffering, where we do all of the drawing
    //  and masking and then blit it to the show context.
    //
    private drawSurface : HTMLCanvasElement;
    private drawCtx : CanvasRenderingContext2D;

    //
    //  A tertiary canvas that is used for intermediate results in some more complex
    //  operations, such as progress bars. And it's used for masking in color masked
    //  image drawing. It is sized up as needed, since it almost never would need to be
    //  full size.
    //
    private tmpSurface : HTMLCanvasElement;
    private tmpCtx : CanvasRenderingContext2D;

    //
    //  When we get a StartDraw we save the update area. When we see the EndDraw we blit
    //  this area from the drawing context to the display context. This effectively
    //  provides our overall clipping, so we don't need to depend on a clipping path
    //  for this (which causes lots of problems because of the stupid canvas anti-
    //  aliasing that we can't prevent.)
    //
    private updateArea : BaseTypes.RIVAArea;

    //
    //  A stack for nested clip areas the server sets on us. We just save them and update
    //  the actual clip stack only when it is necessary, see below.
    //
    private clipStack : Array<BaseTypes.RIVAArea>;

    //
    //  We don't want to rebuild the clip path every time the clip stack changes because
    //  often there are multiple push or pop operations without any actual drawing in
    //  between. So every time the stack is changed in such a way that would affect the
    //  clip path, this is set. Upon receipt of a drawing command, if this is set we
    //  rebuild the clip path and clear this flag. This is a substantial win in most
    //  cases.
    //
    private clipPathRebuildNeeded : boolean;

    // A stack for fonts to be pushed and popped on us
    private fontStack : Array<FontInfo>;

    // The info object for the currently set font
    private curFont : FontInfo;

    //
    //  The height of the currently set font, which we use for inter-line spacing when
    //  doing multi-line text (we have to handle that ourselves.) We actually set it
    //  to slightly higher than the font heigh to best match the Windows IV output.
    //
    private curFontHeight : number;

    //
    //  Our own context stack and the current context object. We pop the top pof the
    //  stack into the current context member.
    //
    private ctxStack : Array<GraphCtx>;
    private curContext : GraphCtx;

    //
    //  The color of the HTML DIV surrounding our canvas. Because of the need to translate
    //  the drawing canvas while drawing (to avoid stupid anti-aliasing) when we blit it to the
    //  show canvas, it leaves a line on the right/bottom. After updating the canvas we draw
    //  lines over that using the background color. Initially it's assumed to be white since
    //  that is what we set in the CSS. After that the app will update us if it is changed by
    //  the command that supports that.
    //
    private surroundColor : string = "#FFFFFF";

    // The main app tells us if we should do debug logging
    private doDebug : boolean;

    constructor(doDebug : boolean) {

        //
        //  Get our visible canvas from the DOM, and create our background drawing canvas
        //  as well.
        //
        this.showSurface = <HTMLCanvasElement>document.getElementById("TemplateCont");
        this.drawSurface = document.createElement("canvas");

        //
        // Allocate our push/pop stacks, initially empty until something is pushed by
        //  the server.
        //
        this.clipStack = new Array<BaseTypes.RIVAArea>();
        this.fontStack = new Array<FontInfo>();
        this.ctxStack = new Array<GraphCtx>();

        // Allocate an initial current context object with default settings
        this.curContext = new GraphCtx(null);

        // And an initial font
        this.curFont = new FontInfo(null);

        // Store the passed debug flag
        this.doDebug = doDebug;

        // And call our reset method to set up our contexts to defaults
        this.resetContext();
    }


    //
    //  Called with any graphics command msgs we get. We just do a switch and pass each one
    //  to its own private handler method, or handle it here if really simple.
    //
    //  We also get a ref to the image cache in case we need to draw an image.
    //
    handleGraphicsCmd(  opCode : RIVAProto.OpCodes,
                        cmdMsg : Object,
                        imgCache : ImgCache.ImgCache) : boolean {


        //
        //  We handle any of the non-drawing ones separately because we want to be able
        //  to potentially rebuild the clip path when a drawing command is received.
        //
        if (opCode >= RIVAProto.OpCodes.LastDrawing) {

            switch(opCode) {
                case RIVAProto.OpCodes.EndDraw :

                    // Blit the update area from the buffering canvas to the display one
                    this.showCtx.drawImage
                    (
                        this.drawSurface,
                        this.updateArea.x,
                        this.updateArea.y,
                        this.updateArea.width,
                        this.updateArea.height,
                        this.updateArea.x,
                        this.updateArea.y,
                        this.updateArea.width,
                        this.updateArea.height
                    );

                    // Fill in the right/bottom half line that our translation leaves unfilled
                    this.setLRBoundaryColor(this.surroundColor);
                    break;

                case RIVAProto.OpCodes.PopClipArea :
                    this.popClipStack();
                    break;

                case RIVAProto.OpCodes.PopContext :
                    this.popContext();
                    break;

                case RIVAProto.OpCodes.PopFont :
                    // Just pop the top of the stack and set it as the current font
                    if (this.fontStack.length) {
                        this.curFont = this.fontStack.pop();

                        // We add two to better match Windows (this is our inter-line spacing)
                        this.curFontHeight = this.curFont.height + 2;

                        // Update the drawing context's font style from this new current font
                        this.drawCtx.font = this.curFont.canvasFmt;
                    } else {
                        throw Error("Font stack underflow");
                    }
                    break;

                case RIVAProto.OpCodes.PushContext :
                    this.pushContext();
                    break;

                case RIVAProto.OpCodes.PushFont :
                    // Format the font info out and push/set it
                    this.pushFont(cmdMsg);
                    break;

                case RIVAProto.OpCodes.PushClipArea  :
                    var tarArea : BaseTypes.RIVAArea =
                                    new BaseTypes.RIVAArea(cmdMsg[RIVAProto.kAttr_ClipArea]);
                    tarArea.rounding = cmdMsg[RIVAProto.kAttr_Rounding];
                    tarArea.clipMode = <RIVAProto.ClipModes>cmdMsg[RIVAProto.kAttr_ClipMode];
                    this.pushClipArea(tarArea);
                    break;

                case RIVAProto.OpCodes.SetColor :
                    // Get the color to set
                    var toSet : RIVAProto.DrawingColors = <RIVAProto.DrawingColors>
                    (
                        cmdMsg[RIVAProto.kAttr_ToSet]
                    );

                    // Just store them in our current context for later use
                    if (toSet === RIVAProto.DrawingColors.Background)
                        this.curContext.bgnColor = cmdMsg[RIVAProto.kAttr_Color];
                    else
                        this.curContext.textColor = cmdMsg[RIVAProto.kAttr_Color];
                    break;

                case RIVAProto.OpCodes.StartDraw :
                    // Clear the area that is going to be updated
                    this.updateArea = new BaseTypes.RIVAArea(cmdMsg[RIVAProto.kAttr_UpdateArea]);
                    this.drawCtx.clearRect
                    (
                        this.updateArea.x,
                        this.updateArea.y,
                        this.updateArea.width,
                        this.updateArea.height
                    );
                    break;

                default :
                    // Never handled it so return false
                    return false;
            };

        } else {

            //
            //  If the clip stack has been modified in a way that affected the clip path
            //  then we need to rebuild it. The rebuild method will clear the flag.
            //
            if (this.clipPathRebuildNeeded)
                this.rebuildClipArea();

            switch(opCode)
            {
                case RIVAProto.OpCodes.AlphaBlit :
                    this.alphaBlit(cmdMsg, imgCache);
                    break;

                case RIVAProto.OpCodes.AlphaBlitST :
                    this.alphaBlitST(cmdMsg, imgCache);
                    break;

                case RIVAProto.OpCodes.DrawBitmap :
                    this.drawBitmap(cmdMsg, imgCache);
                    break;

                case RIVAProto.OpCodes.DrawBitmapST :
                    this.drawBitmapST(cmdMsg, imgCache);
                    break;

                case RIVAProto.OpCodes.DrawLine :
                    this.drawLine(cmdMsg);
                    break;

                case RIVAProto.OpCodes.DrawText :
                    this.drawText(cmdMsg);
                    break;

                case RIVAProto.OpCodes.DrawTextFX :
                    this.drawTextFX(cmdMsg);
                    break;

                case RIVAProto.OpCodes.DrawMultiText :
                    this.drawMultiText(cmdMsg);
                    break;

                case RIVAProto.OpCodes.DrawPBar  :
                    this.drawPBar(cmdMsg, imgCache);
                    break;

                case RIVAProto.OpCodes.FillArea  :
                    this.fillArea(cmdMsg);
                    break;

                case RIVAProto.OpCodes.FillWithBmp :
                    this.fillWithBmp(cmdMsg, imgCache);
                    break;

                case RIVAProto.OpCodes.GradientFill  :
                    this.gradientFill(cmdMsg);
                    break;

                case RIVAProto.OpCodes.StrokeArea  :
                    this.strokeArea(cmdMsg);
                    break;

                default :
                    // Never handled it, so return false
                    return false;
            };
        }
        return true;
    }


    //
    //  Called when we get a new template comand. We need to resize the canvas and
    //  the drawing surfaces.
    //
    newTemplate(msgData : Object){

        // Get the size of the new template
        var newSize : BaseTypes.RIVASize = new  BaseTypes.RIVASize(msgData[RIVAProto.kAttr_Size]);

        // Set the drawing surface size
        this.drawSurface.width = newSize.width;
        this.drawSurface.height = newSize.height;
        this.drawSurface.style.width = newSize.width + "px";
        this.drawSurface.style.height = newSize.height + "px";

        // Do the same for the display and clipping surfaces
        this.showSurface.width = newSize.width;
        this.showSurface.height = newSize.height;
        this.showSurface.style.width = newSize.width + "px";
        this.showSurface.style.height = newSize.height + "px";

        // And reset our contect to get our canvases updated
        this.resetContext();

        //
        //  Initially fill the show surface with the surround color, so that we don't
        //  get a flash of black before the template draws.
        //
        this.showCtx.fillStyle = this.surroundColor;
        this.showCtx.fillRect(0, 0, newSize.width, newSize.height);

        return newSize;
    }


    //
    //  A helper to set the right and bottom half pixels with the parent DIV's background
    //  color, since otherwise they woudln't get set because we translate the canvas over
    //  half a pixel to get rid of the stupidity of how the canvas works. At the point
    //  that this is called the translation has already been done, so we have to temporarily
    //  undo it.
    //
    setLRBoundaryColor(newColor : string) {

        if (!newColor || (newColor === ""))
            newColor = "#FFFFFF";

        // Save this as the new surround color
        this.surroundColor = newColor;

        this.showCtx.save();
        this.drawCtx.translate(-0.5, -0.5);
        this.showCtx.fillStyle = this.surroundColor;
        this.showCtx.fillRect(this.drawSurface.width - 1, 0, 1, this.drawSurface.height);
        this.showCtx.fillRect(0, this.drawSurface.height - 1, this.drawSurface.width, 1);
        this.showCtx.restore();
    }


    // ------------------------------------------------
    //  Handlers for each of the graphics commands, called by the main command
    //  handler method above.
    // ------------------------------------------------

    //
    //  These are called to do a alpha blit of an image. We have one that just takes
    //  an origin point. And another that takes the full source/target areas.
    //
    private alphaBlit(imgMsg : Object, imgCache : ImgCache.ImgCache) {

        //
        //  Get the image from the image cache. In this case, we pass null for the target
        //  image res, since we are drawing in the natural image size. So we let it
        //  pick the default one.
        //
        var imgPath : string = imgMsg[RIVAProto.kAttr_Path];
        var toDraw : HTMLImageElement = imgCache.findImg(imgPath);

        // If not found, give up now
        if (!toDraw) {
            if (this.doDebug)
                console.info("Image not found. Key=" + imgPath);
            return;
        }

        try {
            this.drawCtx.globalAlpha = imgMsg[RIVAProto.kAttr_ConstAlpha];
            var tarPnt : BaseTypes.RIVAPoint = new BaseTypes.RIVAPoint(imgMsg[RIVAProto.kAttr_ToPnt]);
            this.drawCtx.drawImage(toDraw, tarPnt.x, tarPnt.y);
            this.drawCtx.globalAlpha = 1.0;
        }

        catch(e) {
            this.drawCtx.globalAlpha = 1.0;
        }
    }

    private alphaBlitST(imgMsg : Object, imgCache : ImgCache.ImgCache) {

        // Get the image from the image cache. If not found, we do nothing
        var imgPath : string = imgMsg[RIVAProto.kAttr_Path];
        var toDraw : HTMLImageElement = imgCache.findImg(imgPath);
        if (!toDraw) {
            if (this.doDebug)
                console.info("Image not found. Key=" + imgPath);
            return;
        }

        try {
            var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(imgMsg[RIVAProto.kAttr_TarArea]);
            var srcArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(imgMsg[RIVAProto.kAttr_SrcArea]);

            // Set our constant alpha to the global alpha of the context
            this.drawCtx.globalAlpha = imgMsg[RIVAProto.kAttr_ConstAlpha];
            this.drawCtx.drawImage
            (
                toDraw,
                srcArea.x,
                srcArea.y,
                srcArea.width,
                srcArea.height,
                tarArea.x,
                tarArea.y,
                tarArea.width,
                tarArea.height
            );
            this.drawCtx.globalAlpha = 1.0;
        }

        catch(e) {
            this.drawCtx.globalAlpha = 1.0;
            throw(e);
        }
    }


    //
    //  These are called to do a draw of an image. We have one that just takes
    //  an origin point. And another that takes the full source/target areas.
    //
    private drawBitmap(imgMsg : Object, imgCache : ImgCache.ImgCache) {

        //
        //  Get the image from the image cache. In this case, we pass null for the target
        //  image res, since we are drawing in the natural image size. So we let it
        //  pick the default one.
        //
        var imgPath : string = imgMsg[RIVAProto.kAttr_Path];
        var toDraw : HTMLImageElement = imgCache.findImg(imgPath);

        // If not found, give up now
        if (!toDraw) {
            if (this.doDebug)
                console.info("Image not found. Key=" + imgPath);
            return;
        }

        // The target point to draw to
        var tarPnt : BaseTypes.RIVAPoint = new BaseTypes.RIVAPoint(imgMsg[RIVAProto.kAttr_ToPnt]);

        // Get the mode out and convert that to a canvas equivalent
        var canvasMode : string = this.xlatBmpMode
        (
            <RIVAProto.BmpModes>imgMsg[RIVAProto.kAttr_Mode]
        );

        this.drawCtx.globalCompositeOperation = canvasMode;
        try {
            this.drawCtx.drawImage(toDraw, tarPnt.x, tarPnt.y);
            this.drawCtx.globalCompositeOperation = this.curContext.bmpMode;
        }

        catch(e) {
            this.drawCtx.globalCompositeOperation = this.curContext.bmpMode;
            throw(e);
        }
    }

    private drawBitmapST(imgMsg : Object, imgCache : ImgCache.ImgCache) {

        //
        //  Get the image from the image cache. In this case, we pass null for the target
        //  image res, since we are drawing in the natural image size. So we let it
        //  pick the default one.
        //
        var imgPath : string = imgMsg[RIVAProto.kAttr_Path];
        var toDraw : HTMLImageElement = imgCache.findImg(imgPath);

        // If not found, give up now
        if (!toDraw) {
            if (this.doDebug)
                console.info("Image not found. Key=" + imgPath);
            return;
        }

        // Get the source and target areas
        var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(imgMsg[RIVAProto.kAttr_TarArea]);
        var srcArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(imgMsg[RIVAProto.kAttr_SrcArea]);

        // Get the mode out and convert that to a canvas equivalent
        var canvasMode : string = this.xlatBmpMode
        (
            <RIVAProto.BmpModes>imgMsg[RIVAProto.kAttr_Mode]
        );
        this.drawCtx.globalCompositeOperation = canvasMode;

        try {
            this.drawCtx.drawImage
            (
                toDraw,
                srcArea.x,
                srcArea.y,
                srcArea.width,
                srcArea.height,
                tarArea.x,
                tarArea.y,
                tarArea.width,
                tarArea.height
             );
             this.drawCtx.globalCompositeOperation = this.curContext.bmpMode;
        }

        catch(e) {
            this.drawCtx.globalCompositeOperation = this.curContext.bmpMode;
            throw(e);
        }
    }


    // This is called when we get a draw line command
    private drawLine(textMsg : Object) {

        var saveStyle = this.drawCtx.strokeStyle;
        try {
            this.drawCtx.strokeStyle = textMsg[RIVAProto.kAttr_Color];
            var fromPnt : BaseTypes.RIVAPoint = new BaseTypes.RIVAPoint(textMsg[RIVAProto.kAttr_FromPnt]);
            var toPnt : BaseTypes.RIVAPoint = new BaseTypes.RIVAPoint(textMsg[RIVAProto.kAttr_ToPnt]);

            this.drawCtx.beginPath();
            this.drawCtx.moveTo(fromPnt.x, fromPnt.y);
            this.drawCtx.lineTo(toPnt.x, toPnt.y);
            this.drawCtx.stroke();

            this.drawCtx.strokeStyle = saveStyle;
        }

        catch(e) {
            this.drawCtx.strokeStyle = saveStyle;
            throw(e);
        }
    }



    //
    //  This is called to draw text in the normal (non-FX) sort of way, and for single
    //  line text. There is a separate one for multi-line text.
    //
    private drawText(textMsg : Object) {

        var toDraw : string = textMsg[RIVAProto.kAttr_ToDraw];
        var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(textMsg[RIVAProto.kAttr_TarArea]);
        var hJust : RIVAProto.HJustifys = <RIVAProto.HJustifys>textMsg[RIVAProto.kAttr_HJustify];
        var vJust : RIVAProto.VJustifys = <RIVAProto.VJustifys>textMsg[RIVAProto.kAttr_VJustify];

        //
        //  In this case, we want to avoid having to redo the clip area just for this one
        //  thing. So we will do a save/restore and just directly add the target area to the
        //  clip path. The areas are never rounded in this case. We know any existing clip
        //  stack is set since we are in a graphics operation.
        //
        this.drawCtx.save();
        this.setRectPath(this.drawCtx, tarArea, true, 1, 1.5);
        this.drawCtx.clip();
        try {

            // Make sure the current font style is set
            this.drawCtx.font = this.curFont.canvasFmt;

            // Make sure the curent context text color is set.
            this.drawCtx.fillStyle = this.curContext.textColor;

            // Call the single line helper, in fill mode
            this.drawSingleLineText
            (
                toDraw, this.drawCtx, tarArea, hJust, vJust, false, null, null
            );

            this.drawCtx.restore();
        }

        catch(e) {
            this.drawCtx.restore();
            throw(e);
        }
    }


    //
    //  This is called when we get a command draw F/X style text. There's a lot going
    //  on for this one, vastly complicated by the fact that we have to handle multi-
    //  line text ourself.
    //
    private drawTextFX(textMsg : Object) {

        // Convert the formated area to an area object
        var toDraw : string = textMsg[RIVAProto.kAttr_ToDraw];
        var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(textMsg[RIVAProto.kAttr_TarArea]);
        var fxType : RIVAProto.TextFXs = <RIVAProto.TextFXs>textMsg[RIVAProto.kAttr_Effect];
        var pntOfs : BaseTypes.RIVAPoint = new BaseTypes.RIVAPoint(textMsg[RIVAProto.kAttr_PntOffset]);
        var hJust : RIVAProto.HJustifys = <RIVAProto.HJustifys>textMsg[RIVAProto.kAttr_HJustify];
        var vJust : RIVAProto.VJustifys = <RIVAProto.VJustifys>textMsg[RIVAProto.kAttr_VJustify];
        var flags : number = textMsg[RIVAProto.kAttr_Flags];

        //
        //  In this case, we want to avoid having to redo the clip area just for this one
        //  thing. So we will do a save/restore and just directly add the target area to the
        //  clip path. The areas are never rounded in this case. This also means we don't
        //  need to push our context, since we'll end up not changing anything.
        //
        this.drawCtx.save();
        this.setRectPath(this.drawCtx, tarArea, true, 1, 1.5);
        this.drawCtx.clip();
        try {

            // Make sure the current font style is set
            this.drawCtx.font = this.curFont.canvasFmt;

            // Depending on the F/X type we do different things
            switch(fxType)
            {
                case RIVAProto.TextFXs.DropShadow :
                    //
                    //  This one is relatively normal. We set the canvas for drop shadow
                    //  style and just draw the text, adjusting the position to account
                    //  for the shadow offset, which can be negative or positive in either
                    //  axis.
                    //
                    this.drawCtx.shadowBlur = 6;
                    this.drawCtx.shadowOffsetX = pntOfs.x;
                    this.drawCtx.shadowOffsetY = pntOfs.y;
                    this.drawCtx.shadowColor  = textMsg[RIVAProto.kAttr_Color2];

                    // This uses the currently set fill color
                    this.drawCtx.fillStyle = textMsg[RIVAProto.kAttr_Color];

                    //
                    //  Adjust the target area to account for the drop shadow and the
                    //  justification, since the text itself has to be bring inwards
                    //  so that the shadow ends up being what is justified.
                    //
                    var realArea : BaseTypes.RIVAArea = tarArea;
                    if (pntOfs.x !== 0)
                        realArea.deflate(Math.abs(pntOfs.x), 0);

                    if (pntOfs.y !== 0)
                        realArea.deflate(0, Math.abs(pntOfs.y))

                    // Adjust the target a bit more for the blur size
                    if (hJust === RIVAProto.HJustifys.Left)
                        realArea.x += 3;
                    else if (hJust === RIVAProto.HJustifys.Right)
                        realArea.x -= 3;

                    if (vJust === RIVAProto.VJustifys.Top)
                        realArea.y += 3;
                    else if (vJust === RIVAProto.VJustifys.Bottom)
                        realArea.y -= 3;

                    if (flags & RIVAProto.kTextFXFlag_NoWrap)
                        this.drawSingleLineText
                        (
                            toDraw, this.drawCtx, realArea, hJust, vJust, false, null, null
                        );
                    else
                        this.drawMultiLineText(toDraw, this.drawCtx, realArea, hJust, vJust, false);
                    break;

                case RIVAProto.TextFXs.GaussianBlur :
                    break;

                case RIVAProto.TextFXs.Gradient :
                    // Then draw the text in the fill style, passing along the gradient colors
                    if (flags & RIVAProto.kTextFXFlag_NoWrap)
                        this.drawSingleLineText
                        (
                            toDraw,
                            this.drawCtx,
                            tarArea,
                            hJust,
                            vJust,
                            false,
                            textMsg[RIVAProto.kAttr_Color],
                            textMsg[RIVAProto.kAttr_Color2]
                        );
                    else
                        this.drawMultiLineText(toDraw, this.drawCtx, tarArea, hJust, vJust, false);
                    break;

                case RIVAProto.TextFXs.GradientRefl :
                    break;

                default :
                    throw(Error("Unknown text F/X type"));
            };

            this.drawCtx.restore();
        }

        catch(e) {
            this.drawCtx.restore();
            throw(e);
        }
    }


    //
    //  There's no means to do multi-line text in the Canvas API, so we have to do it
    //  ourself.
    //
    private drawMultiText(textMsg : Object) {

        // Get the text to draw
        var toDraw : string = textMsg[RIVAProto.kAttr_ToDraw];

        // Convert the formatted area to an area object
        var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(textMsg[RIVAProto.kAttr_TarArea]);

        // This uses the currently set fill color. Don't need to restore since it's the current value
        this.drawCtx.fillStyle = this.curContext.textColor;

        //
        //  In this case, we want to avoid having to redo the clip area just for this one
        //  thing. So we will do a save/restore and just directly add the target area to the
        //  clip path. The areas are never rounded in this case. This also means we don't
        //  need to push our context, since we'll end up not changing anything.
        //
        this.drawCtx.save();
        this.setRectPath(this.drawCtx, tarArea, true, 1, 1.5);
        this.drawCtx.clip();
        try {
            // Make sure the current font style is set
            this.drawCtx.font = this.curFont.canvasFmt;

            this.drawMultiLineText
            (
                toDraw,
                this.drawCtx,
                tarArea,
                <RIVAProto.HJustifys>textMsg[RIVAProto.kAttr_HJustify],
                <RIVAProto.VJustifys>textMsg[RIVAProto.kAttr_VJustify],
                false
            );

            this.drawCtx.restore();
        }

        catch(e) {
            this.drawCtx.restore();
            throw(e);
        }
    }


    //
    //  This is called to draw a progress bar. We get the basic image, and we
    //  have to use that to create the masked image, and then draw as much of
    //  that as required to indicate the current percentage value, filling the
    //  rest with the background color.
    //
    //  We have to use a separate canvas to do this, then we'll blit it back.
    //
    private drawPBar(pbarMsg : Object, imgCache : ImgCache.ImgCache) {

        // Get the image from the image cache. If not found, we do nothing
        var imgPath : string = pbarMsg[RIVAProto.kAttr_Path];
        var toDraw : HTMLImageElement = imgCache.findImg(imgPath);
        if (!toDraw)
            return;

        // Get the source and target areas, and the percentage value
        var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(pbarMsg[RIVAProto.kAttr_TarArea]);
        var srcArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(pbarMsg[RIVAProto.kAttr_SrcArea]);
        var pbPercent : number = pbarMsg[RIVAProto.kAttr_Percent];

        // Use the temp buffering canvas for the fill
        var dbufFill : HTMLCanvasElement = this.getDBufCanvas(srcArea.size());
        var fillCtx : CanvasRenderingContext2D = dbufFill.getContext("2d");

        //
        //  Now get the two colors and do a gradient fill of the area. We have to convert
        //  the generic direction enum to one of our gradient enums and flip the colors
        //  as required. And we need to figure out the area that we need to fill, based
        //  on color and direction.
        //
        var pbDir : RIVAProto.Dirs = <RIVAProto.Dirs>pbarMsg[RIVAProto.kAttr_Dir];
        var clr1 : string;
        var clr2 : string;
        var perArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(null);
        var fillArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(null);

        var fillDir : RIVAProto.GradDirs;
        switch(pbDir) {
            case RIVAProto.Dirs.Down :
                fillDir = RIVAProto.GradDirs.Vertical;
                clr1 = pbarMsg[RIVAProto.kAttr_Color];
                clr2 = pbarMsg[RIVAProto.kAttr_Color2];
                break;

            case RIVAProto.Dirs.Left :
                fillDir = RIVAProto.GradDirs.Horizontal;
                clr1 = pbarMsg[RIVAProto.kAttr_Color];
                clr2 = pbarMsg[RIVAProto.kAttr_Color2];
                break;

            case RIVAProto.Dirs.Right :
                fillDir = RIVAProto.GradDirs.Horizontal;
                clr2 = pbarMsg[RIVAProto.kAttr_Color];
                clr1 = pbarMsg[RIVAProto.kAttr_Color2];
                break;

            case RIVAProto.Dirs.Up :
                fillDir = RIVAProto.GradDirs.Vertical;
                clr2 = pbarMsg[RIVAProto.kAttr_Color];
                clr1 = pbarMsg[RIVAProto.kAttr_Color2];
                break;

            default :
                throw(Error("Unknown Dir value"));
        };

        // Do two optimizations for 0 and 100%, else the general scenario
        if (pbPercent === 0) {
            fillArea.width = srcArea.width;
            fillArea.height = srcArea.height;
        } else if (pbPercent === 100) {
            perArea.width = srcArea.width;
            perArea.height = srcArea.height;
        } else {
            switch(pbDir) {
                case RIVAProto.Dirs.Down :
                    // Percent area goes from bottom, up by percentage
                    perArea.width = srcArea.width;
                    perArea.height = Math.floor(srcArea.height * pbPercent);
                    perArea.y = srcArea.bottom() - perArea.height;

                    // Fill area gets the reset
                    fillArea.width = srcArea.width;
                    fillArea.height = srcArea.height - perArea.height;
                    break;

                case RIVAProto.Dirs.Left :
                    // Percent area goes from left over by percentage amount
                    perArea.height = srcArea.height;
                    perArea.width = Math.floor(srcArea.width * pbPercent);

                    // Fill area gets the rest
                    fillArea.height = srcArea.height;
                    fillArea.x = perArea.right() + 1;
                    fillArea.width = srcArea.width  - perArea.width;
                    break;

                case RIVAProto.Dirs.Right :
                    // Percent area goes from right back by percentage amount
                    perArea.height = srcArea.height;
                    perArea.width = Math.floor(srcArea.width * pbPercent);
                    perArea.x = srcArea.right() - perArea.width;

                    fillArea.height = srcArea.height;
                    fillArea.width = srcArea.width  - perArea.width;
                    break;

                case RIVAProto.Dirs.Up :
                    // Percent area goes from top, down by percentage amount
                    perArea.width = srcArea.width;
                    perArea.height = Math.floor(srcArea.height * pbPercent);

                    // Fill area gets the reset
                    fillArea.width = srcArea.width;
                    fillArea.y = perArea.bottom() + 1;
                    fillArea.height = srcArea.height - perArea.height;
                    break;

                default :
                    throw(Error("Unknown Dir value"));
            };
        }

        this.pushContext();
        try {
            // Gradient fill the percent area if not 0%
            // fillCtx.globalAlpha = 1.0;
            // this.drawCtx.globalAlpha = 1.0;

            // And do a regular fill on the fill area if not 100%
            if (pbPercent !== 100) {

                fillCtx.clearRect(0, 0, srcArea.width, srcArea.height);
                fillCtx.globalCompositeOperation = "source-over";
                fillCtx.drawImage(toDraw, 0, 0);
                fillCtx.globalCompositeOperation = "source-in";
                fillCtx.fillStyle = pbarMsg[RIVAProto.kAttr_BgnColor];
                fillCtx.fillRect(fillArea.x, fillArea.y, fillArea.width, fillArea.height);

                this.drawCtx.drawImage
                (
                    dbufFill,
                    fillArea.x,
                    fillArea.y,
                    fillArea.width,
                    fillArea.height,
                    tarArea.x + fillArea.x,
                    tarArea.y + fillArea.y,
                    fillArea.width,
                    fillArea.height
                );
            }

            if (pbPercent !== 0) {

                fillCtx.clearRect(0, 0, srcArea.width, srcArea.height);
                fillCtx.globalCompositeOperation = "source-over";
                fillCtx.drawImage(toDraw, 0, 0);
                fillCtx.globalCompositeOperation = "source-in";
                this.doGradientFill(fillCtx, fillDir, perArea, clr1, clr2);

                this.drawCtx.drawImage
                (
                    dbufFill,
                    perArea.x,
                    perArea.y,
                    perArea.width,
                    perArea.height,
                    tarArea.x + perArea.x,
                    tarArea.y + perArea.y,
                    perArea.width,
                    perArea.height
                );
            }

            this.popContext();
        }

        catch(e) {
            this.popContext();
            throw(e);
        }
    }


    //
    //  This is called to do an area fill. We inflate the area by one because the canvas
    //  doesn't include the boundary while the IV assumes it does.
    //
    private fillArea(fillMsg : Object) {

        var saveStyle = this.drawCtx.fillStyle;
        try {
            this.drawCtx.fillStyle = fillMsg[RIVAProto.kAttr_Color];
            var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(fillMsg[RIVAProto.kAttr_TarArea]);

            var rounding = fillMsg[RIVAProto.kAttr_Rounding];
            if (rounding !== 0) {

                // We have to do rounded ones, so set the rounded path and then fill it
                this.setRoundedPath(this.drawCtx, tarArea, rounding, true, 0.5, 0.5);
                this.drawCtx.fill();

            } else {
                this.setRectPath(this.drawCtx, tarArea, true, 0.5, 0.5);
                this.drawCtx.fill();
            }
            this.drawCtx.fillStyle = saveStyle;
        }

        catch(e) {
            this.drawCtx.fillStyle = saveStyle;
            throw(e);
        }
    }


    //
    //  This guy does a tiled blit of an image, to fill the passed area. We get a point that
    //  represents the origin point in the pattern, but currently it's not used. The target
    //  area is the whole area to fill, since we need to know where the fill starts. We just
    //  allow clipping to get rid of any that aren't needed.
    //
    private fillWithBmp(fillMsg : Object, imgCache : ImgCache.ImgCache) {

        // Get the image from the image cache. If not found, we do nothing
        var imgPath : string = fillMsg[RIVAProto.kAttr_Path];
        var toDraw : ImgCache.ImgCacheItem = imgCache.findItem(imgPath);
        if (!toDraw)
            return;

        // Get the target area to fill
        var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(fillMsg[RIVAProto.kAttr_TarArea]);

        //
        //  Adjust it to account for canvas silliness, for reasons unknown. It only seems to
        //  be required on IE, but doesn't hurt any others.
        //
        tarArea.x -= 0.5;
        tarArea.y -= 1;
        tarArea.width += 1;
        tarArea.height += 1;

        //
        //  In this case, we want to avoid having to redo the clip area just for this one
        //  thing. So we will do a save/restore and just directly add the target area to the
        //  clip path. The areas are never rounded in this case. This also means we don't
        //  need to push our context, since we'll end up not changing anything.
        //
        this.drawCtx.save();
        this.setRectPath(this.drawCtx, tarArea, true, 1.5, 1.5);
        this.drawCtx.clip();
        try {

            // Start out at the origin of the target area
            var curOrg : BaseTypes.RIVAPoint = new BaseTypes.RIVAPoint(null);
            curOrg.set(tarArea.x, tarArea.y);

            // Do a double loop until the current origin Y ends up past the target area bottom
            var endBottom : number = tarArea.bottom();
            var endRight : number = tarArea.right();

            // These are never alpha based or have transparency, so just a simple copy
            while (curOrg.y <= endBottom) {
                curOrg.x = tarArea.x;
                while (curOrg.x <= endRight) {
                    this.drawCtx.drawImage(toDraw.imgElem, curOrg.x, curOrg.y);
                    curOrg.x += toDraw.imgRes.width;
                }
                curOrg.y += toDraw.imgRes.height;
            }

            this.drawCtx.restore();
        }

        catch(e) {
            this.drawCtx.restore();
            throw(e);
        }
    }


    //
    //  This is called to do a gradient area fill. The IV supports four directions, which
    //  are easy to convert to the two points needed for the canvas.
    //
    //  The message includes a rounding value, but the IV engine never uses that because
    //  the CIDGraphDev graphics device interface doesn't support it. The IV engine just
    //  sets a rounded clip and does a fill for now.
    //
    private gradientFill(fillMsg : Object) {
        // Get the target area
        var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(fillMsg[RIVAProto.kAttr_TarArea]);

        // Convert the direction
        var fillDir : RIVAProto.GradDirs = <RIVAProto.GradDirs>fillMsg[RIVAProto.kAttr_Dir];

        // Call the private helper that does the actual work
        this.doGradientFill
        (
            this.drawCtx,
            fillDir,
            tarArea,
            fillMsg[RIVAProto.kAttr_Color],
            fillMsg[RIVAProto.kAttr_Color2]
        );
    }


    // Stroke an area, handing rounding if needed
    strokeArea(msgData : Object) {

        var saveLine = this.drawCtx.lineWidth;
        var saveStyle = this.drawCtx.strokeStyle;

        try {
            // We have to add half a pixel to get appropriate results
            this.drawCtx.lineWidth = msgData[RIVAProto.kAttr_Width] + 0.5;
            this.drawCtx.strokeStyle = msgData[RIVAProto.kAttr_Color];

            var tarArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea(msgData[RIVAProto.kAttr_TarArea]);
            var rounding = msgData[RIVAProto.kAttr_Rounding];
            if (rounding !== 0) {

                // Set the rounded path and then we can stroke it
                this.setRoundedPath(this.drawCtx, tarArea, rounding, true);
                this.drawCtx.stroke();

            } else {
                this.setRectPath(this.drawCtx, tarArea, true);
                this.drawCtx.stroke();
            }

            this.drawCtx.lineWidth = saveLine;
            this.drawCtx.strokeStyle = saveStyle;
        }

        catch(e) {
            this.drawCtx.lineWidth = saveLine;
            this.drawCtx.strokeStyle = saveStyle;
            throw(e);
        }
    }


    // ------------------------------------------------
    //  Helper methods used by the other drawing stuff above. None of these will do any
    //  clipping or push/pop the context, assuming that the caller does anything that
    //  is needed.
    // ------------------------------------------------
    private doGradientFill( tarCtx : CanvasRenderingContext2D,
                            fillDir : RIVAProto.GradDirs,
                            tarArea : BaseTypes.RIVAArea,
                            clr1 : string,
                            clr2 : string) {

        var saveStyle = tarCtx.fillStyle;
        try {
            // Depending on the direction, set up the two points
            var newGrad : CanvasGradient = null;
            switch(fillDir) {
                case RIVAProto.GradDirs.DownDiagonal :
                    newGrad = tarCtx.createLinearGradient
                    (
                        tarArea.x, tarArea.y, tarArea.right(), tarArea.bottom()
                    );
                    break;

                case RIVAProto.GradDirs.Horizontal :
                    newGrad = tarCtx.createLinearGradient
                    (
                        tarArea.x, tarArea.y, tarArea.right(), tarArea.y
                    );
                    break;

                case RIVAProto.GradDirs.UpDiagonal :
                    newGrad = tarCtx.createLinearGradient
                    (
                        tarArea.x, tarArea.bottom(), tarArea.right(), tarArea.y
                    );
                    break;

                case RIVAProto.GradDirs.Vertical :
                    newGrad = tarCtx.createLinearGradient
                    (
                        tarArea.x, tarArea.y, tarArea.x, tarArea.bottom()
                    );
                    break;

                default :
                    throw(Error("Unknown gradient fill direction"));
            };

            // Add the two color stops required
            newGrad.addColorStop(0, clr1);
            newGrad.addColorStop(1, clr2);

            // Set the gradient and do the fill
            tarCtx.fillStyle = newGrad;
            tarCtx.fillRect(tarArea.x, tarArea.y, tarArea.width, tarArea.height);

            tarCtx.fillStyle = saveStyle;
        }

        catch(e) {
            tarCtx.fillStyle = saveStyle;
            throw(e);
        }
    }



    //
    //  The canvas doesn't support multi-line text, so we have to do it ourself. And
    //  this is greatly complicated by the fact that we have to deal with multi-line
    //  text in a variable of scenarios. Sometimes we need to draw it, sometimes we
    //  need to make it into a clip path that we can fill, sometimes we need to line
    //  stroke it. Sometimes we might need to draw it into a separate canvas for
    //  manipulation and we'll blit it back later.
    //
    //  So this helper method does the grunt work of either filling or stroking the
    //  text into a target area.
    //
    //  The caller has to:
    //
    //  1. Set any clip area he wants
    //  2. Set the fill style if we are drawing
    //  3. Set the stroke style if outlining
    //  4. Set the font that is to be used
    //
    //  We return the ultimate area of the text, some of which of course may be off
    //  of the target area and hence clipped. The horziontal is always just the target
    //  area. it's only the vertical that is adjusted.
    //
    drawMultiLineText(  toDraw      : string,
                        tarCtx      : CanvasRenderingContext2D,
                        tarArea     : BaseTypes.RIVAArea,
                        hJust       : RIVAProto.HJustifys,
                        vJust       : RIVAProto.VJustifys,
                        strokeIt    : boolean) {

        //
        //  Optimize if the string is empty. Otherwise, we will have words, since deal with
        //  white space as well.
        //
        if (toDraw.length === 0)
            return;

        //
        //  Split the input into separate words. But, we have to keep the actual spaces as
        //  words as well, so we can't just do split(). The spaces are important. And we need
        //  to treat new lines as separate words, and expand tabs to spaces.
        //
        var words : Array<string> = new Array<string>();
        var curWord : string;
        var charInd : number = 0;
        var charCnt  : number = toDraw.length;
        while (charInd < charCnt) {

            // Get the first character of this round
            var curChar : number = toDraw.charCodeAt(charInd++);

            // Based on what it is, we know what to do next
            if (curChar === 0xD) {

                // The next char should be LF, so eat that and push them both
                charInd++;
                words.push("\r\n");
            } else if (curChar === 0x9) {

                // Push four spaces
                words.push("    ");

            } else if (curChar === 0x20) {

                // Make this and any subsequent spaces a word
                curWord = " ";
                while (charInd < charCnt) {
                    curChar = toDraw.charCodeAt(charInd);
                    if (curChar != 0x20)
                        break;

                    curWord += " ";
                    charInd++;
                }
                words.push(curWord);

            } else {

                // Make any subsequent non-spaces a word
                curWord = String.fromCharCode(curChar);
                while (charInd < charCnt) {
                    curChar = toDraw.charCodeAt(charInd);
                    if ((curChar === 0x20) || (curChar === 0xD) || (curChar === 0x9))
                        break;

                    curWord += String.fromCharCode(curChar);
                    charInd++;
                }
                words.push(curWord);
            }
        }

        var areaWidth = tarArea.width;
        var leftPos : number = tarArea.x;
        tarCtx.textAlign = "left";
        tarCtx.textBaseline = "top";

        // Now measure all of the words
        var wordLens : Array<number> = new Array<number>();
        for (var wInd : number = 0; wInd < words.length; wInd++)
            wordLens.push(tarCtx.measureText(words[wInd]).width);

        //
        //  We have to break it into lines, because we need to position the whole
        //  thing vertically. So we need to now how many lines we have before we
        //  start. We remember the lengths so we don't have to do them again when
        //  we draw.
        //
        var curWidth : number = 0;
        var lines : Array<string> = new Array<string>();
        var lineLens : Array<number> = new Array<number>();
        var totalLineHeight : number = 0;
        {
            var curLine : string;
            var curWordInd : number = 0;
            var curWordWidth : number = 0;
            var spaceWidth : number = tarCtx.measureText(' ').width;
            while (curWordInd < words.length) {

                // Let's accumulate a line's worth of text
                curLine = "";
                curWidth = 0;
                while (curWordInd < words.length)
                {
                    var curWord = words[curWordInd];

                    // If a new line, then break out
                    if (curWord === "\r\n") {
                        curWordInd++
                        break;
                    }

                    // Get the current word width
                    var testWidth = wordLens[curWordInd];

                    //
                    //  If that is beyond the width, then we can't do this word, unless it's
                    //  the only word on this line (though maybe clipped due to length.) If
                    //  we didn't force at least one word, we'd never move forward and get
                    //  caught in a loop.
                    //
                    if ((curWidth + testWidth > areaWidth) && (curWidth != 0))
                        break;

                    curLine += curWord;
                    curWidth += testWidth;

                    curWordInd++;
                }

                totalLineHeight += this.curFontHeight;
                lines.push(curLine);
                lineLens.push(curWidth);
            }
        }

        //
        //  Calculate where the first line will go. It may be up above the target
        //  area, which is fine. We'll ignore those when we draw below.
        //
        var curYPos : number = 0;
        var bottomPos : number = tarArea.bottom() + 1;
        switch(vJust) {
            case RIVAProto.VJustifys.Top :
                // We just start at the top of the area
                curYPos = tarArea.y;
                break;

            case RIVAProto.VJustifys.Center :
                // Calculate half the full text height, minus the mid point of the area
                curYPos = tarArea.vCenter() - (totalLineHeight / 2);
                break;

            case RIVAProto.VJustifys.Bottom :
                // The bottom of the area minus the total line height
                curYPos = bottomPos - totalLineHeight;
                break;

            default :
                alert("Unknown vertical text justification");
                break;
        };

        //
        //  Now let's loop through the lines and draw them. We will just move down
        //  one text height each time, and position the line correcty in the
        //  horizontal direction.
        //
        var curXPos : number;
        var topPos : number = tarArea.y;
        for (var lineInd : number = 0; lineInd < lines.length; lineInd++) {

            // If this line is fully above the top, then skip it. Else draw it
            if (curYPos + this.curFontHeight > topPos) {

                switch(hJust) {
                    case RIVAProto.HJustifys.Left :
                        curXPos = tarArea.x;
                        break;

                    case RIVAProto.HJustifys.Center :
                        curXPos = tarArea.hCenter() - (lineLens[lineInd] / 2);
                        break;

                    case RIVAProto.HJustifys.Right :
                        curXPos = tarArea.right() - lineLens[lineInd];
                        break;

                    default :
                        alert("Unknown horizontal text justification");
                        break;
                };

                // Either stroke or fill
                if (strokeIt)
                    tarCtx.strokeText(lines[lineInd], curXPos, curYPos);
                else
                    tarCtx.fillText(lines[lineInd], curXPos, curYPos);
            }

            // Move down one text height
            curYPos += this.curFontHeight;

            // If we have now gone beyond the bottom, we can stop
            if (curYPos > bottomPos)
                break;
        }
    }


    //
    //  As above for multi-line text, this is a helper to handle the various types
    //  of scenarios where we need to draw single line text. Sometimes to draw it,
    //  sometimes to stroke it. The caller provides a context to draw into, which
    //  may be ours or may be one used for double   buffering.
    //
    //  The caller must:
    //
    //  1. Set any clip area he wants
    //  2. Set the fill style if we are drawing
    //  3. Set the stroke style if outlining
    //  4. Set the font that is to be used
    //
    //  They can give us two gradient colors optionally. If they are not null, then we
    //  will apply that gradient to where the actual text ended up being drawn.
    //
    drawSingleLineText( toDraw      : string,
                        tarCtx      : CanvasRenderingContext2D,
                        tarArea     : BaseTypes.RIVAArea,
                        hJust       : RIVAProto.HJustifys,
                        vJust       : RIVAProto.VJustifys,
                        strokeIt    : boolean,
                        gradClr1    : string,
                        gradClr2    : string) {

        var saveAlign = tarCtx.textAlign;
        var saveBaseline = tarCtx.textBaseline;
        var saveStyle = strokeIt ? tarCtx.strokeStyle : tarCtx.fillStyle;

        try {
            //
            //  For horizontal, we start wtih the left area position, and we always draw
            //  left aligned. We'll adjust the left position as needed.
            //
            var leftPos : number = tarArea.x;
            tarCtx.textAlign = "left";
            switch(hJust) {
                case RIVAProto.HJustifys.Left :
                    // It's already OK
                    break;

                case RIVAProto.HJustifys.Center :
                    var textMtr : TextMetrics = tarCtx.measureText(toDraw);
                    leftPos = tarArea.hCenter() - (Math.floor(textMtr.width) / 2);
                    break;

                case RIVAProto.HJustifys.Right :
                    var textMtr : TextMetrics = tarCtx.measureText(toDraw);
                    leftPos = tarArea.right() - (Math.floor(textMtr.width) + 1);
                    break;

                default :
                    alert("Unknown horizontal text justification");
                    break;
            };

            //
            //  We always draw with the baseline set to middle and adjust the y
            //  ourself, to put it where we want. We will adjust the bottom position
            //  as required.
            //
            tarCtx.textBaseline = "middle";
            var bottomPos : number;
            switch(vJust) {
                case RIVAProto.VJustifys.Top :
                    //
                    //  We have to set the bottom point to the top plus half the height of
                    //  the text.
                    //
                    bottomPos = tarArea.y + (this.curFontHeight / 2);
                    break;

                case RIVAProto.VJustifys.Center :
                    // The bottom position is vertical
                    bottomPos = tarArea.vCenter();
                    break;

                case RIVAProto.VJustifys.Bottom :
                    bottomPos = tarArea.bottom() - ((this.curFontHeight / 2) + 1);
                    break;

                default :
                    alert("Unknown vertical text justification");
                    break;
            };

            // If we have a gradient, then set it up for where the actual text went
            if (gradClr1 && gradClr2) {
                var gradFill : CanvasGradient = tarCtx.createLinearGradient
                (
                    tarArea.x,
                    bottomPos - (this.curFontHeight / 2),
                    tarArea.x,
                    bottomPos + (this.curFontHeight / 2)
                );
                gradFill.addColorStop(0, gradClr1);
                gradFill.addColorStop(1, gradClr2);

                if (strokeIt)
                    tarCtx.strokeStyle = gradFill;
                else
                    tarCtx.fillStyle = gradFill;
            }

            if (strokeIt)
                tarCtx.strokeText(toDraw, leftPos, bottomPos);
            else
                tarCtx.fillText(toDraw, leftPos, bottomPos);

            tarCtx.textAlign = saveAlign;
            tarCtx.textBaseline = saveBaseline;
            if (strokeIt)
                tarCtx.strokeStyle = saveStyle;
            else
                tarCtx.fillStyle = saveStyle;
        }

        catch(e) {
            tarCtx.textAlign = saveAlign;
            tarCtx.textBaseline = saveBaseline;
            if (strokeIt)
                tarCtx.strokeStyle = saveStyle;
            else
                tarCtx.fillStyle = saveStyle;
            throw(e);
        }
    }


    //
    //  These methods handle pushing and popping the clip stack. They will check to see
    //  if the change is important (see class comments above) and comit any outstanding
    //  operations (within the clipping regime they would have been drawn in.)
    //
    popClipStack() {

        try {
            var curTop : number = this.clipStack.length;
            if (curTop !== 0) {

                var prevArea = this.clipStack.pop();
                curTop--;

                //
                //  If the stack is empty now, or the previous area and the new top are
                //  not equal, then remember we need to rebuild it.
                //
                if ((this.clipStack.length === 0) || !prevArea.sameArea(this.clipStack[curTop - 1]))
                    this.clipPathRebuildNeeded = true;

            } else {
                throw Error("The clip area stack underflowed");
            }
        }

        catch(e) {
            if (this.doDebug)
                console.error("Exception in pop clip stack: " + e);
        }
    }


    private pushClipArea(newArea : BaseTypes.RIVAArea) {

        var copyArea : BaseTypes.RIVAArea = new BaseTypes.RIVAArea();
        copyArea.setFrom(newArea);
        this.clipStack.push(copyArea);

        //
        //  If the new area is different from the current stop of stack then set the
        //  rebuild needed flag.
        //
        var curTop : number = this.clipStack.length;
        if ((curTop === 1) || !this.clipStack[curTop - 2].sameArea(newArea))
            this.clipPathRebuildNeeded = true;
    }

    rebuildClipArea() {

        // Clear the rebuild needed flag now
        this.clipPathRebuildNeeded = false;

        //
        //  Do a restore to get rid of any previous clip area, and save again. Re-apply
        //  the current context and font, since this will whack it.
        //
        this.drawCtx.restore();
        this.drawCtx.save();
        this.curContext.applyTo(this.drawCtx);
        this.drawCtx.font = this.curFont.canvasFmt;

        // While doing this, we untranslate
        this.drawCtx.translate(-0.5, -0.5);

        //
        //  Now we just apply them in sequence. The default is that they combine according
        //  to their overlapping areas, which is what we want.
        //
        //  The clip paths are non-inclusive so we inflate it, plus we add an extra 0.5 to
        //  the lower/right sides because it seems required to avoid issues.
        //
        for (var index = 0; index < this.clipStack.length; index++) {
            var curArea = this.clipStack[index];
            if (curArea.rounding !== 0)
                this.setRoundedPath(this.drawCtx, curArea, curArea.rounding, true, 1, 1.5);
            else
                this.setRectPath(this.drawCtx, curArea, true, 1, 1.5);
            this.drawCtx.clip();
        }

        // Now restore the translation
        this.drawCtx.translate(0.5, 0.5);
    }

    //
    //  Pops our context stack, storing the popped value as our current context, and
    //  applies the stored settings to the context.
    //
    popContext() {
        this.curContext = this.ctxStack.pop();
        this.curContext.applyTo(this.drawCtx);

        // Make sure we've kept our stacks in sync
        if (this.fontStack.length != this.curContext.fontStackSize) {
            alert("Font stack size mismatch on pop");
        }

        if (this.clipStack.length != this.curContext.clipStackSize) {
            alert("Clip stack size mismatch on pop");
        }
    }

    // Pushes a copy of our current context onto the context stack
    pushContext() {
        // Push a copy of our current context object
        var newCtx : GraphCtx = new GraphCtx(this.curContext);

        //
        //  Save the current size of the other stacks so that we can verify
        //  later upon pop that correct nesting has been maintained.
        //
        newCtx.fontStackSize = this.fontStack.length;
        newCtx.clipStackSize = this.clipStack.length;
        this.ctxStack.push(newCtx);
    }

    // Called when we get a push font command
    private pushFont(fontMsg : Object) {

        // Push the current font
        this.fontStack.push(this.curFont);

        // Create a new font object from the incoming msg and store as the new font
        this.curFont = new FontInfo(fontMsg);

        //
        //  Set our inter-line spacing, which we set to two larger than the set height
        //  to better match Windows' output.
        //
        this.curFontHeight = this.curFont.height + 2;

        // Set the new font styles on the canvas
        this.drawCtx.font = this.curFont.canvasFmt;
    }

    //
    //  (re)sets our graphical context info to defaults. We set up our context info
    //  and get a new context object to insure we start with a clean slate.
    //
    resetContext() {
        this.clipStack.length = 0;
        this.fontStack.length = 0;
        this.ctxStack.length = 0;

        // Create our display and drawing contexts from their respective canvi
        this.drawCtx = this.drawSurface.getContext("2d", { alpha : false });
        this.showCtx = this.showSurface.getContext("2d", { alpha : false });

        // We want image smoothing off
        this.drawCtx.imageSmoothingEnabled = false;

        // Default other things
        this.drawCtx.lineWidth = 1;

        //
        //  Now, get around the stupid way the canvas deals with lines. NOTE that we don't
        //  do the show context because our blits will go to the translated position in
        //  his coordinate system, so it will already be translated.
        //
        this.drawCtx.translate(0.5, 0.5);

        // Store a default font
        this.curFont = new FontInfo(null);
        this.drawCtx.font = this.curFont.canvasFmt;
        this.curFontHeight = this.curFont.height;

        // Reset our current context to be a new initial one
        this.curContext.reset();

        // Do an initial save of the context now
        this.drawCtx.save();
    }


    // Set a path around an area.
    setRectPath(tarCtx : CanvasRenderingContext2D,
                tarRect : BaseTypes.RIVAArea,
                doPath : boolean,
                offsetUL : number = 0,
                offsetLR : number = 0) {

        var x = tarRect.x - offsetUL;
        var y = tarRect.y - offsetUL;
        var right = tarRect.right() + offsetLR;
        var bottom = tarRect.bottom() + offsetLR;

        if (doPath)
            tarCtx.beginPath();
        tarCtx.moveTo(x, y);
        tarCtx.lineTo(right, y);
        tarCtx.lineTo(right, bottom);
        tarCtx.lineTo(x, bottom);
        tarCtx.lineTo(x, y);
        if (doPath)
            tarCtx.closePath();
    }


    //
    //  The canvas doesn't support rounded rectanges, so we do it ourself. We need
    //  to do this for both stroking and filling, so we split this out. This sets
    //  the path. They caller can ask us to begin/close path around it or they can
    //  use as part of a larger path.
    //
    setRoundedPath( tarCtx      : CanvasRenderingContext2D,
                    tarArea     : BaseTypes.RIVAArea,
                    rounding    : number,
                    doPath      : boolean,
                    offsetUL    : number = 0,
                    offsetLR    : number = 0) {

        // We have to implement this ourself
        if (tarArea.width < 2 * rounding)
            rounding = tarArea.width / 2;
        if (tarArea.height < 2 * rounding)
            rounding = tarArea.height / 2;

        var x  = tarArea.x - offsetUL;
        var y= tarArea.y - offsetUL;
        var right = tarArea.right() + offsetLR;
        var bottom = tarArea.bottom() + offsetLR;

        if (doPath)
            tarCtx.beginPath();
        tarCtx.moveTo(x + rounding, y);
        tarCtx.arcTo(right, y, right, bottom, rounding);
        tarCtx.arcTo(right, bottom, x, bottom, rounding);
        tarCtx.arcTo(x, bottom, x, y, rounding);
        tarCtx.arcTo(x, y, right, y, rounding);
        if (doPath)
            tarCtx.closePath();
    }


    // Translate a RIVA bitmap mode into a canvas global composite mode
    xlatBmpMode(toXlat : RIVAProto.BmpModes) : string {

        var retVal : string;
        switch(toXlat) {
            case RIVAProto.BmpModes.SrcCopy :
                retVal = "source-over";
                break;

            case RIVAProto.BmpModes.DstInvert :
            case RIVAProto.BmpModes.PatCopy :
            case RIVAProto.BmpModes.PatInvert :
            case RIVAProto.BmpModes.SrcAnd :
            case RIVAProto.BmpModes.SrcErase :
            case RIVAProto.BmpModes.SrcPaint :

            default :
                console.error("Unknown bitmap mode");
                retVal = "source-over";
                break;
        };
        return retVal;
    }


    //
    //  Called to get our temp buffering canvas, faulting it in or upsizing it
    //  if needed.
    //
    getDBufCanvas(reqSize : BaseTypes.RIVASize) : HTMLCanvasElement {

        if (!this.tmpSurface ||
           (this.tmpSurface.width < reqSize.width) ||
           (this.tmpSurface.height < reqSize.height))
        {
            this.tmpSurface = document.createElement("canvas");

            // Set the size and the style size to match
            this.tmpSurface.width = reqSize.width;
            this.tmpSurface.height = reqSize.height;

            // And set the size of the canvas element to match
            this.tmpSurface.style.width = reqSize.width + "px";
            this.tmpSurface.style.height = reqSize.height + "px";
        }
        return this.tmpSurface;
    }
}

