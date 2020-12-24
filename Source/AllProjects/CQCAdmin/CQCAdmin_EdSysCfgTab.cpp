//
// FILE NAME: CQCAdmin_EdSysCfgTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/05/2016
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
//  This file implements a tab for editing the CQC room/devices configuration, which is
//  used for auto-generating content. Internally we implement a set of tab windows that
//  allow for editing of all of the bits and pieces.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "DevClsHdr_Security.hpp"
#include    "CQCAdmin_EdSysCfgTab.hpp"
#include    "CQCAdmin_EdSysCfgTab_.hpp"
#include    "CQCAdmin_AutoGenDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEdSysCfgTab, TContentTab)
RTTIDecls(TEdSCHVACDlg, TDlgBox)
RTTIDecls(TEdSCLoadDlg, TDlgBox)
RTTIDecls(TEdSCMediaDlg, TDlgBox)
RTTIDecls(TEdSCSecDlg, TDlgBox)
RTTIDecls(TEdSCWeatherDlg, TDlgBox)
RTTIDecls(TEdSCXOverDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdSCHVACDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdSCHVACDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdSCHVACDlg::TEdSCHVACDlg() :

    m_eAddOpt(TEdSysCfgTab::EAddOpts::AddToAll)
    , m_pcfcFields(nullptr)
    , m_pscfgCur(nullptr)
    , m_pscliWorking(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndThermo(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndSelThermo(nullptr)
    , m_pwndTitle(nullptr)
{
}

TEdSCHVACDlg::~TEdSCHVACDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdSCHVACDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdSCHVACDlg::bRunDlg(const TWindow&                wndOwner
                    ,       TSCHVACInfo&            scliEdit
                    , const TCQCFldCache&           cfcToUse
                    ,       TEdSysCfgTab::EAddOpts& eAddOpt
                    , const TCQCSysCfg&             scfgEdit)
{
    // Store the incoming values as the starting points
    m_eAddOpt = eAddOpt;
    m_pcfcFields = &cfcToUse;
    m_pscliWorking = &scliEdit;
    m_pscfgCur = &scfgEdit;
    m_strOrgTitle = scliEdit.m_strTitle;

    // Run the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_EdSCHVAC
    );

    if (c4Res == kCQCAdmin::ridDlg_EdSCHVAC_Save)
    {
        eAddOpt = m_eAddOpt;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdSCHVACDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdSCHVACDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCHVAC_AddToAll, m_pwndAddToAll);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCHVAC_AddIfThermos, m_pwndAddIfThermos);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCHVAC_AddToNone, m_pwndAddToNone);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCHVAC_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCHVAC_Thermo, m_pwndThermo);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCHVAC_Save, m_pwndSave);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCHVAC_SelThermo, m_pwndSelThermo);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCHVAC_Title, m_pwndTitle);

    // Load any incoming info. The thermo is a moniker/subunit combo
    m_pwndTitle->strWndText(m_pscliWorking->m_strTitle);

    if (!m_pscliWorking->m_strMoniker.bIsEmpty()
    &&  m_pcfcFields->bDevExists(m_pscliWorking->m_strMoniker)
    &&  !m_pscliWorking->m_strThermoSub.bIsEmpty())
    {
        TString strThermo(m_pscliWorking->m_strMoniker);
        strThermo.Append(L" (");
        strThermo.Append(m_pscliWorking->m_strThermoSub);
        strThermo.Append(L")");
        m_pwndThermo->strWndText(strThermo);
    }
     else
    {
        m_pscliWorking->m_strMoniker.Clear();
        m_pscliWorking->m_strThermoSub.Clear();
    }

    // Default the add options to add to all rooms
    m_pwndAddToAll->SetCheckedState(kCIDLib::True);

    // Register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdSCHVACDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdSCHVACDlg::eClickHandler);
    m_pwndSelThermo->pnothRegisterHandler(this, &TEdSCHVACDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdSCHVACDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TEdSCHVACDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCHVAC_Cancel)
    {
        EndDlg(kCQCAdmin::ridDlg_EdSCHVAC_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCHVAC_Save)
    {
        //
        //  Do basic checking. We make sure the title isn't a dup. It doesn't matter
        //  technically, since they are identified internally by a UID, but for the
        //  user it's important.
        //
        m_pscliWorking->m_strTitle = m_pwndTitle->strWndText();
        if (m_pscliWorking->m_strMoniker.bIsEmpty()
        ||  m_pscliWorking->m_strThermoSub.bIsEmpty()
        ||  m_pscliWorking->m_strTitle.bIsEmpty())
        {
            TErrBox msgbErr(L"You must provide a title and thermo");
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        if (m_pscliWorking->m_strTitle != m_strOrgTitle)
        {
            if (m_pscfgCur->bListTitleExists(tCQCSysCfg::ECfgLists::HVAC, m_pscliWorking->m_strTitle))
            {
                TErrBox msgbErr(L"This title is already used by another HVAC.");
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        // Store the selected add option
        if (m_pwndAddToAll->bCheckedState())
            m_eAddOpt = TEdSysCfgTab::EAddOpts::AddToAll;
        else if (m_pwndAddIfThermos->bCheckedState())
            m_eAddOpt = TEdSysCfgTab::EAddOpts::AddToExisting;
        else
            m_eAddOpt = TEdSysCfgTab::EAddOpts::AddToNone;

        // Looks reasonable so take it
        EndDlg(kCQCAdmin::ridDlg_EdSCHVAC_Save);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCHVAC_SelThermo)
    {
        // Thermos are moniker/sub-units, so use that style of selection
        TString strMon = m_pscliWorking->m_strMoniker;
        TString strSub = m_pscliWorking->m_strThermoSub;
        const tCIDLib::TBoolean bRes = facCQCIGKit().bSelectDevSub
        (
            *this
            , strMon
            , strSub
            , tCQCKit::EDevClasses::Thermostat
            , L"CurrentTemp"
            , *m_pcfcFields
        );

        if (bRes)
        {
            m_pscliWorking->m_strMoniker = strMon;
            m_pscliWorking->m_strThermoSub = strSub;

            //
            //  Now that it's stored, we can use the moniker string to build up
            //  the display value if needed.
            //
            if (!strSub.bIsEmpty())
            {
                strMon.Append(L" (");
                strMon.Append(strSub);
                strMon.Append(L")");
            }
            m_pwndThermo->strWndText(strMon);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TEdSCLoadDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdSCLoadDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdSCLoadDlg::TEdSCLoadDlg() :

    m_eAddOpt(TEdSysCfgTab::EAddOpts::AddToAll)
    , m_pcfcFields(nullptr)
    , m_pscfgCur(nullptr)
    , m_pscliWorking(nullptr)
    , m_pwndAddToAll(nullptr)
    , m_pwndAddIfLoads(nullptr)
    , m_pwndAddToNone(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndCField(nullptr)
    , m_pwndField(nullptr)
    , m_pwndField2(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndSelFld(nullptr)
    , m_pwndSpokenName(nullptr)
    , m_pwndTitle(nullptr)
{
}

TEdSCLoadDlg::~TEdSCLoadDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdSCLoadDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdSCLoadDlg::bRunDlg(  const   TWindow&                wndOwner
                        ,       TSCLoadInfo&            scliEdit
                        , const TCQCFldCache&           cfcToUse
                        ,       TEdSysCfgTab::EAddOpts& eAddOpt
                        , const TCQCSysCfg&             scfgEdit)
{
    // Store the incoming values as the starting points
    m_eAddOpt = eAddOpt;
    m_pcfcFields = &cfcToUse;
    m_pscliWorking = &scliEdit;
    m_pscfgCur = &scfgEdit;

    m_strOrgAltName = scliEdit.m_strAltName;
    m_strOrgTitle = scliEdit.m_strTitle;

    // Run the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_EdSCLoad
    );

    if (c4Res == kCQCAdmin::ridDlg_EdSCLoad_Save)
    {
        eAddOpt = m_eAddOpt;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdSCLoadDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdSCLoadDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_AddToAll, m_pwndAddToAll);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_AddIfLoads, m_pwndAddIfLoads);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_AddToNone, m_pwndAddToNone);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_CField, m_pwndCField);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_Field, m_pwndField);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_Field2, m_pwndField2);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_Save, m_pwndSave);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_SelField, m_pwndSelFld);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_SpokenName, m_pwndSpokenName);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCLoad_Title, m_pwndTitle);

    // Register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdSCLoadDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdSCLoadDlg::eClickHandler);
    m_pwndSelFld->pnothRegisterHandler(this, &TEdSCLoadDlg::eClickHandler);

    // Default the add options to add to all rooms
    m_pwndAddToAll->SetCheckedState(kCIDLib::True);

    // Load any incoming info
    m_pwndTitle->strWndText(m_pscliWorking->m_strTitle);
    m_pwndSpokenName->strWndText(m_pscliWorking->m_strAltName);

    // Format the full field names
    TString strFld;
    if (!m_pscliWorking->m_strClrField.bIsEmpty())
    {
        strFld = m_pscliWorking->m_strMoniker;
        strFld.Append(kCIDLib::chPeriod);
        strFld.Append(m_pscliWorking->m_strClrField);
        m_pwndCField->strWndText(strFld);
    }

    if (!m_pscliWorking->m_strField.bIsEmpty())
    {
        strFld = m_pscliWorking->m_strMoniker;
        strFld.Append(kCIDLib::chPeriod);
        strFld.Append(m_pscliWorking->m_strField);
        m_pwndField->strWndText(strFld);
    }

    if (!m_pscliWorking->m_strField2.bIsEmpty())
    {
        strFld = m_pscliWorking->m_strMoniker;
        strFld.Append(kCIDLib::chPeriod);
        strFld.Append(m_pscliWorking->m_strField2);
        m_pwndField2->strWndText(strFld);
    }
    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdSCLoadDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TEdSCLoadDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCLoad_Cancel)
    {
        EndDlg(kCQCAdmin::ridDlg_EdSCLoad_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCLoad_Save)
    {
        //
        //  Do basic checking. We make sure the title and spoken name isn't a dup.
        //  It doesn't matter technically, since they are identified internally by a
        //  UID, but for the user it's important, and we don't want to have any dups
        //  in any speech recognition grammars driven by this info.
        //
        m_pscliWorking->m_strAltName = m_pwndSpokenName->strWndText();
        m_pscliWorking->m_strTitle = m_pwndTitle->strWndText();

        if (m_pscliWorking->m_strField.bIsEmpty()
        ||  m_pscliWorking->m_strTitle.bIsEmpty())
        {
            TErrBox msgbCopy(L"You must provide a title and field");
            msgbCopy.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        if (m_pscliWorking->m_strTitle != m_strOrgTitle)
        {
            if (m_pscfgCur->bListTitleExists(tCQCSysCfg::ECfgLists::Lighting
                                            , m_pscliWorking->m_strTitle))
            {
                TErrBox msgbErr(L"This title is already used by another light.");
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        if (m_pscliWorking->m_strAltName != m_strOrgAltName)
        {
            if (m_pscfgCur->bListAltNameExists( tCQCSysCfg::ECfgLists::Lighting
                                                , m_pscliWorking->m_strAltName))
            {
                TErrBox msgbErr(L"This spoken name is already used by another light.");
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }


        // Store the selected add option
        if (m_pwndAddToAll->bCheckedState())
            m_eAddOpt = TEdSysCfgTab::EAddOpts::AddToAll;
        else if (m_pwndAddIfLoads->bCheckedState())
            m_eAddOpt = TEdSysCfgTab::EAddOpts::AddToExisting;
        else
            m_eAddOpt = TEdSysCfgTab::EAddOpts::AddToNone;

        // Looks reasonable so take it
        EndDlg(kCQCAdmin::ridDlg_EdSCLoad_Save);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCLoad_SelField)
    {
        //
        //  Set up the field filter correctly. We use one that allows us to filter
        //  for both semantic field types and device classes.
        //
        //  They select the primary load field, so we let them select switch and
        //  dimmer fields from any V2 compatible drivers that implement the lighting
        //  device class.
        //
        tCQCKit::TFldSTypeList fcolSTypes(2UL);
        tCQCKit::TDevClassList fcolCls(2UL);

        fcolSTypes.c4AddElement(tCQCKit::EFldSTypes::BoolSwitch);
        fcolSTypes.c4AddElement(tCQCKit::EFldSTypes::LightSwitch);
        fcolSTypes.c4AddElement(tCQCKit::EFldSTypes::Dimmer);
        fcolCls.c4AddElement(tCQCKit::EDevClasses::Lighting);

        TCQCFldFiltDevCls* pffiltSelect = new TCQCFldFiltDevCls
        (
            fcolCls
            , kCIDLib::False
            , fcolSTypes
            , tCQCKit::EFldTypes::Count
            , tCQCKit::EReqAccess::ReadAndWrite
        );

        // Build up any previous field to pass in as the default
        TString strMon = m_pscliWorking->m_strMoniker;
        TString strFld = m_pscliWorking->m_strField;

        const tCIDLib::TBoolean bRes = facCQCIGKit().bSelectField
        (
            *this, pffiltSelect, strMon, strFld
        );

        if (bRes)
        {
            // Format out the full selected field for loading into the list window item
            TString strFmtFld = strMon;
            strFmtFld.Append(kCIDLib::chPeriod);
            strFmtFld.Append(strFld);
            m_pwndField->strWndText(strFmtFld);

            // Store the new main field
            m_pscliWorking->m_strMoniker = strMon;
            m_pscliWorking->m_strField = strFld;

            // Clear the other two entry fields in case we don't find anything else
            m_pwndField2->ClearText();
            m_pwndCField->ClearText();

            //
            //  Now we use the provided cache, which is not limited to the stuff
            //  we filtered for above, to see if this driver has other fields related
            //  to the same load.
            //
            if (m_pscliWorking->m_strField.bStartsWith(L"LGHT#Dim_"))
            {
                // They selected a dimmer field. See if there's a switch
                TString strSwitch(m_pscliWorking->m_strField);
                strSwitch.Cut(0, 9);
                strSwitch.Prepend(L"LGHT#Sw_");
                if (m_pcfcFields->bFldExists(m_pscliWorking->m_strMoniker, strSwitch))
                {
                    m_pscliWorking->m_strField2 = strSwitch;
                    m_pwndField2->strWndText(strSwitch);
                }
            }

            // See if there's a color field for it
            {
                TString strClr(m_pscliWorking->m_strField);
                if (m_pscliWorking->m_strField.bStartsWith(L"LGHT#Dim_"))
                    strClr.Cut(0, 9);
                else
                    strClr.Cut(0, 8);

                strClr.Prepend(L"CLGHT#");
                if (m_pcfcFields->bFldExists(m_pscliWorking->m_strMoniker, strClr))
                {
                    m_pscliWorking->m_strClrField = strClr;
                    m_pwndCField->strWndText(strClr);
                }
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TEdSCMediaDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdSCMediaDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdSCMediaDlg::TEdSCMediaDlg() :

    m_pcfcFields(nullptr)
    , m_pscfgCur(nullptr)
    , m_pscliWorking(nullptr)
    , m_pwndAudioMon(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndSelAudio(nullptr)
    , m_pwndSelRepo(nullptr)
    , m_pwndSelRend(nullptr)
    , m_pwndRendMon(nullptr)
    , m_pwndRepoMon(nullptr)
    , m_pwndTitle(nullptr)
{
}

TEdSCMediaDlg::~TEdSCMediaDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdSCMediaDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdSCMediaDlg::bRunDlg( const   TWindow&        wndOwner
                        ,       TSCMediaInfo&   scliEdit
                        , const TCQCFldCache&   cfcToUse
                        , const TCQCSysCfg&     scfgEdit)
{
    // Store the incoming values as the starting points
    m_pcfcFields = &cfcToUse;
    m_pscliWorking = &scliEdit;
    m_pscfgCur = &scfgEdit;
    m_strOrgTitle = scliEdit.m_strTitle;

    // Run the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_EdSCMedia
    );

    return (c4Res == kCQCAdmin::ridDlg_EdSCMedia_Save);
}


// ---------------------------------------------------------------------------
//  TEdSCMediaDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdSCMediaDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCMedia_AudioMon, m_pwndAudioMon);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCMedia_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCMedia_Save, m_pwndSave);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCMedia_RendMon, m_pwndRendMon);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCMedia_RepoMon, m_pwndRepoMon);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCMedia_SelAudio, m_pwndSelAudio);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCMedia_SelRend, m_pwndSelRend);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCMedia_SelRepo, m_pwndSelRepo);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCMedia_Title, m_pwndTitle);

    // Register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdSCMediaDlg::eClickHandler);
    m_pwndSelAudio->pnothRegisterHandler(this, &TEdSCMediaDlg::eClickHandler);
    m_pwndSelRend->pnothRegisterHandler(this, &TEdSCMediaDlg::eClickHandler);
    m_pwndSelRepo->pnothRegisterHandler(this, &TEdSCMediaDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdSCMediaDlg::eClickHandler);

    // Load any incoming info
    TString strCurVal = m_pscliWorking->m_strAudioMon;
    if (!m_pscliWorking->m_strAudioSub.bIsEmpty())
    {
        strCurVal.Append(L" (");
        strCurVal.Append(m_pscliWorking->m_strAudioSub);
        strCurVal.Append(L")");
    }
    m_pwndAudioMon->strWndText(strCurVal);

    m_pwndRendMon->strWndText(m_pscliWorking->m_strRendMon);
    m_pwndRepoMon->strWndText(m_pscliWorking->m_strRepoMon);
    m_pwndTitle->strWndText(m_pscliWorking->m_strTitle);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdSCMediaDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TEdSCMediaDlg::bSelectMon(  const   tCQCKit::EDevClasses    eClass
                            ,       TString&                strToFill)
{
    //
    //  Use the field cache we have to get a list of all drivers that implement
    //  the passed device class.
    //
    tCIDLib::TBoolean bRes = kCIDLib::False;
    tCIDLib::TStrList colDevs;
    if (m_pcfcFields->bQueryDevList(colDevs, eClass))
    {
        // Build up the title string
        TString strTitle(L"Select a %(1)", tCQCKit::strXlatEDevClasses(eClass));

        // Use a generic list selection dialog to get a selection
        bRes = facCIDWUtils().bListSelect(*this, strTitle, colDevs, strToFill);
    }
     else
    {
        TErrBox msgbErCopy(L"No drivers of the required type were found");
        msgbErCopy.ShowIt(*this);
    }
    return bRes;
}


tCIDCtrls::EEvResponses TEdSCMediaDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCMedia_SelAudio)
    {
        TString strMon = m_pscliWorking->m_strAudioMon;
        TString strSub = m_pscliWorking->m_strAudioSub;
        const tCIDLib::TBoolean bRes = facCQCIGKit().bSelectDevSub
        (
            *this
            , strMon
            , strSub
            , tCQCKit::EDevClasses::Audio
            , L"Volume"
            , *m_pcfcFields
        );

        if (bRes)
        {
            m_pscliWorking->m_strAudioMon = strMon;
            m_pscliWorking->m_strAudioSub = strSub;

            //
            //  Now that it's stored, we can use the moniker string to build up
            //  the display value if needed.
            //
            if (!strSub.bIsEmpty())
            {
                strMon.Append(L" (");
                strMon.Append(strSub);
                strMon.Append(L")");
            }
            m_pwndAudioMon->strWndText(strMon);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCMedia_SelRepo)
    {
        // Get a list of drivers that implement the repo device class
        TString strSel;
        if (bSelectMon(tCQCKit::EDevClasses::MediaRepository, strSel))
        {
            m_pwndRepoMon->strWndText(strSel);
            m_pscliWorking->m_strRepoMon = strSel;
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCMedia_SelRend)
    {
        // Get a list of drivers that implement the rend device class
        TString strSel;
        if (bSelectMon(tCQCKit::EDevClasses::MediaRenderer, strSel))
        {
            m_pwndRendMon->strWndText(strSel);
            m_pscliWorking->m_strRendMon = strSel;

            //
            //  If the audio moniker isn't set, and this device implements the
            //  audio class, set it as the likely default.
            //
            if (m_pscliWorking->m_strAudioMon.bIsEmpty()
            &&  m_pcfcFields->fcolDevClasses(strSel).bElementPresent(tCQCKit::EDevClasses::Audio))
            {
                m_pscliWorking->m_strAudioMon = strSel;
                m_pscliWorking->m_strAudioSub.Clear();
                m_pwndAudioMon->strWndText(strSel);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCMedia_Cancel)
    {
        EndDlg(kCQCAdmin::ridDlg_EdSCMedia_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCMedia_Save)
    {
        const TString& strAudio = m_pwndAudioMon->strWndText();
        const TString& strRend = m_pwndRendMon->strWndText();
        const TString& strRepo = m_pwndRepoMon->strWndText();
        const TString& strTitle = m_pwndTitle->strWndText();

        if (strTitle.bIsEmpty()
        ||  strAudio.bIsEmpty()
        ||  strRend.bIsEmpty()
        ||  strRepo.bIsEmpty())
        {
            TErrBox msgbErr(L"You must provide all values before you can save.");
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        //
        //  Do basic checking. We make sure the title isn't a dup. It doesn't matter
        //  technically, since they are identified internally by a UID, but for the
        //  user it's important.
        //
        if (strTitle != m_strOrgTitle)
        {
            if (m_pscfgCur->bListTitleExists(tCQCSysCfg::ECfgLists::Media, strTitle))
            {
                TErrBox msgbErr(L"This title is already used by another media configuration.");
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }
        m_pscliWorking->m_strTitle = strTitle;

        EndDlg(kCQCAdmin::ridDlg_EdSCMedia_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TEdSCSecDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdSCSecDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdSCSecDlg::TEdSCSecDlg() :

    m_pcfcFields(nullptr)
    , m_pscfgCur(nullptr)
    , m_pscliWorking(nullptr)
    , m_pwndAreaList(nullptr)
    , m_pwndArmingCode(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndMoniker(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndSelMon(nullptr)
    , m_pwndTitle(nullptr)
{
}

TEdSCSecDlg::~TEdSCSecDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdSCSecDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdSCSecDlg::bRunDlg(const  TWindow&        wndOwner
                    ,       TSCSecInfo&     scliEdit
                    , const TCQCFldCache&   cfcToUse
                    , const TCQCSysCfg&     scfgEdit)
{
    // Store the incoming values as the starting points
    m_pcfcFields = &cfcToUse;
    m_pscliWorking = &scliEdit;
    m_pscfgCur = &scfgEdit;
    m_strOrgTitle = scliEdit.m_strTitle;

    // Run the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_EdSCSec
    );

    return (c4Res == kCQCAdmin::ridDlg_EdSCSec_Save);
}


// ---------------------------------------------------------------------------
//  TEdSCSecDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdSCSecDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCSec_AreaList, m_pwndAreaList);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCSec_ArmingCode, m_pwndArmingCode);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCSec_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCSec_Moniker, m_pwndMoniker);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCSec_Save, m_pwndSave);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCSec_SelMon, m_pwndSelMon);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCSec_Title, m_pwndTitle);

    // Load any incoming info
    m_pwndTitle->strWndText(m_pscliWorking->m_strTitle);

    // Set our list columns
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"Security Areas");
    m_pwndAreaList->SetColumns(colCols);

    //
    //  If we have an existing moniker, we have to ask it for its list of security
    //  areas in order to fill in the list box.
    //
    if (!m_pscliWorking->m_strMoniker.bIsEmpty())
    {
        m_pwndMoniker->strWndText(m_pscliWorking->m_strMoniker);
        bLoadAreaList(m_pscliWorking->m_strMoniker, m_pscliWorking->m_strAreaName);
    }

    // Load any other bits
    m_pwndArmingCode->strWndText(m_pscliWorking->m_strArmingCode);

    // Register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdSCSecDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdSCSecDlg::eClickHandler);
    m_pwndSelMon->pnothRegisterHandler(this, &TEdSCSecDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdSCSecDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TEdSCSecDlg::bLoadAreaList(const TString& strMoniker, const TString& strAreaName)
{
    // Clear out any current contents
    m_pwndAreaList->RemoveAll();

    // We need to ask the target driver for a list of area names
    try
    {
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        TString strVal;
        const tCIDLib::TBoolean bRes = orbcSrv->bQueryTextVal
        (
            strMoniker
            , kDevCls_Security::pszSecQN_SecPanelInfo
            , kDevCls_Security::pszSecQT_AreaList
            , strVal
        );

        if (bRes)
        {
            // Parse out the list values
            tCIDLib::TCard4 c4ErrInd;
            tCIDLib::TStrList colAreas;
            if (TStringTokenizer::bParseQuotedCommaList(strVal, colAreas, c4ErrInd))
            {
                tCIDLib::TStrList colCols(1);
                colCols.objAdd(TString::strEmpty());
                const tCIDLib::TCard4 c4Count = colAreas.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    colCols[0] = colAreas[c4Index];
                    m_pwndAreaList->c4AddItem(colCols, 0);
                }
            }

            if (!strAreaName.bIsEmpty())
                m_pwndAreaList->c4SelectByText(strAreaName);
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TErrBox msgbErr(L"Could not query the security area list");
        msgbErr.ShowIt(*this);
    }
    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TEdSCSecDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCSec_Cancel)
    {
        EndDlg(kCQCAdmin::ridDlg_EdSCSec_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCSec_Save)
    {
        //
        //  Do basic checking. We make sure the title isn't a dup. It doesn't matter
        //  technically, since they are identified internally by a UID, but for the
        //  user it's important.
        //
        m_pscliWorking->m_strArmingCode = m_pwndArmingCode->strWndText();
        m_pscliWorking->m_strMoniker = m_pwndMoniker->strWndText();
        m_pscliWorking->m_strTitle = m_pwndTitle->strWndText();

        if (m_pscliWorking->m_strMoniker.bIsEmpty()
        ||  m_pscliWorking->m_strTitle.bIsEmpty())
        {
            TErrBox msgbErr(L"You must provide a title and moniker");
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        if (m_pscliWorking->m_strTitle != m_strOrgTitle)
        {
            if (m_pscfgCur->bListTitleExists(tCQCSysCfg::ECfgLists::Security, m_pscliWorking->m_strTitle))
            {
                TErrBox msgbErr(L"This title is already used by another security configuration.");
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        // Make sure an area is selected in the list box
        const tCIDLib::TCard4 c4ListInd = m_pwndAreaList->c4CurItem();
        if (c4ListInd == kCIDLib::c4MaxCard)
        {
            TErrBox msgbCopy(L"A security area must be selected");
            msgbCopy.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }
         else
        {
            m_pwndAreaList->QueryColText(c4ListInd, 0, m_pscliWorking->m_strAreaName);
        }

        // Looks reasonable so take it
        EndDlg(kCQCAdmin::ridDlg_EdSCSec_Save);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCSec_SelMon)
    {
        //
        //  Use the field cache we have to get a list of all drivers that implement
        //  the security device class.
        //
        tCIDLib::TStrList colDevs;
        if (m_pcfcFields->bQueryDevList(colDevs, tCQCKit::EDevClasses::Security))
        {
            // Use a generic list selection dialog to get a selection
            TString strSelected;
            const tCIDLib::TBoolean bRes = facCIDWUtils().bListSelect
            (
                *this, L"Select a security device", colDevs, strSelected
            );

            if (bRes)
            {
                m_pwndMoniker->strWndText(strSelected);
                bLoadAreaList(strSelected, TString::strEmpty());
            }
        }
         else
        {
            TErrBox msgbErCopy(L"No drivers of the required type were found");
            msgbErCopy.ShowIt(*this);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TEdSCWeatherDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdSCWeatherDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdSCWeatherDlg::TEdSCWeatherDlg() :

    m_pscliWorking(nullptr)
    , m_pscfgCur(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndMoniker(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndScale(nullptr)
    , m_pwndSelMon(nullptr)
    , m_pwndTitle(nullptr)
{
}

TEdSCWeatherDlg::~TEdSCWeatherDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdSCWeatherDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdSCWeatherDlg::bRunDlg(const  TWindow&        wndOwner
                        ,       TSCWeatherInfo& scliEdit
                        , const TCQCFldCache&   cfcToUse
                        , const TCQCSysCfg&     scfgEdit)
{
    // Store the incoming values as the starting points
    m_pcfcFields = &cfcToUse;
    m_pscliWorking = &scliEdit;
    m_pscfgCur = &scfgEdit;
    m_strOrgTitle = scliEdit.m_strTitle;

    // Run the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_EdSCWeath
    );

    return (c4Res == kCQCAdmin::ridDlg_EdSCWeath_Save);
}


// ---------------------------------------------------------------------------
//  TEdSCWeatherDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdSCWeatherDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCWeath_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCWeath_Moniker, m_pwndMoniker);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCWeath_Save, m_pwndSave);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCWeath_Scale, m_pwndScale);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCWeath_SelMon, m_pwndSelMon);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCWeath_Title, m_pwndTitle);

    // Register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdSCWeatherDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdSCWeatherDlg::eClickHandler);
    m_pwndSelMon->pnothRegisterHandler(this, &TEdSCWeatherDlg::eClickHandler);

    // Load the two temp scale options
    m_pwndScale->c4AddItem(L"F");
    m_pwndScale->c4AddItem(L"C");

    // Load any incoming info
    m_pwndMoniker->strWndText(m_pscliWorking->m_strMoniker);
    m_pwndScale->SelectByIndex(m_pscliWorking->m_bFScale ? 0 : 1);

    m_pwndTitle->strWndText(m_pscliWorking->m_strTitle);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdSCWeatherDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TEdSCWeatherDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCWeath_SelMon)
    {
        //
        //  Use the field cache we have to get a list of all drivers that implement
        //  the weather device class.
        //
        tCIDLib::TStrList colDevs;
        if (m_pcfcFields->bQueryDevList(colDevs, tCQCKit::EDevClasses::Weather))
        {
            // Use a generic list selection dialog to get a selection
            TString strSelected;
            const tCIDLib::TBoolean bRes = facCIDWUtils().bListSelect
            (
                *this, L"Select a weather device", colDevs, strSelected
            );

            if (bRes)
            {
                m_pwndMoniker->strWndText(strSelected);
                m_pscliWorking->m_strMoniker = strSelected;
            }
        }
         else
        {
            TErrBox msgbErCopy(L"No drivers of the required type were found");
            msgbErCopy.ShowIt(*this);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCWeath_Cancel)
    {
        EndDlg(kCQCAdmin::ridDlg_EdSCWeath_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCWeath_Save)
    {
        m_pscliWorking->m_strTitle = m_pwndTitle->strWndText();
        if (m_pscliWorking->m_strMoniker.bIsEmpty()
        ||  m_pscliWorking->m_strTitle.bIsEmpty())
        {
            TErrBox msgbErr
            (
                L"You must provide at least the moniker and title before you can save."
            );
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        //
        //  Do basic checking. We make sure the title isn't a dup. It doesn't matter
        //  technically, since they are identified internally by a UID, but for the
        //  user it's important.
        //
        if (m_pscliWorking->m_strTitle != m_strOrgTitle)
        {
            if (m_pscfgCur->bListTitleExists(tCQCSysCfg::ECfgLists::Weather, m_pscliWorking->m_strTitle))
            {
                TErrBox msgbErr(L"This title is already used by another weather configuration.");
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        // Store the scale, which can't be incorrect
        m_pscliWorking->m_bFScale = (m_pwndScale->c4CurItem() == 0);
        EndDlg(kCQCAdmin::ridDlg_EdSCWeath_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TEdSCXOverDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdSCXOverDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdSCXOverDlg::TEdSCXOverDlg() :

    m_pscliWorking(nullptr)
    , m_pscfgCur(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndContTmpl(nullptr)
    , m_pwndDelCont(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndSelCont(nullptr)
    , m_pwndSelTile(nullptr)
    , m_pwndTileTmpl(nullptr)
    , m_pwndTitle(nullptr)
{
}

TEdSCXOverDlg::~TEdSCXOverDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdSCXOverDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdSCXOverDlg::bRunDlg( const   TWindow&        wndOwner
                        ,       TSCXOverInfo&   scliEdit
                        , const TCQCSysCfg&     scfgEdit)
{
    // Store the incoming values as the starting points
    m_pscliWorking = &scliEdit;
    m_pscfgCur = &scfgEdit;
    m_strOrgTitle = scliEdit.m_strTitle;

    // Run the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_EdSCXOver
    );
    return (c4Res == kCQCAdmin::ridDlg_EdSCXOver_Save);
}


// ---------------------------------------------------------------------------
//  TEdSCXOverDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdSCXOverDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCXOver_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCXOver_ContTmpl, m_pwndContTmpl);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCXOver_DelCont, m_pwndDelCont);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCXOver_Save, m_pwndSave);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCXOver_SelCont, m_pwndSelCont);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCXOver_SelTile, m_pwndSelTile);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCXOver_TileTmpl, m_pwndTileTmpl);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdSCXOver_Title, m_pwndTitle);

    // Register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdSCXOverDlg::eClickHandler);
    m_pwndDelCont->pnothRegisterHandler(this, &TEdSCXOverDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdSCXOverDlg::eClickHandler);
    m_pwndSelCont->pnothRegisterHandler(this, &TEdSCXOverDlg::eClickHandler);
    m_pwndSelTile->pnothRegisterHandler(this, &TEdSCXOverDlg::eClickHandler);

    // Load any incoming info
    m_pwndContTmpl->strWndText(m_pscliWorking->m_strContentTmpl);
    m_pwndTileTmpl->strWndText(m_pscliWorking->m_strTileTmpl);
    m_pwndTitle->strWndText(m_pscliWorking->m_strTitle);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdSCXOverDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// We have to select a template from two places, so we break out the code
tCIDLib::TBoolean
TEdSCXOverDlg::bSelectTmpl(TString& strToFill, const tCIDLib::TBoolean bTile)
{
    TString strTitle
    (
        bTile ? kCQCAMsgs::midPrompt_TileTmpl : kCQCAMsgs::midPrompt_ContTmpl
        , facCQCAdmin
    );

    return facCQCTreeBrws().bSelectFile
    (
        *this
        , strTitle
        , tCQCRemBrws::EDTypes::Template
        , strToFill
        , facCQCAdmin.cuctxToUse()
        , tCQCTreeBrws::EFSelFlags::SelectItems
        , strToFill
    );
}


tCIDCtrls::EEvResponses
TEdSCXOverDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCXOver_DelCont)
    {
        m_pscliWorking->m_strContentTmpl.Clear();
        m_pwndContTmpl->ClearText();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCXOver_SelCont)
    {
        TString strTmpl = m_pscliWorking->m_strContentTmpl;
        if (bSelectTmpl(strTmpl, kCIDLib::False))
        {
            m_pwndContTmpl->strWndText(strTmpl);
            m_pscliWorking->m_strContentTmpl = strTmpl;
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCXOver_SelTile)
    {
        TString strTmpl = m_pscliWorking->m_strTileTmpl;
        if (bSelectTmpl(strTmpl, kCIDLib::True))
        {
            m_pwndTileTmpl->strWndText(strTmpl);
            m_pscliWorking->m_strTileTmpl = strTmpl;
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCXOver_Cancel)
    {
        EndDlg(kCQCAdmin::ridDlg_EdSCXOver_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdSCXOver_Save)
    {
        const TString& strTile = m_pwndTileTmpl->strWndText();
        const TString& strTitle = m_pwndTitle->strWndText();

        if (strTile.bIsEmpty() || strTitle.bIsEmpty())
        {
            TErrBox msgbErr
            (
                L"You must provide at least the title and tile template before you can save."
            );
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        if (strTitle != m_strOrgTitle)
        {
            if (m_pscfgCur->bListTitleExists(tCQCSysCfg::ECfgLists::XOvers, strTitle))
            {
                TErrBox msgbErr(L"This title is already used by another overlay.");
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        m_pscliWorking->m_strTitle = m_pwndTitle->strWndText();
        EndDlg(kCQCAdmin::ridDlg_EdSCXOver_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TESCGlobalTab
//  PREFIXL wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TESCGlobalTab: constructors and Destructor
// ---------------------------------------------------------------------------
TESCGlobalTab::TESCGlobalTab(       TCQCSysCfg* const       pscfgEdit
                            ,       TCQCFldCache* const     pcfcDevs) :

    TTabWindow(L"/SystemCfg/Global", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_pscfgEdit(pscfgEdit)
    , m_pcfcDevs(pcfcDevs)
    , m_pwndAddHVAC(nullptr)
    , m_pwndAddLoad(nullptr)
    , m_pwndAddMedia(nullptr)
    , m_pwndAddSec(nullptr)
    , m_pwndAddTarRes(nullptr)
    , m_pwndAddWeath(nullptr)
    , m_pwndAddXOver(nullptr)
    , m_pwndCont(nullptr)
    , m_pwndCustLayout(nullptr)
    , m_pwndDelCustLayout(nullptr)
    , m_pwndDelHVAC(nullptr)
    , m_pwndDelLoad(nullptr)
    , m_pwndDelMedia(nullptr)
    , m_pwndDelSec(nullptr)
    , m_pwndDelTarRes(nullptr)
    , m_pwndDelWeath(nullptr)
    , m_pwndDelXOver(nullptr)
    , m_pwndHVACList(nullptr)
    , m_pwndLoadList(nullptr)
    , m_pwndMediaList(nullptr)
    , m_pwndScroll(nullptr)
    , m_pwndSecList(nullptr)
    , m_pwndSelCustLayout(nullptr)
    , m_pwndSelTarScope(nullptr)
    , m_pwndTarResList(nullptr)
    , m_pwndTarScope(nullptr)
    , m_pwndUseCustLights(nullptr)
    , m_pwndWeatherList(nullptr)
    , m_pwndXOverList(nullptr)
{
}

TESCGlobalTab::~TESCGlobalTab()
{
}


// ---------------------------------------------------------------------------
//  TESCGlobalTab: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the user selects an error that belongs to us, this is called to let us select
//  the offending control, so that the user can immediately make changes.
//
tCIDLib::TVoid TESCGlobalTab::SelectErrTarget(const TValErrInfo& verrToSel)
{
    // If there's a window id, put the focus on it
    if (verrToSel.m_widFocus)
        m_pwndCont->pwndChildById(verrToSel.m_widFocus)->TakeFocus();

    // Try to make this control visible
}


// ---------------------------------------------------------------------------
//  TESCGlobalTab: Public, non-virtual methods methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TESCGlobalTab::Reload()
{
    LoadData();
}


// The main tab calls us here when it needs to validate the currently entered info
tCIDLib::TVoid TESCGlobalTab::Validate(TValErrInfo::TErrList& colList)
{
    // Make sure the target scope is there and is under User
    const TString& strTarScope = m_pwndTarScope->strWndText();
    if (!strTarScope.bStartsWithI(L"/User"))
    {
        colList.objAdd
        (
            TValErrInfo
            (
                m_pwndTarScope->widThis()
                , facCQCAdmin.strMsg(kCQCAErrs::errcScale_BadTarScope)
            )
        );
    }
    m_pscfgEdit->strTarScope(strTarScope);

    // The resolution list can't be empty
    if (!m_pwndTarResList->c4ItemCount())
    {
        colList.objAdd
        (
            TValErrInfo
            (
                m_pwndAddTarRes->widThis()
                , facCQCAdmin.strMsg(kCQCAErrs::errcSysCfg_NoTarRes)
            )
        );
    }

    //
    //  Make sure the custom layout template, if set, is not under the output
    //  directory. If use custom lights is checked, it will be forced off if we set
    //  an empty custom layout, so we don't have to validate that.
    //
    if (!m_pwndCustLayout->bIsEmpty()
    &&  m_pwndCustLayout->strWndText().bStartsWithI(m_pscfgEdit->strTarScope()))
    {
        TString strErr
        (
            kCQCAErrs::errcSysCfg_NotInTarget
            , facCQCAdmin
            , TString(L"custom layout template")
        );
        colList.objAdd(TValErrInfo(m_pwndSelCustLayout->widThis(), strErr));
    }
    m_pscfgEdit->strCustLayout(m_pwndCustLayout->strWndText());

    // We need to refresh our field cache in case something has changed
    m_pcfcDevs->Reload();

    //
    //  Make sure any configured devices are found. So we go through each list that
    //  references drivers, and check them.
    //
    tCIDLib::TStrHashSet colMissing(109, TStringKeyOps());
    TString strErr;
    tCIDLib::TCard4 c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::HVAC);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCHVACInfo& scliCur = m_pscfgEdit->scliHVACAt(c4Index);
        if (!m_pcfcDevs->bDevExists(scliCur.m_strMoniker)
        &&  !colMissing.bHasElement(scliCur.m_strMoniker))
        {
            strErr.LoadFromMsg
            (
                kCQCAErrs::errcSysCfg_DrvNotFound
                , facCQCAdmin
                , TString(L"HVAC")
                , scliCur.m_strMoniker
            );

            colList.objAdd(TValErrInfo(m_pwndHVACList->widThis(), strErr));
            colMissing.objAdd(scliCur.m_strMoniker);
        }
    }


    colMissing.RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Lighting);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCLoadInfo& scliCur = m_pscfgEdit->scliLoadAt(c4Index);
        if (!m_pcfcDevs->bDevExists(scliCur.m_strMoniker)
        &&  !colMissing.bHasElement(scliCur.m_strMoniker))
        {
            strErr.LoadFromMsg
            (
                kCQCAErrs::errcSysCfg_DrvNotFound
                , facCQCAdmin
                , TString(L"Load")
                , scliCur.m_strMoniker
            );

            colList.objAdd(TValErrInfo(m_pwndLoadList->widThis(), strErr));
            colMissing.objAdd(scliCur.m_strMoniker);
        }
    }

    colMissing.RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Media);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCMediaInfo& scliCur = m_pscfgEdit->scliMediaAt(c4Index);

        if (!scliCur.m_strAudioMon.bIsEmpty()
        &&  !m_pcfcDevs->bDevExists(scliCur.m_strAudioMon)
        &&  !colMissing.bHasElement(scliCur.m_strAudioMon))
        {
            strErr.LoadFromMsg
            (
                kCQCAErrs::errcSysCfg_DrvNotFound
                , facCQCAdmin
                , TString(L"Audio")
                , scliCur.m_strAudioMon
            );

            colList.objAdd(TValErrInfo(m_pwndMediaList->widThis(), strErr));
            colMissing.objAdd(scliCur.m_strAudioMon);
        }

        if (!scliCur.m_strRepoMon.bIsEmpty()
        &&  !m_pcfcDevs->bDevExists(scliCur.m_strRepoMon)
        &&  !colMissing.bHasElement(scliCur.m_strRepoMon))
        {
            strErr.LoadFromMsg
            (
                kCQCAErrs::errcSysCfg_DrvNotFound
                , facCQCAdmin
                , TString(L"Repo")
                , scliCur.m_strRepoMon
            );

            colList.objAdd(TValErrInfo(m_pwndMediaList->widThis(), strErr));
            colMissing.objAdd(scliCur.m_strRepoMon);
        }

        if (!scliCur.m_strRendMon.bIsEmpty()
        &&  !m_pcfcDevs->bDevExists(scliCur.m_strRendMon)
        &&  !colMissing.bHasElement(scliCur.m_strRendMon))
        {
            strErr.LoadFromMsg
            (
                kCQCAErrs::errcSysCfg_DrvNotFound
                , facCQCAdmin
                , TString(L"Renderer")
                , scliCur.m_strRendMon
            );

            colList.objAdd(TValErrInfo(m_pwndMediaList->widThis(), strErr));
            colMissing.objAdd(scliCur.m_strRendMon);
        }
    }


    colMissing.RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Security);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCSecInfo& scliCur = m_pscfgEdit->scliSecAt(c4Index);
        if (!m_pcfcDevs->bDevExists(scliCur.m_strMoniker)
        &&  !colMissing.bHasElement(scliCur.m_strMoniker))
        {
            strErr.LoadFromMsg
            (
                kCQCAErrs::errcSysCfg_DrvNotFound
                , facCQCAdmin
                , TString(L"Security")
                , scliCur.m_strMoniker
            );

            colList.objAdd(TValErrInfo(m_pwndSecList->widThis(), strErr));
            colMissing.objAdd(scliCur.m_strMoniker);
        }
    }


    colMissing.RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Weather);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCWeatherInfo& scliCur = m_pscfgEdit->scliWeatherAt(c4Index);
        if (!m_pcfcDevs->bDevExists(scliCur.m_strMoniker)
        &&  !colMissing.bHasElement(scliCur.m_strMoniker))
        {
            strErr.LoadFromMsg
            (
                kCQCAErrs::errcSysCfg_DrvNotFound
                , facCQCAdmin
                , TString(L"Weather")
                , scliCur.m_strMoniker
            );

            colList.objAdd(TValErrInfo(m_pwndWeatherList->widThis(), strErr));
            colMissing.objAdd(scliCur.m_strMoniker);
        }
    }
}


// ---------------------------------------------------------------------------
//  TESCGlobalTab: Protected, inherited methods
// ---------------------------------------------------------------------------

// We just need to keep our scroll area child sized to fit
tCIDLib::TVoid
TESCGlobalTab::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TTabWindow::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && m_pwndScroll && (ePosState != tCIDCtrls::EPosStates::Minimized))
        m_pwndScroll->SetSize(areaNew.szArea(), kCIDLib::True);
}

tCIDLib::TBoolean TESCGlobalTab::bCreated()
{
    TTabWindow::bCreated();

    // Create our scroll area
    m_pwndScroll = new TScrollArea();
    m_pwndScroll->Create
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , areaClient()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EScrAStyles::None
        , tCIDCtrls::EExWndStyles::ControlParent
    );

    // Now load the dialog resource
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_SysCfgGlob, dlgdChildren);

    // Create a generic window of the size to fit this content
    m_pwndCont = new TGenericWnd();
    m_pwndCont->CreateGenWnd
    (
        *m_pwndScroll
        , dlgdChildren.areaPos()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , kCIDCtrls::widFirstCtrl
    );

    // Tell the scroll area this is what he is to manage
    m_pwndScroll->SetChild(m_pwndCont);

    // Load the dialog description of our controls and create them under the content window
    tCIDLib::TCard4 c4InitFocus;
    m_pwndCont->PopulateFromDlg(dlgdChildren, c4InitFocus);

    // Look up our controls that we care about
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_AddHVAC, m_pwndAddHVAC);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_AddLoad, m_pwndAddLoad);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_AddMedia, m_pwndAddMedia);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_AddSec, m_pwndAddSec);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_AddTarRes, m_pwndAddTarRes);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_AddWeath, m_pwndAddWeath);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_AddXO, m_pwndAddXOver);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_CustLayout, m_pwndCustLayout);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_DelCustLayout, m_pwndDelCustLayout);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_DelHVAC, m_pwndDelHVAC);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_DelLoad, m_pwndDelLoad);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_DelMedia, m_pwndDelMedia);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_DelSec, m_pwndDelSec);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_DelTarRes, m_pwndDelTarRes);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_DelWeath, m_pwndDelWeath);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_DelXO, m_pwndDelXOver);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_HVACList, m_pwndHVACList);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_LoadList, m_pwndLoadList);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_MediaList, m_pwndMediaList);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_SecList, m_pwndSecList);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_SelCustLayout, m_pwndSelCustLayout);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_SelTarScope, m_pwndSelTarScope);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_TarScope, m_pwndTarScope);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_TarResList, m_pwndTarResList);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_CustLights, m_pwndUseCustLights);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_WeathList, m_pwndWeatherList);
    CastChildWnd(*m_pwndCont, kCQCAdmin::ridTab_SysCfgGlob_XOList, m_pwndXOverList);

    // Set up our notification handlers
    m_pwndAddHVAC->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndAddLoad->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndAddMedia->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndAddSec->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndAddTarRes->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndAddWeath->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndAddXOver->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndDelCustLayout->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndDelHVAC->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndDelLoad->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndDelMedia->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndDelSec->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndDelTarRes->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndDelWeath->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndDelXOver->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndHVACList->pnothRegisterHandler(this, &TESCGlobalTab::eLBHandler);
    m_pwndLoadList->pnothRegisterHandler(this, &TESCGlobalTab::eLBHandler);
    m_pwndMediaList->pnothRegisterHandler(this, &TESCGlobalTab::eLBHandler);
    m_pwndSecList->pnothRegisterHandler(this, &TESCGlobalTab::eLBHandler);
    m_pwndSelCustLayout->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndSelTarScope->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndTarResList->pnothRegisterHandler(this, &TESCGlobalTab::eLBHandler);
    m_pwndUseCustLights->pnothRegisterHandler(this, &TESCGlobalTab::eClickHandler);
    m_pwndWeatherList->pnothRegisterHandler(this, &TESCGlobalTab::eLBHandler);
    m_pwndXOverList->pnothRegisterHandler(this, &TESCGlobalTab::eLBHandler);

    // Set the titles on all of the list boxes
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());

    colCols[0] = L"Thermostats";
    m_pwndHVACList->SetColumns(colCols);

    colCols[0] = L"Lighting Loads";
    m_pwndLoadList->SetColumns(colCols);

    colCols[0] = L"Media Systems";
    m_pwndMediaList->SetColumns(colCols);

    colCols[0] = L"Security Devices";
    m_pwndSecList->SetColumns(colCols);

    colCols[0] = L"Target Resolutions";
    m_pwndTarResList->SetColumns(colCols);

    colCols[0] = L"Weather Sources";
    m_pwndWeatherList->SetColumns(colCols);

    colCols[0] = L"Extra Overlays";
    m_pwndXOverList->SetColumns(colCols);

    // Load up our incoming content
    LoadData();

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TESCGlobalTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called from the various delete button click handlers below, at least the ones
//  that deal with the standard configuration lists. This doesn't require dealing
//  with the list specific classes, so we can handle them all generically.
//
tCIDLib::TVoid
TESCGlobalTab::DeleteListItem(  const   tCIDLib::TCh* const     pszType
                                ,       TMultiColListBox* const pwndList
                                , const tCQCSysCfg::ECfgLists   eList)
{
    const tCIDLib::TCard4 c4LIndex = pwndList->c4CurItem();
    if (c4LIndex != kCIDLib::c4MaxCard)
    {
        TString strName;
        pwndList->QueryColText(c4LIndex, 0, strName);

        TYesNoBox msgbQ
        (
            facCQCAdmin.strMsg(kCQCAMsgs::midQ_Delete, TString(pszType), strName)
        );

        if (msgbQ.bShowIt(*this))
        {
            // Get the id so we can look it up
            TString strID = pwndList->strUserDataAt(c4LIndex);

            // Now remove it from our list and from the configuration
            pwndList->RemoveCurrent();
            m_pscfgEdit->DeleteListId(eList, strID);
        }
    }
}

