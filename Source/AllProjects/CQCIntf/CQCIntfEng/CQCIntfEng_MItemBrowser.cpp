//
// FILE NAME: CQCIntfEng_MItemBrowser.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/13/2005
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
//  This file implements the media item list browser.
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
#include    "CQCIntfEng_MItemBrowser.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfMIBrwRTV, TCQCIntfHVBrwRTV)
AdvRTTIDecls(TCQCIntfMItemBrowser,TCQCIntfHVBrowserBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_MItemBrowser
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage format version
        //
        //  Version 2 -
        //      We changed the associated field from a col/title cookie to a play
        //      list serial number. So upon reading in a V1, we change the field
        //      name.
        //
        //  Version 3 -
        //      We bumped the version because we got rid of an intervening parent
        //      class in 2.4.15. So we now handle reading in its data and toss it
        //      alway on write back out.
        //
        //  Version 4 -
        //      We moved support for single field association from the base widget
        //      class to a mixin, so we need to know whether we've already converted
        //      over or not.
        //
        //  Version 5 -
        //      We no longer associate our browser with a repository. Those are
        //      now in the actual playlist items.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 5;


        // -----------------------------------------------------------------------
        //  The caps flags for our class
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::TakesFocus
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMIBrwRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMIBrwRTV: Public Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMIBrwRTV::TCQCIntfMIBrwRTV(const TCQCUserCtx& cuctxToUse) :

    TCQCIntfHVBrwRTV
    (
        TString::strEmpty()
        , kCIDLib::c4MaxCard
        , kCQCMedia::strRTVId_ItemName
        , kCQCKit::strRTVId_SelectIndex
        , cuctxToUse
    )
{
}

TCQCIntfMIBrwRTV::TCQCIntfMIBrwRTV( const   TCQCMediaPLItem&    mpliSelected
                                    , const tCIDLib::TCard4     c4SelectIndex
                                    , const TCQCUserCtx&        cuctxToUse) :

    TCQCIntfHVBrwRTV
    (
        mpliSelected.strItemName()
        , c4SelectIndex
        , kCQCMedia::strRTVId_ItemName
        , kCQCKit::strRTVId_SelectIndex
        , cuctxToUse
    )
    , m_mpliSelected(mpliSelected)
{
}

TCQCIntfMIBrwRTV::~TCQCIntfMIBrwRTV()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMIBrwRTV: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIntfMIBrwRTV::bRTValue( const   TString&    strId
                            ,       TString&    strToFill)  const
{
    // Call our parent first
    if (TParent::bRTValue(strId, strToFill))
        return kCIDLib::True;

    // Not one of his so see if it's one of ours
    if (strId == kCQCMedia::strRTVId_ItemCookie)
        strToFill = m_mpliSelected.strItemCookie();
    else if (strId == kCQCMedia::strRTVId_ListItemId)
        strToFill.SetFormatted(m_mpliSelected.c4ListItemId());
    else if (strId == kCQCMedia::strRTVId_LocInfo)
        strToFill = m_mpliSelected.strLocInfo();
    else if (strId == kCQCMedia::strRTVId_ItemArtist)
        strToFill = m_mpliSelected.strItemArtist();
    else if (strId == kCQCMedia::strRTVId_MediaRepoMoniker)
        strToFill = m_mpliSelected.strRepoMoniker();
    else
        return kCIDLib::False;

    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMItemBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMItemBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMItemBrowser::TCQCIntfMItemBrowser() :

    TCQCIntfHVBrowserBase
    (
        CQCIntfEng_MItemBrowser::eCapFlags
        , facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemName)
        , kCQCMedia::strRTVId_ItemName
        , facCQCKit().strMsg(kKitMsgs::midRT_SelectedIndex)
        , kCQCKit::strRTVId_SelectIndex
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_MItemBrowser)
    )
    , MCQCIntfSingleFldIntf
      (
          new TCQCFldFiltTypeName
          (
              tCQCKit::EReqAccess::MReadCWrite
              , tCQCKit::EFldTypes::Card
              , TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PLSerialNum)
              , TCQCStdMediaRendDrv::strRendFld2(tCQCMedia::ERendFlds::PLSerialNum)
          )
      )
{
    //
    //  In addition to the RTVs provided by our parent class (and we tell him
    //  in the ctor call above to use our own RTV name/key for the actual item
    //  text, which is the name of the media item), we provide the other info
    //  available for the item, plus our media repository moniker.
    //
    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemArtist)
        , kCQCMedia::strRTVId_ItemArtist
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemCookie)
        , kCQCMedia::strRTVId_ItemCookie
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_LocInfo)
        , kCQCMedia::strRTVId_LocInfo
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_MediaRepoMoniker)
        , kCQCMedia::strRTVId_MediaRepoMoniker
        , tCQCKit::ECmdPTypes::Driver
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ListItemId)
        , kCQCMedia::strRTVId_ListItemId
        , tCQCKit::ECmdPTypes::Unsigned
    );

    // Default to no text wrap
    bNoTextWrap(kCIDLib::True);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/MediaItemBrowser");
}

