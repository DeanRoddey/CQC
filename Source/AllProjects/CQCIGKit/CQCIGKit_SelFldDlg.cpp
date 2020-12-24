//
// FILE NAME: CQCIGKit_SelFldDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/23/2006
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
//  This file implements a dialog that let's the user choose a driver.field
//  value.
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
#include    "CQCIGKit_SelFldDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSelFldDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCQCSelFldDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSelFldDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSelFldDlg::TCQCSelFldDlg() :

    m_bNameOnly(kCIDLib::False)
    , m_pcfcFldInfo(nullptr)
    , m_pwndByName(nullptr)
    , m_pwndByValue(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDrivers(nullptr)
    , m_pwndFields(nullptr)
{
}

TCQCSelFldDlg::~TCQCSelFldDlg()
{
}


// ---------------------------------------------------------------------------
//  TCQCSelFldDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCSelFldDlg::bRun(const   TWindow&        wndOwner
                    ,       TCQCFldCache&   cfcFldInfo
                    ,       TString&        strDriver
                    ,       TString&        strField)
{
    // If there are no drivers, then can't do anything
    if (!cfcFldInfo.c4DevCount())
    {
        TErrBox msgbEmpty
        (
            L"Field Selection", L"No devices were available for the selection criteria"
        );
        msgbEmpty.ShowIt(wndOwner);
        return kCIDLib::False;
    }

    //
    //  Store incoming stuff for later use. For this version, we only allow by name
    //  selection.
    //
    m_bNameOnly = kCIDLib::True;
    m_pcfcFldInfo = &cfcFldInfo;
    m_strDriver = strDriver;
    m_strField = strField;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_SelFld
    );

    if ((c4Res == kCQCIGKit::ridDlg_SelFld_ByName)
    ||  (c4Res == kCQCIGKit::ridDlg_SelFld_ByValue))
    {
        strDriver = m_strDriver;
        strField = m_strField;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCSelFldDlg::bRun(const   TWindow&            wndOwner
                    ,       TCQCFldCache&       cfcFldInfo
                    ,       TString&            strDriver
                    ,       TString&            strField
                    ,       tCIDLib::TBoolean&  bByValue
                    , const tCIDLib::TBoolean   bNameOnly)
{
    // If there are no drivers, then can't do anything
    if (!cfcFldInfo.c4DevCount())
    {
        TErrBox msgbEmpty
        (
            L"Field Selection", L"No devices were available for the selection criteria"
        );
        msgbEmpty.ShowIt(wndOwner);
        return kCIDLib::False;
    }

    // Store incoming stuff for later use
    m_bNameOnly = bNameOnly;
    m_pcfcFldInfo = &cfcFldInfo;
    m_strDriver = strDriver;
    m_strField = strField;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_SelFld
    );

    if ((c4Res == kCQCIGKit::ridDlg_SelFld_ByName)
    ||  (c4Res == kCQCIGKit::ridDlg_SelFld_ByValue))
    {
        bByValue = (c4Res == kCQCIGKit::ridDlg_SelFld_ByValue);
        strDriver = m_strDriver;
        strField = m_strField;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCSelFldDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCSelFldDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelFld_ByName, m_pwndByName);
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelFld_ByValue, m_pwndByValue);
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelFld_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelFld_Drivers, m_pwndDrivers);
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelFld_Fields, m_pwndFields);

    // Set the columns on the drivers/fields lists. We have one column each
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_SelFld_Drivers));
    m_pwndDrivers->SetColumns(colCols);

    colCols[0] = facCQCIGKit().strMsg(kIGKitMsgs::midDlg_SelFld_Fields);
    m_pwndFields->SetColumns(colCols);

    // Load up the driver list, selecting the initial one, if any.
    TVector<TString> colMonikers;
    m_pcfcFldInfo->QueryDevList(colMonikers);
    colMonikers.Sort(TString::eCompI);

    const tCIDLib::TCard4 c4DrvCount = colMonikers.c4ElemCount();
    tCIDLib::TCard4 c4InitSel = kCIDLib::c4MaxCard;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DrvCount; c4Index++)
    {
        colCols[0] = colMonikers[c4Index];
        m_pwndDrivers->c4AddItem(colCols, c4Index);
        if (colCols[0].bCompareI(m_strDriver))
            c4InitSel = c4Index;
    }

    // Select the initial one we found above, if any. else just the 0th one
    if (c4InitSel == kCIDLib::c4MaxCard)
        m_pwndDrivers->SelectByIndex(0);
    else
        m_pwndDrivers->c4SelectById(c4InitSel);

    //
    //  Now get the current selection. If there was none incoming or it was not found
    //  we get the selected 0th one.
    //
    TString strSelDrv;
    tCIDLib::TCard4 c4CurItem = m_pwndDrivers->c4CurItem();
    m_pwndDrivers->QueryColText(c4CurItem, 0, strSelDrv);

    //
    //  If we did not find the incoming driver, then we want to pass an empty field
    //  to just get the default.
    //
    TString strSelFld = m_strField;
    if (!m_strDriver.bIsEmpty() && (strSelDrv != m_strDriver))
        strSelFld.Clear();

    // Register our handlers
    m_pwndCancel->pnothRegisterHandler(this, &TCQCSelFldDlg::eClickHandler);
    m_pwndDrivers->pnothRegisterHandler(this, &TCQCSelFldDlg::eLBHandler);
    m_pwndByName->pnothRegisterHandler(this, &TCQCSelFldDlg::eClickHandler);

    //
    //  If not allowing by value, destroy that button, else register it. If we delete
    //  by value, change the text on the by name button to just be Select, and give it
    //  group and default button styles. And we have to set the id to the standard OK
    //  id.
    //
    if (m_bNameOnly)
    {
        m_pwndByValue->Destroy();
        delete m_pwndByValue;
        m_pwndByValue = nullptr;

        m_pwndByName->strWndText(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_SelFld_Select));
        m_pwndByName->SetWndStyle(tCIDCtrls::EWndStyles::Group, kCIDLib::True);
        m_pwndByName->SetDefButton();
        m_pwndByName->SetId(kCIDCtrls::widOK);
    }
     else
    {
        m_pwndByValue->pnothRegisterHandler(this, &TCQCSelFldDlg::eClickHandler);
    }

    //
    //  Now load fields for the driver we got, and possibly select the incoming field
    //  if it proved viable.
    //
    LoadFldsFor(strSelDrv, strSelFld);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCSelFldDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TCQCSelFldDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_SelFld_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_SelFld_Cancel);
    }
     else if ((wnotEvent.widSource() == kCQCIGKit::ridDlg_SelFld_ByName)
          ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_SelFld_ByValue)
          ||  (wnotEvent.widSource() == kCIDCtrls::widOK))
    {
        // Rmemeber which we are doing
        const tCIDLib::TBoolean bByValue
        (
            wnotEvent.widSource() == kCQCIGKit::ridDlg_SelFld_ByValue
        );

        // If we have something in both lists, we are ok
        const tCIDLib::TCard4 c4DrvIndex = m_pwndDrivers->c4CurItem();
        const tCIDLib::TCard4 c4FldIndex = m_pwndFields->c4CurItem();

        if ((c4DrvIndex != kCIDLib::c4MaxCard)
        &&  (c4FldIndex != kCIDLib::c4MaxCard))
        {
            m_pwndDrivers->QueryColText(c4DrvIndex, 0, m_strDriver);
            m_pwndFields->QueryColText(c4FldIndex, 0, m_strField);

            //
            //  If they are doing it by value, make sure that this field is readable. If
            //  not, then warn them.
            //
            if (bByValue)
            {
                tCIDLib::TCard4 c4Id;
                const TCQCFldDef* pflddTest
                (
                    m_pcfcFldInfo->pflddFor(m_strDriver, m_strField, c4Id)
                );

                if (!pflddTest)
                {
                    TErrBox msgbNotFnd
                    (
                        L"INTERNAL ERROR: The selected field was not found in the field cache."
                    );
                    msgbNotFnd.ShowIt(*this);
                    return tCIDCtrls::EEvResponses::Handled;
                }
                 else
                {
                    if (!tCIDLib::bAllBitsOn(pflddTest->eAccess(), tCQCKit::EFldAccess::Read))
                    {
                        TErrBox msgbNotR
                        (
                            facCQCIGKit().strMsg
                            (
                                kIGKitErrs::errcFldSel_NotReadable, m_strDriver, m_strField
                            )
                        );
                        msgbNotR.ShowIt(*this);
                        return tCIDCtrls::EEvResponses::Handled;
                    }
                }
            }
            EndDlg(wnotEvent.widSource());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// We only register the drivers list box, so we know the events are from it
tCIDCtrls::EEvResponses
TCQCSelFldDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    //
    //  We only register the drivers list. If we get a cursored event, then
    //  update the field list with the fields for this driver.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Get a copy of the current driver, since we are going to reload that list
        TString strMoniker;
        m_pwndDrivers->QueryColText(wnotEvent.c4Index(), 0, strMoniker);
        LoadFldsFor(strMoniker, TString::strEmpty());
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        m_pwndFields->RemoveAll();
    }
    return tCIDCtrls::EEvResponses::Handled;
}