tCIDCtrls::EEvResponses TESCGlobalTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_AddHVAC)
    {
        TEdSysCfgTab::EAddOpts  eOpt;
        TEdSCHVACDlg            dlgHVAC;
        TSCHVACInfo             scliNew;

        if (dlgHVAC.bRunDlg(*this, scliNew, *m_pcfcDevs, eOpt, *m_pscfgEdit))
        {
            tCIDLib::TCard4 c4Ind;
            m_pscfgEdit->bAddOrUpdateHVAC(scliNew, c4Ind);

            tCIDLib::TStrList colCols(1);
            colCols.objAdd(scliNew.m_strTitle);

            const tCIDLib::TCard4 c4At = m_pwndHVACList->c4AddItem(colCols, 0);
            m_pwndHVACList->SetUserDataAt(c4At, scliNew.m_strUniqueId);

            //
            //  If the option is to add to all or enabled rooms, then let's go
            //  through the rooms and update them.
            //
            if (eOpt != TEdSysCfgTab::EAddOpts::AddToNone)
            {
                m_pscfgEdit->AddToRooms
                (
                    tCQCSysCfg::ECfgLists::HVAC
                    , scliNew.m_strUniqueId
                    , eOpt == TEdSysCfgTab::EAddOpts::AddToAll
                );
            }

            // Select this new one
            m_pwndHVACList->SelectByIndex(c4At, kCIDLib::True);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_AddLoad)
    {
        TEdSysCfgTab::EAddOpts  eOpt;
        TEdSCLoadDlg            dlgLoad;
        TSCLoadInfo             scliNew;

        if (dlgLoad.bRunDlg(*this, scliNew, *m_pcfcDevs, eOpt, *m_pscfgEdit))
        {
            tCIDLib::TCard4 c4Ind;
            m_pscfgEdit->bAddOrUpdateLoad(scliNew, c4Ind);

            tCIDLib::TStrList colCols(1);
            colCols.objAdd(scliNew.m_strTitle);

            const tCIDLib::TCard4 c4At = m_pwndLoadList->c4AddItem(colCols, 0);
            m_pwndLoadList->SetUserDataAt(c4At, scliNew.m_strUniqueId);

            //
            //  If the option is to add to all or enabled rooms, then let's go
            //  through the rooms and update them.
            //
            if (eOpt != TEdSysCfgTab::EAddOpts::AddToNone)
            {
                m_pscfgEdit->AddToRooms
                (
                    tCQCSysCfg::ECfgLists::Lighting
                    , scliNew.m_strUniqueId
                    , eOpt == TEdSysCfgTab::EAddOpts::AddToAll
                );
                m_pwndLoadList->SelectByIndex(c4At, kCIDLib::True);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_AddMedia)
    {
        TEdSCMediaDlg   dlgLoad;
        TSCMediaInfo    scliNew;
        if (dlgLoad.bRunDlg(*this, scliNew, *m_pcfcDevs, *m_pscfgEdit))
        {
            tCIDLib::TCard4 c4Ind;
            m_pscfgEdit->bAddOrUpdateMedia(scliNew, c4Ind);

            tCIDLib::TStrList colCols(1);
            colCols.objAdd(scliNew.m_strTitle);

            const tCIDLib::TCard4 c4At = m_pwndMediaList->c4AddItem(colCols, 0);
            m_pwndMediaList->SetUserDataAt(c4At, scliNew.m_strUniqueId);

            // Select this new one
            m_pwndMediaList->SelectByIndex(c4At);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_AddSec)
    {
        TEdSCSecDlg     dlgLoad;
        TSCSecInfo      scliNew;
        if (dlgLoad.bRunDlg(*this, scliNew, *m_pcfcDevs, *m_pscfgEdit))
        {
            tCIDLib::TCard4 c4Ind;
            m_pscfgEdit->bAddOrUpdateSec(scliNew, c4Ind);

            tCIDLib::TStrList colCols(1);
            colCols.objAdd(scliNew.m_strTitle);

            const tCIDLib::TCard4 c4At = m_pwndSecList->c4AddItem(colCols, 0);
            m_pwndSecList->SetUserDataAt(c4At, scliNew.m_strUniqueId);

            // Select this new one
            m_pwndSecList->SelectByIndex(c4At);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_AddTarRes)
    {
        //
        //  Start it at our native res, to make it easier for them to do a
        //  relative adjustment to fit our content into one axis and keep AR.
        //
        TSize szInit(1280, 800);
        TSize szNew;
        const tCIDLib::TBoolean bRes = facCIDWUtils().bGetSize
        (
            *this
            , L"Enter a new target resolution"
            , szNew
            , szInit
            , TSize(64, 64)
            , TSize(8192, 8192)
            , kCIDLib::False
        );

        if (bRes)
        {
            // Make sure it's a unique resolution
            tCIDLib::TCard4 c4Index;
            if (m_pscfgEdit->bAddRes(szNew, c4Index))
            {
                TString strRes;
                szNew.FormatToText(strRes, tCIDLib::ERadices::Dec, kCIDLib::chLatin_x);

                // Add this new guy. We put the packed size into the row id
                tCIDLib::TStrList colCols(1);
                colCols.objAdd(strRes);
                const tCIDLib::TCard4 c4At = m_pwndTarResList->c4AddItem(colCols, szNew.c4Packed());
                m_pwndTarResList->SelectByIndex(c4At);
            }
             else
            {
                TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_NotUniqueRes));
                msgbErr.ShowIt(*this);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_AddWeath)
    {
        TEdSCWeatherDlg     dlgLoad;
        TSCWeatherInfo      scliNew;
        if (dlgLoad.bRunDlg(*this, scliNew, *m_pcfcDevs, *m_pscfgEdit))
        {
            tCIDLib::TCard4 c4Ind;
            m_pscfgEdit->bAddOrUpdateWeather(scliNew, c4Ind);

            tCIDLib::TStrList colCols(1);
            colCols.objAdd(scliNew.m_strTitle);

            const tCIDLib::TCard4 c4At = m_pwndWeatherList->c4AddItem(colCols, 0);
            m_pwndWeatherList->SetUserDataAt(c4At, scliNew.m_strUniqueId);

            // Select this new one
            m_pwndWeatherList->SelectByIndex(c4At);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_AddXO)
    {
        TSCXOverInfo scliNew;
        TEdSCXOverDlg dlgAdd;
        if (dlgAdd.bRunDlg(*this, scliNew, *m_pscfgEdit))
        {
            tCIDLib::TCard4 c4Ind;
            m_pscfgEdit->bAddOrUpdateXOver(scliNew, c4Ind);

            tCIDLib::TStrList colCols(1);
            colCols.objAdd(scliNew.m_strTitle);
            const tCIDLib::TCard4 c4At = m_pwndXOverList->c4AddItem(colCols, 0);
            m_pwndXOverList->SetUserDataAt(c4At, scliNew.m_strUniqueId);

            // Select this new one
            m_pwndXOverList->SelectByIndex(c4At);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_SelTarScope)
    {
        //
        //  We select it as though it's a template scope, though ultimately we output into
        //  the image repo as well (in the same scope path.)
        //
        TString strSel = m_pwndTarScope->strWndText();
        tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
        (
            *this
            , L"Select a target Auto-Gen scope"
            , tCQCRemBrws::EDTypes::Template
            , strSel
            , facCQCAdmin.cuctxToUse()
            , tCIDLib::eOREnumBits
              (
                tCQCTreeBrws::EFSelFlags::SelectScopes, tCQCTreeBrws::EFSelFlags::Edit
              )
            , strSel
        );

        if (bRes)
        {
            // Make sure it's not '/User' itself
            if (strSel == L"/User")
            {
                TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_BadTarScope));
                msgbErr.ShowIt(*this);
            }
             else
            {
                m_pwndTarScope->strWndText(strSel);
                m_pscfgEdit->strTarScope(strSel);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_DelCustLayout)
    {
        m_pwndCustLayout->ClearText();
        m_pscfgEdit->strCustLayout(TString::strEmpty());
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_DelHVAC)
    {
        DeleteListItem(L"HVAC", m_pwndHVACList, tCQCSysCfg::ECfgLists::HVAC);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_DelLoad)
    {
        DeleteListItem(L"light", m_pwndLoadList, tCQCSysCfg::ECfgLists::Lighting);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_DelMedia)
    {
        DeleteListItem(L"media", m_pwndMediaList, tCQCSysCfg::ECfgLists::Media);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_DelSec)
    {
        DeleteListItem(L"security", m_pwndSecList, tCQCSysCfg::ECfgLists::Security);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_DelTarRes)
    {
        const tCIDLib::TCard4 c4ListInd = m_pwndTarResList->c4CurItem();
        if (c4ListInd != kCIDLib::c4MaxCard)
        {
            //
            //  Save the size before we remove it from the list box. We stored the
            //  packed size as the row id, so get it back out.
            //
            TSize szRem;
            szRem.SetFromPacked(m_pwndTarResList->c4IndexToId(c4ListInd));
            m_pwndTarResList->RemoveCurrent();

            // And now from the configuration
            m_pscfgEdit->DeleteRes(szRem);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_DelWeath)
    {
        DeleteListItem(L"weather", m_pwndWeatherList, tCQCSysCfg::ECfgLists::Weather);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_DelXO)
    {
        DeleteListItem(L"overlay", m_pwndXOverList, tCQCSysCfg::ECfgLists::XOvers);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_SelCustLayout)
    {
        // Run the standard template selection dialog
        TString strTitle(kCQCAMsgs::midPrompt_CustLayout, facCQCAdmin);

        TString strSel = m_pwndCustLayout->strWndText();
        const tCIDLib::TBoolean bSel = facCQCTreeBrws().bSelectFile
        (
            *this
            , strTitle
            , tCQCRemBrws::EDTypes::Template
            , strSel
            , facCQCAdmin.cuctxToUse()
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strSel
        );

        if (bSel)
        {
            // They selected one, so let's store it and update our display
            m_pscfgEdit->strCustLayout(strSel);
            m_pwndCustLayout->strWndText(strSel);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_CustLights)
    {
        // Update the setting to match the check box state
        m_pscfgEdit->bUseCustLights(m_pwndUseCustLights->bCheckedState());
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TESCGlobalTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_HVACList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_HVAC)->ClearText();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            tCIDLib::TCard4 c4Ind;
            TSCHVACInfo scliEdit = m_pscfgEdit->scliHVACByUID
            (
                m_pwndHVACList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            TEdSysCfgTab::EAddOpts  eOpt;
            TEdSCHVACDlg            dlgHVAC;
            if (dlgHVAC.bRunDlg(*this, scliEdit, *m_pcfcDevs, eOpt, *m_pscfgEdit))
            {
                m_pscfgEdit->bAddOrUpdateHVAC(scliEdit, c4Ind);

                // Update the title and force a reselect to update dislay info
                m_pwndHVACList->SetColText(wnotEvent.c4Index(), 0, scliEdit.m_strTitle);
                m_pwndHVACList->SelectByIndex(wnotEvent.c4Index(), kCIDLib::True);
            }
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Get the selected one and display his info
            tCIDLib::TCard4 c4Ind;
            const TSCHVACInfo& scliCur = m_pscfgEdit->scliHVACByUID
            (
                m_pwndHVACList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            TString strText(scliCur.m_strMoniker);
            if (!scliCur.m_strThermoSub.bIsEmpty())
            {
                strText.Append(L" (");
                strText.Append(scliCur.m_strThermoSub);
                strText.Append(L")");
            }
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_HVAC)->strWndText(strText);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_LoadList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_LoadMon)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_DimFld)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_SwitchFld)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_ClrFld)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_AsSpoken)->ClearText();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            tCIDLib::TCard4 c4Ind;
            TSCLoadInfo scliEdit = m_pscfgEdit->scliLoadByUID
            (
                m_pwndLoadList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            // The add option is ignored here. It's only for when adding new ons
            TEdSysCfgTab::EAddOpts  eOpt;
            TEdSCLoadDlg            dlgLoad;
            if (dlgLoad.bRunDlg(*this, scliEdit, *m_pcfcDevs, eOpt, *m_pscfgEdit))
            {
                m_pscfgEdit->bAddOrUpdateLoad(scliEdit, c4Ind);

                // The title might have been edited, so update and force a re-select
                m_pwndLoadList->SetColText(wnotEvent.c4Index(), 0, scliEdit.m_strTitle);
                m_pwndLoadList->SelectByIndex(wnotEvent.c4Index(), kCIDLib::True);
            }
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Get the selected one and display his info
            tCIDLib::TCard4 c4Ind;
            const TSCLoadInfo& scliCur = m_pscfgEdit->scliLoadByUID
            (
                m_pwndLoadList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            // If a dimmer, then we should also have a switch, else just a switch
            if (scliCur.bIsDimmer())
            {
                m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_DimFld)->strWndText(scliCur.m_strField);
                m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_SwitchFld)->strWndText(scliCur.m_strField2);
            }
             else
            {
                m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_SwitchFld)->strWndText(scliCur.m_strField);
                m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_DimFld)->ClearText();
            }
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_ClrFld)->strWndText(scliCur.m_strClrField);
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_LoadMon)->strWndText(scliCur.m_strMoniker);
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_AsSpoken)->strWndText(scliCur.m_strAltName);
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_MediaList)
    {
        // Keep the display fields updated for the selected item
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_RendMon)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_RepoMon)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_AudioMon)->ClearText();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            tCIDLib::TCard4 c4Ind;
            const TSCMediaInfo& scliCur = m_pscfgEdit->scliMediaByUID
            (
                m_pwndMediaList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_RendMon)->strWndText(scliCur.m_strRendMon);
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_RepoMon)->strWndText(scliCur.m_strRepoMon);

            TString strText(scliCur.m_strAudioMon);
            if (!scliCur.m_strAudioSub.bIsEmpty())
            {
                strText.Append(L" (");
                strText.Append(scliCur.m_strAudioSub);
                strText.Append(L")");
            }
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_AudioMon)->strWndText(strText);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            tCIDLib::TCard4 c4Ind;
            TSCMediaInfo scliEdit = m_pscfgEdit->scliMediaByUID
            (
                m_pwndMediaList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            TEdSCMediaDlg dlgLoad;
            if (dlgLoad.bRunDlg(*this, scliEdit, *m_pcfcDevs, *m_pscfgEdit))
            {
                m_pscfgEdit->bAddOrUpdateMedia(scliEdit, c4Ind);

                // The title might have been edited, so update and refresh
                m_pwndMediaList->SetColText(wnotEvent.c4Index(), 0, scliEdit.m_strTitle);
                m_pwndMediaList->SelectByIndex(wnotEvent.c4Index(), kCIDLib::True);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_SecList)
    {
        // Keep the display fields updated for the selected item
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_SecMon)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_SecArea)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_SecArmingCode)->ClearText();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            tCIDLib::TCard4 c4Ind;
            const TSCSecInfo& scliCur = m_pscfgEdit->scliSecByUID
            (
                m_pwndSecList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_SecMon)->strWndText(scliCur.m_strMoniker);
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_SecArea)->strWndText(scliCur.m_strAreaName);
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_SecArmingCode)->strWndText(scliCur.m_strArmingCode);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            tCIDLib::TCard4 c4Ind;
            TSCSecInfo scliEdit = m_pscfgEdit->scliSecByUID
            (
                m_pwndSecList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            TEdSCSecDlg dlgLoad;
            if (dlgLoad.bRunDlg(*this, scliEdit, *m_pcfcDevs, *m_pscfgEdit))
            {
                m_pscfgEdit->bAddOrUpdateSec(scliEdit, c4Ind);

                // The title might have been edited, so update and refresh
                m_pwndSecList->SetColText(wnotEvent.c4Index(), 0, scliEdit.m_strTitle);
                m_pwndSecList->SelectByIndex(wnotEvent.c4Index(), kCIDLib::True);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_WeathList)
    {
        // Keep the display fields updated for the selected item
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_WeathMon)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_WeathScale)->ClearText();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            tCIDLib::TCard4 c4Ind;
            const TSCWeatherInfo& scliCur = m_pscfgEdit->scliWeatherByUID
            (
                m_pwndWeatherList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_WeathMon)->strWndText(scliCur.m_strMoniker);
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_WeathScale)->strWndText
            (
                scliCur.m_bFScale ? L"Fahrenheit" : L"Celcius"
            );
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            tCIDLib::TCard4 c4Ind;
            TSCWeatherInfo scliCur = m_pscfgEdit->scliWeatherByUID
            (
                m_pwndWeatherList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            TEdSCWeatherDlg dlgLoad;
            if (dlgLoad.bRunDlg(*this, scliCur, *m_pcfcDevs, *m_pscfgEdit))
            {
                m_pscfgEdit->bAddOrUpdateWeather(scliCur, c4Ind);

                // The title might have been edited, so update and refresh
                m_pwndWeatherList->SetColText(wnotEvent.c4Index(), 0, scliCur.m_strTitle);
                m_pwndWeatherList->SelectByIndex(wnotEvent.c4Index(), kCIDLib::True);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_SysCfgGlob_XOList)
    {
        // Keep the display fields updated for the selected item
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_ContTmpl)->ClearText();
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_TileTmpl)->ClearText();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            tCIDLib::TCard4 c4Ind;
            const TSCXOverInfo& scliCur = m_pscfgEdit->scliXOverByUID
            (
                m_pwndXOverList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_ContTmpl)->strWndText(scliCur.m_strContentTmpl);
            m_pwndCont->pwndChildById(kCQCAdmin::ridTab_SysCfgGlob_TileTmpl)->strWndText(scliCur.m_strTileTmpl);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            tCIDLib::TCard4 c4Ind;
            TSCXOverInfo scliCur = m_pscfgEdit->scliXOverByUID
            (
                m_pwndXOverList->strUserDataAt(wnotEvent.c4Index()), c4Ind
            );

            TEdSCXOverDlg dlgLoad;
            if (dlgLoad.bRunDlg(*this, scliCur, *m_pscfgEdit))
            {
                m_pscfgEdit->bAddOrUpdateXOver(scliCur, c4Ind);

                // The title might have been edited, so update and refresh
                m_pwndXOverList->SetColText(wnotEvent.c4Index(), 0, scliCur.m_strTitle);
                m_pwndXOverList->SelectByIndex(wnotEvent.c4Index(), kCIDLib::True);
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Called after window creation, to load up the incoming data.
tCIDLib::TVoid TESCGlobalTab::LoadData()
{
    // The non-list based stuff
    m_pwndTarScope->strWndText(m_pscfgEdit->strTarScope());
    m_pwndCustLayout->strWndText(m_pscfgEdit->strCustLayout());
    m_pwndUseCustLights->SetCheckedState(m_pscfgEdit->bUseCustLights());

    // And now do all the list oriented stuff
    tCIDLib::TCard4 c4Count;
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());

    // Load the resolutions list
    m_pwndTarResList->RemoveAll();
    c4Count = m_pscfgEdit->c4ResCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSize& szCur = m_pscfgEdit->szResAt(c4Index);
        szCur.FormatToText(colCols[0], tCIDLib::ERadices::Dec, kCIDLib::chLatin_x);

        // Encode the sizes and set them as the row id
        m_pwndTarResList->c4AddItem(colCols, szCur.c4Packed());
    }
    if (c4Count)
        m_pwndTarResList->SelectByIndex(0);

    // Load up the HVACs
    m_pwndHVACList->RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::HVAC);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCHVACInfo& scliCur = m_pscfgEdit->scliHVACAt(c4Index);
        colCols[0] = scliCur.m_strTitle;
        const tCIDLib::TCard4 c4At = m_pwndHVACList->c4AddItem(colCols, 0);
        m_pwndHVACList->SetUserDataAt(c4At, scliCur.m_strUniqueId);
    }
    if (c4Count)
        m_pwndHVACList->SelectByIndex(0);

    // Load up the lighting loads
    m_pwndLoadList->RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Lighting);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCLoadInfo& scliCur = m_pscfgEdit->scliLoadAt(c4Index);
        colCols[0] = scliCur.m_strTitle;
        const tCIDLib::TCard4 c4At = m_pwndLoadList->c4AddItem(colCols, 0);
        m_pwndLoadList->SetUserDataAt(c4At, scliCur.m_strUniqueId);
    }
    if (c4Count)
        m_pwndLoadList->SelectByIndex(0);

    // Load up the media info objects
    m_pwndMediaList->RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Media);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCMediaInfo& scliCur = m_pscfgEdit->scliMediaAt(c4Index);
        colCols[0] = scliCur.m_strTitle;
        const tCIDLib::TCard4 c4At = m_pwndMediaList->c4AddItem(colCols, 0);
        m_pwndMediaList->SetUserDataAt(c4At, scliCur.m_strUniqueId);
    }
    if (c4Count)
        m_pwndMediaList->SelectByIndex(0);

    // Load up the security info objects
    m_pwndSecList->RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Security);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCSecInfo& scliCur = m_pscfgEdit->scliSecAt(c4Index);
        colCols[0] = scliCur.m_strTitle;
        const tCIDLib::TCard4 c4At = m_pwndSecList->c4AddItem(colCols, 0);
        m_pwndSecList->SetUserDataAt(c4At, scliCur.m_strUniqueId);
    }
    if (c4Count)
        m_pwndSecList->SelectByIndex(0);

    // Load up the weather objects
    m_pwndWeatherList->RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::Weather);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCWeatherInfo& scliCur = m_pscfgEdit->scliWeatherAt(c4Index);
        colCols[0] = scliCur.m_strTitle;
        const tCIDLib::TCard4 c4At = m_pwndWeatherList->c4AddItem(colCols, 0);
        m_pwndWeatherList->SetUserDataAt(c4At, scliCur.m_strUniqueId);
    }
    if (c4Count)
        m_pwndWeatherList->SelectByIndex(0);

    // Load up the extra overlay objects
    m_pwndXOverList->RemoveAll();
    c4Count = m_pscfgEdit->c4ListIdCount(tCQCSysCfg::ECfgLists::XOvers);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCXOverInfo& scliCur = m_pscfgEdit->scliXOverAt(c4Index);
        colCols[0] = scliCur.m_strTitle;
        const tCIDLib::TCard4 c4At = m_pwndXOverList->c4AddItem(colCols, 0);
        m_pwndXOverList->SetUserDataAt(c4At, scliCur.m_strUniqueId);
    }
    if (c4Count)
        m_pwndXOverList->SelectByIndex(0);
}




