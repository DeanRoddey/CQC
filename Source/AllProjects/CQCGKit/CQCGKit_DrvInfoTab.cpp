//
// FILE NAME: CQCGKit_DrvInfoTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/07/2016
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
//  This module implements a tab window that will display the information from a
//  driver configuration object in a multi-column list box.
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
#include    "CQCGKit_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCDrvInfoTab, TTabWindow)



// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvInfoTab
//  PREFIX: drvit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDrvInfoTab: Constructors and destructor
// ---------------------------------------------------------------------------

// The info will be set at some point after creation
TCQCDrvInfoTab::TCQCDrvInfoTab(const tCIDLib::TBoolean bCanClose) :

    TTabWindow(L"DrvInfo", L"Driver Information", bCanClose, kCIDLib::True)
    , m_pwndList(nullptr)
{
}


// To display the info immediately upon creation
TCQCDrvInfoTab::TCQCDrvInfoTab( const   TString&            strMoniker
                                , const TCQCDriverObjCfg&   cqcdcInfo
                                , const tCIDLib::TBoolean   bCanClose) :

    TTabWindow(L"DrvInfo", L"Driver Information", bCanClose, kCIDLib::True)
    , m_cqcdcInfo(cqcdcInfo)
    , m_pwndList(nullptr)
    , m_strMoniker(strMoniker)
{
}


TCQCDrvInfoTab::~TCQCDrvInfoTab()
{
}


// ---------------------------------------------------------------------------
//  TCQCDrvInfoTab: Pubic, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCDrvInfoTab::ClearInfo()
{
    m_pwndList->RemoveAll();
}


tCIDLib::TVoid
TCQCDrvInfoTab::UpdateInfo(const TString& strMoniker, const TCQCDriverObjCfg& cqcdcInfo)
{
    // Store the new info
    m_cqcdcInfo = cqcdcInfo;
    m_strMoniker = strMoniker;

    // And load up the new info
    LoadInfo();
}


// ---------------------------------------------------------------------------
//  TCQCDrvInfoTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCDrvInfoTab::AreaChanged(const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TTabWindow::AreaChanged
    (
        areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged
    );

    // If size changed and we are not min and our list window exists, resize it
    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndList)
        m_pwndList->SetSize(areaClient().szArea(), kCIDLib::True);
}