tCIDLib::TVoid
TCQCSelFldDlg::LoadFldsFor(const TString& strMon, const TString& strFldSel)
{
    // Get a sorted list for the passed mon
    TFICacheDrvItem::TRefFldList colFlds(tCIDLib::EAdoptOpts::NoAdopt);
    m_pcfcFldInfo->QueryFieldsSorted(strMon, colFlds);

    // Remove existing content
    m_pwndFields->RemoveAll();

    // Stop drawing while we load it
    TWndPaintJanitor janPaint(m_pwndFields);

    // Load them up and find the passed initial field to select if any
    const tCIDLib::TCard4 c4FldCount = colFlds.c4ElemCount();
    if (c4FldCount)
    {
        tCIDLib::TCard4 c4InitSel = kCIDLib::c4MaxCard;
        tCIDLib::TStrList colCols(1);
        colCols.objAdd(TString::strEmpty());
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FldCount; c4Index++)
        {
            colCols[0] = colFlds[c4Index]->strName();
            m_pwndFields->c4AddItem(colCols, c4Index);

            if (colCols[0].bCompareI(strFldSel))
                c4InitSel = c4Index;
        }

        if (c4InitSel == kCIDLib::c4MaxCard)
            m_pwndFields->SelectByIndex(0);
        else
            m_pwndFields->c4SelectById(c4InitSel);
    }
}


