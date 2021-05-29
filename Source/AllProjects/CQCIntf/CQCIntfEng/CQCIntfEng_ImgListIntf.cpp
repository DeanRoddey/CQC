//
// FILE NAME: CQCIntfEng_ImgListIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/17/2003
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
//  A number of widget types share the characteristic of displaying a user
//  uploaded image, so they implement this interface so that they can be
//  configured via the image config tab.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//



// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfEng_.hpp"



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_ImgListIntf
    {
        // -----------------------------------------------------------------------
        //  Our persistent format versions
        //
        //  Version 1 -
        //      Reset the version as of 1.4
        //
        //  Version 2 -
        //      Convert paths to 5.x format
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 2;


        // -----------------------------------------------------------------------
        //  The key we store our attributes in when asked to put them into an
        //  attribute pack.
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const   pszAttrPackKey = L"Image List Attributes";
    }
}



// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfImgListIntf::TImgInfo
//  PREFIX: imgi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCIntfImgListIntf::TImgInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
MCQCIntfImgListIntf::TImgInfo::TImgInfo() :

    m_bEnabled(kCIDLib::False)
    , m_c1Opacity(0xFF)
    , m_cptrImg(facCQCIntfEng().cptrEmptyImg())
    , m_eReqFlag(tCQCIntfEng::EImgReqFlags::Required)
    , m_i4HOffset(0)
    , m_i4VOffset(0)
{
}

MCQCIntfImgListIntf::
TImgInfo::TImgInfo( const   TString&                    strKey
                    , const TString&                    strImageName
                    , const tCQCIntfEng::EImgReqFlags   eReqFlag
                    , const tCIDLib::TCard1             c1Opacity
                    , const tCIDLib::TBoolean           bEnabled) :

    m_bEnabled(bEnabled)
    , m_c1Opacity(c1Opacity)
    , m_cptrImg(facCQCIntfEng().cptrEmptyImg())
    , m_eReqFlag(eReqFlag)
    , m_i4HOffset(0)
    , m_i4VOffset(0)
    , m_strImageName(strImageName)
    , m_strKey(strKey)
{
}

MCQCIntfImgListIntf::
TImgInfo::TImgInfo( const   TString&                    strKey
                    , const tCQCIntfEng::EImgReqFlags   eReqFlag
                    , const tCIDLib::TCard1             c1Opacity
                    , const tCIDLib::TBoolean           bEnabled) :

    m_bEnabled(bEnabled)
    , m_c1Opacity(c1Opacity)
    , m_cptrImg(facCQCIntfEng().cptrEmptyImg())
    , m_eReqFlag(eReqFlag)
    , m_i4HOffset(0)
    , m_i4VOffset(0)
    , m_strKey(strKey)
{
}

MCQCIntfImgListIntf::
TImgInfo::TImgInfo(const MCQCIntfImgListIntf::TImgInfo& imgiToCopy) :

    m_bEnabled(imgiToCopy.m_bEnabled)
    , m_c1Opacity(imgiToCopy.m_c1Opacity)
    , m_cptrImg(imgiToCopy.m_cptrImg)
    , m_eReqFlag(imgiToCopy.m_eReqFlag)
    , m_i4HOffset(imgiToCopy.m_i4HOffset)
    , m_i4VOffset(imgiToCopy.m_i4VOffset)
    , m_strImageName(imgiToCopy.m_strImageName)
    , m_strKey(imgiToCopy.m_strKey)
{
}

MCQCIntfImgListIntf::TImgInfo::~TImgInfo()
{
}


// ---------------------------------------------------------------------------
//  MCQCIntfImgListIntf::TImgInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid MCQCIntfImgListIntf::
TImgInfo::operator=(const MCQCIntfImgListIntf::TImgInfo& imgiSrc)
{
    if (this != &imgiSrc)
    {
        m_bEnabled      = imgiSrc.m_bEnabled;
        m_c1Opacity     = imgiSrc.m_c1Opacity;
        m_cptrImg       = imgiSrc.m_cptrImg;
        m_eReqFlag      = imgiSrc.m_eReqFlag;
        m_i4HOffset     = imgiSrc.m_i4HOffset;
        m_i4VOffset     = imgiSrc.m_i4VOffset;
        m_strImageName  = imgiSrc.m_strImageName;
        m_strKey        = imgiSrc.m_strKey;
    }
}