// ---------------------------------------------------------------------------
//  CLASS: TEdSysCfgTab
// PREFIX: wnd
//
//  The main admin client tab for editing the system configuration.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdSysCfgTab: Constructors and destructor
// ---------------------------------------------------------------------------
TEdSysCfgTab::TEdSysCfgTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Config", kCIDLib::True)
    , m_pwndGenerate(nullptr)
    , m_pwndGlobTab(nullptr)
    , m_pwndRevert(nullptr)
    , m_pwndTabs(nullptr)
    , m_pwndValidate(nullptr)
{
}

TEdSysCfgTab::~TEdSysCfgTab()
{
}



// ---------------------------------------------------------------------------
//  TEdSysCfgTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TEdSysCfgTab::CreateTab(TTabbedWnd& wndParent, const TCQCSysCfg& scfgEdit)
{
    // Save a copy of the incoming system configuration data
    m_scfgEdit = scfgEdit;

    // And another for testing for changes
    m_scfgOrg = scfgEdit;

    //
    //  Initialize our field cache. Tell it to only load V2 drivers since that's all
    //  we care about.
    //
    m_cfcDevs.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite), kCIDLib::True);

    // And call through to our parent to create this tab
    wndParent.c4CreateTab(this, L"System Config", 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TEdSysCfgTab: Protected, inherited methods
// ---------------------------------------------------------------------------

// We just need to keep our controls arranged to fit
tCIDLib::TVoid
TEdSysCfgTab::AreaChanged(  const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TTabWindow::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && m_pwndTabs && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TEdSysCfgTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_EdSysCfg, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to get them sized to fit us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get pointers to some of our controls
    CastChildWnd(*this, kCQCAdmin::ridTab_EdSysCfg_Generate, m_pwndGenerate);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdSysCfg_Tabs, m_pwndTabs);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdSysCfg_Revert, m_pwndRevert);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdSysCfg_Validate, m_pwndValidate);

    // Create and load up our own tabs
    m_pwndGlobTab = new TESCGlobalTab(&m_scfgEdit, &m_cfcDevs);
    m_pwndTabs->c4CreateTab(m_pwndGlobTab, L"Global", 0, kCIDLib::True);

    m_pwndRoomsTab = new TESCRoomsTab(&m_scfgEdit, &m_cfcDevs);
    m_pwndTabs->c4CreateTab(m_pwndRoomsTab, L"Rooms", 1, kCIDLib::False);

    // Register notification handlers
    m_pwndGenerate->pnothRegisterHandler(this, &TEdSysCfgTab::eClickHandler);
    m_pwndRevert->pnothRegisterHandler(this, &TEdSysCfgTab::eClickHandler);
    m_pwndValidate->pnothRegisterHandler(this, &TEdSysCfgTab::eClickHandler);

    return kCIDLib::True;
}