TCQCIntfMItemBrowser::
TCQCIntfMItemBrowser(const TCQCIntfMItemBrowser& iwdgSrc) :

    TCQCIntfHVBrowserBase(iwdgSrc)
    , MCQCIntfSingleFldIntf(iwdgSrc)
    , m_colItems(iwdgSrc.m_colItems)
{
}

TCQCIntfMItemBrowser::~TCQCIntfMItemBrowser()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMItemBrowser: Public operators
// ---------------------------------------------------------------------------
TCQCIntfMItemBrowser&
TCQCIntfMItemBrowser::operator=(const TCQCIntfMItemBrowser& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfSingleFldIntf::operator=(iwdgSrc);
        m_colItems = iwdgSrc.m_colItems;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMItemBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfMItemBrowser::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfMItemBrowser& iwdgOther
    (
        static_cast<const TCQCIntfMItemBrowser&>(iwdgSrc)
    );

    // All we have is a mixin
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


//
//  Called to let us indicate if we can accept, and to optionally conform our
//  self to, a new field. In our case we just check that it's a playlist serial
//  number field. We don't need to do anything special to update ourself if
//  storing.
//
tCIDLib::TBoolean
TCQCIntfMItemBrowser::bCanAcceptField(const TCQCFldDef& flddTest
                                    , const TString&
                                    , const TString&
                                    , const tCIDLib::TBoolean)
{
    //
    //  We only accept Card4 fields where the name indicates that it is
    //  the play list serial number field of a renderer driver. We have to check
    //  for both V1 and V2 variations.
    //
    if ((flddTest.eType() == tCQCKit::EFldTypes::Card)
    &&  ((flddTest.strName() == TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PLSerialNum))
    ||   (flddTest.strName() == TCQCStdMediaRendDrv::strRendFld2(tCQCMedia::ERendFlds::PLSerialNum))))
    {
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TVoid TCQCIntfMItemBrowser::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfMItemBrowser&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfMItemBrowser::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LinkToField)
    {
        // Just pass the field name on to our field mixin
        ChangeField(ccfgToDo.piAt(0).m_strValue);
    }
     else
    {
        // Not one of ours, so let our parent try it
        return TParent::eDoCmd
        (
            ccfgToDo
            , c4Index
            , strUserId
            , strEventId
            , ctarGlobals
            , bResult
            , acteTar
        );
    }
    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid
TCQCIntfMItemBrowser::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


// Create a new runtime values object
TCQCCmdRTVSrc*
TCQCIntfMItemBrowser::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    const tCIDLib::TCard4 c4Sel = c4SelectedIndex();
    if (c4Sel == kCIDLib::c4MaxCard)
        return new TCQCIntfMIBrwRTV(cuctxToUse);

    return new TCQCIntfMIBrwRTV(m_colItems[c4Sel], c4Sel, cuctxToUse);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfMItemBrowser::QueryCmds(        TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We just support the changing of the field
    {
        TCQCCmd& cmdLinkToFld = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_LinkToField
                , facCQCKit().strMsg(kKitMsgs::midCmd_LinkToField)
                , tCQCKit::ECmdPTypes::Field
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            )
        );

        cmdLinkToFld.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::MReadCWrite);
    }
}


