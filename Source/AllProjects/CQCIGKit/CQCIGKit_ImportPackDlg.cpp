//
// FILE NAME: CQCIGKit_ImportPackDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/23/2016
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
//  This file implements the driver import helper dialog.
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
#include    "CQCIGKit_.hpp"
#include    "CQCIGKit_ImportPackDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TImportPackDlg,TDlgBox)




// ---------------------------------------------------------------------------
//  CLASS: TAmbigDlg
// PREFIX: dlg
//
//  This is a simple internal dialog to let the user decide which drivers to use when there
//  more multiple possible choices for a given source moniker.
// ---------------------------------------------------------------------------
class TAmbigDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TAmbigDlg();

        TAmbigDlg(const TAmbigDlg&) = delete;
        TAmbigDlg(TAmbigDlg&&) = delete;

        ~TAmbigDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       tCIDLib::TKValsList&    colList
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pcolInfo
        //      We get a pointer to the data to load. It's a list of key/values. The key
        //      is the moniker, the first value is the make and the second is the model,
        //      and the third is a space separated list of possible replacement monikers.
        //
        //      We update the third value with the selected one, if any is available.
        //
        //  m_pwndXXX
        //      We store a few typed pointers to our widgets for convenience. We don't own
        //      them, we just reference them.
        // -------------------------------------------------------------------
        tCIDLib::TKValsList*    m_pcolInfo;
        TPushButton*            m_pwndCancel;
        TPushButton*            m_pwndImport;
        TStaticMultiText*       m_pwndInstruct;
        TMultiColListBox*       m_pwndList;
        TStaticMultiText*       m_pwndWarn;
};

TAmbigDlg::TAmbigDlg() :

    m_pcolInfo(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndImport(nullptr)
    , m_pwndList(nullptr)
    , m_pwndWarn(nullptr)
{
}

TAmbigDlg::~TAmbigDlg()
{
}

tCIDLib::TBoolean
TAmbigDlg::bRunDlg(const TWindow& wndOwner, tCIDLib::TKValsList& colList)
{
    m_pcolInfo = &colList;
    const tCIDLib::TCard4 c4Res = c4RunDlg(wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_ImpPackMap);
    return (c4Res == kCQCIGKit::ridDlg_ImpPackMap_Import);
}

tCIDLib::TBoolean TAmbigDlg::bCreated()
{
    const tCIDLib::TBoolean bRet = TDlgBox::bCreated();

    CastChildWnd(*this, kCQCIGKit::ridDlg_ImpPackMap_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ImpPackMap_Import, m_pwndImport);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ImpPackMap_List, m_pwndList);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ImpPackMap_Warn, m_pwndWarn);

    // Set the columns
    tCIDLib::TStrList colCols(4);
    colCols.objAdd(L"Driver Make");
    colCols.objAdd(L"Driver Model");
    colCols.objAdd(L"Driver Moniker");
    colCols.objAdd(L"Map to This Moniker");
    m_pwndList->SetColumns(colCols);

    // Load up the list
    TString strInitVal;
    TString strList;
    tCIDLib::TCard4 c4WarnCnt = 0;
    const tCIDLib::TCard4 c4Count = m_pcolInfo->c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TKeyValues& kvalsCur = m_pcolInfo->objAt(c4Index);

        // If this one has more than one entry or no entries
        if (kvalsCur.strVal3().bContainsChar(kCIDLib::chSpace) || kvalsCur.strVal3().bIsEmpty())
        {
            // Save the full list of available values
            strList = kvalsCur.strVal3();

            // Take the first one, if any, as the initial one
            strInitVal.Clear();
            if (strList.bIsEmpty())
            {
                c4WarnCnt++;
            }
             else
            {
                strInitVal = strList;
                if (!strInitVal.bCapAtChar(kCIDLib::chSpace))
                {
                    strInitVal.Clear();
                    c4WarnCnt++;
                }
            }

            //
            //  Update the map now for the initial one, so that it's got something valid
            //  if they never make a choice.
            //
            kvalsCur.strVal3(strInitVal);

            colCols[0] = kvalsCur.strVal1();
            colCols[1] = kvalsCur.strVal2();
            colCols[2] = kvalsCur.strKey();
            colCols[3] = strInitVal;
            const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colCols, c4Index);

            // Store the original list in the user data for this row
            m_pwndList->SetUserDataAt(c4At, strList);
        }
    }

    //
    //  If all are mappable, then destroy the warning, else set its color. If we get rid
    //  of the warning, move the bottom of the list down to take up that space.
    //
    if (c4WarnCnt)
    {
        m_pwndWarn->SetTextColor(TRGBClr(0xFF, 0, 0));
    }
     else
    {
        TArea areaWarn = m_pwndWarn->areaWnd();
        m_pwndWarn->Destroy();
        delete m_pwndWarn;

        TArea areaList = m_pwndList->areaWnd();
        areaList.i4Bottom(areaWarn.i4Bottom());
        m_pwndList->SetSize(areaList.szArea(), kCIDLib::False);
    }

    // Set up our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TAmbigDlg::eClickHandler);
    m_pwndImport->pnothRegisterHandler(this, &TAmbigDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TAmbigDlg::eLBHandler);

    return bRet;
}

