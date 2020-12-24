//
//  This file just provides some basic types that the other files need
//  to share.
//
import * as RIVAProto from "./RIVAProto";

export class RIVAArea {
    constructor(encodedArea : string = null) {
        if (encodedArea) {
            var vals : string[] = encodedArea.split(',');
            this.x = Number(vals[0]);
            this.y = Number(vals[1]);
            this.width = Number(vals[2]);
            this.height = Number(vals[3]);
        } else {
            this.x = 0;
            this.y = 0;
            this.width = 0;
            this.height = 0;
        }

        this.rounding = 0;
        this.clipMode = RIVAProto.ClipModes.And;
    }

    deflate(byX : number, byY : number) {
        this.x += byX;
        this.y += byY;
        this.width -= byX * 2;
        this.height -= byY * 2;

        if (this.width < 0)
            this.width = 0;
        if (this.height < 0)
            this.height = 0;
    }

    inflate(by : number) {
        this.x -= by;
        this.y -= by;
        this.width += by * 2;
        this.height += by * 2;
    }

    sameArea(toComp : RIVAArea) : boolean {
        return (
            (this.x  === toComp.x) &&
            (this.y  === toComp.y) &&
            (this.width === toComp.width) &&
            (this.height === toComp.height) &&
            (this.rounding === toComp.rounding)
        );
    }

    sameSize(toComp : RIVAArea) : boolean {
        return (this.width === toComp.width) && (this.height === toComp.height);
    }

    size() : RIVASize {
        var retVal : RIVASize = new RIVASize(null);
        retVal.width = this.width;
        retVal.height =this.height;
        return retVal;
    }


    bottom() : number {
        var retVal : number = (this.y + this.height);
        if (this.height !== 0)
            retVal -= 1;
        return retVal;
    }

    right() : number {
        var retVal : number = (this.x + this.width);
        if (this.width !== 0)
            retVal -= 1;
        return retVal;
    }

    hCenter() : number {
        return (this.x + Math.floor((this.width / 2)));
    }

    vCenter() : number {
        return (this.y + Math.floor((this.height / 2)));
    }

    set(x : number, y : number, width : number, height: number) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    setFrom(srcArea : RIVAArea) {
        this.x = srcArea.x;
        this.y = srcArea.y;
        this.width = srcArea.width;
        this.height = srcArea.height;
    }

    toText() : string {
        return "{" + this.x + "," + this.y + "," + this.width + "," + this.height +
                    ", round=" + this.rounding + "}";
    }

    public x : number;
    public y : number;
    public width : number;
    public height : number;

    // To support rounded rectangles
    public rounding : number;

    // For convenience to use these as our clipping stack elements as well
    public clipMode : RIVAProto.ClipModes;
}


export class RIVAPoint  {
    constructor(encodedPnt  : string = null) {
        if (encodedPnt) {
            var vals : string[] = encodedPnt.split(',');
            this.x = Number(vals[0]);
            this.y = Number(vals[1]);
        } else {
            this.x = 0;
            this.y = 0;
        }

    }

    samePoint(toComp : RIVAPoint) : boolean {
        return ((toComp.x === this.x) && (toComp.y === this.y));
    }

    set(x : number, y : number) {
        this.x = x;
        this.y = y;
    }

    setFrom(srcPnt : RIVAPoint) {
        this.x = srcPnt.x;
        this.y = srcPnt.y;
    }

    public x : number;
    public y : number;
}


export class RIVASize  {
    constructor(encodedSize  : string = null) {
        if (encodedSize) {
            var vals : string[] = encodedSize.split(',');
            this.width = Number(vals[0]);
            this.height = Number(vals[1]);
        } else {
            this.width = 0;
            this.height = 0;
        }
    }

    set(width : number, height : number) {
        this.width = width;
        this.height = height;
    }

    swap() {
        var tmp = this.width;
        this.width = this.height;
        this.height = tmp;
    }

    public width : number;
    public height : number;
}