tCIDLib::TBoolean TCQCDrvInfoTab::bCreated()
{
    TTabWindow::bCreated();

    //
    //  Create our list window. We fit it fully to our area without any border so that
    //  it just fills up the tab area.
    //
    tCIDLib::TStrList colEmpty;
    m_pwndList = new T2ColSectList();
    m_pwndList->CreateMCLB
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , areaClient()
        , colEmpty
        , 0
        , tCIDCtrls::EWndStyles::VisTabChild
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EMCLBStyles::AutoLastCol
            , tCIDCtrls::EMCLBStyles::FullRowSel
          )
        , tCIDCtrls::EExWndStyles::None
    );

    // Load up the information if we got some in the ctor
    if (!m_strMoniker.bIsEmpty())
        LoadInfo();

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCDrvInfoTab::LoadInfo()
{
    //
    //  Remove all existing rows and load up again. We'll stop redrawing
    //  while we do this.
    //
    TWndPaintJanitor janUpdate(this);
    m_pwndList->RemoveAll();

    // And now let's load up the info to the list
    TString strFmt;
    m_pwndList->AddSectTitle(L"General Information");
    m_pwndList->AddDataRow(L"Moniker", m_strMoniker);

    strFmt.SetFormatted(m_cqcdcInfo.c4MajVersion());
    strFmt.Append(kCIDLib::chPeriod);
    strFmt.AppendFormatted(m_cqcdcInfo.c4MinVersion());
    m_pwndList->AddDataRow(L"Version", strFmt);

    m_pwndList->AddDataRow(L"Server Library", m_cqcdcInfo.strServerLibName());
    if (!m_cqcdcInfo.strClientLibName().bIsEmpty())
        m_pwndList->AddDataRow(L"Client Library", m_cqcdcInfo.strClientLibName());

    m_pwndList->AddDataRow(L"Category", tCQCKit::strXlatEDevCats(m_cqcdcInfo.eCategory()));

    strFmt = m_cqcdcInfo.strMake();
    strFmt.Append(L" / ");
    strFmt.Append(m_cqcdcInfo.strModel());
    m_pwndList->AddDataRow(L"Make/Model", strFmt);

    m_pwndList->AddDataRow
    (
        L"V2 Compat.", facCQCKit().strBoolYesNo(m_cqcdcInfo.c4ArchVersion() > 1)
    );

    m_pwndList->AddDataRow(L"Author", m_cqcdcInfo.strAuthor());


    if (m_cqcdcInfo.conncfgReal().clsIsA() == TCQCSerialConnCfg::clsThis())
    {
        m_pwndList->AddSectTitle(L"Serial Options");
        const TCQCSerialConnCfg& conncfgSer
        (
            static_cast<const TCQCSerialConnCfg&>(m_cqcdcInfo.conncfgReal())
        );
        m_pwndList->AddDataRow(L"Port", conncfgSer.strPortPath());

        const TCommPortCfg& cpcfgSer = conncfgSer.cpcfgSerial();
        strFmt.SetFormatted(cpcfgSer.c4Baud());
        m_pwndList->AddDataRow(L"Baud", strFmt);
        strFmt.SetFormatted(tCIDLib::TCard4(cpcfgSer.eDataBits()));
        m_pwndList->AddDataRow(L"Data Bits", strFmt);

        switch(cpcfgSer.eStopBits())
        {
            case tCIDComm::EStopBits::One :
                strFmt = L"1";
                break;

            case tCIDComm::EStopBits::OnePointFive :
                strFmt = L"1.5";
                break;

            case tCIDComm::EStopBits::Two :
                strFmt = L"2";
                break;

            default :
                strFmt = L"?";
                break;
        };
        m_pwndList->AddDataRow(L"Stop Bits", strFmt);
    }
     else if (m_cqcdcInfo.conncfgReal().clsIsA() == TCQCIPConnCfg::clsThis())
    {
        m_pwndList->AddSectTitle(L"IP Options");

        // Don't display the resolved address, show the text info
        const TCQCIPConnCfg& conncfgIP
        (
            static_cast<const TCQCIPConnCfg&>(m_cqcdcInfo.conncfgReal())
        );
        m_pwndList->AddDataRow(L"Address", conncfgIP.strAddr());

        strFmt.SetFormatted(conncfgIP.ippnTarget());
        m_pwndList->AddDataRow(L"Port", strFmt);
    }

    //
    //  Do all of the prompts. Keep in mind that some may not be available. They could
    //  have been added after this instance of the driver was installed.
    //
    const tCIDLib::TCard4 c4PromptCnt = m_cqcdcInfo.c4PromptCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PromptCnt; c4Index++)
    {
        const TCQCDCfgPrompt& cqcdcpCur = m_cqcdcInfo.cqcdcpAt(c4Index);
        m_pwndList->AddSectTitle(cqcdcpCur.strTitle());

        // Get a cursor for the sub-values
        const TString& strCurKey = cqcdcpCur.strName();

        //
        //  Get a cursor for the sub-values inside this prompt item. It might not be
        //  found if it was added after this driver instance was installed, so catch
        //  any errors.
        //
        try
        {
            TCQCDriverCfg::TValCursor cursVals
            (
                m_cqcdcInfo.cursPromptSubVals(strCurKey)
            );

            // And display those
            for (; cursVals; ++cursVals)
            {
                const TCQCDriverCfg::TPair& kobjCur = *cursVals;
                m_pwndList->AddDataRow(kobjCur.objKey(), kobjCur.objValue());
            }
        }

        catch(TError& errToCatch)
        {
            if (errToCatch.bCheckEvent(facCIDLib().strName(), kCIDErrs::errcNVM_ItemNotFound))
            {
                m_pwndList->AddDataRow(strCurKey, L"[Not Set]");
            }
             else
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                m_pwndList->AddDataRow(strCurKey, L"[Failed]");
            }
        }
    }

    // Auto-size the first column to fit the values we loaded
    m_pwndList->AutoSizeCol(1, kCIDLib::False);
}
