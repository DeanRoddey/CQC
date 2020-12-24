//
// FILE NAME: ZWaveUSB3C_SelUTypeDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/18/2018
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
//  This file implements a small dialog that lets the user select the device info
//  file for a unit.
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
#include    "ZWaveUSB3C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWUSB3SelUTypeDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3SelUTypeDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3SelUTypeDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWUSB3SelUTypeDlg::TZWUSB3SelUTypeDlg() :

    m_pwndAccept(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndInstruct(nullptr)
    , m_pwndMakes(nullptr)
    , m_pwndModels(nullptr)
{
}

TZWUSB3SelUTypeDlg::~TZWUSB3SelUTypeDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3SelUTypeDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TZWUSB3SelUTypeDlg::bRunDlg(        TZWaveUSB3CWnd&     wndDriver
                            ,       tCIDLib::TCard8&    c8ManIds
                            , const TString&            strUnitName)
{
    // Save the info for later use
    m_c8ManIds = c8ManIds;
    m_pwndDriver = &wndDriver;
    m_strUnitName = strUnitName;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndDriver, facZWaveUSB3C(), kZWaveUSB3C::ridDlg_SelUType
    );

    if (c4Res == kZWaveUSB3C::ridDlg_SelUType_Accept)
    {
        c8ManIds = m_c8ManIds;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TZWUSB3SelUTypeDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3SelUTypeDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_SelUType_Accept, m_pwndAccept);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_SelUType_Cancel, m_pwndCancel);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_SelUType_Instruct, m_pwndInstruct);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_SelUType_Makes, m_pwndMakes);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_SelUType_Models, m_pwndModels);

    // And register our button event handlers
    m_pwndAccept->pnothRegisterHandler(this, &TZWUSB3SelUTypeDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TZWUSB3SelUTypeDlg::eClickHandler);
    m_pwndMakes->pnothRegisterHandler(this, &TZWUSB3SelUTypeDlg::eLBHandler);
    m_pwndModels->pnothRegisterHandler(this, &TZWUSB3SelUTypeDlg::eLBHandler);

    // Set up the columns for the models list
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"Model Name");
    colCols.objAdd(L"Model Number");
    m_pwndModels->SetColumns(colCols);

    //
    //  Update the title text to include the unit name. Save the original text for
    //  use in popups.
    //
    m_strTitle = strWndText();
    TString strTitle = m_strTitle;
    strTitle += L" [";
    strTitle += m_strUnitName;
    strTitle += L"]";
    strWndText(strTitle);

    //
    //  Load up the makes list. If it works, and there are incoming man ids, see if
    //  we can select that one again. If not, just select the first make, which will
    //  load the model list.
    //
    tCIDLib::TStrList colMakes;
    if (!facZWaveUSB3Sh().bQueryMakes(colMakes))
    {
        TErrBox msgbErr
        (
            m_strTitle, L"The device makes list could not be loaded, see the logs"
        );
        msgbErr.ShowIt(*this);
    }
     else
    {
        const tCIDLib::TCard4 c4Count = colMakes.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_pwndMakes->c4AddItem(colMakes[c4Index]);

        // If there's incoming man ids, see if we can select that one
        TString strInitMake;
        TString strInitModel;
        if (m_c8ManIds)
        {
            TString strClass;
            facZWaveUSB3Sh().bQueryDevInfo
            (
                m_c8ManIds, strInitMake, strInitModel, strClass
            );
        }

        tCIDLib::TCard4 c4At = 0;
        if (m_pwndMakes->bSelectByText(strInitMake, c4At))
        {
            // See if we can select the initial model, else the 0th one if any
            if (!m_pwndModels->bSelectByText(strInitModel, c4At))
            {
                // If we got any, select the 0th one
                if (m_pwndModels->c4ItemCount())
                    m_pwndModels->SelectByIndex(0);
            }
        }
         else
        {
            //
            //  Can't do it, so just take the 0th one, forcing an event, which will
            //  load the models for that make.
            //
            if (m_pwndMakes->c4ItemCount())
                m_pwndMakes->SelectByIndex(0, kCIDLib::True);
        }
    }
    return bRet;
}


// ---------------------------------------------------------------------------
//  TZWUSB3SelUTypeDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handle button presses
tCIDCtrls::EEvResponses TZWUSB3SelUTypeDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_SelUType_Accept)
    {
        // Get the selected make and model. If either isn't available, we can't do it
        tCIDLib::TCard4 c4MakeInd = m_pwndMakes->c4CurItem();
        tCIDLib::TCard4 c4ModelInd = m_pwndModels->c4CurItem();

        if ((c4MakeInd == kCIDLib::c4MaxCard) || (c4ModelInd == kCIDLib::c4MaxCard))
        {
            TErrBox msgbErr(m_strTitle, L"You must select a make and model");
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        //
        //  Get the user data from the model, which is the man ids we stored there
        //  when we loaded the list.
        //
        m_c8ManIds = m_pwndModels->c8UserDataAt(c4ModelInd);
        EndDlg(kZWaveUSB3C::ridDlg_SelUType_Accept);
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_SelUType_Cancel)
    {
        EndDlg(kZWaveUSB3C::ridDlg_SelUType_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  If the make selection changes we grab the models for that make and load them
//  up. We get back the descriptive name, the actual model, and the man ids for
//  each model under the selected make.
//
//  We store the man ids as user data on the row. We also store the instruction text
//  as user data, so that we can access both quickly.
//
tCIDCtrls::EEvResponses TZWUSB3SelUTypeDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_SelUType_Makes)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Get the selected make
            TString strMake;
            m_pwndMakes->ValueAt(wnotEvent.c4Index(), strMake);

            // And query the models for that make
            tCIDLib::TStrList colCols;
            colCols.AddXCopies(TString::strEmpty(), 2);

            tCIDLib::TKValsList colModels;
            if (facZWaveUSB3Sh().bQueryModels(strMake, colModels))
            {
                TWndPaintJanitor janDraw(m_pwndModels);
                m_pwndModels->RemoveAll();

                const tCIDLib::TCard4 c4Count = colModels.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    const TKeyValues& kvalsCur = colModels[c4Index];
                    colCols[0] = kvalsCur.strKey();
                    colCols[1] = kvalsCur.strVal1();
                    const tCIDLib::TCard4 c4At = m_pwndModels->c4AddItem
                    (
                        colCols, 0, kvalsCur.strVal3()
                    );

                    // Store the man ids as the user data for this row
                    m_pwndModels->SetUserDataAt
                    (
                        c4At, kvalsCur.strVal2().c8Val(tCIDLib::ERadices::Hex)
                    );
                }
            }
        }
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_SelUType_Models)
    {
        // Update the instruction text when a new model is selected
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndInstruct->ClearText();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            m_pwndInstruct->strWndText(m_pwndModels->strUserDataAt(wnotEvent.c4Index()));
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}

