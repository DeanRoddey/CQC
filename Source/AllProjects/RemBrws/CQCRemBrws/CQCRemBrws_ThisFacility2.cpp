//
// FILE NAME: CQCRemBrws_ThisFacility2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2018
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
//  This file holds some temporary code that is required to convert from the old
//  (pre-5.3.903) data server file formats to the new, consolidated, TChunkedFile
//  based format.
//
//  We can get rid of this at whatever point we now longer support upgrades from
//  5.3.x.
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
//  Local helper methods
// ---------------------------------------------------------------------------

// No change in the format, just move it to the data chunk of teh chunked file
static tCIDLib::TVoid ConvertEventMon(TBinInStream& strmSrc, TChunkedFile& chflToFill)
{
    try
    {
        TCQCEvMonCfg emoncOld;
        strmSrc >> emoncOld;

        // These guys didn't have a serial number, so take the default 1
        facCQCRemBrws().ChunkUpEvMonitor(emoncOld, chflToFill);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



static tCIDLib::TVoid ConvertGlobalAct(TBinInStream& strmSrc, TChunkedFile& chflToFill)
{
    //
    //  These are keyed command source objects. However, the key is not used anymore.
    //  Since we moved them to our hierarchical storage scheme, the path to them is
    //  the unique key. So we can actually drop that by assigning it to the underlying
    //  parent class, which will slice off the id.
    //
    //  We set that as the main data chunk. We set the internal title as a meta value
    //  so it can be quickly accessed without understanding the file format.
    //
    try
    {
        TCQCStdKeyedCmdSrc csrcOld;
        strmSrc >> csrcOld;

        // Slice it back to the underlying non-keyed variation
        TCQCStdCmdSrc csrcNew(csrcOld);

        // And chunk that up
        facCQCRemBrws().ChunkUpGlobalAct(csrcNew, chflToFill);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



//
//  For this one we reset it with the main image data in the main data chunk, then add
//  a new extension chunk for the thumbnail. And we set up the standard meta values for
//  images.
//
static tCIDLib::TVoid ConvertImage(TBinInStream& strmSrc, TChunkedFile& chflToFill)
{
    try
    {
        TCQCImgStoreItem isiSrc;
        strmSrc >> isiSrc;

        //
        //  Stream in the PNG image. Kind of redundant, but this lets us use the same
        //  code that is used for images elsewhere to get it into chunked file format.
        //  And this is just used for conversion.
        //
        TPNGImage imgiOrg;
        {
            TBinMBufInStream strmThumb(&isiSrc.m_mbufImage, isiSrc.m_c4ImageSz);
            strmThumb >> imgiOrg;
        }

        facCQCRemBrws().ChunkUpImage(imgiOrg, chflToFill, isiSrc.m_c4SerialNum);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Macros can be encrypted, so this one is a little more complicated. In order to use
//  the standard 'chunk up' helper, if encrypted we decrypt it first.
//
static tCIDLib::TVoid ConvertMacro(         TBinInStream&   strmSrc
                                    ,       TChunkedFile&   chflToFill
                                    , const TString&        strSource)
{
    try
    {
        // Make sure we get the start marker
        strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

        // Stream in the format version and make sure its good
        tCIDLib::TCard2 c2FmtVersion;
        strmSrc >> c2FmtVersion;
        if (!c2FmtVersion || (c2FmtVersion > 2))
        {
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_UnknownFmtVersion
                , strSource
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c2FmtVersion)
                , TString(L"CQC Macro")
            );
        }

        //
        //  If V2, then read in the old encrypted flag and discard it
        //
        if (c2FmtVersion == 2)
        {
            tCIDLib::TBoolean bDiscard;
            strmSrc >> bDiscard;
        }

        TString strMacro;
        strmSrc >> strMacro;

        // And there should be an end marker
        strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);

        // These guys didn't have a serial number, so just take the default 1
        facCQCRemBrws().ChunkUpMacro(strMacro, chflToFill);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// No change in the format, just move it to the data chunk of teh chunked file
static tCIDLib::TVoid ConvertSchedEvent(TBinInStream& strmSrc, TChunkedFile& chflToFill)
{
    try
    {
        TCQCSchEvent csrcOld;
        strmSrc >> csrcOld;

        facCQCRemBrws().ChunkUpSchedEvent(csrcOld, chflToFill);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  If it's a template this is called. We get a template info object. We move the
//  data to the main data chunk and set up the standard meta values that we redundantly
//  store for templates.
//
//  The old data class has been stripped and just has public class members now and enough
//  smarts to read in the data.
//
static tCIDLib::TVoid ConvertTemplate(TBinInStream& strmSrc, TChunkedFile& chflToFill)
{
    try
    {
        TCQCIntfTmplData itfdSrc;
        strmSrc >> itfdSrc;

        facCQCRemBrws().ChunkUpTemplate
        (
            itfdSrc.m_mbufData
            , itfdSrc.m_c4Bytes
            , chflToFill
            , itfdSrc.m_eMinRole
            , itfdSrc.m_strNotes
            , itfdSrc.m_c4SerialNum
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



// No change in the format, just move it to the data chunk of teh chunked file
static tCIDLib::TVoid ConvertTrigEvent(TBinInStream& strmSrc, TChunkedFile& chflToFill)
{
    try
    {
        TCQCTrgEvent csrcOld;
        strmSrc >> csrcOld;
        facCQCRemBrws().ChunkUpTrigEvent(csrcOld, chflToFill);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCRemBrws: Public, non-virtual methods (continued)
// ---------------------------------------------------------------------------
//
//  The caller provides us with an input stream that has the data in it to be read,
//  and tells us the data type. We use that to stream in the old style format and
//  fill in the passed chunked file object.
//
//  They can create a higher level tree iteration scheme over this to convert a whole
//  tree.
//
//  strSource is just something to use in errors. It will often be a system file path,
//  but whatever will help identify what it came from that was bad.
//
tCIDLib::TVoid
TFacCQCRemBrws::ConvertToChunkedFormat( const   tCQCRemBrws::EDTypes    eType
                                        , const TString&                strSource
                                        ,       TBinInStream&           strmSrc
                                        ,       TChunkedFile&           chflToFill)
{
    if (eType == tCQCRemBrws::EDTypes::EvMonitor)
    {
        ConvertEventMon(strmSrc, chflToFill);
    }
     else if (eType == tCQCRemBrws::EDTypes::GlobalAct)
    {
        ConvertGlobalAct(strmSrc, chflToFill);
    }
     else if (eType == tCQCRemBrws::EDTypes::Image)
    {
        ConvertImage(strmSrc, chflToFill);
    }
     else if (eType == tCQCRemBrws::EDTypes::Macro)
    {
        ConvertMacro(strmSrc, chflToFill, strSource);
    }
     else if (eType == tCQCRemBrws::EDTypes::SchEvent)
    {
        ConvertSchedEvent(strmSrc, chflToFill);
    }
     else if (eType == tCQCRemBrws::EDTypes::Template)
    {
        ConvertTemplate(strmSrc, chflToFill);
    }
     else if (eType == tCQCRemBrws::EDTypes::TrgEvent)
    {
        ConvertTrigEvent(strmSrc, chflToFill);
    }
}
