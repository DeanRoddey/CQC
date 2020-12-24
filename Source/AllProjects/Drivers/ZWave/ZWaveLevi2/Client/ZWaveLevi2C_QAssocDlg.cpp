//
// FILE NAME: ZWaveLevi2C_QAssocDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/11/2014
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
//  This file implements a small dialog that lets the user query group associations.
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
#include    "ZWaveLevi2C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveQAssocDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveQAssocDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveQAssocDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveQAssocDlg::TZWaveQAssocDlg() :

    m_pdcfgCur(nullptr)
    , m_punitSrc(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndGrpNum(nullptr)
    , m_pwndOutput(nullptr)
    , m_pwndQuery(nullptr)
{
}

TZWaveQAssocDlg::~TZWaveQAssocDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWaveQAssocDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWaveQAssocDlg::RunDlg(const   TWindow&            wndOwner
                        , const TString&            strMoniker
                        , const TZWaveUnit&         unitSrc
                        , const TZWaveDrvConfig&    dcfgCur)

{
    m_pdcfgCur = &dcfgCur;
    m_punitSrc = &unitSrc;
    m_strMoniker = strMoniker;

    c4RunDlg(wndOwner, facZWaveLevi2C(), kZWaveLevi2C::ridDlg_QAssoc);
}


// ---------------------------------------------------------------------------
//  TZWaveQAssocDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWaveQAssocDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_QAssoc_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_QAssoc_GrpNum, m_pwndGrpNum);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_QAssoc_Output, m_pwndOutput);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_QAssoc_Query, m_pwndQuery);

    // And register our button event handlers
    m_pwndQuery->pnothRegisterHandler(this, &TZWaveQAssocDlg::eClickHandler);
    m_pwndClose->pnothRegisterHandler(this, &TZWaveQAssocDlg::eClickHandler);

    // Set up the group # spin box
    m_pwndGrpNum->SetNumRange(1, 255, 1);

    return bRet;
}



// ---------------------------------------------------------------------------
//  TZWaveQAssocDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TZWaveQAssocDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_QAssoc_Close)
    {
        EndDlg(kZWaveLevi2C::ridDlg_QAssoc_Close);
    }
     else if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_QAssoc_Query)
    {
        try
        {
            tCQCKit::TCQCSrvProxy orbcTarget;

            // Try to get a client proxy
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            orbcTarget = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);

            // We got it, so get out the values to send and format the command
            const tCIDLib::TCard4 c4GrpNum = tCIDLib::TCard4(m_pwndGrpNum->i4CurValue() + 1);

            TString strDataVal;
            strDataVal.Append(m_punitSrc->strName());
            strDataVal.Append(L'/');
            strDataVal.AppendFormatted(c4GrpNum);

            TString strRes;
            tCIDLib::TBoolean bRes = orbcTarget->bQueryTextVal
            (
                m_strMoniker
                , kZWaveLevi2Sh::strQTextV_QueryAssoc
                , strDataVal
                , strRes
            );

            if (bRes)
            {
                // Parse out the list of unit ids
                tCIDLib::TCardList fcolList;

                TStringTokenizer stokUnits(&strRes, L",");
                TString strOutput(L"The linked target units are:\n\n");
                TString strId;
                tCIDLib::TCard4 c4Id;
                while (stokUnits.bGetNextToken(strId))
                {
                    // See if we have it named and display that, else the id
                    if (strId.bToCard4(c4Id, tCIDLib::ERadices::Dec))
                    {
                        strOutput.Append(L"   ");
                        const TZWaveUnit* punitCur = m_pdcfgCur->punitFindById(c4Id);
                        if (punitCur)
                        {
                            strOutput.Append(punitCur->strName());
                        }
                         else
                        {
                            strOutput.Append(L"[");
                            strOutput.AppendFormatted(c4Id);
                            strOutput.Append(L"]");
                        }
                    }
                     else
                    {
                        strOutput.Append(L"   [Invalid node id returned");
                    }
                    strOutput.Append(L"\n");
                }

                m_pwndOutput->strWndText(strOutput);
            }
             else
            {
                TOkBox msgbOK
                (
                    facZWaveLevi2C().strMsg(kZW2CMsgs::midGen_Title)
                    , facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_QAssocFailed, strRes)
                );
                msgbOK.ShowIt(*this);
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgbFail
            (
                *this
                , strWndText()
                , facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_RemOpFailed)
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



