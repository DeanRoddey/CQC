//
// FILE NAME: CQCAdmin_TrigEvTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/07/2015
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
//  This file implements a tab for editing triggered events
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_TrigEvTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TTrigEvTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TTrigEvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTrigEvTab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TTrigEvTab::TTrigEvTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Trig Event", kCIDLib::True)
    , m_pwndEditor(nullptr)
{
}

TTrigEvTab::~TTrigEvTab()
{
}


// ---------------------------------------------------------------------------
//  TTrigEvTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TTrigEvTab::CreateTab(          TTabbedWnd&         wndParent
                        , const TString&            strTabText
                        , const TCQCTrgEvent&       csrcEdit)
{
    // Store away the data till we can get it to the editor window
    m_csrcEdit = csrcEdit;

    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


tCIDLib::TVoid TTrigEvTab::UpdatePauseState(const tCIDLib::TBoolean bToSet)
{
    m_csrcEdit.bPaused(bToSet);
}


// ---------------------------------------------------------------------------
//  TTrigEvTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TTrigEvTab::AreaChanged(const   TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If the size changed, and we have the editor created, then resize it.
    if (bSizeChanged && m_pwndEditor && (ePosState != tCIDCtrls::EPosStates::Minimized))
        m_pwndEditor->SetSize(areaClient().szArea(), kCIDLib::False);
}


tCIDLib::TBoolean TTrigEvTab::bCreated()
{
    TParent::bCreated();

    // Create our editor window, tell hime he is not in dialog mode, but in tab mode
    m_pwndEditor = new TEditTrigEvWnd(kCIDLib::False);
    m_pwndEditor->CreateTEvEd
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , m_csrcEdit
        , nullptr
        , tCIDCtrls::EWndThemes::MainWnd
        , facCQCAdmin.cuctxToUse()
    );

    //  Set us to use the standard window background color.
    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    return kCIDLib::True;
}


tCIDLib::ESaveRes
TTrigEvTab::eDoSave(        TString&                strErrMsg
                    , const tCQCAdmin::ESaveModes   eMode
                    ,       tCIDLib::TBoolean&      bChangesSaved)
{
    const tCIDLib::ESaveRes eRes = m_pwndEditor->eValidate(strErrMsg);
    if (eMode == tCQCAdmin::ESaveModes::Test)
        return eRes;

    // If there were changes, so write our current editor contents out
	if (eRes == tCIDLib::ESaveRes::OK)
    {
		try
		{
			TDataSrvClient dsclToUse;

			// The id member is used to store the path
			tCIDLib::TCard4 c4SerialNum = 0;
			tCIDLib::TEncodedTime enctLastChange;
			tCIDLib::TKVPFList colXMeta;
			dsclToUse.WriteTriggeredEvent
			(
				strRelPath()
				, c4SerialNum
				, enctLastChange
				, m_csrcEdit
				, kCQCRemBrws::c4Flag_OverwriteOK
				, colXMeta
				, facCQCAdmin.sectUser()
			);

			// Tell the editor to commit these changes
			m_pwndEditor->CommitChanges();
			bChangesSaved = kCIDLib::True;
		}

		catch (TError& errToCatch)
		{
			errToCatch.AddStackLevel(CID_FILE, CID_LINE);
			TModule::LogEventObj(errToCatch);

			strErrMsg = errToCatch.strErrText();
			return tCIDLib::ESaveRes::Errors;
		}
	}

    // No changes anymore since we saved any that there were
    return tCIDLib::ESaveRes::NoChanges;
}

