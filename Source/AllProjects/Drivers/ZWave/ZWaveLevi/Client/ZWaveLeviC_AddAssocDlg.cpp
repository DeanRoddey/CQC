//
// FILE NAME: ZWaveLeviC_AddAssocDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/16/2012
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
//  This file implements a small dialog that lets the user set inter-unit
//  associations.
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
#include    "ZWaveLeviC_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveAddAssocDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveAddAssocDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveAddAssocDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveAddAssocDlg::TZWaveAddAssocDlg() :

    m_pdcfgUnits(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndGrpNum(nullptr)
    , m_pwndSet(nullptr)
    , m_pwndSrcUnit(nullptr)
    , m_pwndTarUnit(nullptr)
{
}

TZWaveAddAssocDlg::~TZWaveAddAssocDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWaveAddAssocDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWaveAddAssocDlg::Run( const   TWindow&            wndOwner
                        , const TString&            strMoniker
                        , const TZWaveDrvConfig&    dcfgUnits)
{
    // Save a pointer to the configuration info for later
    m_pdcfgUnits = &dcfgUnits;
    m_strMoniker = strMoniker;

    c4RunDlg(wndOwner, facZWaveLeviC(), kZWaveLeviC::ridDlg_AddAssoc);
}


// ---------------------------------------------------------------------------
//  TZWaveAddAssocDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWaveAddAssocDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveLeviC::ridDlg_AddAssoc_Set, m_pwndSet);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_AddAssoc_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_AddAssoc_GrpNum, m_pwndGrpNum);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_AddAssoc_SrcUnit, m_pwndSrcUnit);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_AddAssoc_TarUnit, m_pwndTarUnit);

    // And register our button event handlers
    m_pwndSet->pnothRegisterHandler(this, &TZWaveAddAssocDlg::eClickHandler);
    m_pwndClose->pnothRegisterHandler(this, &TZWaveAddAssocDlg::eClickHandler);

    // Load up the available source and target units list boxs
    tCIDLib::TStrList colNames;
    const tCIDLib::TCard4 c4UnitCnt = m_pdcfgUnits->c4UnitCnt();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
        colNames.objAdd(m_pdcfgUnits->unitAt(c4Index).strName());
    m_pwndSrcUnit->SetEnumList(colNames);
    m_pwndTarUnit->SetEnumList(colNames);

    // Set up the group # spin box
    m_pwndGrpNum->SetNumRange(0, 254, 0);

    return bRet;
}



// ---------------------------------------------------------------------------
//  TZWaveAddAssocDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TZWaveAddAssocDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveLeviC::ridDlg_AddAssoc_Close)
    {
        EndDlg(kZWaveLeviC::ridDlg_AddAssoc_Close);
    }
     else if (wnotEvent.widSource() == kZWaveLeviC::ridDlg_AddAssoc_Set)
    {
        try
        {
            tCQCKit::TCQCSrvProxy orbcTarget;

            // Try to get a client proxy
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            orbcTarget = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);

            // We got it, so get out the values to send and format the command
            const tCIDLib::TCard4 c4GrpNum = tCIDLib::TCard4(m_pwndGrpNum->i4CurValue());


            //
            //  It's between two units, so regular association command
            //  where we pass target, source, and source group number.
            //
            //  Make sure that the source and target aren't the same. If the
            //  source is equal to the target, they are the same.
            //
            if (m_pwndSrcUnit->c4SpinIndex() == m_pwndTarUnit->c4SpinIndex())
            {
                TOkBox msgbErr
                (
                    facZWaveLeviC().strMsg(kZWCMsgs::midGen_Title)
                    , facZWaveLeviC().strMsg(kZWCMsgs::midStatus_SameUnit)
                );
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }

            TString strTar;
            m_pwndTarUnit->QueryCurText(strTar);
            TString strSrc;
            m_pwndSrcUnit->QueryCurText(strSrc);

            TString strCmd = kZWaveLeviSh::strDrvCmd_AddAssoc;
            strCmd.Append(kCIDLib::chSpace);
            strCmd.Append(strTar);
            strCmd.Append(kCIDLib::chSpace);
            strCmd.Append(strSrc);
            strCmd.Append(kCIDLib::chSpace);
            strCmd.AppendFormatted(c4GrpNum);

            orbcTarget->WriteStringByName
            (
                m_strMoniker
                , kZWaveLeviSh::strFld_Command
                , strCmd
                , tCQCKit::EDrvCmdWaits::DontCare
            );

            // No exception, so report it appeared to work
            TOkBox msgbOK
            (
                facZWaveLeviC().strMsg(kZWCMsgs::midGen_Title)
                , facZWaveLeviC().strMsg(kZWCMsgs::midStatus_CmdOpOK)
            );
            msgbOK.ShowIt(*this);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgbFail
            (
                *this
                , strWndText()
                , facZWaveLeviC().strMsg(kZWCMsgs::midStatus_RemOpFailed)
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