tCIDLib::TBoolean MCQCIntfImgListIntf::
TImgInfo::operator==(const MCQCIntfImgListIntf::TImgInfo& imgiSrc) const
{
    if (this != &imgiSrc)
    {
        if ((m_bEnabled      != imgiSrc.m_bEnabled)
        ||  (m_c1Opacity     != imgiSrc.m_c1Opacity)
        ||  (m_eReqFlag      != imgiSrc.m_eReqFlag)
        ||  (m_i4HOffset     != imgiSrc.m_i4HOffset)
        ||  (m_i4VOffset     != imgiSrc.m_i4VOffset)
        ||  (m_strImageName  != imgiSrc.m_strImageName)
        ||  (m_strKey        != imgiSrc.m_strKey))
        {
            return kCIDLib::False;
        }

        if (m_cptrImg->strImageKey() != imgiSrc.m_cptrImg->strImageKey())
            return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean MCQCIntfImgListIntf::
TImgInfo::operator!=(const MCQCIntfImgListIntf::TImgInfo& imgiSrc) const
{
    return !operator==(imgiSrc);
}


// ---------------------------------------------------------------------------
//  MCQCIntfImgListIntf::TImgInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid MCQCIntfImgListIntf::TImgInfo::Reset()
{
    m_c1Opacity = 0xFF;
    m_i4HOffset = 0;
    m_i4VOffset = 0;
    m_strImageName.Clear();

    facCQCIntfEng().DropImgRef(m_cptrImg);
}



// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfImgListIntf
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCIntfImgListIntf: Constructors and Destructor
// ---------------------------------------------------------------------------
MCQCIntfImgListIntf::MCQCIntfImgListIntf() :

    m_colList(8)
{
}

MCQCIntfImgListIntf::MCQCIntfImgListIntf(const MCQCIntfImgListIntf& iwdgToCopy) :

    m_colList(iwdgToCopy.m_colList)
{
}

MCQCIntfImgListIntf::~MCQCIntfImgListIntf()
{
}


// ---------------------------------------------------------------------------
//  MCQCIntfImgListIntf: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid
MCQCIntfImgListIntf::operator=(const MCQCIntfImgListIntf& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        m_colList = iwdgToAssign.m_colList;
    }
}


// ---------------------------------------------------------------------------
//  MCQCIntfImgListIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We calculate the bounding area of the images, taking offset into account.
//  the returned bounding area is centered on the origin, so the caller must
//  adjust it for their actual position if needed.
//
TArea MCQCIntfImgListIntf::areaImgBounds() const
{
    const TArea areaCenterIn(-1000, -1000, 2000, 2000);

    TArea areaBounds;
    TArea areaTmp;
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TImgInfo& imgiCur = m_colList[c4Index];
        if (!imgiCur.m_bEnabled)
            continue;
        areaTmp.Set(TPoint::pntOrigin, imgiCur.m_cptrImg->szImage());
        areaTmp.CenterIn(areaCenterIn);
        areaTmp.AdjustOrg(imgiCur.m_i4HOffset, imgiCur.m_i4VOffset);
        areaBounds |= areaTmp;
    }

    //
    //  And now we need to take the larger of the left/right and top/bottom
    //  and set the other one to that value (sign inverted) because we have
    //  to be symetric around the origin to fully contain the images. I.e.,
    //  if the furthest image goes 20 off to the left, we have to provide for
    //  20 off to the right to get the right size, even if no image is
    //  actually going that far off to the right.
    //
    tCIDLib::TInt4 i4L = areaBounds.i4Left();
    tCIDLib::TInt4 i4R = areaBounds.i4Right();
    tCIDLib::TInt4 i4T = areaBounds.i4Top();
    tCIDLib::TInt4 i4B = areaBounds.i4Bottom();

    //
    //  NOTE:   We have to use the point version here, since we have to
    //          have it construct from corners. If we pass the raw 4 values,
    //          it will take them as origin/size values, not UL/LR points.
    //
    return TArea
    (
        TPoint(tCIDLib::MinVal(i4L, -i4R), tCIDLib::MinVal(i4T, -i4B))
        , TPoint(tCIDLib::MaxVal(i4R, i4L * -1), tCIDLib::MaxVal(i4B, i4T * -1))
    );
}


