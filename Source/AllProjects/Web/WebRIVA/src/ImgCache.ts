//
//  This file implements some helper classes used by the main app class to
//  maintain an image cache. For each image we have some sort of path, a serial
//  number, and the loaded image ready to draw.
//
//  We provide the following classes to do  this work:
//
//  1. An image cache item, which represents one image in the live image
//     cache data, i.e. one that has been loaded and is ready for drawing.
//  2. An image cache class, that provides the actual image cache, keyed
//     on the path discussed above.
//
//
//  Our images are always either PNG or JPEG. The server will tell us what
//  form the image is when it sends it. All the CQC image repo images are
//  PNG. But it will send media and web images as JPEGs to speed up the
//  transmission process and they were likely that form to begin with.
//
//  Each image is stored under a path which is in the form:
//
//      [passedpath]\[WxH]
//
//  The passed path is what we get from the server and it has a prefix that
//  indicates what type (repo, media, web, etc...) and then some unique id
//  for that image. We then add the width by height to it so that we can
//  easily deal with different resolutions of the same image.
//
//  In some cases the 'path' may just be some unique id. For instance, for
//  images the IV engine creates on the fly, such as color palettes, it
//  just generate a unique id and set it on the image, just so that we both
//  can have a way to know which image is being referred to in drawing cmds.
//  Those will have a prefix on them that indicates it's an on the fly image,
//  and they will be PNGs.
//
import * as BaseTypes from "./BaseTypes";
import * as RIVAProto from "./RIVAProto";


//
//  This class stores an image at a given resolution in the cache.
//
export class ImgCacheItem {

    // The name of a custom event we post when the load completes
    public static readonly CompleteEvName : string = "RIVAImageLoadComplete";

    //
    //  We assume the path already has the resolution added to it, as per the
    //  module comments above. We store the image res separately for
    //  later possible use.
    //
    constructor(imgPath : string,
                serialNum : number,
                imgRes : BaseTypes.RIVASize) {

        this.imgPath = imgPath;
        this.serialNum = serialNum;
        this.imgRes = imgRes;
        this.imgElem = null;
    }

    //
    //  Sets our bitmap from a base64 encoded version of it. This happens async, so
    //  the caller passes us a callback. When it succeeds. we will post a custom
    //  event.
    //
    //  We don't take a path here since the path was set in the constructor and never
    //  changes. This is only called to set the data for that path. However, the serial
    //  number could change. The image resolution is part of the path, so it would
    //  never change either.
    //
    //  If we fail, we null the image member back out.
    //
    setImageData(b64Img : string,
                serialNum : number,
                isPNG : boolean) {

        this.imgElem = new Image();
        this.imgElem.onload = () => {

            // It worked to store the new serial number
            this.serialNum = serialNum;

            // Post an event
            window.dispatchEvent
            (
                new CustomEvent(ImgCacheItem.CompleteEvName, { detail : this.imgPath })
            );
        }
        this.imgElem.onerror = (e) => {
            // Just null the image back out
            this.imgElem = null;
        }

        // Start the load process, indicating what type of image it is
        if (isPNG)
            this.imgElem.src = 'data:image/png;base64,' + b64Img;
        else
            this.imgElem.src = 'data:image/jpeg;base64,' + b64Img;
    }


    // The image information for this cache item
    public imgPath      : string;
    public serialNum    : number;
    public imgElem      : HTMLImageElement;
    public imgRes       : BaseTypes.RIVASize;

    //
    //  If the image uses color transparency, not common but they still exist, then
    //  this is something other than 0xFFFFFFFF.
    //
    public transClr     : number;
}


//
//  This class provides the actual cache, and is just a wrapper around a string
//  based lookup table of image cache items.
//
interface PathHashTable<T> {
    [imgPath : string] : T;
}

export class ImgCache {
    constructor() {

        // Create our object which acts as our lookup table
        this.imgCache = {};

        //
        //  Create an empty image we can set on any that fail. Don't bother to wait for
        //  it to load. It wouldn't ever display anything even if it's drawn correctly.
        //
        this.emptyImg = new Image();
        this.emptyImg.src = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNkYAAAAAYAAjCB0C8AAAAASUVORK5CYII=";
    }

    // Add a new item or update an existing item
    addImage(toAdd : ImgCacheItem) {

        // Add the image using its own path as the key
        this.imgCache[toAdd.imgPath] = toAdd;
    }


    // Find an image by its full path
    findImg(fullPath : string) : HTMLImageElement {

        var retVal : HTMLImageElement = null;
        try {
            var cacheItem : ImgCacheItem = this.imgCache[fullPath];
            if (cacheItem)
                retVal = cacheItem.imgElem;
        }

        catch(e) {
            // Just eat it and return our empty image
            return this.emptyImg;
        }

        return retVal;
    }

    // Find a cache item by its full path
    findItem(fullPath : string) : ImgCacheItem {

        var retVal : ImgCacheItem = null;
        try {
            retVal = this.imgCache[fullPath];

            // Make sure we return null, not undefined
            if (!retVal)
                retVal = null;
        }

        catch(e) {
            return null;
        }
        return retVal;
    }


    //
    //  Upon connection we send a session state message. Part of that is an array of
    //  images we have.
    //
    buildImgMapMsg(tarMsg : Object) {

        // Add an array to the message that we can start adding to
        var imgList = [];
        tarMsg[RIVAProto.kAttr_List] = imgList;

        // Then add an object to the array for each image
        var imgItem : Object = { };

        var keys : Array<string> = Object.keys(this.imgCache);
        for (var ind = 0; ind < keys.length; ind++) {
            var curImg : ImgCacheItem = this.imgCache[keys[ind]];

            var imgItem : Object = new Object;
            imgItem[RIVAProto.kAttr_Path] = curImg.imgPath;
            imgItem[RIVAProto.kAttr_SerialNum] = curImg.serialNum;
            imgList.push(imgItem);
        }
    }

    // This is the actual list of image cache items that make up the cache
    private imgCache : PathHashTable<ImgCacheItem>;

    // And empty image, we can return if we don't find the requested one
    private readonly emptyImg : HTMLImageElement;
}
