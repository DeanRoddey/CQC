//
// FILE NAME: ZWaveLevi2C_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2014
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
//  This file implements the main window of the V2 Leviton ZWave client driver
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2C_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveLevi2CWnd,TCQCDriverClient)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLevi2C_Window
{
    const TString strAttr_BasicType(L"/Unit/BasicType");
    const TString strAttr_Enabled(L"/Unit/Enabled");
    const TString strAttr_MakeModel(L"/Unit/MakeModel");
    const TString strAttr_Name(L"/Unit/Name");

    // The actual option name is added after this, for each available option
    const TString strAttr_OptBase(L"/Unit/Option/");
}



// ---------------------------------------------------------------------------
//  CLASS: TUnitAttrWnd
// PREFIX: wnd
//
//  We just need a simple derivative of the attribute editor, so that we can provide some
//  custom visual editing.
// ---------------------------------------------------------------------------
class TUnitAttrWnd : public TAttrEditWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TUnitAttrWnd();

        TUnitAttrWnd(const TUnitAttrWnd&) = delete;

        ~TUnitAttrWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TUnitAttrWnd& operator=(const TUnitAttrWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetEditInfo
        (
                    TZWaveLevi2CWnd* const  pwndDriver
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bVisEdit
        (
                    TAttrData&              adatEdit
            , const TArea&                  areaCellScr
            , const tCIDLib::TCard8         c8UserId
        )   override;

        tCIDLib::TVoid CellClicked
        (
            const   tCIDLib::TCard4         c4Row
            , const tCIDLib::TCard4         c4Column
            , const tCIDLib::TBoolean       bLeftButton
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndDriver
        //      We are given a pointer to the driver window which we need in order to
        //      get to the unit and device index stuff.
        // -------------------------------------------------------------------
        TZWaveLevi2CWnd*    m_pwndDriver;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TUnitAttrWnd, TAttrEditWnd);
};
AdvRTTIDecls(TUnitAttrWnd, TAttrEditWnd);


// ---------------------------------------------------------------------------
//  TUnitAttrWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TUnitAttrWnd::TUnitAttrWnd() :

    m_pwndDriver(nullptr)
{
}

TUnitAttrWnd::~TUnitAttrWnd()
{
}


// ---------------------------------------------------------------------------
//  TUnitAttrWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TUnitAttrWnd::SetEditInfo(TZWaveLevi2CWnd* const pwndDriver)
{
    m_pwndDriver = pwndDriver;
}


// ---------------------------------------------------------------------------
//  TUnitAttrWnd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TUnitAttrWnd::bVisEdit(         TAttrData&      adatEdit
                        , const TArea&          areaCellScr
                        , const tCIDLib::TCard8 c8UserId)
{
    // We have to intercept this one
    if (adatEdit.strId() == ZWaveLevi2C_Window::strAttr_MakeModel)
    {
        // Get the unit and index info
        TZWaveUnit* punitEdit = m_pwndDriver->punitById(tCIDLib::TCard4(c8UserId));
        if (punitEdit)
        {
            TZWaveSelTypeDlg dlgEdit;
            if (dlgEdit.bRunDlg(*this, *punitEdit, m_pwndDriver->swdinToUse()))
            {
                // Format the value back out so the attribute editor will see it
                TString strNewVal = punitEdit->strMake();
                strNewVal.Append(kCIDLib::chForwardSlash);
                strNewVal.Append(punitEdit->strModel());
                adatEdit.SetString(strNewVal);
            }
        }
        return kCIDLib::True;
    }

    // Let the base class check it first
    if (!TAttrEditWnd::bVisEdit(adatEdit, areaCellScr, c8UserId))
        return kCIDLib::False;

    return kCIDLib::True;
}


tCIDLib::TVoid
TUnitAttrWnd::CellClicked(  const   tCIDLib::TCard4     c4Row
                            , const tCIDLib::TCard4     c4Column
                            , const tCIDLib::TBoolean   bLeftButton)
{
    //
    //  If the driver is scanning, we prevent editing. Else we just let it go on to
    //  the underlying class.
    //
    if (bLeftButton && m_pwndDriver->bInRescan())
    {
        TErrBox msgbBusy(L"The driver is currently rescanning, please wait until its done");
        msgbBusy.ShowIt(*this);
        return;
    }

    TParent::CellClicked(c4Row, c4Column, bLeftButton);
}