//
//  This is used to put the initial values into the list, when only the keys
//  are known.
//
tCIDLib::TVoid
MCQCIntfImgListIntf::AddNewImgKey(const TString&                    strKey
                                , const tCQCIntfEng::EImgReqFlags   eReqFlags
                                , const tCIDLib::TCard1             c1Opacity
                                , const tCIDLib::TBoolean           bEnabled
                                , const tCIDLib::TCh* const         pszImagePath)
{
    // Make sure this key isn't already in the list
    if (pimgiFindByKey(strKey))
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcImg_ImgKeyExists
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , strKey
        );
    }

    // If the image is required, then force enabled on
    tCIDLib::TBoolean bActEnabled = bEnabled;
    if (!bActEnabled && (eReqFlags == tCQCIntfEng::EImgReqFlags::Required))
        bActEnabled = kCIDLib::True;

    // Add it and get a ref in case we need to add more
    TImgInfo& imgiNew = m_colList.objAdd
    (
        TImgInfo(strKey, eReqFlags, c1Opacity, bActEnabled)
    );

    // If they provided an initial path, store that
    if (pszImagePath)
        imgiNew.m_strImageName = pszImagePath;
}


tCIDLib::TBoolean MCQCIntfImgListIntf::bAnyActiveKeys() const
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        if (m_colList[c4Index].m_bEnabled)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean MCQCIntfImgListIntf::bDeleteImgKey(const TString& strKey)
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        const TImgInfo& imgiCur = m_colList[c4Index];
        if (imgiCur.m_strKey == strKey)
        {
            m_colList.RemoveAt(c4Index);
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean MCQCIntfImgListIntf::bImgEnabled(const TString& strKey) const
{
    const TImgInfo* pimgiFind = pimgiFindByKey(strKey, kCIDLib::True);
    return pimgiFind->m_bEnabled;
}


tCIDLib::TBoolean
MCQCIntfImgListIntf::bImgTransparent(const TString& strKey)
{
    TImgInfo* pimgiFind = pimgiFindByKey(strKey, kCIDLib::True);
    return pimgiFind->m_cptrImg->bTransparent();
}


tCIDLib::TBoolean
MCQCIntfImgListIntf::bSameImgList(const MCQCIntfImgListIntf& miwdgSrc) const
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    if (miwdgSrc.m_colList.c4ElemCount() != c4Count)
        return kCIDLib::False;

    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        if (m_colList[c4Index] != miwdgSrc.m_colList[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


const TBitmap& MCQCIntfImgListIntf::bmpImage(const TString& strKey) const
{
    const TImgInfo* pimgiFind = pimgiFindByKey(strKey, kCIDLib::True);
    return pimgiFind->m_cptrImg->bmpImage();
}


const TBitmap& MCQCIntfImgListIntf::bmpMask(const TString& strKey) const
{
    const TImgInfo* pimgiFind = pimgiFindByKey(strKey, kCIDLib::True);
    return pimgiFind->m_cptrImg->bmpMask();
}


tCIDLib::TCard4 MCQCIntfImgListIntf::c4ImgKeyCount() const
{
    return m_colList.c4ElemCount();
}


MCQCIntfImgListIntf::TImgCursor MCQCIntfImgListIntf::cursImgList() const
{
    return TImgList::TCursor(&m_colList);
}


// Remove all of our image keys
tCIDLib::TVoid MCQCIntfImgListIntf::DeleteAllImgKeys()
{
    m_colList.RemoveAll();
}


// Disable any images that are not required
tCIDLib::TVoid MCQCIntfImgListIntf::DisableAllImgs()
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        TImgInfo& imgiCur = m_colList[c4Index];

        // If it can be disabled, then do it
        if (imgiCur.m_eReqFlag != tCQCIntfEng::EImgReqFlags::Required)
            imgiCur.m_bEnabled = kCIDLib::False;
    }
}


tCIDLib::TVoid
MCQCIntfImgListIntf::EnableImgKey(  const   TString&            strKey
                                    , const tCIDLib::TBoolean   bNewState)
{
    TImgInfo* pimgiFind = pimgiFindByKey(strKey, kCIDLib::True);
    pimgiFind->m_bEnabled = bNewState;

    // If disabled, then let the image go
    if (!bNewState)
        facCQCIntfEng().DropImgRef(pimgiFind->m_cptrImg);
}


const MCQCIntfImgListIntf::TImgInfo&
MCQCIntfImgListIntf::imgiAt(const tCIDLib::TCard4 c4At) const
{
    return m_colList[c4At];
}

MCQCIntfImgListIntf::TImgInfo&
MCQCIntfImgListIntf::imgiAt(const tCIDLib::TCard4 c4At)
{
    return m_colList[c4At];
}


MCQCIntfImgListIntf::TImgInfo&
MCQCIntfImgListIntf::imgiForKey(const TString& strKey)
{
    return *pimgiFindByKey(strKey, kCIDLib::True);
}


tCIDLib::TVoid
MCQCIntfImgListIntf::InitImgList(const  TCQCIntfView&           civOwner
                                ,       TDataSrvClient&         dsclToUse
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TCQCIntfWidget&         iwdgThis)
{
    // Loop through the list and load them up
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    if (!c4Count)
        return;

    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        TImgInfo& imgiCur = m_colList[c4Index];

        // Try to load up the image and store the info away
        if (!imgiCur.m_strImageName.bIsEmpty())
        {
            try
            {
                LoadImgInfo(civOwner, dsclToUse, imgiCur);
            }

            catch(const TError& errToCatch)
            {
                if (!TFacCQCIntfEng::bDesMode())
                {
                    if (facCQCIntfEng().bShouldLog(errToCatch))
                        TModule::LogEventObj(errToCatch);
                }

                // Put an error in the list
                TString strErr
                (
                    kIEngErrs::errcVal_ImgNotFound
                    , facCQCIntfEng()
                    , imgiCur.m_strImageName
                );
                colErrs.objAdd
                (
                    TCQCIntfValErrInfo
                    (
                        iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId()
                    )
                );

                //
                //  If in designer mode, just make sure the image is blanked.
                //  In viewer mode, we can reset so that we won't keep trying
                //  to load it.
                //
                if (TFacCQCIntfEng::bDesMode())
                    facCQCIntfEng().DropImgRef(imgiCur.m_cptrImg);
                else
                    imgiCur.m_strImageName.Clear();
            }
        }
    }
}