tCIDCtrls::EEvResponses TAmbigDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    EndDlg(wnotEvent.widSource());
    return tCIDCtrls::EEvResponses::Handled;
}

tCIDCtrls::EEvResponses TAmbigDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    // If they double click a entry, let them select a replacement moniker for that one
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        const tCIDLib::TCard4 c4ListInd = wnotEvent.c4Index();
        const tCIDLib::TCard4 c4MapInd = m_pwndList->c4IndexToId(c4ListInd);

        // The user data is a space separated list of the available replacements
        tCIDLib::TStrList colList;
        TStringTokenizer stokMons(&m_pwndList->strUserDataAt(c4ListInd), L" ");

        TKeyValues& kvalsCur = m_pcolInfo->objAt(c4MapInd);
        const tCIDLib::TCard4 c4Count = stokMons.c4BreakApart(colList);
        if (c4Count)
        {
            //
            //  Use a generic list selection dialog to make the selection. Pass in
            //  the currently selected one, if any, as the initial value.
            //
            TString strSel = kvalsCur.strVal3();
            const tCIDLib::TBoolean bRes = facCIDWUtils().bListSelect
            (
                *this, L"Select a Driver", colList, strSel
            );

            if (bRes)
            {
                // Update the list and the map for this item
                m_pwndList->SetColText(wnotEvent.c4Index(), 3, strSel);
                kvalsCur.strVal3(strSel);
            }
        }
         else
        {
            TErrBox msgbErr(L"There are no available replacements for this driver");
            msgbErr.ShowIt(*this);
        }
    }

    return tCIDCtrls::EEvResponses::Handled;
}




// ---------------------------------------------------------------------------
//   CLASS: TImportPackDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TImportPackDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TImportPackDlg::TImportPackDlg() :

    m_strmImg(4 * 0x10000, 16 * 0x100000)
    , m_strmThumb(0x10000, 2 * 0x100000)
    , m_pwndCancel(nullptr)
    , m_pwndImport(nullptr)
    , m_pwndList(nullptr)
{
}

TImportPackDlg::~TImportPackDlg()
{
}


// ---------------------------------------------------------------------------
//  TImportPackDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TImportPackDlg::bRunDlg(const   TWindow&        wndOwner
                        , const TString&        strPackFile
                        , const TCQCUserCtx&    cuctxToUse)
{
    // Store any incoming info we need for later
    m_cuctxToUse = cuctxToUse;
    m_strPackFile = strPackFile;

    //
    //  Let's try to read in the package file before we bother to create the dialog. The
    //  package file has a helper that will load it up and do a lot of useful validation.
    //
    try
    {
        TCQCPackage::LoadPackage(strPackFile, m_packImport);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbErr(L"The package file could not be imported");
        msgbErr.ShowIt(wndOwner);
        return kCIDLib::False;
    }

    // If there are no files, tell them and give up
    if (!m_packImport.c4FileCount())
    {
        TErrBox msgbInfo(L"The package file is empty, so nothing to do");
        msgbInfo.ShowIt(wndOwner);
        return kCIDLib::False;
    }

    // If the version is too old, then tell them
    if (m_packImport.c8CreateVer() < kCQCIGKit_::c8MinCQCPackVer)
    {
        TErrBox msgbErr(L"The package file is too old to import into this CQC vresion");
        msgbErr.ShowIt(wndOwner);
        return kCIDLib::False;
    }

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_ImportPack
    );
    return (c4Res == kCQCIGKit::ridDlg_ImportPack_Import);
}


