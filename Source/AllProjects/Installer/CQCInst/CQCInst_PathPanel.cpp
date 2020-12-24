//
// FILE NAME: CQCInst_PathPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2004
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
//  This file implements the info panel that gets the installation path from
//  the user.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInst.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TInstPathPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstPathPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstPathPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstPathPanel::TInstPathPanel( TCQCInstallInfo* const  pinfoCur
                                , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Installation Path", pinfoCur, pwndParent)
    , m_pwndBrowse(nullptr)
    , m_pwndPath(nullptr)
{
}

TInstPathPanel::~TInstPathPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstPathPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstPathPanel::bPanelIsVisible() const
{
    // If not using previous options
    return !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstPathPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // Any failure from here and we want to put the focus on the entry field
    widFailed = kCQCInst::ridPan_Path_Target;

    // Make sure that they entered a path
    TPathStr pathTar = infoCur().strPath();
    if (pathTar.bIsEmpty())
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NoInstDir, facCQCInst);
        return kCIDLib::False;
    }

    // Check for a UNC type name and tell them that they cannot use one
    if (pathTar.bStartsWith(L"\\\\"))
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NoUNCPath, facCQCInst);
        return kCIDLib::False;
    }

    // If it is in the form X: that's not legal so reject it now
    if ((pathTar.c4Length() == 2) && (pathTar[1] == L':'))
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_BadPathFmt, facCQCInst);
        return kCIDLib::False;
    }

    // Make sure it's fully qualified
    if (!pathTar.bIsFullyQualified())
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NotFQInstPath, facCQCInst);
        return kCIDLib::False;
    }

    // Normalize the path now and put it back
    pathTar.Normalize();
    infoCur().strPath(pathTar);

    //
    //  If there is an old path stored (meaning that we found an old version
    //  to upgrade), but the new path isn't the same, make sure that they
    //  really want to do this.
    //
    if (infoCur().bFoundOldVer()
    &&  (infoCur().strOldPath() != infoCur().strPath()))
    {
        TYesNoBox msgbUpgradeQ(facCQCInst.strMsg(kCQCInstMsgs::midQ_DontUpgrade));
        if (!msgbUpgradeQ.bShowIt(*this))
        {
            strErrText.LoadFromMsg(kCQCInstErrs::errcVal_ChangePath, facCQCInst);
            return kCIDLib::False;
        }

        // They do, so change the install type
        infoCur().eTargetType(tCQCInst::ETargetTypes::Clean);
    }

    // Check the target type
    if (infoCur().eTargetType() == tCQCInst::ETargetTypes::Upgrade)
    {
        //
        //  Looks like there's a legal previous version there, so ask
        //  them if its ok to upgrade it.
        //
        TString strVer;
        strVer.AppendFormatted(infoCur().viiOldInfo().c4MajVersion());
        strVer.Append(kCIDLib::chPeriod);
        strVer.AppendFormatted(infoCur().viiOldInfo().c4MinVersion());
        strVer.Append(kCIDLib::chPeriod);
        strVer.AppendFormatted(infoCur().viiOldInfo().c4Revision());

        TYesNoBox msgbUpgradeQ(facCQCInst.strMsg(kCQCInstMsgs::midQ_Upgrade, strVer));
        if (!msgbUpgradeQ.bShowIt(*this))
        {
            strErrText.LoadFromMsg(kCQCInstErrs::errcVal_ChangeTarget, facCQCInst);
            return kCIDLib::False;
        }
    }

    //
    //  Make sure the path exists or can be created. If it exists, make
    //  sure it's not the directory of the installer itself.
    //
    if (TFileSys::bExists(infoCur().strPath(), tCIDLib::EDirSearchFlags::NormalDirs))
    {
        if (infoCur().strPath() == facCQCInst.strPath())
        {
            strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NotInInstDir, facCQCInst);
            return kCIDLib::False;
        }
    }
     else
    {
        // See if they want to create it
        TYesNoBox msgbAsk
        (
            facCQCInst.strMsg(kCQCInstMsgs::midQ_CreatePath, infoCur().strPath())
        );

        if (!msgbAsk.bShowIt(*this))
        {
            strErrText.LoadFromMsg(kCQCInstErrs::errcVal_ChangeTarget, facCQCInst);
            return kCIDLib::False;
        }

        //
        //  They want to so, create all the path components. Catch any error
        //  and just tell them it couldn't be created.
        //
        try
        {
            TFileSys::MakePath(infoCur().strPath());
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                facCQCInst.LogEventObj(errToCatch);
            }

            strErrText.LoadFromMsg(kCQCInstErrs::errcVal_CantMakePath, facCQCInst);
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TInstPathPanel::Entered()
{
    // Load up the current path text
    m_pwndPath->strWndText(infoCur().strPath());
}


tCIDLib::TVoid TInstPathPanel::SaveContents()
{
    //
    //  Get the path text out and store it. We never want it to end in a
    //  separator, so remove any trailing separators from it before storing
    //  it.
    //
    TString strTmp(m_pwndPath->strWndText());
    strTmp.Strip(L"\\", tCIDLib::EStripModes::Trailing);
    infoCur().strPath(strTmp);
}



// ---------------------------------------------------------------------------
//  TInstPathPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstPathPanel::bCreated()
{
    TParent::bCreated();

    // Load our controls
    LoadDlgItems(kCQCInst::ridPan_Path);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Path_Browse, m_pwndBrowse);
    CastChildWnd(*this, kCQCInst::ridPan_Path_Target, m_pwndPath);

    // Register a click handler for our browse button
    m_pwndBrowse->pnothRegisterHandler(this, &TInstPathPanel::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstPathPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstPathPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_Path_Browse)
    {
        // Set up the dialog title string
        TString strTitle(kCQCInstMsgs::midPan_Path_BrowseTarTitle, facCQCInst);

        //
        //  Get the current contents of the entry field. Check it to see
        //  its a legal directory. If so, then keep it and use it as the
        //  initial browse path. Else, toss it and clear the initial string.
        //
        TString strCurPath(m_pwndPath->strWndText());

        if (strCurPath.chLast() == L'\\')
            strCurPath.DeleteLast();
        if (!TFileSys::bIsDirectory(strCurPath))
            strCurPath.Clear();

        tCIDLib::TKVPList colFileTypes;
        tCIDLib::TStrList colSel(1);
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , strTitle
            , strCurPath
            , colSel
            , colFileTypes
            , tCIDLib::eOREnumBits
              (
                tCIDCtrls::EFOpenOpts::SelectFolders
                , tCIDCtrls::EFOpenOpts::PathMustExist
                , tCIDCtrls::EFOpenOpts::FileSystemOnly
              )
        );

        if (bRes)
            m_pwndPath->strWndText(colSel[0]);
    }

    return tCIDCtrls::EEvResponses::Handled;
}