tCIDLib::TVoid
MCQCIntfImgListIntf::LoadNewImgKeys(const TCollection<TString>& colNewKeys)
{
    const tCIDLib::TCard4 c4NewCount = colNewKeys.c4ElemCount();

    // If no new keys, then just clear list and return
    if (!c4NewCount)
    {
        m_colList.RemoveAll();
        return;
    }

    //
    //  If the new list is the same, then we don't do anything since we are just getting
    //  the new keys at load time when the field info is set, or perhaps an update
    //  of the list after editing something else in the widget that affects the list, but
    //  in fact the actual names didn't end up changing.
    //
    TColCursor<TString>* pcursKeys = colNewKeys.pcursNew();
    TJanitor<TColCursor<TString> > janCursor(pcursKeys);

    tCIDLib::TCard4 c4Index = 0;
    if (c4NewCount == m_colList.c4ElemCount())
    {
        pcursKeys->bReset();
        do
        {
            if (pcursKeys->objRCur() != m_colList[c4Index].m_strKey)
                break;
            c4Index++;
        }   while (pcursKeys->bNext());
    }

    // If we went through all of them, then they were equal, else update
    if (c4Index < c4NewCount)
    {
        // Clean out the current list
        m_colList.RemoveAll();

        // ANd load a new set
        pcursKeys->bReset();
        do
        {
            m_colList.objAdd
            (
                TImgInfo
                (
                    pcursKeys->objRCur()
                    , tCQCIntfEng::EImgReqFlags::Required
                    , 255
                    , kCIDLib::True
                )
            );
        }   while (pcursKeys->bNext());
    }
}