// ---------------------------------------------------------------------------
//   CLASS: TZWaveLevi2CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveLevi2CWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveLevi2CWnd::TZWaveLevi2CWnd(const  TCQCDriverObjCfg&   cqcdcThis
                                , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis, L"TZWaveLevi2CWnd", tCQCKit::EActLevels::Low, cuctxToUse
    )
    , m_bNewConfig(kCIDLib::False)
    , m_bInRescan(kCIDLib::False)
    , m_c4ColInd_BasicType(3)
    , m_c4ColInd_Enabled(0)
    , m_c4ColInd_Id(1)
    , m_c4ColInd_MakeModel(4)
    , m_c4ColInd_Name(5)
    , m_c4ColInd_Status(2)
    , m_c4LastSerialNum(0)
    , m_pwndAddAssoc(nullptr)
    , m_pwndAttrEd(nullptr)
    , m_pwndAutoCfg(nullptr)
    , m_pwndDelete(nullptr)
    , m_pwndPanes(nullptr)
    , m_pwndQAssoc(nullptr)
    , m_pwndRename(nullptr)
    , m_pwndRescan(nullptr)
    , m_pwndScanLED(nullptr)
    , m_pwndScanText(nullptr)
    , m_pwndSetCfgParm(nullptr)
    , m_pwndUnits(nullptr)
{
}

TZWaveLevi2CWnd::~TZWaveLevi2CWnd()
{
}


// ---------------------------------------------------------------------------
//  TZWaveLevi2CWnd: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWaveLevi2CWnd::bIPETestCanEdit(const  tCIDCtrls::TWndId   widSrc
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    // We just want to make sure that the driver isn't scanning
    if (m_bInRescan)
    {
        TErrBox msgbBusy(L"The driver is currently rescanning, please wait until its done");
        msgbBusy.ShowIt(*this);
        return kCIDLib::False;
    }
    return bIPETestCanEdit(widSrc, c4ColInd, adatFill, bValueSet);
}


tCIDLib::TBoolean TZWaveLevi2CWnd::bChanges() const
{
    return (m_dcfgPrev != m_dcfgCur);
}


