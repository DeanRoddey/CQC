//
// FILE NAME: CQCRemBrws_ThisFacility3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2018
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2020
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and 
//  the author (Dean Roddey.) It is licensed under the MIT Open Source 
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//  This file implements some methods that will take the various data server file
//  types and package them up in the TChunkedFile format in the way that the data
//  server (and the users of the data that get it from the data server) expects.
//  Keeping it here in one place is very important. This is also used by the conversion
//  code in the 2 version of this file, which is used by the installer.
//
//  Because it is used for conversions, we take an initial serial number to set on
//  the chunked file. It defaults to 1 so for creating new ones, just take that
//  default.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRemBrws_.hpp"


// ---------------------------------------------------------------------------
//  TFacCQCRemBrws: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TFacCQCRemBrws::ChunkUpEvMonitor(const  TCQCEvMonCfg&       emoncSrc
                                ,       TChunkedFile&       chflToFill
                                , const tCIDLib::TCard4     c4InitSN) const
{
    try
    {
        // We store the internal, human readable description as a meta value
        tCIDLib::TKVPFList colMeta(2);
        colMeta.objPlace(kCIDMData::strChFlMetaKey_Title, emoncSrc.strDescr());

        // And the paused state
        colMeta.objPlace
        (
            kCQCRemBrws::strMetaKey_Paused, facCQCKit().strBoolVal(emoncSrc.bPaused())
        );

        // Let's flatten the command source back out for storage
        TBinMBufOutStream strmTar(16 * 1024UL);
        strmTar << emoncSrc << kCIDLib::FlushIt;

        chflToFill.Reset(strmTar.c4CurSize(), strmTar.mbufData(), colMeta, c4InitSN);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TFacCQCRemBrws::ChunkUpGlobalAct(const  TCQCStdCmdSrc&      csrcSrc
                                ,       TChunkedFile&       chflToFill
                                , const tCIDLib::TCard4     c4InitSN) const
{
    try
    {
        // We store the internal, human readable title as a meta value
        tCIDLib::TKVPFList colMeta(1);
        colMeta.objPlace(kCIDMData::strChFlMetaKey_Title, csrcSrc.strTitle());

        // Let's flatten the command source back out for storage
        TBinMBufOutStream strmTar(16 * 1024UL);
        strmTar << csrcSrc << kCIDLib::FlushIt;

        chflToFill.Reset(strmTar.c4CurSize(), strmTar.mbufData(), colMeta, c4InitSN);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TFacCQCRemBrws::ChunkUpImage(const  TPNGImage&          imgiSrc
                            ,       TChunkedFile&       chflToFill
                            , const tCIDLib::TCard4     c4InitSN) const
{
    try
    {
        // Package up the image and a thumbnail for it
        TSize szThumb;
        const tCIDLib::TCard4 c4ImgBytes = imgiSrc.c4ImageSz() + 1024;
        TBinMBufOutStream strmImage(c4ImgBytes, c4ImgBytes + (c4ImgBytes / 4));
        TBinMBufOutStream strmThumb(c4ImgBytes / 4, c4ImgBytes);
        facCQCKit().PackageImg(imgiSrc, strmImage, strmThumb, szThumb);


        //
        //  Set up the standard meta values that images have. These let clients get info
        //  about the images without having to actually download and decode them.
        //
        tCIDLib::TKVPFList colMeta(4);
        colMeta.objPlace
        (
            kCQCRemBrws::strMetaKey_ImgFormat
            , TString(TCardinal(tCIDLib::c4EnumOrd(imgiSrc.ePixFmt())))
        );

        colMeta.objPlace
        (
            kCQCRemBrws::strMetaKey_ImgDepth
            , TString(TCardinal(tCIDLib::c4EnumOrd(imgiSrc.eBitDepth())))
        );

        colMeta.objPlace(kCQCRemBrws::strMetaKey_ImgSize, TString(imgiSrc.szImage()));

        colMeta.objPlace(kCQCRemBrws::strMetaKey_ThumbSize, TString(szThumb));

        // Reset the chunked file with our main info
        chflToFill.Reset(strmImage.c4CurSize(), strmImage.mbufData(), colMeta, c4InitSN);

        // Add an extension chunk for the thumb, say its not a data change chunk
        chflToFill.AddExtChunk
        (
            kCQCRemBrws::strChunkId_Thumbnail
            , strmThumb.c4CurSize()
            , strmThumb.mbufData()
            , kCIDLib::False
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  We have two scenarios here, one for the normal thing and another for encrypted
//  macros. For encrypted, the key needs to be the real key, not the derived one.
//
tCIDLib::TVoid
TFacCQCRemBrws::ChunkUpMacro(const  TString&            strText
                            ,       TChunkedFile&       chflToFill
                            , const tCIDLib::TCard4     c4InitSN) const
{
    try
    {
        // Flatten out the text
        TBinMBufOutStream strmTar(strText.c4Length() * 2);
        strmTar << strText << kCIDLib::FlushIt;

        // And set it as the data chunk. There are no meta values
        tCIDLib::TKVPFList colMeta(1);
        chflToFill.Reset(strmTar.c4CurSize(), strmTar.mbufData(), colMeta, c4InitSN);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TFacCQCRemBrws::ChunkUpSchedEvent(  const   TCQCSchEvent&       csrcSrc
                                    ,       TChunkedFile&       chflToFill
                                    , const tCIDLib::TCard4     c4InitSN) const
{
    try
    {
        // We store the internal, human readable description as a meta value
        tCIDLib::TKVPFList colMeta(2);
        colMeta.objPlace(kCIDMData::strChFlMetaKey_Title, csrcSrc.strTitle());

        // And the paused state
        colMeta.objPlace
        (
            kCQCRemBrws::strMetaKey_Paused, facCQCKit().strBoolVal(csrcSrc.bPaused())
        );

        // Let's flatten the command source back out for storage
        TBinMBufOutStream strmTar(16 * 1024UL);
        strmTar << csrcSrc<< kCIDLib::FlushIt;

        chflToFill.Reset(strmTar.c4CurSize(), strmTar.mbufData(), colMeta, c4InitSN);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TFacCQCRemBrws::ChunkUpTemplate(const   TMemBuf&            mbufTmpl
                                , const tCIDLib::TCard4     c4Bytes
                                ,       TChunkedFile&       chflToFill
                                , const tCQCKit::EUserRoles eMinRole
                                , const TString&            strNotes
                                , const tCIDLib::TCard4     c4InitSN) const
{
    try
    {
        // Set up the standard meta values that templates have
        tCIDLib::TKVPFList colMeta;
        colMeta.objPlace
        (
            kCQCRemBrws::strMetaKey_MinRole, tCQCKit::strAltXlatEUserRoles(eMinRole)
        );

        colMeta.objPlace(kCIDMData::strChFlMetaKey_Notes, strNotes);

        //
        //  Reset the chunked file with our data. We pass along the current serial number
        //  so as not to have to reset it back to one.
        //
        chflToFill.Reset(c4Bytes, mbufTmpl, colMeta, c4InitSN);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TFacCQCRemBrws::ChunkUpTrigEvent(const  TCQCTrgEvent&       csrcSrc
                                ,       TChunkedFile&       chflToFill
                                , const tCIDLib::TCard4     c4InitSN) const
{
    try
    {
        // We store the internal, human readable description as a meta value
        tCIDLib::TKVPFList colMeta;
        colMeta.objPlace(kCIDMData::strChFlMetaKey_Title, csrcSrc.strTitle());

        // And the paused state
        colMeta.objPlace
        (
            kCQCRemBrws::strMetaKey_Paused
            , facCQCKit().strBoolVal(csrcSrc.bPaused())
        );

        // Let's flatten the command source back out for storage
        TBinMBufOutStream strmTar(16 * 1024UL);
        strmTar << csrcSrc << kCIDLib::FlushIt;

        chflToFill.Reset(strmTar.c4CurSize(), strmTar.mbufData(), colMeta, c4InitSN);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