MCQCIntfImgListIntf::TImgInfo* MCQCIntfImgListIntf::pimgiFirstKey()
{
    // Just find the first one via a cursor
    if (m_colList.bIsEmpty())
        return 0;
    return &m_colList[0];
}


//
//  Return the image info object for the indicated key, if there is one.
//  If not, they'll get back a null.
//
MCQCIntfImgListIntf::TImgInfo*
MCQCIntfImgListIntf::pimgiForKey(const TString& strKey)
{
    return pimgiFindByKey(strKey);
}


//
//  Same as above, but only if the image is enabled. So, even if the key
//  is found, if that image slot is not enabled, we return null. Mostly
//  used by widgets during drawing, whereas the above is more for editing.
//
MCQCIntfImgListIntf::TImgInfo*
MCQCIntfImgListIntf::pimgiForKeyEO(const TString& strKey)
{
    MCQCIntfImgListIntf::TImgInfo* pimgiRet = pimgiFindByKey(strKey);
    if (pimgiRet && pimgiRet->m_bEnabled)
        return pimgiRet;
    return 0;
}


//
//  We play tricks here. The image list can't be done directly via the attribute
//  editor, so it has to be done via a dialog box. And we want the user to see the
//  changes as they are made. So the editor invokes the appropriate dialog box on us
//  to make those changes.
//
//  However, we still have to store the original content in the attribute for undo
//  purposes, and accept in our set attributes handler.
//
tCIDLib::TVoid
MCQCIntfImgListIntf::QueryImgListAttrs( tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    adatTmp.Set(L"Image Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    TBinMBufOutStream strmTar(16 * 1024UL);
    WriteOutImgList(strmTar);
    strmTar.Flush();
    colAttrs.objPlace
    (
        L"Images"
        , kCQCIntfEng::strAttr_Images
        , strmTar.mbufData()
        , strmTar.c4CurPos()
        , TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    ).SetSpecType(kCQCIntfEng::strAttrType_Images);
}


// Returns any of our image pahts requested and not already in the list
tCIDLib::TVoid MCQCIntfImgListIntf::
QueryImgPaths(          tCIDLib::TStrHashSet&   colToFill
                , const tCIDLib::TBoolean       bIncludeSysImgs) const
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        const TImgInfo& imgiCur = m_colList[c4Index];
        if (!imgiCur.m_bEnabled || imgiCur.m_strImageName.bIsEmpty())
            continue;

        if (!bIncludeSysImgs
        &&  imgiCur.m_strImageName.bStartsWith(kCQCKit::strRepoScope_System))
        {
            continue;
        }

        if (colToFill.bHasElement(imgiCur.m_strImageName))
            continue;

        colToFill.objAdd(imgiCur.m_strImageName);
    }
}


tCIDLib::TVoid MCQCIntfImgListIntf::ReadInImgList(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ImgListIntf::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCIntfImgListIntf")
        );
    }

    // Flush any current content
    m_colList.RemoveAll();

    //
    //  We only write out the key/imagename parts, so we read those back
    //  in and set up image info objects with just that info. When we are
    //  initialized, we'll load up the images and other info. The first
    //  thing written is the count and XOR'd count, so get it out and
    //  check it for validity.
    //
    tCIDLib::TCard4 c4Count;
    tCIDLib::TCard4 c4Xor;
    strmToReadFrom >> c4Count >> c4Xor;
    if (c4Count != (c4Xor ^ kCIDLib::c4MaxCard))
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcCfg_BadImgCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    tCIDLib::TBoolean           bEnabled;
    tCIDLib::TCard1             c1Opacity;
    tCQCIntfEng::EImgReqFlags   eReqFlags;
    TString                     strKey;
    TString                     strImageName;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToReadFrom  >> strKey
                        >> strImageName
                        >> bEnabled
                        >> c1Opacity
                        >> eReqFlags;

        // Convert the path to 5.x format if needed
        if (c2FmtVersion < 2)
            facCQCKit().Make50Path(strImageName);

        // Fix up the path if required (slashes, double slashes, etc...)
        facCQCKit().PrepRemBrwsPath(strImageName);

        // Now we can set up the info object for this guy
        TImgInfo imgiNew(strKey, strImageName, eReqFlags, c1Opacity, bEnabled);

        // Read in the H/V offsets and set them on the image object
        tCIDLib::TInt4 i4HOfs, i4YOfs;
        strmToReadFrom >> i4HOfs >> i4YOfs;
        imgiNew.m_i4HOffset = i4HOfs;
        imgiNew.m_i4VOffset = i4YOfs;

        m_colList.objAdd(imgiNew);
    }

    // And it should end with another frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