// ---------------------------------------------------------------------------
//  TImportPackDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TImportPackDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we use a lot
    CastChildWnd(*this, kCQCIGKit::ridDlg_ImportPack_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ImportPack_Import, m_pwndImport);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ImportPack_List, m_pwndList);

    // Install our handlers
    m_pwndCancel->pnothRegisterHandler(this, &TImportPackDlg::eClickHandler);
    m_pwndImport->pnothRegisterHandler(this, &TImportPackDlg::eClickHandler);

    // Set the columns
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"File Type");
    colCols.objAdd(L"File Path");
    m_pwndList->SetColumns(colCols);

    // Let's load the files into the list
    const tCIDLib::TCard4 c4Count = m_packImport.c4FileCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCPackageFl& pflCur = m_packImport.pflAt(c4Index);

        // Skip some that are for internal use only
        if (pflCur.eFileType() == tCQCKit::EPackFlTypes::DevMap)
            continue;

        colCols[0] = tCQCKit::strXlatEPackFlTypes(pflCur.eFileType());
        colCols[1] = pflCur.strName();
        m_pwndList->c4AddItem(colCols, tCIDLib::c4EnumOrd(pflCur.eFileType()));
    }

    // Size the first column to the content
    m_pwndList->AutoSizeCol(1, kCIDLib::False);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TImportPackDlg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  If a template pack, this is called to preload the driver map. We'll create a list of
