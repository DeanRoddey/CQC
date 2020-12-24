//
// FILE NAME: CQCRPortSrv_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
//  This file implements the facilty class for the remote server facility.
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
#include    "CQCRPortSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCRPortSrv,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCRPortSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCRPortSrv: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCRPortSrv::TFacCQCRPortSrv() :

    TGUIFacility
    (
        L"CQCRPortSrv"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_colPorts(tCIDLib::EAdoptOpts::Adopt, 8, tCIDLib::EMTStates::Safe)
    , m_strTitle(L"CQC Remote Port Server")
{
    // Iterate the local ports and load up an item for each of them, initially now allowed
    TVector<TString> colLocals;
    facCIDComm().bQueryPorts(colLocals, kCIDLib::True, kCIDLib::False);
    const tCIDLib::TCard4 c4Count = colLocals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colPorts.Add(new TCQCSerPortInfo(colLocals[c4Index]));
}

TFacCQCRPortSrv::~TFacCQCRPortSrv()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCRPortSrv: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Returns a list of the remote paths of the allowed ports
tCIDLib::TBoolean TFacCQCRPortSrv::bQueryCommPorts(TVector<TString>& colToFill)
{
    // Lock the port list while we do this
    TLocker lockrSync(&m_colPorts);

    const tCIDLib::TCard4 c4Count = m_colPorts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCSerPortInfo& spiCur = *m_colPorts[c4Index];
        if (spiCur.bAllow())
            colToFill.objAdd(spiCur.strRemotePath());
    }

    // Return true if we put any in the list
    return !colToFill.bIsEmpty();
}


//
//  This will close any open local ports. The main window calls this when
//  the user disables access, since the client cannot then close them on
//  his own.
//
tCIDLib::TVoid TFacCQCRPortSrv::CloseAllPorts()
{
    // Lock the port list while we do this
    TLocker lockrSync(&m_colPorts);

    const tCIDLib::TCard4 c4Count = m_colPorts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCSerPortInfo* pspiCur = m_colPorts[c4Index];
        pspiCur->pcommPort()->Close();
    }
}


tCIDLib::EExitCodes TFacCQCRPortSrv::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    try
    {
        // Now create the main frame window
        m_wndMainFrame.Create();
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            TWindow::wndDesktop(), m_strTitle, L"Port server init failed", errToCatch
        );
        return tCIDLib::EExitCodes::InitFailed;
    }

    catch(...)
    {
        TErrBox msgbErr(m_strTitle, L"Port server init failed");
        msgbErr.ShowIt(TWindow::wndDesktop());
        return tCIDLib::EExitCodes::InitFailed;
    }

    // Process messages until we are told to exit
    facCIDCtrls().uMainMsgLoop(m_wndMainFrame);

    try
    {
        // First clean up the frame window
        m_wndMainFrame.Destroy();

        // And now we can terminate the ORB, just in case
        facCIDOrb().Terminate();
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            TWindow::wndDesktop(), m_strTitle, L"Termination failed", errToCatch
        );
    }

    catch(...)
    {
        TErrBox msgbErr(m_strTitle, L"Termination failed");
        msgbErr.ShowIt(TWindow::wndDesktop());
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  Load up the list with all of the available ports, checking those that are currently
//  marked as allowed. We set the cookie as the row id, so that the main window can know
//  the cookies for each one.
//
tCIDLib::TVoid TFacCQCRPortSrv::LoadList(TCheckedMCListWnd& wndPortList)
{
    tCIDLib::TStrList colCols;
    colCols.AddXCopies(TString::strEmpty(), 2);

    const tCIDLib::TCard4 c4Count = m_colPorts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCSerPortInfo& spiCur = *m_colPorts[c4Index];

        // Zeroth column doesn't matter
        colCols[1] = spiCur.strLocalPath();
        wndPortList.c4AddItem(colCols, spiCur.c4Cookie());
    }
}



TCQCSerPortInfo* TFacCQCRPortSrv::pspiFind(const tCIDLib::TCard4 c4Cookie)
{
    // Lock the port list while we do this
    TLocker lockrSync(&m_colPorts);

    // Go through our list and find the one with the indicated cookie
    TCQCSerPortInfo* pspiRet = 0;
    const tCIDLib::TCard4 c4Count = m_colPorts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCSerPortInfo* pspiCur = m_colPorts[c4Index];
        if (pspiCur->c4Cookie() == c4Cookie)
        {
            pspiRet = pspiCur;
            break;
        }
    }

    if (!pspiRet)
    {
        facCQCRPortSrv().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRPortSErrs::errcSPort_BadCookie
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }
    return pspiRet;
}


TCQCSerPortInfo* TFacCQCRPortSrv::pspiFindByPath(const TString& strPath)
{
    // Lock the port list while we do this
    TLocker lockrSync(&m_colPorts);

    // Go through our list and find the one with the indicated path
    TCQCSerPortInfo* pspiRet = 0;
    const tCIDLib::TCard4 c4Count = m_colPorts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCSerPortInfo* pspiCur = m_colPorts[c4Index];
        if (pspiCur->strRemotePath() == strPath)
        {
            pspiRet = pspiCur;
            break;
        }
    }

    if (!pspiRet)
    {
        facCQCRPortSrv().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRPortSErrs::errcSPort_BadPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strPath
        );
    }
    return pspiRet;
}


const TString& TFacCQCRPortSrv::strTitle() const
{
    return m_strTitle;
}


//
//  The main window tells us which ports the user as enabled, via cookie. We mark them
//  in our list accordingly.
//
tCIDLib::TVoid TFacCQCRPortSrv::SetAllowedPorts(const tCIDLib::TCardList& fcolCookies)
{
    const tCIDLib::TCard4 c4Count = m_colPorts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCSerPortInfo& spiCur = *m_colPorts[c4Index];
        spiCur.bAllow(fcolCookies.bElementPresent(spiCur.c4Cookie()));
    }
}