// Supports the search and replace system
tCIDLib::TVoid
MCQCIntfImgListIntf::ReplaceImage(  const   TString&            strSrc
                                    , const TString&            strTar
                                    , const tCIDLib::TBoolean   bRegEx
                                    , const tCIDLib::TBoolean   bFull
                                    ,       TRegEx&             regxFind)
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        TImgInfo& imgiCur = m_colList[c4Index];

        // Only bother with enabled ones
        if (imgiCur.m_bEnabled)
        {
            facCQCKit().bDoSearchNReplace
            (
                strSrc, strTar, imgiCur.m_strImageName, bRegEx, bFull, regxFind
            );

            // Fix up the path if required (slashes, double slashes, etc...)
            facCQCKit().PrepRemBrwsPath(imgiCur.m_strImageName);
        }
    }
}


TSize MCQCIntfImgListIntf::szImgDefault() const
{
    return TSize(64, 64);
}


TSize MCQCIntfImgListIntf::szLargestImg() const
{
    TSize szImg;

    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
        {
            // Only look at enabled ones
            const TImgInfo& imgiCur = m_colList[c4Index];
            if (imgiCur.m_bEnabled)
                szImg.TakeLarger(imgiCur.m_cptrImg->szImage());
        }
    }
     else
    {
        // There aren't any yet, so set a default
        szImg.Set(32, 32);
    }
    return szImg;
}


//
//  When a template is being scaled, this is called for any widgets that implement our
//  interface. Tis is not for scaling the images, but other stuff, in our case the image
//  offsets.
//
tCIDLib::TVoid
MCQCIntfImgListIntf::ScaleImgList(  const   tCIDLib::TFloat8    f8XScale
                                    , const tCIDLib::TFloat8    f8YScale)
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        TImgInfo& imgiCur = m_colList[c4Index];

        imgiCur.m_i4HOffset = TMathLib::i4Round(imgiCur.m_i4HOffset * f8XScale);
        imgiCur.m_i4VOffset = TMathLib::i4Round(imgiCur.m_i4VOffset * f8YScale);
    }
}


//
//  This one is needed to allow migration of images from widgets that used
//  to do all the image management themselves down into here. When they stream
//  in, they need to pass the images key/names on to us.
//
tCIDLib::TVoid
MCQCIntfImgListIntf::SetImgKey( const   TString&                    strKey
                                , const TString&                    strName
                                , const tCQCIntfEng::EImgReqFlags   eReqFlags
                                , const tCIDLib::TCard1             c1Opacity
                                , const tCIDLib::TBoolean           bEnabled)
{
    TImgInfo* pimgiSet = pimgiFindByKey(strKey, kCIDLib::True);

    // Just update the image with the passed info
    pimgiSet->m_strImageName = strName;
    pimgiSet->m_bEnabled = bEnabled;
    pimgiSet->m_c1Opacity = c1Opacity;
    pimgiSet->m_eReqFlag = eReqFlags;
    pimgiSet->m_i4HOffset = 0;
    pimgiSet->m_i4VOffset = 0;
}


//
//  See the attribute getter method above for comments. The changes were made directly
//  to the widget, however in the case of an undo, we do get changes back to us, so we
//  have to stream it in even if it's almost always redundant.
//
tCIDLib::TVoid
MCQCIntfImgListIntf::SetImgListAttr(const   TCQCIntfView&   civOwner
                                    ,       TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    if (adatNew.strId() == kCQCIntfEng::strAttr_Images)
    {
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        ReadInImgList(strmSrc);

        // We have to now update all of our images
        TDataSrvClient dsclUpdate;
        UpdateAllImgs(civOwner, dsclUpdate);
    }
}