// Check for changes, saving them if requested
tCIDLib::ESaveRes
TEdSysCfgTab::eDoSave(          TString&                strErrMsg
                        , const tCQCAdmin::ESaveModes   eMode
                        ,       tCIDLib::TBoolean&      bChangesSaved)
{
    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (m_scfgEdit == m_scfgOrg)
            return tCIDLib::ESaveRes::NoChanges;
        return tCIDLib::ESaveRes::OK;
    }

    // Do a validation pass. If any errors, return the first one
    TValErrInfo::TErrList colErrs;
    m_pwndGlobTab->Validate(colErrs);
    m_pwndRoomsTab->Validate(colErrs);
    if (!colErrs.bIsEmpty())
    {
        strErrMsg = colErrs[0].m_strErr;
        return tCIDLib::ESaveRes::Errors;
    }

    try
    {
        TCQCSysCfgClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TCQCSysCfgClientProxy>
        (
            TCQCSysCfgClientProxy::strBinding, 4000
        );
        TCntPtr<TCQCSysCfgClientProxy> orbcProxy(porbcProxy);

        tCIDLib::TCard4 c4SerialNum;
        orbcProxy->StoreRoomCfg(m_scfgEdit, c4SerialNum, facCQCAdmin.sectUser());
        bChangesSaved = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        strErrMsg = L"The system configuration could not be saved";
        return tCIDLib::ESaveRes::Errors;
    }

    // Copy over the edits to the original
    m_scfgOrg = m_scfgEdit;

    // No more changes now
    return tCIDLib::ESaveRes::NoChanges;
}




