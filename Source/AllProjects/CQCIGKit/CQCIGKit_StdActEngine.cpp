//
// FILE NAME: CQCIGKit_StdActEngine.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/18/2009
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
//  This file implements a derivative of the standard action engine that is
//  suitable for non-interfce engine type GUI based action invocation.
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


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCStdGUIActEngine, TCQCStdActEngine)



// ---------------------------------------------------------------------------
//   CLASS: TCQCStdGUIActEngine
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCStdGUIActEngine: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCStdGUIActEngine::TCQCStdGUIActEngine(const  TWindow* const  pwndOwner
                                        ,       TWindow* const  pwndModal
                                        , const TString&        strTitle
                                        , const TCQCUserCtx&    cuctxToUse) :

    TCQCStdActEngine(cuctxToUse)
    , m_pwndModal(pwndModal)
    , m_pwndOwner(pwndOwner)
    , m_strTitle(strTitle)
{
}

TCQCStdGUIActEngine::~TCQCStdGUIActEngine()
{
}


// ---------------------------------------------------------------------------
//  TCQCStdGUIActEngine: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCStdGUIActEngine::CmdInvokeErr(  const   TError&         errToCatch
                                    , const tCIDLib::TCard4 c4Index)
{
    TExceptDlg dlgErr
    (
        *m_pwndOwner
        , m_strTitle
        , facCQCKit().strMsg(kKitErrs::errcCmd_CmdFailed, TCardinal(c4Index))
        , errToCatch
    );
}


tCIDLib::TVoid TCQCStdGUIActEngine::WaitEnd(tCIDLib::TBoolean& bDoneFlag)
{
    //
    //  Just enter a modal loop until the background thread doing the work
    //  sets the done flag.
    //
    m_pwndModal->eDoModalLoop
    (
        bDoneFlag
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EModalFlags::NoClicks
            , tCIDCtrls::EModalFlags::NoInput
            , tCIDCtrls::EModalFlags::NoKeys
          )
        , 0
        , kCIDLib::c4MaxCard
    );
}