tCIDLib::TVoid
MCQCIntfImgListIntf::SetImgOpacity( const   TString&        strKey
                                    , const tCIDLib::TCard1 c1Opacity)
{
    TImgInfo* pimgiSet = pimgiFindByKey(strKey, kCIDLib::True);
    pimgiSet->m_c1Opacity = c1Opacity;
}


//
//  Given another image list, go through its keys and, if we have that key and ours is
//  enabled and his is enabled, we will set ours to the same image.
//
tCIDLib::TBoolean
MCQCIntfImgListIntf::bTakeMatchingImgKeys(const MCQCIntfImgListIntf& miwdgSrc)
{
    tCIDLib::TCard4 c4Taken = 0;
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        if (m_colList[c4Index].m_bEnabled
        &&  miwdgSrc.m_colList[c4Index].m_bEnabled
        &&  (m_colList[c4Index].m_strKey == miwdgSrc.m_colList[c4Index].m_strKey))
        {
            m_colList[c4Index].m_strImageName = miwdgSrc.m_colList[c4Index].m_strImageName;
            c4Taken++;
        }
    }
    return (c4Taken != 0);
}


// This is called to get us to update all our images from the cache
tCIDLib::TVoid
MCQCIntfImgListIntf::UpdateAllImgs(const TCQCIntfView& civOwner, TDataSrvClient& dsclToUse)
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        TImgInfo& imgiCur = m_colList[c4Index];
        if (imgiCur.m_bEnabled)
        {
            try
            {
                LoadImgInfo(civOwner, dsclToUse, imgiCur);
            }

            catch(const TError& errToCatch)
            {
                if (facCQCIntfEng().bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);

                // Clean this one out
                facCQCIntfEng().DropImgRef(imgiCur.m_cptrImg);
            }
        }
    }
}


tCIDLib::TVoid
MCQCIntfImgListIntf::ValidateImgList(       tCQCIntfEng::TErrList&  colErrs
                                    ,       TDataSrvClient&         dsclToUse
                                    , const TCQCIntfWidget&         iwdgThis) const
{
    tCIDLib::TCard4 c4SetCnt = 0;
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        const TImgInfo& imgiCur = m_colList[c4Index];

        //
        //  Make sure that required images have a path set. If they have a path
        //  make sure that it's a syntactically valid path. If so, make sure that it
        //  actually exists.
        //
        if (imgiCur.m_strImageName.bIsEmpty())
        {
            if (imgiCur.m_eReqFlag == tCQCIntfEng::EImgReqFlags::Required)
            {
                TString strErr(kIEngErrs::errcImg_ReqImage, facCQCIntfEng());
                colErrs.objAdd
                (
                    TCQCIntfValErrInfo
                    (
                        iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId()
                    )
                );
            }
        }
         else
        {
            TString strErr;

            // First make sure the path is reasonable, no relative paths in this case
            if (!facCQCKit().bIsValidRemTypeRelPath(imgiCur.m_strImageName, strErr, kCIDLib::False))
            {
                TString strName(imgiCur.m_strKey, 12UL);
                strName.Append(L" image");
                colErrs.objAdd
                (
                    TCQCIntfValErrInfo
                    (
                        iwdgThis.c4UniqueId()
                        , facCQCIntfEng().strMsg(kIEngErrs::errcVal_BadPath, strName, strErr)
                        , kCIDLib::True
                        , iwdgThis.strWidgetId()
                    )
                );
            }
             else
            {
                // Looks reasonable, see if it exists
                tCIDLib::TCard4 c4SerNum = 0;
                if (!dsclToUse.bFileExists(imgiCur.m_strImageName, tCQCRemBrws::EDTypes::Image))
                {
                    // Put an error in the list
                    strErr.LoadFromMsg
                    (
                        kIEngErrs::errcVal_ImgNotFound, facCQCIntfEng(), imgiCur.m_strImageName
                    );
                    colErrs.objAdd
                    (
                        TCQCIntfValErrInfo
                        (
                            iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId()
                        )
                    );
                }
            }

            c4SetCnt++;
        }
    }

    //
    //  If any were set, then we have to do another pass to check for any that are required
    //  if any images are set.
    //
    if (c4SetCnt)
    {
        for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
        {
            const TImgInfo& imgiCur = m_colList[c4Index];

            if (imgiCur.m_strImageName.bIsEmpty())
            {
                if (imgiCur.m_eReqFlag == tCQCIntfEng::EImgReqFlags::RequiredIfAny)
                {
                    TString strErr(kIEngErrs::errcImg_ReqImage, facCQCIntfEng());
                    colErrs.objAdd
                    (
                        TCQCIntfValErrInfo
                        (
                            iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId()
                        )
                    );
                }
            }
        }
    }
}