// ---------------------------------------------------------------------------
//  TEdSysCfgTab: Private, non-virtaul methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TEdSysCfgTab::bDoValidation(const tCIDLib::TBoolean bNoPassNot)
{
    //
    //  Force a validation pass and show any errors. We just ask each of the tabs to add
    //  any errors to our list.
    //
    TValErrInfo::TErrList colErrs;
    m_pwndGlobTab->Validate(colErrs);
    m_pwndRoomsTab->Validate(colErrs);

    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (colErrs.bIsEmpty())
    {
        if (!bNoPassNot)
        {
            TOkBox msgbOK(L"There were no validation errors");
            msgbOK.ShowIt(*this);
        }
        bRet = kCIDLib::True;
    }
     else
    {
        tCIDLib::TBoolean bSel;
        tCIDLib::TCard4 c4SelErr = kCIDLib::c4MaxCard;
        {
            TEdSCValErrDlg dlgErrs;
            bSel = dlgErrs.bRunDlg(*this, colErrs, c4SelErr);
        }

        if (bSel)
        {
            TValErrInfo& verrSel = colErrs[c4SelErr];

            // Activate the tab that contains the error
            if ((verrSel.m_eType == TValErrInfo::ETypes::Room)
            ||  (verrSel.m_eType == TValErrInfo::ETypes::RoomCfg))
            {

                //
                //  It's one of the room tabs or something relatd to the room
                //  config, not the global config. So activate the room tab
                //  and let him react to the error.
                //
                m_pwndTabs->c4ActivateById(m_pwndRoomsTab->c4TabId());
                m_pwndRoomsTab->SelectErrTarget(verrSel);
            }
             else if (verrSel.m_eType == TValErrInfo::ETypes::General)
            {
                // Activate the global tab and let it react to the error
                m_pwndTabs->c4ActivateById(m_pwndGlobTab->c4TabId());
                m_pwndGlobTab->SelectErrTarget(verrSel);
            }
        }
    }
    return bRet;
}