tCIDLib::TBoolean
TZWaveLevi2CWnd::bIPEValidate(  const   TString&            strSrc
                                ,       TAttrData&          adatVal
                                , const TString&            strNewVal
                                ,       TString&            strErrMsg
                                ,       tCIDLib::TCard8&    c8UserId) const
{
    //
    //  Call the base mixin class to do basic validation. If that fails, then
    //  no need to look further.
    //
    if (!MIPEIntf::bIPEValidate(strSrc, adatVal, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    if (adatVal.strId() == ZWaveLevi2C_Window::strAttr_Name)
    {
        // Make sure the name is a valid field name
        if (!facCQCKit().bIsValidFldName(strNewVal, kCIDLib::True))
        {
            strErrMsg = facCQCKit().strMsg(kKitErrs::errcFld_BadNameChars);
            return kCIDLib::False;
        }

        // Make sure it's not a dup
        if (m_dcfgCur.bCheckDupName(strNewVal))
        {
            strErrMsg = L"There is already a unit with that name";
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TZWaveLevi2CWnd::bSaveChanges(TString& strErrMsg)
{
    // If no changes, then nothing to do
    if (m_dcfgPrev == m_dcfgCur)
        return kCIDLib::True;

    //
    //  See if a unit has been marked as the VRCOP. If not, warn about that
    //  since some stuff depends on it.
    //
    if (!m_dcfgCur.c4FindVRCOP(kCIDLib::False))
    {
        strErrMsg = facZWaveLevi2Sh().strMsg(kZW2ShErrs::errcCfg_NoVRCOP);
        return kCIDLib::False;
    }

    // Let's upload the configuration to the server side driver
    try
    {
        // Flatten it down to a buffer and upload that
        tCIDLib::TCard4 c4Bytes;
        THeapBuf mbufCfg(8192UL);
        {
            TBinMBufOutStream strmCfg(&mbufCfg);
            strmCfg << m_dcfgCur << kCIDLib::FlushIt;
            c4Bytes = strmCfg.c4CurSize();
        }

        orbcServer()->SetConfig
        (
            strMoniker()
            , c4Bytes
            , mbufCfg
            , cuctxToUse().sectUser()
            , tCQCKit::EDrvCmdWaits::NoWait
        );

        // It worked, so copy the current stuff to the previous config copy.
        m_dcfgPrev = m_dcfgCur;
    }

    catch(TError& errToCatch)
    {
        if (facZWaveLevi2C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_CfgUploadFailed);
        kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TZWaveLevi2CWnd: Public, non-virtual methods
//
//  These are for our attribute editor window, which has to access some of our info. He
//  gets a pointer to us and uses that to get some info he needs.
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWaveLevi2CWnd::bInRescan() const
{
    return m_bInRescan;
}


const TZDIIndex& TZWaveLevi2CWnd::swdinToUse()
{
    // Load the index if it's not been loaded yet
    LoadIndex();
    return m_swdinList;
}


TZWaveUnit* TZWaveLevi2CWnd::punitById(const tCIDLib::TCard4 c4IdToFind)
{
    return m_dcfgCur.punitFindById(c4IdToFind);
}



// ---------------------------------------------------------------------------
//  TZWaveLevi2CWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWaveLevi2CWnd::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the main controls
    TDlgDesc dlgdMain;
    facZWaveLevi2C().bCreateDlgDesc(kZWaveLevi2C::ridMain_Intf, dlgdMain);

    // Update the window class of the attribute editor to our own derivative
    dlgdMain.SetCppType(kZWaveLevi2C::ridMain_Attrs, L"TUnitAttrWnd");

    // And now create the controls
    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdMain, widInitFocus);

    //
    //  Now do an initial auto-size to fit our client area. This will cause the
    //  the pane window to be resized, and it in turn will resize the panes, which
    //  will resize their contents.
    //
    AutoAdjustChildren(dlgdMain.areaPos(), areaClient());

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kZWaveLevi2C::ridMain_AddAssoc, m_pwndAddAssoc);
    CastChildWnd(*this, kZWaveLevi2C::ridMain_AutoCfg, m_pwndAutoCfg);
    CastChildWnd(*this, kZWaveLevi2C::ridMain_Delete, m_pwndDelete);
    CastChildWnd(*this, kZWaveLevi2C::ridMain_Attrs, m_pwndAttrEd);
    CastChildWnd(*this, kZWaveLevi2C::ridMain_QAssoc, m_pwndQAssoc);
    CastChildWnd(*this, kZWaveLevi2C::ridMain_ScanLED, m_pwndScanLED);
    CastChildWnd(*this, kZWaveLevi2C::ridMain_ScanText, m_pwndScanText);
    CastChildWnd(*this, kZWaveLevi2C::ridMain_Rescan, m_pwndRescan);
    CastChildWnd(*this, kZWaveLevi2C::ridMain_SetCfgParm, m_pwndSetCfgParm);
    CastChildWnd(*this, kZWaveLevi2C::ridMain_Units, m_pwndUnits);

    // Set our text so that any popups we do will pick it up
    strWndText(facZWaveLevi2C().strMsg(kZW2CMsgs::midGen_Title));

    // Initially hide the scan widgets
    m_pwndScanLED->SetVisibility(kCIDLib::False);
    m_pwndScanText->SetVisibility(kCIDLib::False);

    // Load a small red LED image into the scan LED control, and set the text to red
    m_pwndScanLED->SetBitmap(facCIDCtrls().bmpLoadSmallAppImg(L"Red Ball"));
    m_pwndScanText->SetTextColor(facCIDGraphDev().rgbRed);

    // Register event handlers
    m_pwndAddAssoc->pnothRegisterHandler(this, &TZWaveLevi2CWnd::eClickHandler);
    m_pwndAutoCfg->pnothRegisterHandler(this, &TZWaveLevi2CWnd::eClickHandler);
    m_pwndDelete->pnothRegisterHandler(this, &TZWaveLevi2CWnd::eClickHandler);
    m_pwndQAssoc->pnothRegisterHandler(this, &TZWaveLevi2CWnd::eClickHandler);
    m_pwndRescan->pnothRegisterHandler(this, &TZWaveLevi2CWnd::eClickHandler);
    m_pwndSetCfgParm->pnothRegisterHandler(this, &TZWaveLevi2CWnd::eClickHandler);

    m_pwndAttrEd->pnothRegisterHandler(this, &TZWaveLevi2CWnd::eAttrEditHandler);
    m_pwndUnits->pnothRegisterHandler(this, &TZWaveLevi2CWnd::eLBHandler);

    // Give the attribute editor window a pointer to us
    m_pwndAttrEd->SetEditInfo(this);

    //
    //  Set us as the IPE interface.
    //
    m_pwndAttrEd->SetIPEHandler(this);

    // Set up the units columns (in the order indicated by our column index members)
    tCIDLib::TStrList colCols(6);
    colCols.objAdd(L"Enabled");
    colCols.objAdd(L"Unit Id");
    colCols.objAdd(L"Unit Status");
    colCols.objAdd(L"Basic Z-Wave Unit Type");
    colCols.objAdd(L"CQC Unit Make/Model");
    colCols.objAdd(L"Unit Name");
    m_pwndUnits->SetColumns(colCols);

    // Size all but the last to their titles. The last is autosized
    m_pwndUnits->AutoSizeCol(0, kCIDLib::True);
    m_pwndUnits->AutoSizeCol(1, kCIDLib::True);
    m_pwndUnits->AutoSizeCol(2, kCIDLib::True);
    m_pwndUnits->AutoSizeCol(3, kCIDLib::True);
    m_pwndUnits->AutoSizeCol(4, kCIDLib::True);

    // Now force a redraw of the whole area
    Redraw(areaClient(), tCIDCtrls::ERedrawFlags::Full);

    return kCIDLib::True;
}


tCIDLib::TBoolean
TZWaveLevi2CWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    m_bNewConfig = kCIDLib::False;
    m_c4LastSerialNum = 0;

    // Do any initial load of any config
    try
    {
        // Query the driver configuration. If we get it, then store it away.
        QueryConfig(orbcTarget, m_dcfgNew, m_c4NewSerialNum);
        LoadConfig(orbcTarget, m_dcfgNew, m_c4NewSerialNum);
    }

    catch(...)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We just check to see if the driver is there there, and if so what his
//  current configuration serial number is.
//
tCIDLib::TBoolean TZWaveLevi2CWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        //
        //  Tell the server to do this as a NON-QUEUED command, so that we can get to it
        //  even if the server driver is off doing a network scan.
        //
        tCIDLib::TCard4 c4SerialNum = orbcServer()->c4QueryVal
        (
            strMoniker(), kZWaveLevi2Sh::strQCardV_Status, kCIDLib::True
        );

        //
        //  The high bit is the flag that indicates whether or not the driver is
        //  currently doing a rescan pass.
        //
        const tCIDLib::TBoolean bPrevRescan = m_bInRescan;
        m_bInRescan = ((c4SerialNum & 0x80000000) != 0);
        c4SerialNum &= 0x7FFFFFFF;

        // Set the scan widget's visibility based on the rescan flag
        if (m_bInRescan != bPrevRescan)
        {
            m_pwndScanLED->SetVisibility(m_bInRescan);
            m_pwndScanText->SetVisibility(m_bInRescan);
        }

        //
        //  If the serial number has changed, then try to query the
        //  current configuration data. If we get it stored away, then
        //  set the new config flag.
        //
        if (c4SerialNum != m_c4LastSerialNum)
        {
            QueryConfig(orbcTarget, m_dcfgNew, m_c4NewSerialNum);
            m_bNewConfig = kCIDLib::True;

            // Return true so UpdateDisplayData will be called
            bRet = kCIDLib::True;
        }
    }

    catch(TError& errToCatch)
    {
        if (facZWaveLevi2C().bLogWarnings() && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        throw;
    }
    return bRet;
}


tCIDLib::TVoid TZWaveLevi2CWnd::Connected()
{
}


tCIDLib::TVoid TZWaveLevi2CWnd::DoCleanup()
{
}

tCIDLib::TVoid TZWaveLevi2CWnd::LostConnection()
{

}


//
//  If the poll thread indicates we have new data, then let's load it up. Clear the new
//  data flag first, so that if something goes wrong we don't get stuck in an endless
//  loop.
//
tCIDLib::TVoid TZWaveLevi2CWnd::UpdateDisplayData()
{
    if (m_bNewConfig)
    {
        m_bNewConfig = kCIDLib::False;
        LoadConfig(orbcServer(), m_dcfgNew, m_c4NewSerialNum);
    }
}


// ---------------------------------------------------------------------------
//  TZWaveLevi2CWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The user has asked for an auto-config on the selected unit. We check it to make
//  sure it's set up. We need device info set on it, since that's where the config
//  info is. And we see if it defines any auto-configuration operations.
//
tCIDLib::TVoid TZWaveLevi2CWnd::DoAutoCfg(const tCIDLib::TCard4 c4Id)
{
    const TZWaveUnit& unitCur = m_dcfgCur.unitFindById(c4Id);

    // If the device info hasn't been set, can't do it
    if (!unitCur.bDevInfoLoaded())
    {
       TErrBox msgbVRCOP(facZWaveLevi2C().strMsg(kZW2CErrs::errcCfg_NoDevInfo));
        msgbVRCOP.ShowIt(*this);
        return;
    }

    // If no auto-config steps defined, then do nothing
    const tCIDLib::TCard4 c4Steps = unitCur.zwdiThis().m_colAutoCfg.c4ElemCount();
    if (!c4Steps)
    {
        TErrBox msgbNA(facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_NoAutoCfgInfo));
        msgbNA.ShowIt(*this);
        return;
    }

    // The VRCOP has to be configured
    const tCIDLib::TCard4 c4VRCOPId = m_dcfgCur.c4FindVRCOP(kCIDLib::False);
    if (!c4VRCOPId)
    {
        TErrBox msgbVRCOP(facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_NoVRCOP));
        msgbVRCOP.ShowIt(*this);
        return;
    }

    try
    {
        // Start the operation
        orbcServer()->c4SendCmd
        (
            strMoniker()
            , kZWaveLevi2Sh::strSendCmd_AutoCfg
            , unitCur.strName()
            , sectUser()
        );

        TOkBox msgbOk(facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_CfgStarted));
        msgbOk.ShowIt(*this);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this
            , facZWaveLevi2C().strMsg(kZW2CMsgs::midGen_Title)
            , facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_AutoCfgFailed)
            , errToCatch
        );
    }
}