//
//  Changes the image name associated with a given key, and loads up the new
//  image.
//
tCIDLib::TVoid
MCQCIntfImgListIntf::UpdateImg( const   TCQCIntfView&           civOwner
                                ,       TDataSrvClient&         dsclToUse
                                , const TString&                strKey
                                , const TString&                strToSet)
{
    TImgInfo* pimgiUpdate = pimgiFindByKey(strKey, kCIDLib::True);

    // And load up the new image info
    pimgiUpdate->m_strImageName = strToSet;
    LoadImgInfo(civOwner, dsclToUse, *pimgiUpdate);
}

tCIDLib::TVoid
MCQCIntfImgListIntf::UpdateImg( const   TCQCIntfView&           civOwner
                                ,       TDataSrvClient&         dsclToUse
                                , const TString&                strKey
                                , const TString&                strToSet
                                , const tCIDLib::TCard1         c1Opacity)
{
    TImgInfo* pimgiUpdate = pimgiFindByKey(strKey, kCIDLib::True);

    // And load up the new image info
    pimgiUpdate->m_bEnabled = kCIDLib::True;
    pimgiUpdate->m_c1Opacity = c1Opacity;
    pimgiUpdate->m_strImageName = strToSet;
    LoadImgInfo(civOwner, dsclToUse, *pimgiUpdate);
}


tCIDLib::TVoid
MCQCIntfImgListIntf::WriteOutImgList(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEng_ImgListIntf::c2FmtVersion;

    //
    //  We only need to store the key/image names and enable/disable info,
    //  since we get the images back when we start up and reload them. Write
    //  out the number of items and it's XOR, so we can know how many to read
    //  back in.
    //
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    const tCIDLib::TCard4 c4Xor = c4Count ^ kCIDLib::c4MaxCard;
    strmToWriteTo << c4Count << c4Xor;

    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        const TImgInfo& imgiCur = m_colList[c4Index];
        strmToWriteTo   << imgiCur.m_strKey
                        << imgiCur.m_strImageName
                        << imgiCur.m_bEnabled
                        << imgiCur.m_c1Opacity
                        << imgiCur.m_eReqFlag
                        << imgiCur.m_i4HOffset
                        << imgiCur.m_i4VOffset;
    }

    // End it with a frame marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame;
}


// ---------------------------------------------------------------------------
//  MCQCIntfImgListIntf: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
MCQCIntfImgListIntf::LoadImgInfo(const  TCQCIntfView&           civOwner
                                ,       TDataSrvClient&         dsclToUse
                                ,       TImgInfo&               imgiToLoad)
{
    imgiToLoad.m_cptrImg = facCQCIntfEng().cptrGetImage
    (
        imgiToLoad.m_strImageName, civOwner, dsclToUse
    );
}


MCQCIntfImgListIntf::TImgInfo*
MCQCIntfImgListIntf::pimgiFindByKey(const   TString&            strKey
                                    , const tCIDLib::TBoolean   bThrowIfNot)
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        TImgInfo& imgiCur = m_colList[c4Index];
        if (imgiCur.m_strKey == strKey)
            return &imgiCur;
    }

    if (bThrowIfNot)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcImg_ImgKeyNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strKey
        );
    }
    return 0;
}

const MCQCIntfImgListIntf::TImgInfo*
MCQCIntfImgListIntf::pimgiFindByKey(const   TString&            strKey
                                    , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0;  c4Index < c4Count; c4Index++)
    {
        const TImgInfo& imgiCur = m_colList[c4Index];
        if (imgiCur.m_strKey == strKey)
            return &imgiCur;
    }

    if (bThrowIfNot)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcImg_ImgKeyNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strKey
        );
    }
    return 0;
}


