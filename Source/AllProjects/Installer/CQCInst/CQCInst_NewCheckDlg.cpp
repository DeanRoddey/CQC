//
// FILE NAME: CQCInst_NewCheckDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/04/2002
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
//  This file implements the new install check dialog.
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
#include    "CQCInst.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCNewInstCheckDlg, TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TCQCNewInstCheckDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCNewInstCheckDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCNewInstCheckDlg::TCQCNewInstCheckDlg() :

    m_pwndCancelButton(nullptr)
    , m_pwndFindButton(nullptr)
    , m_pwndNewButton(nullptr)
{
}

TCQCNewInstCheckDlg::~TCQCNewInstCheckDlg()
{
}


// ---------------------------------------------------------------------------
//  TCQCNewInstCheckDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCInst::ENewInstRes
TCQCNewInstCheckDlg::eRunDlg(TWindow&               wndOwner
                            , TString&              strToFill
                            , TCQCVerInstallInfo&   viiToFill)
{
    // And run the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCInst, kCQCInst::ridDlg_NewCheck
    );

    // If they canceled return that enum value
    if (!c4Res || (c4Res == kCIDLib::c4MaxCard))
        return tCQCInst::ENewInstRes::Cancel;

    // If they found an old version for us, return the path
    if (c4Res == kCQCInst::ridDlg_NewCheck_Find)
    {
        strToFill = m_strPathFound;
        viiToFill = m_viiFound;
        return tCQCInst::ENewInstRes::Found;
    }
     else if (c4Res == kCQCInst::ridDlg_NewCheck_New)
    {
        return tCQCInst::ENewInstRes::New;
    }

    // Not found or new, so assume cancel
    return tCQCInst::ENewInstRes::Cancel;
}


// ---------------------------------------------------------------------------
//  TCQCNewInstCheckDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCNewInstCheckDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    //
    //  Store pre-typed child widget pointers for the widgets that are in the
    //  base config dialog and won't come and go.
    //
    CastChildWnd(*this, kCQCInst::ridDlg_NewCheck_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCInst::ridDlg_NewCheck_Find, m_pwndFindButton);
    CastChildWnd(*this, kCQCInst::ridDlg_NewCheck_New, m_pwndNewButton);

    // And register our button event handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TCQCNewInstCheckDlg::eClickHandler);
    m_pwndFindButton->pnothRegisterHandler(this, &TCQCNewInstCheckDlg::eClickHandler);
    m_pwndNewButton->pnothRegisterHandler(this, &TCQCNewInstCheckDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCNewInstCheckDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCNewInstCheckDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridDlg_NewCheck_Cancel)
    {
        EndDlg(kCQCInst::ridDlg_NewCheck_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCInst::ridDlg_NewCheck_Find)
    {
        //
        //  Pop up the file selection dialog and let them select a directory for us. We
        //  set it to look for the version info file, which is easier to correctly find
        //  than the directory, and makes it more likely we found the right place.
        //
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"CQC Version Info", L"CQCInstVersion.Info"));
        tCIDLib::TStrList colSel(1);
        TString strTitle(kCQCInstMsgs::midDlg_NewCheck_SelectTitle, facCQCInst);
        const tCIDLib::TBoolean bFound = facCIDCtrls().bOpenFileDlg
        (
            *this
            , strTitle
            , m_strPathFound
            , colSel
            , colFileTypes
            , tCIDLib::eOREnumBits
              (
                tCIDCtrls::EFOpenOpts::StrictTypes
                , tCIDCtrls::EFOpenOpts::FileMustExist
                , tCIDCtrls::EFOpenOpts::FileSystemOnly
              )
        );

        if (bFound)
        {
            // Keep the path part of the selected file name for next time
            TPathStr pathSelected = colSel[0];
            pathSelected.bRemoveNameExt();
            pathSelected.bRemoveTrailingSeparator();
            m_strPathFound = pathSelected;

            // Check this target. If it's good, exit with this path
            if (facCQCInst.bCheckTarget(*this, m_strPathFound, m_viiFound))
            {
                EndDlg(kCQCInst::ridDlg_NewCheck_Find);
            }
             else
            {
                // Tell them we couldn't find anything
                TErrBox msgbNoInfo(facCQCInst.strMsg(kCQCInstMsgs::midStatus_NoVerFound));
                msgbNoInfo.ShowIt(*this);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCInst::ridDlg_NewCheck_New)
    {
        EndDlg(kCQCInst::ridDlg_NewCheck_New);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



