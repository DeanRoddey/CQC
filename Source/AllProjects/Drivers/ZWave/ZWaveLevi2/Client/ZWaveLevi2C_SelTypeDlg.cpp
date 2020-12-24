//
// FILE NAME: ZWaveLevi2C_SelTypeDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2014
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
//  for a unit.
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
RTTIDecls(TZWaveSelTypeDlg,TDlgBox)





// ---------------------------------------------------------------------------
//   CLASS: TZWaveSelTypeDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveSelTypeDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveSelTypeDlg::TZWaveSelTypeDlg() :

    m_pwndCancel(nullptr)
    , m_pwndMakes(nullptr)
    , m_pwndModels(nullptr)
    , m_punitEdit(nullptr)
    , m_pzwdinDevs(nullptr)
{
}

TZWaveSelTypeDlg::~TZWaveSelTypeDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWaveSelTypeDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TZWaveSelTypeDlg::bRunDlg(  const   TWindow&        wndOwner
                            ,       TZWaveUnit&     unitEdit
                            , const TZDIIndex&      zwdinDevs)
{
    // Save incoming stuff
    m_punitEdit = &unitEdit;
    m_pzwdinDevs = &zwdinDevs;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facZWaveLevi2C(), kZWaveLevi2C::ridDlg_SelType
    );

    return (c4Res == kZWaveLevi2C::ridDlg_SelType_Save);
}


// ---------------------------------------------------------------------------
//  TZWaveSelTypeDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWaveSelTypeDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_SelType_Cancel, m_pwndCancel);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_SelType_Makes, m_pwndMakes);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_SelType_Models, m_pwndModels);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_SelType_Save, m_pwndSave);

    // Update the title bar text with the unit name and generic type
    TString strTitle = strWndText();
    strTitle.Append(L" [");
    strTitle.Append(m_punitEdit->strName());
    strTitle.Append(L"]  [");
    strTitle.Append(tZWaveLevi2Sh::strXlatEGenTypes(m_punitEdit->eGenType()));
    strTitle.Append(L"]");
    strWndText(strTitle);

    // Register our handlers
    m_pwndSave->pnothRegisterHandler(this, &TZWaveSelTypeDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TZWaveSelTypeDlg::eClickHandler);

    m_pwndMakes->pnothRegisterHandler(this, &TZWaveSelTypeDlg::eLBHandler);
    m_pwndModels->pnothRegisterHandler(this, &TZWaveSelTypeDlg::eLBHandler);

    // Set the list column titles
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facZWaveLevi2C().strMsg(kZW2CMsgs::midDlg_SelType_Makes));
    m_pwndMakes->SetColumns(colCols);

    colCols[0] = facZWaveLevi2C().strMsg(kZW2CMsgs::midDlg_SelType_Models);
    m_pwndModels->SetColumns(colCols);

    // Load the makes list
    LoadMakes();

    const tCIDLib::TCard4 c4Count = m_pzwdinDevs->m_colMakeModel.c4ElemCount();
    if (c4Count)
    {
        //
        //  We got some stuff loaded. If there's a previously set make, then try to
        //  load up for that and select the previous model. Else, just select the
        //  0th make.
        //
        if (!m_punitEdit->strMake().bIsEmpty())
            LoadModels(m_punitEdit->strMake(), m_punitEdit->strModel());
        else
            m_pwndMakes->SelectByIndex(0, kCIDLib::True);
    }

    return bRet;
}