// There's only one button, so we don't have to check the source id
tCIDCtrls::EEvResponses TEdSysCfgTab::eClickHandler(TButtClickInfo& wnotInfo)
{
    if (wnotInfo.widSource() == kCQCAdmin::ridTab_EdSysCfg_Generate)
    {
        //
        //  First do a validation pass to make sure they are ok. Tell it not to show any
        //  acknowledgement if there are no errors.
        //
        if (bDoValidation(kCIDLib::True))
        {
            //
            //  Force a save so that we know we are working on the current stuff. We
            //  don't really use the changes saved flag here. We are going to regen
            //  even if nothing was saved.
            //
            tCIDLib::TBoolean bChangesSaved = kCIDLib::False;
            TString strErrMsg;
            tCIDLib::ESaveRes eRes = eSaveChanges
            (
                strErrMsg, tCQCAdmin::ESaveModes::Save, bChangesSaved
            );
            if (eRes == tCIDLib::ESaveRes::Errors)
            {
                TErrBox msgbErr(strErrMsg);
                msgbErr.ShowIt(*this);
            }
             else
            {
                // It worked so let's do it. We invoke the auto-gen dialog who does the work
                TAutoGenDlg dlgAutoGen;
                dlgAutoGen.RunDlg(*this);
            }
        }
    }
     else if (wnotInfo.widSource() == kCQCAdmin::ridTab_EdSysCfg_Revert)
    {
        if (m_scfgEdit != m_scfgOrg)
        {
            TYesNoBox msgbRevert(facCQCAdmin.strMsg(kCQCAMsgs::midQ_DiscardChanges));
            if (msgbRevert.bShowIt(*this))
            {
                //
                //  They want to do it, so put the previous info back into the
                //  edit object, then ask our two main tabs to reset themselves.
                //
                m_scfgEdit = m_scfgOrg;
                m_pwndGlobTab->Reload();
                m_pwndRoomsTab->Reload();
            }
        }
    }
     else if (wnotInfo.widSource() == kCQCAdmin::ridTab_EdSysCfg_Validate)
    {
        bDoValidation(kCIDLib::False);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