//  string lists. The first string in each list is the original moniker in the template,
//  followed by any potential replacements in the current system.
//
tCIDLib::TBoolean TImportPackDlg::bLoadDriverMap()
{
    const TCQCPackageFl* ppflCur = nullptr;

    const tCIDLib::TCard4 c4Count = m_packImport.c4FileCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCPackageFl& pflCur = m_packImport.pflAt(c4Index);
        if (pflCur.eFileType() == tCQCKit::EPackFlTypes::DevMap)
        {
            ppflCur = &pflCur;
            break;
        }
    }

    if (!ppflCur)
    {
        TErrBox msgbErr(L"The template package contained no device map");
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }

    try
    {
        //
        //  Get a list of the current configured drivers, and the info about them that
        //  we need.
        //
        tCIDLib::TStrList       colCfgMons;
        tCIDLib::TStrList       colCfgMakes;
        tCIDLib::TStrList       colCfgModels;
        tCQCKit::TDevCatList    fcolCfgCats;

        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        orbcIS->DrvCfgQueryDrvsInfo(colCfgMons, colCfgMakes, colCfgModels, fcolCfgCats, m_cuctxToUse.sectUser());
        const tCIDLib::TCard4 c4CfgDrvCnt = colCfgMons.c4ElemCount();

        // Stream in the string that contains the map
        TString strMap;
        {
            TBinMBufInStream strmMap(&ppflCur->mbufData(), ppflCur->c4Size());
            strmMap >> strMap;
        }

        // Put a text input stream over the map so we can parse the data
        TTextStringInStream strmMap(&strMap);
        tCIDLib::TCard4 c4AmbigCnt = 0;
        tCIDLib::TCard4 c4UnmapCnt = 0;
        TString strMapItem;
        TString strMoniker;
        TString strMake;
        TString strModel;
        TString strMatches;
        m_colDevMap.RemoveAll();
        while (!strmMap.bEndOfStream())
        {
            if (!strmMap.c4GetLine(strMapItem, 0, kCIDLib::True))
                continue;

            //
            //  The format of each line is:
            //
            //      moniker: make, model
            //
            //  So for each moniker that was found in the original template, we
            //  get the make and model of the device it was referencing.
            //
            TStringTokenizer stokCur(&strMapItem, L":,");
            if (!stokCur.bGetNextToken(strMoniker)
            ||  !stokCur.bGetNextToken(strMake)
            ||  !stokCur.bGetNextToken(strModel))
            {
                facCQCIGKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kIGKitErrs::errcPack_BadMapItem
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }

            //
            //  Be sure to get rid of leading/trailing whitespace. We can't
            //  add space to the whitspace list above because makes and models
            //  can have spaces in them.
            //
            strMoniker.StripWhitespace();
            strMake.StripWhitespace();
            strModel.StripWhitespace();

            //
            //  Now search the currently configured drivers and find any of the same
            //  make/model and create a space separated list of them, and set that as
            //  the third value.
            //
            tCIDLib::TCard4 c4Matches = 0;
            strMatches.Clear();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CfgDrvCnt; c4Index++)
            {
                if ((colCfgMakes[c4Index] == strMake) && (colCfgModels[c4Index] == strModel))
                {
                    if (!strMatches.bIsEmpty())
                        strMatches.Append(kCIDLib::chSpace);
                    strMatches.Append(colCfgMons[c4Index]);
                    c4Matches++;
                }
            }

            // Remember if this is one we have to deal with specially
            if (c4Matches > 1)
                c4AmbigCnt++;
            else if (c4Matches == 0)
                c4UnmapCnt++;

            // Add this one to our map
            m_colDevMap.objPlace(strMoniker, strMake, strModel, strMatches);
        }

        //
        //  If there are any driver references that have no possible equivalent on this
        //  current target system, or that have multiples, tell the user.
        //
        if (c4UnmapCnt || c4AmbigCnt)
        {
            TAmbigDlg dlgAmbig;
            if (!dlgAmbig.bRunDlg(*this, m_colDevMap))
                return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbErr(L"An error occurred while processing the device map");
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TImportPackDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_ImportPack_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_ImportPack_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_ImportPack_Import)
    {
        try
        {
            // OK, let's do it. We go through the files and
            TDataSrvClient dsclLoad;

            //
            //  If this is a template pack, then let's get out the driver mapping blob
            //  first and process it, so that it'll be there when we get to the template
            //  (which is actually before it in the file.
            //
            if (m_packImport.eType() == tCQCKit::EPackTypes::IntfTemplate)
            {
                if (!bLoadDriverMap())
                    return tCIDCtrls::EEvResponses::Handled;
            }

            const tCIDLib::TCard4 c4Count = m_packImport.c4FileCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TCQCPackageFl& pflCur = m_packImport.pflAt(c4Index);
                UploadFile(dsclLoad, pflCur);
            }
            EndDlg(kCQCIGKit::ridDlg_ImportPack_Import);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"An error occurred while upload the content"
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Templates are a special case, with a fair amount of work, so we split it out. There's
//  currently only one template per template pack.
//
tCIDLib::TVoid
TImportPackDlg::ImportTemplate(TDataSrvClient& dsclLoad, const TCQCPackageFl& pflTmpl)
{
    //
    //  We have one messy issue here. If the interface engine is not in designer mode, it
    //  will auto-name all of the unnamed widgets, since that's required at viewing time.
    //  But we don't want that to happen here, so we need to set the edit mode flag around
    //  the streaming in of the template, but make sure it gets back to the original before
    //  we continue.
    //
    //  We also have to deal with legacy packages vs. new ones. If it's old one the data
    //  is just the flatted template. Else it's a chunked file.
    //
    TChunkedFile chflTmpl;
    TCQCIntfTemplate iwdgTmpl;
    {
        const tCIDLib::TBoolean bOrgDesMode = TFacCQCIntfEng::bDesMode();
        TFacCQCIntfEng::bDesMode(kCIDLib::True);
        try
        {
            if (pflTmpl.bIsChunked())
            {
                TBinMBufInStream strmFile(&pflTmpl.mbufData(), pflTmpl.c4Size());
                strmFile >> chflTmpl;

                // Get the template out for the code below to update
                TBinMBufInStream strmTmpl(&chflTmpl.mbufData(), chflTmpl.c4DataBytes());
                strmTmpl >> iwdgTmpl;
            }
             else
            {
                // Get teh template out for the code below to update
                TBinMBufInStream strmTmpl(&pflTmpl.mbufData(), pflTmpl.c4Size());
                strmTmpl >> iwdgTmpl;

                // Chunkify the template so we are only dealing with the new format below
                facCQCRemBrws().ChunkUpTemplate
                (
                    pflTmpl.mbufData()
                    , pflTmpl.c4Size()
                    , chflTmpl
                    , iwdgTmpl.eMinRole()
                    , iwdgTmpl.strNotes()
                    , 1
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TFacCQCIntfEng::bDesMode(bOrgDesMode);
            throw;
        }
        TFacCQCIntfEng::bDesMode(bOrgDesMode);
    }

    //
    //  Call the update fixup method so that we can catch some errors from older versions
    //  that might otherwise get into the system via import. Normally this happens during
    //  the installer's data upgrade process, but package import gets around that.
    //
    iwdgTmpl.DoUpdateFixes();

    //
    //  We now do a set of search and replace operations to replace each original referenced
    //  driver moniker with the selected replacement, if one was available.
    //
    TRegEx regxDummy;
    TString strOldMon;
    TString strNewMon;
    const tCIDLib::TCard4 c4Count = m_colDevMap.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValues& kvalsCur = m_colDevMap[c4Index];

        // If there's no replacement, skip this one
        if (kvalsCur.strVal3().bIsEmpty())
            continue;

        //
        //  Else let's do the replacement. First do an associated device replacement,
        //  with the raw device names. It has to be a full match
        //
        strOldMon = kvalsCur.strKey();
        strNewMon = kvalsCur.strVal3();

        iwdgTmpl.Replace
        (
            tCQCIntfEng::ERepFlags::ActionParms
            | tCQCIntfEng::ERepFlags::AssocDevice
            | tCQCIntfEng::ERepFlags::Children
            | tCQCIntfEng::ERepFlags::Expressions
            | tCQCIntfEng::ERepFlags::FilterParms
            , strOldMon
            , strNewMon
            , kCIDLib::False
            , kCIDLib::True
            , regxDummy
        );

        //
        //  Now put a period at the end and do an associated field replacement. This one
        //  needs to do partial replacements, which is why we put the period at the end, so
        //  that it can can't accidentally match part of a moniker at the start of a field
        //  name.
        //
        strOldMon.Append(kCIDLib::chPeriod);
        strNewMon.Append(kCIDLib::chPeriod);
        iwdgTmpl.Replace
        (
            tCIDLib::eOREnumBits
            (
                tCQCIntfEng::ERepFlags::ActionParms
                , tCQCIntfEng::ERepFlags::AssocField
                , tCQCIntfEng::ERepFlags::Children
                , tCQCIntfEng::ERepFlags::Expressions
                , tCQCIntfEng::ERepFlags::FilterParms
            )
            , strOldMon
            , strNewMon
            , kCIDLib::False
            , kCIDLib::False
            , regxDummy
        );
    }

    // Make sure the target scope exists
    TString strScope(pflTmpl.strName());
    facCQCRemBrws().bRemoveLastPathItem(strScope);
    dsclLoad.MakePath(strScope, tCQCRemBrws::EDTypes::Template, m_cuctxToUse.sectUser());

    // OK, now we can save it, so get the name
    TString strRelPath = pflTmpl.strName();

    // Handle pre=3.1.2 templates which were before the hierarchical storage
    if (!strRelPath.bStartsWithI(kCQCKit::strRepoScope_UserTS)
    &&  !strRelPath.bStartsWithI(kCQCKit::strRepoScope_SystemTS))
    {
        strRelPath.Prepend(kCQCKit::strRepoScope_UserTS);
    }

    //
    //  Flatten the new changes back out again and update the chunked file with the
    //  new info.
    //
    {
        TBinMBufOutStream strmFixed(128 * 1024);
        strmFixed << iwdgTmpl << kCIDLib::FlushIt;
        chflTmpl.bSetChunkById
        (
            kCIDMData::strChunkId_Data, strmFixed.c4CurSize(), strmFixed.mbufData()
        );
    }

    // Tell it that, if not already in the memory cache, don't add it
    tCIDLib::TCard4 c4SerialNum = 0;
    tCIDLib::TCard4 c4Flags(kCQCRemBrws::c4Flag_NoDataCache | kCQCRemBrws::c4Flag_OverwriteOK);
    tCIDLib::TEncodedTime enctLastChange;

    dsclLoad.UploadFile
    (
        strRelPath
        , tCQCRemBrws::EDTypes::Template
        , c4SerialNum
        , enctLastChange
        , chflTmpl
        , c4Flags
        , m_cuctxToUse.sectUser()
    );
}



// Handles uploading a single file in the appropriate way
tCIDLib::TVoid
TImportPackDlg::UploadFile(TDataSrvClient& dsclLoad, const TCQCPackageFl& pflLoad)
{
    tCIDLib::TCard4         c4SerialNum = 0;
    tCIDLib::TEncodedTime   enctLastChange;

    //
    //  Don't force them into the cache if not already, and of course we can overwite
    //  since the user controls whether we do it or not.
    //
    tCIDLib::TCard4 c4Flags
    (
        kCQCRemBrws::c4Flag_NoDataCache | kCQCRemBrws::c4Flag_OverwriteOK
    );

    if (pflLoad.eFileType() == tCQCKit::EPackFlTypes::PDLFile)
    {
        // We need an installation server proxy for this one
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        orbcIS->UploadProtocol
        (
            pflLoad.strName()
            , pflLoad.c4Size()
            , pflLoad.mbufData()
            , m_packImport.strUserData() == L"System"
            , m_cuctxToUse.sectUser()
        );
    }
     else if (pflLoad.eFileType() == tCQCKit::EPackFlTypes::CMLMacro)
    {
        // Make sure the target scope exists
        TString strScope(pflLoad.strName());
        facCQCRemBrws().bRemoveLastPathItem(strScope);
        dsclLoad.MakePath(strScope, tCQCRemBrws::EDTypes::Macro, m_cuctxToUse.sectUser());

        //
        //  It's either just a flattened string with the macro contents or it's
        //  a new style chunked file.
        //
        TChunkedFile chflMacro;
        if (pflLoad.bIsChunked())
        {
            {
                TBinMBufInStream strmSrc(&pflLoad.mbufData(), pflLoad.c4Size());
                strmSrc >> chflMacro;
            }

        }
         else
        {
            TString strMacro;
            {
                TBinMBufInStream strmSrc(&pflLoad.mbufData(), pflLoad.c4Size());
                strmSrc >> strMacro;
            }
            facCQCRemBrws().ChunkUpMacro(strMacro, chflMacro, 1);
        }

        dsclLoad.UploadFile
        (
            pflLoad.strName()
            , tCQCRemBrws::EDTypes::Macro
            , c4SerialNum
            , enctLastChange
            , chflMacro
            , c4Flags
            , m_cuctxToUse.sectUser()
        );
    }
     else if (pflLoad.eFileType() == tCQCKit::EPackFlTypes::Manifest)
    {
        // We need an installation server proxy for this one
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        TPathStr pathMan(pflLoad.strName());
        pathMan.AppendExt(L"Manifest");
        orbcIS->UploadManifest
        (
            pathMan
            , pflLoad.c4Size()
            , pflLoad.mbufData()
            , m_packImport.strUserData() == L"System"
            , m_cuctxToUse.sectUser()
        );
    }
     else if (pflLoad.eFileType() == tCQCKit::EPackFlTypes::PNGImage)
    {
        // Make sure the target scope exists
        TString strScope(pflLoad.strName());
        facCQCRemBrws().bRemoveLastPathItem(strScope);
        dsclLoad.MakePath(strScope, tCQCRemBrws::EDTypes::Image, m_cuctxToUse.sectUser());

        //
        //  It's either just a PNG image or a new style chunked file. If the former,
        //  we need to chunk it so we are dealing with a whole chunked file either way.
        //
        TChunkedFile chflImg;
        if (pflLoad.bIsChunked())
        {
            TBinMBufInStream strmSrc(&pflLoad.mbufData(), pflLoad.c4Size());
            strmSrc >> chflImg;
        }
         else
        {
            TBinMBufInStream strmSrc(&pflLoad.mbufData(), pflLoad.c4Size());
            strmSrc >> m_imgToLoad;

            // Create a chunked file for this guy
            facCQCRemBrws().ChunkUpImage(m_imgToLoad, chflImg, c4SerialNum);
        }

        dsclLoad.UploadFile
        (
            pflLoad.strName()
            , tCQCRemBrws::EDTypes::Image
            , c4SerialNum
            , enctLastChange
            , chflImg
            , c4Flags
            , m_cuctxToUse.sectUser()
        );
    }
     else if (pflLoad.eFileType() == tCQCKit::EPackFlTypes::IntfTmpl)
    {
        ImportTemplate(dsclLoad, pflLoad);
    }
}