//
//  We handle notifications from the attribute editor window, so that we can update the
//  main list with changes.
//
tCIDCtrls::EEvResponses TZWaveLevi2CWnd::eAttrEditHandler(TAttrEditInfo& wnotInfo)
{
    // Find the target unit
    const tCIDLib::TCard4 c4UnitId = tCIDLib::TCard4(wnotInfo.c8UserId());
    TZWaveUnit& unitTar = m_dcfgCur.unitFindById(c4UnitId);

    // And the index of this item in the main list
    const tCIDLib::TCard4 c4ListInd = m_pwndUnits->c4IdToIndex(c4UnitId);

    //
    //  Watch for changes to attributes we need to store. Some are directly edited into the
    //  unit, where it requires a dialog box to do the editing, so those we have already
    //  stored. The others we need to store here.
    //
    //  We have to store it in the unit and update the main list
    //
    const TAttrData& adatChanged = wnotInfo.adatNew();
    if (adatChanged.strId() == ZWaveLevi2C_Window::strAttr_Name)
    {
        unitTar.strName(adatChanged.strValue());
        m_pwndUnits->SetColText(c4ListInd, m_c4ColInd_Name, adatChanged.strValue());
    }
     else if (adatChanged.strId() == ZWaveLevi2C_Window::strAttr_Enabled)
    {
        // Set the unit disabled state (to the opposite of our enabled semantics.)
        unitTar.bDisabled(!adatChanged.bVal());
        m_pwndUnits->SetColText
        (
            c4ListInd, m_c4ColInd_Enabled, facCQCKit().strBoolYesNo(adatChanged.bVal())
        );
    }
     else if (adatChanged.strId() == ZWaveLevi2C_Window::strAttr_MakeModel)
    {
        //
        //  THE UNIT was updated during editing, just update the display. The gotcha is
        //  that this could change the attributes that are available. So we have to actually
        //  reload the attribute editor.
        //
        unitTar.bQueryOpts(m_colUnitOpts);
        LoadUnitAttributes(unitTar);

        // And update the list box
        m_pwndUnits->SetColText(c4ListInd, m_c4ColInd_MakeModel, adatChanged.strValue());

        // Make sure we don't do anything below that would assume the previous contents
        return tCIDCtrls::EEvResponses::Handled;
    }
     else if (adatChanged.strId().bStartsWith(ZWaveLevi2C_Window::strAttr_OptBase))
    {
        //
        //  These are all module options. Get the bit after the base prefix, and use that
        //  to find this one in the options list.
        //
        //  We can then find the option value key of the selected option, based on the
        //  display value we get.
        //
        TString strOptKey = adatChanged.strId();
        strOptKey.Cut(0, ZWaveLevi2C_Window::strAttr_OptBase.c4Length());

        const TString& strNewVal = adatChanged.strValue();

        // Search the current unit options for the key that changed
        const tCIDLib::TCard4 c4Count = m_colUnitOpts.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TZWOption& zwoptCur = m_colUnitOpts[c4Index];
            if (zwoptCur.m_strKey == strOptKey)
            {
                //
                //  This is our guy. Search the option values list for the display value
                //  we got.
                //
                const tCIDLib::TCard4 c4VCount = zwoptCur.m_colValues.c4ElemCount();
                for (tCIDLib::TCard4 c4VIndex = 0; c4VIndex < c4VCount; c4VIndex++)
                {
                    if (zwoptCur.m_colValues[c4VIndex].strValue() == strNewVal)
                        unitTar.SetOptVal(strOptKey, zwoptCur.m_colValues[c4VIndex].strKey());
                }
                break;
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called on a button click
//
tCIDCtrls::EEvResponses TZWaveLevi2CWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    //
    //  If we are not connected, then tell them we can't do it. The only thing valid to do
    //  is save new config, which isn't done via a button.
    //
    if (eConnState() != tCQCGKit::EConnStates::Connected)
    {
        TErrBox msgbErr(facZWaveLevi2C().strMsg(kZW2CErrs::errcGen_NotConnected));
        msgbErr.ShowIt(*this);
        return tCIDCtrls::EEvResponses::Handled;
    }

    if (m_bInRescan)
    {
        TOkBox msgbBusy(facZWaveLevi2C().strMsg(kZW2CErrs::errcGen_RescanActive));
        msgbBusy.ShowIt(*this);
        return tCIDCtrls::EEvResponses::Handled;
    }

    // Get the selected unit index for later use
    const tCIDLib::TCard4 c4ListInd = m_pwndUnits->c4CurItem();

    //
    //  Don't allow timers while we are in here, so that the configuration
    //  cannot change behind our backs. If we allowed timers, then the
    //  poll callback could happen as soon as we enter a dialog and start
    //  processing messages again.
    //
    TTimerSusJanitor janTimers(this);

    try
    {
        if (wnotEvent.widSource() == kZWaveLevi2C::ridMain_Rescan)
        {
            // Just send the driver a command to start a new scan
            try
            {
                orbcServer()->c4SendCmd
                (
                    strMoniker()
                    , kZWaveLevi2Sh::strSendCmd_StartScan
                    , TString::strEmpty()
                    , sectUser()
                );

                TOkBox msgbOk(facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_ScanStarted));
                msgbOk.ShowIt(*this);
            }

            catch(TError& errToCatch)
            {
                if (facZWaveLevi2C().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                TExceptDlg dlgbFail
                (
                    *this
                    , facZWaveLevi2C().strMsg(kZW2CMsgs::midGen_Title)
                    , facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_Rescan)
                    , errToCatch
                );
            }
        }
         else if (c4ListInd != kCIDLib::c4MaxCard)
        {
            // All these require a unit be selected
            TZWaveUnit& unitCur = m_dcfgCur.unitFindById(m_pwndUnits->c4IndexToId(c4ListInd));

            if (wnotEvent.widSource() == kZWaveLevi2C::ridMain_AddAssoc)
            {
                // Get the current unit and the VRCOP
                const tCIDLib::TCard4 c4VRCOPId = m_dcfgCur.c4FindVRCOP(kCIDLib::False);
                if (c4VRCOPId)
                {
                    TZWaveAddAssocDlg dlgAddAssoc;
                    dlgAddAssoc.RunDlg(*this, strMoniker(), unitCur, c4VRCOPId);
                }
                 else
                {
                    TErrBox msgbVRCOP(facZWaveLevi2Sh().strMsg(kZW2ShErrs::errcCfg_NoVRCOP));
                    msgbVRCOP.ShowIt(*this);
                }
            }
             else if (wnotEvent.widSource() == kZWaveLevi2C::ridMain_QAssoc)
            {
                TZWaveQAssocDlg dlgQAssoc;
                dlgQAssoc.RunDlg(*this, strMoniker(), unitCur, m_dcfgCur);
            }
             else if (wnotEvent.widSource() == kZWaveLevi2C::ridMain_AutoCfg)
            {
                DoAutoCfg(unitCur.c4Id());
            }
             else if (wnotEvent.widSource() == kZWaveLevi2C::ridMain_SetCfgParm)
            {
                // Invoke the configuration parameter setting dialog, if any units
                if (m_dcfgCur.c4UnitCnt())
                {
                    TZWaveSetCfgParamDlg dlgSetCP;
                    dlgSetCP.RunDlg(*this, strMoniker(), m_dcfgCur);
                }
            }
             else if (wnotEvent.widSource() == kZWaveLevi2C::ridMain_Delete)
            {
                // Make sure they want to do it
                TYesNoBox msgbDel
                (
                    facZWaveLevi2C().strMsg(kZW2CMsgs::midQ_DelUnit, unitCur.strName())
                );

                if (msgbDel.bShowIt(*this))
                {
                    // Remove it from the list and from the configuration
                    m_dcfgCur.DeleteUnit(unitCur.c4Id());
                    m_pwndUnits->RemoveCurrent();
                }
            }
        }
         else
        {
            TErrBox msgbErr(facZWaveLevi2C().strMsg(kZW2CErrs::errcGen_NoSelection));
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }
    }

    catch(const TError& errToCatch)
    {
        if (facZWaveLevi2C().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        TExceptDlg dlgbFail
        (
            *this
            , facZWaveLevi2C().strMsg(kZW2CMsgs::midGen_Title)
            , facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_RemOpFailed)
            , errToCatch
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  When the selection changes, we update the attribute editor with the new info.
//
tCIDCtrls::EEvResponses TZWaveLevi2CWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveLevi2C::ridMain_Units)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            // Clear the attribute editor
            m_pwndAttrEd->RemoveAll();

            // Disable stuff that depends on a unit being selected
            m_pwndAddAssoc->SetEnable(kCIDLib::False);
            m_pwndAutoCfg->SetEnable(kCIDLib::False);
            m_pwndQAssoc->SetEnable(kCIDLib::False);
            m_pwndSetCfgParm->SetEnable(kCIDLib::False);
            m_pwndDelete->SetEnable(kCIDLib::False);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            TZWaveUnit& unitCur = m_dcfgCur.unitFindById(wnotEvent.c4Id());

            //
            //  Query the options for the new unit and keep them around for later use
            //
            //  DO THIS BEFORE we call LoadUnitAttributes below!!
            //
            unitCur.bQueryOpts(m_colUnitOpts);

            // Load the newly selected unit into the attribute editor
            LoadUnitAttributes(unitCur);

            // Disable stuff if this one is missing
            m_pwndAddAssoc->SetEnable(!unitCur.bMissing());
            m_pwndAutoCfg->SetEnable(!unitCur.bMissing());
            m_pwndQAssoc->SetEnable(!unitCur.bMissing());
            m_pwndSetCfgParm->SetEnable(!unitCur.bMissing());
            m_pwndDelete->SetEnable(kCIDLib::True);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



//
//  Asks the server side driver for the list of groups and units, which we
//  load into the list boxes. We sort the lists by name, and load them in
//  that order, so the index in the list is the index back into the lists.
//
tCIDLib::TVoid
TZWaveLevi2CWnd::LoadConfig(        tCQCKit::TCQCSrvProxy&  orbcSrv
                            , const TZWaveDrvConfig&        dcfgNew
                            , const tCIDLib::TCard4         c4SerialNum)
{
    CIDAssert(&dcfgNew != &m_dcfgCur, L"Load config with same object");

    // Store the new data
    m_dcfgCur = dcfgNew;
    m_c4LastSerialNum = c4SerialNum;

    // Load up the device info for those that have had a device info file set
    m_dcfgCur.LoadDevInfo(m_mzwsfDummy);

    //
    //  Save the current configuration as the previous config now. This is used to
    //  watch for changes. DO THIS AFTER setting the device info above, so that it
    //  gets that info as well.
    //
    m_dcfgPrev = m_dcfgCur;

    // And load it up
    LoadUnits();
}


//
//  This is called when the user asks to select a device type. We make sure that the
//  index has been loaded, and try to load it if not.
//
tCIDLib::TVoid TZWaveLevi2CWnd::LoadIndex()
{
    // If it's already set up, then nothing to do
    if (!m_swdinList.m_colMakeModel.bIsEmpty())
        return;

    m_swdinList.ParseFromXML(*this);
}



//
//  Load up the unit list from the current config, optionally selecting an initial
//  unit id.
//
tCIDLib::TVoid TZWaveLevi2CWnd::LoadUnits()
{
    tCIDLib::TStrList colCols(m_c4ColInd_Name + 1);
    for (tCIDLib::TCard4 c4Index = 0; c4Index <= m_c4ColInd_Name; c4Index++)
        colCols.objAdd(TString::strEmpty());

    // Clear the list and reload it if we have any defined units
    TWndPaintJanitor janPaint(m_pwndUnits);

    m_pwndUnits->RemoveAll();
    TString strFmt;
    const tCIDLib::TCard4 c4UnitCnt = m_dcfgCur.c4UnitCnt();
    if (c4UnitCnt)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
        {
            const TZWaveUnit& unitCur = m_dcfgCur.unitAt(c4Index);

            colCols[m_c4ColInd_Enabled] = facCQCKit().strBoolYesNo(!unitCur.bDisabled());

            colCols[m_c4ColInd_Id] = L"0x";
            if (unitCur.c4Id() < 10)
                colCols[m_c4ColInd_Id].Append(L'0');
            colCols[m_c4ColInd_Id].AppendFormatted(unitCur.c4Id(), tCIDLib::ERadices::Hex);

            if (unitCur.bMissing())
                colCols[m_c4ColInd_Status] = L"Missing";
            else if (unitCur.bFailed())
                colCols[m_c4ColInd_Status] = L"Failed";
            else
                colCols[m_c4ColInd_Status] = L"Online";

            // The Z-Wave unit type
            colCols[m_c4ColInd_BasicType] = tZWaveLevi2Sh::strXlatEGenTypes(unitCur.eGenType());

            // The make model
            strFmt = unitCur.strMake();
            if (!strFmt.bIsEmpty())
            {
                strFmt.Append(kCIDLib::chForwardSlash);
                strFmt.Append(unitCur.strModel());
            }
            colCols[m_c4ColInd_MakeModel] = strFmt;

            // The unit name
            colCols[m_c4ColInd_Name] = unitCur.strName();

            // Add this set of colums now, the row id being the unit id
            m_pwndUnits->c4AddItem(colCols, unitCur.c4Id());
        }
        m_pwndUnits->SelectByIndex(0, kCIDLib::True);
    }
}


//
//  When a unit is selected, this is called to load up its info for editing.
//
tCIDLib::TVoid TZWaveLevi2CWnd::LoadUnitAttributes(const TZWaveUnit& unitShow)
{
    TAttrData               adatTmp;
    tCIDMData::TAttrList    colAttrs;
    TString                 strFmt;

    adatTmp.Set(L"Unit Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set(L"Enabled", ZWaveLevi2C_Window::strAttr_Enabled, tCIDMData::EAttrTypes::Bool);
    adatTmp.SetBool(!unitShow.bDisabled());
    colAttrs.objAdd(adatTmp);

    adatTmp.Set(L"Unit Name", ZWaveLevi2C_Window::strAttr_Name, tCIDMData::EAttrTypes::String);
    adatTmp.SetString(unitShow.strName());
    colAttrs.objAdd(adatTmp);

    // This one is visual only, to pop up the type selection dialog
    adatTmp.Set(L"Make/Model", ZWaveLevi2C_Window::strAttr_MakeModel, tCIDMData::EAttrTypes::String);
    adatTmp.eEditType(tCIDMData::EAttrEdTypes::Visual);
    if (!unitShow.strMake().bIsEmpty() && !unitShow.strModel().bIsEmpty())
    {
        strFmt = unitShow.strMake();
        strFmt.Append(L"/");
        strFmt.Append(unitShow.strModel());
        adatTmp.SetString(strFmt);
    }
    colAttrs.objAdd(adatTmp);

    // Load up any options that it has
    if (!m_colUnitOpts.bIsEmpty())
    {
        adatTmp.Set(L"Model Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
        colAttrs.objAdd(adatTmp);
        PrepUnitOpts(unitShow, m_colUnitOpts, colAttrs, adatTmp);
    }

    // Load them up to the attribute editor
    m_pwndAttrEd->LoadValues(colAttrs, unitShow.c4Id());
}



const TZWDIInfo*
TZWaveLevi2CWnd::pzwdiiFindDevInfo(const TString& strMake, const TString& strModel) const
{
    const tCIDLib::TCard4 c4Count = m_swdinList.m_colMakeModel.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWDIInfo& zwdiiCur = m_swdinList.m_colMakeModel[c4Index];
        if ((zwdiiCur.m_strMake == strMake) && (zwdiiCur.m_strModel == strModel))
            return &zwdiiCur;
    }
    return nullptr;
}


tCIDLib::TVoid
TZWaveLevi2CWnd::PrepUnitOpts(  const   TZWaveUnit&                 unitCur
                                , const tZWaveLevi2Sh::TOptList&    colUnitOpts
                                ,       tCIDMData::TAttrList&       colAttrs
                                ,       TAttrData&                  adatTmp)
{
    TString strLimits;
    TString strAttrId;
    const tCIDLib::TCard4 c4OptCount = colUnitOpts.c4ElemCount();
    for (tCIDLib::TCard4 c4OptIndex = 0; c4OptIndex < c4OptCount; c4OptIndex++)
    {
        const TZWOption& zwoptCur = colUnitOpts[c4OptIndex];

        //
        //  Check the configured value of this option. If it's marked as fixed
        //  then we don't display this one.
        //
        if (unitCur.bOptValIsFixed(zwoptCur.m_strKey))
            continue;

        // If there's no description, then we can't use itS
        if (zwoptCur.m_strInstDescr.bIsEmpty())
            continue;

        // Get the current value for this option
        const TString& strCurValKey = unitCur.strFindOptVal(zwoptCur.m_strKey);

        // Build up an enumerated limit string based on the available options
        const tCIDLib::TStrList& colAllowed = unitCur.colAllowedOptVals(zwoptCur.m_strKey);
        const tCIDLib::TCard4 c4AllowedCnt = colAllowed.c4ElemCount();

        TString strCurVal;
        strLimits = kCIDMData::strAttrLim_EnumPref;
        const tCIDLib::TCard4 c4ValCnt = zwoptCur.m_colValues.c4ElemCount();
        tCIDLib::TCard4 c4KeptCnt = 0;
        for (tCIDLib::TCard4 c4VInd = 0; c4VInd < c4ValCnt; c4VInd++)
        {
            const TKeyValuePair& kvalCur = zwoptCur.m_colValues[c4VInd];

            // See if this one is in the list of allowed values (or the list is empty)
            if (c4AllowedCnt)
            {
                tCIDLib::TCard4 c4AlInd = 0;
                while (c4AlInd < c4AllowedCnt)
                {
                    if (kvalCur.strKey() == colAllowed[c4AlInd])
                        break;
                    c4AlInd++;
                }

                // If not found in the list, skip this one
                if (c4AlInd == c4AllowedCnt)
                    continue;
            }

            // It's acceptable so take it
            if (c4KeptCnt)
                strLimits.Append(kCIDLib::chComma);
            strLimits.Append(kvalCur.strValue());

            // If this is the current value key, save the display value for it
            if (kvalCur.strKey() == strCurValKey)
                strCurVal = kvalCur.strValue();

            c4KeptCnt++;
        }

        // Build up the attribute id for this guy and set it
        if (c4KeptCnt)
        {
            strAttrId = ZWaveLevi2C_Window::strAttr_OptBase;
            strAttrId.Append(zwoptCur.m_strKey);
            adatTmp.Set(zwoptCur.m_strPrompt, strAttrId, strLimits, tCIDMData::EAttrTypes::String);
            adatTmp.SetString(strCurVal);
            colAttrs.objAdd(adatTmp);
        }
    }
}


//
//  Query the configuration from the server and store it away in the passed
//  parameters.
//
tCIDLib::TVoid
TZWaveLevi2CWnd::QueryConfig(tCQCKit::TCQCSrvProxy& orbcSrv
                            , TZWaveDrvConfig&      dcfgToFill
                            , tCIDLib::TCard4&      c4NewSerialNum)
{
    THeapBuf mbufCfg(8192UL);
    tCIDLib::TCard4 c4Sz;
    orbcSrv->QueryConfig(strMoniker(), c4Sz, mbufCfg, sectUser());

    // Make an input stream over the data and stream in the config
    TBinMBufInStream strmIn(&mbufCfg, c4Sz);
    strmIn.CheckForStartMarker(CID_FILE, CID_LINE);
    strmIn  >> dcfgToFill
            >> c4NewSerialNum;
    strmIn.CheckForEndMarker(CID_FILE, CID_LINE);

}