// ---------------------------------------------------------------------------
//  TZWaveSelTypeDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TZWaveSelTypeDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_SelType_Cancel)
    {
        EndDlg(kZWaveLevi2C::ridDlg_SelType_Cancel);
    }
     else if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_SelType_Save)
    {
        //
        //  Get the current one and let's get that device info and store it on the
        //  target unit.
        //
        const tCIDLib::TCard4 c4LIndex = m_pwndModels->c4CurItem();
        if (c4LIndex != kCIDLib::c4MaxCard)
        {
            const tCIDLib::TCard4 c4OrgIndex = m_pwndModels->c4IndexToId(c4LIndex);
            const TString& strName = m_pzwdinDevs->m_colMakeModel[c4LIndex].m_strName;

            //
            //  Get the device info file from the extra info and try to load the info.
            //  We have to provide him with an XML parser in case it's not already
            //  loaded. We won't use this device info, we are just making sure it
            //  be loaded.
            //
            const TString& strFile = m_pwndModels->strUserDataAt(c4LIndex);
            TXMLTreeParser xtprsToUse;
            const TZWDevInfo* pzwdiNew = TZWaveDrvConfig::pzwdiFind(strFile, &xtprsToUse);
            if (pzwdiNew)
            {
                // Store the new device info file and force a reload of the info
                m_punitEdit->SetDevInfoFile(strFile);
                m_punitEdit->bLoadDevInfo(xtprsToUse);

                // And we can exit now
                EndDlg(kZWaveLevi2C::ridDlg_SelType_Save);
            }
             else
            {
                TErrBox msgbFailed
                (
                    facZWaveLevi2C().strMsg(kZW2CErrs::errcZWInd_CantFind, strName)
                );
                msgbFailed.ShowIt(*this);
            }
        }
         else
        {
            TErrBox msgbFailed(facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_BothMakeModel));
            msgbFailed.ShowIt(*this);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}



tCIDCtrls::EEvResponses TZWaveSelTypeDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_SelType_Makes)
    {
        // When the make selection changes, load up all of the models for that make
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            TString strMake;
            m_pwndMakes->QueryColText(wnotEvent.c4Index(), 0, strMake);
            LoadModels(strMake, TString::strEmpty());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Load up the makes list, which stays the same the whole time
tCIDLib::TVoid TZWaveSelTypeDlg::LoadMakes()
{
    const tCIDLib::TCard4 c4Count = m_pzwdinDevs->m_colMakeModel.c4ElemCount();
    if (c4Count)
    {
        TString             strLastMake;
        tCIDLib::TStrList   colCols(1);
        colCols.objAdd(TString::strEmpty());

        TString strInitMake = m_punitEdit->strMake();
        if (strInitMake.bIsEmpty())
            strInitMake = m_pzwdinDevs->m_colMakeModel[0].m_strMake;

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TZWDIInfo& zwdiiCur = m_pzwdinDevs->m_colMakeModel[c4Index];
            if (zwdiiCur.m_strMake != strLastMake)
            {
                strLastMake = zwdiiCur.m_strMake;
                colCols[0] = strLastMake;
                m_pwndMakes->c4AddItem(colCols, 0);
            }
        }
    }
}


//
//  Load up the models for the indicated make, optionally selecting an incoming
//  model. We only load those models that are of the same generic type as the unit
//  object we are editing.
//
//
tCIDLib::TVoid
TZWaveSelTypeDlg::LoadModels(const TString& strForMake, const TString& strInitModel)
{
    TString strGenType = tZWaveLevi2Sh::strAltXlatEGenTypes(m_punitEdit->eGenType());

    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());

    m_pwndModels->RemoveAll();
    const tCIDLib::TCard4   c4Count = m_pzwdinDevs->m_colMakeModel.c4ElemCount();
    for (tCIDLib::TCard4    c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWDIInfo& zwdiiCur = m_pzwdinDevs->m_colMakeModel[c4Index];

        //
        //  If this one is for the incoming make, and it has the generic type of our
        //  unit we are editing, then take it.
        //
        if ((zwdiiCur.m_strMake == strForMake)
        &&  (zwdiiCur.m_strType == strGenType))
        {
            colCols[0] = zwdiiCur.m_strModel;
            const tCIDLib::TCard4 c4At = m_pwndModels->c4AddItem(colCols, c4Index);
            m_pwndModels->SetUserDataAt(c4At, zwdiiCur.m_strFileName);
        }
    }

    // Select the incoming one if any. If not found or none, select the 0th one
    if (m_pwndModels->c4ItemCount())
    {
        if (!strInitModel.bIsEmpty())
        {
            tCIDLib::TCard4 c4At;
            if (!m_pwndModels->bSelectByText(strInitModel, c4At))
                m_pwndModels->SelectByIndex(0);
        }
         else
        {
            m_pwndModels->SelectByIndex(0);
        }
    }
     else
    {
    }
}