// Report any monikers we reference
tCIDLib::TVoid
TCQCIntfMItemBrowser::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfMItemBrowser::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Serial Num Fld.");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfMItemBrowser::RegisterFields(       TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfMItemBrowser::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocField))
        MCQCIntfSingleFldIntf::ReplaceField(strSrc, strTar, bRegEx, bFull, regxFind);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfMItemBrowser::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


// Report any validation errors in our configuration
tCIDLib::TVoid
TCQCIntfMItemBrowser::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And pass on to our field mixin
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);

}


// ---------------------------------------------------------------------------
//  TCQCIntfMItemBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

// Our field mixin saw a change in our field, so react to that
tCIDLib::TVoid
TCQCIntfMItemBrowser::FieldChanged(         TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    tCIDLib::TBoolean bReloaded = kCIDLib::False;
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        if (bLoadItems())
        {
            bReloaded = kCIDLib::True;
            ReloadedList();
        }
    }
     else
    {
        // We lost the field, so clear the list
        m_colItems.RemoveAll();
        ReloadedList();
        bReloaded = kCIDLib::True;
    }

    // If we reloaded and not hidden or supressed, then redraw
    if (bReloaded
    &&  (eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden)
    &&  !bNoRedraw)
    {
        Redraw();
    }
}


tCIDLib::TVoid TCQCIntfMItemBrowser::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MItemBrowser::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    //
    //  If this < V3, then eat the stuff from an old parent class that was
    //  factored out. It had no data, so just the start/stop markers.
    //
    if (c2FmtVersion < 3)
    {
        strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);
        tCIDLib::TCard2 c2Tmp;
        strmToReadFrom >> c2Tmp;
    }

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    if (c2FmtVersion < 3)
        strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    //
    //  In theory version 3 and lower objects would require some adjustment to
    //  move over to the single field mixing scheme for association of fields,
    //  but we will never see such versions anymore. They cannot upgrade to
    //  anything post 3.0 until they go to 3.0 first and it does all the
    //  refactoring and conversion.
    //
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // If prior to V5, read in ad discard the repository
    if (c2FmtVersion < 5)
    {
        TString strDummy;
        strmToReadFrom >> strDummy;
    }

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Clear the list until we get good data
    m_colItems.RemoveAll();
}


tCIDLib::TVoid
TCQCIntfMItemBrowser::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_MItemBrowser::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do our mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // Do our stuff and end the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfMItemBrowser::ValueUpdate(          TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}


// ---------------------------------------------------------------------------
//  TCQCIntfMItemBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Lets the base vert list browser class know how many display values we have
//  available.
//
tCIDLib::TCard4 TCQCIntfMItemBrowser::c4NameCount() const
{
    return m_colItems.c4ElemCount();
}


//
//  Lets the base vert list browser class get access to our display values
//  when he needs them.
//
const TString&
TCQCIntfMItemBrowser::strDisplayTextAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colItems[c4Index].strItemName();
}


// ---------------------------------------------------------------------------
//  TCQCIntfMItemBrowser: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to get us to download our list of items from the
//  associated renderer.
//
tCIDLib::TBoolean TCQCIntfMItemBrowser::bLoadItems()
{
    try
    {
        // Get a CQCServer proxy for our renderer driver
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy
        (
            strMoniker()
        );

        //
        //  Query the item list data. The collection cookie field is
        //  not used in this case, so we pass an empty string.
        //
        const tCIDLib::TCard4 c4RetItems = facCQCMedia().c4QueryPLItems
        (
            orbcSrv, strMoniker(), m_colItems
        );
    }

    catch(const TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // Tell the user it failed
        civOwner().ShowErr
        (
            civOwner().strTitle()
            , facCQCIntfEng().strMsg
              (
                kIEngMsgs::midStatus_CantLoadItems, strMoniker()
              )
            , errToCatch
        );

        m_colItems.RemoveAll();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


