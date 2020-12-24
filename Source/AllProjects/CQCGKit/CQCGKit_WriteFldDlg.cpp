//
// FILE NAME: CQCGKit_WriteFldDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/22/2002
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
//  This module implements the simple dialog that allows the user to modify
//  the value of a writeable field. According to the data type we load up
//  a different dialog template.
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
#include    "CQCGKit_WriteFldDlg_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TSelLimitValBaseDlg, TDlgBox)
RTTIDecls(TSelLimitValDlg, TSelLimitValBaseDlg)
RTTIDecls(TWrtFldDlg, TSelLimitValBaseDlg)



// ---------------------------------------------------------------------------
//  CLASS: TSelLimitValBaseDlg
// PREFIX: dlgb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSelLimitValBaseDlg: Constructors and Destructors
// ---------------------------------------------------------------------------
TSelLimitValBaseDlg::~TSelLimitValBaseDlg()
{
    delete m_pfldlTarget;
}


// ---------------------------------------------------------------------------
//  TSelLimitValBaseDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TSelLimitValBaseDlg::bCreated()
{
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Set the title we were given
    strWndText(m_strTitle);

    //
    //  Get a typed pointer to our close button. The rest of these are done
    //  below because they are dependent on which flavor of data type we
    //  loaded up for.
    //
    CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldTxt_Close, m_pwndCloseButton);
    CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldTxt_Set, m_pwndSetButton);

    m_pwndCloseButton->pnothRegisterHandler(this, &TSelLimitValBaseDlg::eClickHandler);
    m_pwndSetButton->pnothRegisterHandler(this, &TSelLimitValBaseDlg::eClickHandler);

    //
    //  According to the type of the field, we need to initialize the
    //  value widget with the current value of the field. While we are at
    //  it, we'll get some typed pointers to the widgets that are active for
    //  this invocation and register event handlers for them.
    //
    if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldSlider)
    {
        CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldSlider_Slider, m_pwndSlider);
        CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldSlider_Value, m_pwndValue);

        //
        //  Get the range values. We get them into signed values, but that's
        //  ok because the ranges that will get this representation are way
        //  too small to worry about overflowing the signed value.
        //
        tCIDLib::TInt4 i4Min = 0;
        tCIDLib::TInt4 i4Max = 0;
        if (m_eType == tCQCKit::EFldTypes::Card)
        {
            TCQCFldCRangeLimit* pfldlCR = static_cast<TCQCFldCRangeLimit*>(m_pfldlTarget);
            i4Min = tCIDLib::TInt4(pfldlCR->c4Min());
            i4Max = tCIDLib::TInt4(pfldlCR->c4Max());
        }
         else if (m_eType == tCQCKit::EFldTypes::Int)
        {
            TCQCFldIRangeLimit* pfldlIR = static_cast<TCQCFldIRangeLimit*>(m_pfldlTarget);
            i4Min = pfldlIR->i4Min();
            i4Max = pfldlIR->i4Max();
        }
         else
        {
            // <TBD> This is bad and shouldn't happen
        }

        // If we got an initial value, then use that if valid
        tCIDLib::TInt4 i4Init = i4Min;
        if (!m_strInValue.bIsEmpty())
            m_strInValue.bToInt4(i4Init);

        //
        //  If the range is pretty close to the slider's pixel width, drop the
        //  tick frequency. Else set it to 2.
        //
        if (tCIDLib::TCard4(i4Max - i4Min) > (m_pwndSlider->areaWndSize().c4Width() / 2))
            m_pwndSlider->SetTickFreq(8);
        else
            m_pwndSlider->SetTickFreq(2);

        // Now set up the slider and the initial value display
        m_pwndSlider->SetRange(i4Min, i4Max);
        m_pwndSlider->SetValue(i4Init);

        TString strVal;
        strVal.AppendFormatted(m_pwndSlider->i4CurValue());
        m_pwndValue->strWndText(strVal);

        // Register a handler for the slider
        m_pwndSlider->pnothRegisterHandler(this, &TSelLimitValBaseDlg::eSliderHandler);
    }
     else if ((m_ridLoaded == kCQCGKit::ridDlg_WrtFldBList)
          ||  (m_ridLoaded == kCQCGKit::ridDlg_WrtFldList))
    {
        //
        //  It's an enumerated field, so cast the limits to the right
        //  type, and enumerate the limits values into either the combo
        //  or list box according to which child we are loading.
        //
        TCQCFldEnumLimit* pfldlEnum = static_cast<TCQCFldEnumLimit*>(m_pfldlTarget);

        TCQCFldEnumLimit::TEnumCursor cursVals(pfldlEnum->cursEnum());
        if (cursVals.bIsValid())
        {
            // And load to the appropriate child window
            if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldBList)
            {
                CastChildWnd(*this, kCQCGKit::ridDlg_WrtBList_List, m_pwndBList);
                for (; cursVals; ++cursVals)
                    m_pwndBList->c4AddItem(*cursVals);

                tCIDLib::TCard4 c4At;
                if (!m_pwndBList->bSelectByText(m_strInValue, c4At))
                    m_pwndBList->SelectByIndex(0, kCIDLib::True, kCIDLib::False);
            }
             else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldList)
            {
                CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldList_Value, m_pwndList);

                tCIDLib::TStrList colToLoad;
                for (; cursVals; ++cursVals)
                    colToLoad.objAdd(*cursVals);

                m_pwndList->LoadList(colToLoad);

                tCIDLib::TCard4 c4At;
                if (!m_pwndList->bSelectByText(m_strInValue, c4At))
                    m_pwndList->SelectByIndex(0, kCIDLib::True, kCIDLib::False);
            }
             else
            {
                CIDAssert2(L"Incorrect field type for spin box")
            }
        }
    }
     else if (m_ridLoaded == kCQCGKit::ridDlg_WrtNumFldSpin)
    {
        CastChildWnd(*this, kCQCGKit::ridDlg_WrtNumFldSpin_Value, m_pwndNumSpin);

        //
        //  Get the range values. We get them into signed values, but
        //  that's ok because the ranges that will get this representation
        //  are way too small to worry about overflowing the signed value.
        //
        m_i4Min = 0;
        tCIDLib::TInt4 i4Max = 0;
        if (m_eType == tCQCKit::EFldTypes::Card)
        {
            TCQCFldCRangeLimit* pfldlCR = static_cast<TCQCFldCRangeLimit*>(m_pfldlTarget);
            m_i4Min = tCIDLib::TInt4(pfldlCR->c4Min());
            i4Max = tCIDLib::TInt4(pfldlCR->c4Max());
        }
         else if (m_eType == tCQCKit::EFldTypes::Int)
        {
            TCQCFldIRangeLimit* pfldlIR = static_cast<TCQCFldIRangeLimit*>(m_pfldlTarget);
            m_i4Min = pfldlIR->i4Min();
            i4Max = pfldlIR->i4Max();
        }
         else
        {
            CIDAssert2(L"Incorrect field type for spin box")
        }

        // Load up the spin box
        tCIDLib::TInt4 i4Init = m_i4Min;
        m_strInValue.bToInt4(i4Init, tCIDLib::ERadices::Dec);
        m_pwndNumSpin->SetNumRange(m_i4Min, i4Max, i4Init);
    }
     else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldBool)
    {
        CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldBool_Check, m_pwndCheck);

        tCIDLib::TBoolean bRes;
        if (facCQCKit().bCheckBoolVal(m_strInValue, bRes))
            m_pwndCheck->SetCheckedState(bRes);

        m_pwndCheck->pnothRegisterHandler(this, &TSelLimitValBaseDlg::eClickHandler);
    }
     else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldText)
    {
        CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldTxt_Field, m_pwndText);
        m_pwndText->strWndText(m_strInValue);
    }
     else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldTime)
    {
        CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldTime_Hours, m_pwndNumSpin);
        CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldTime_Mins, m_pwndNumSpin2);
        CastChildWnd(*this, kCQCGKit::ridDlg_WrtFldTime_Secs, m_pwndNumSpin3);

        // Load up the first spin boxs with hours, minuts and seconds
        m_pwndNumSpin->SetNumRange(0, 72, 0);
        m_pwndNumSpin2->SetNumRange(0, 60, 0);
        m_pwndNumSpin3->SetNumRange(0, 60, 0);

        // Calculate the hours and minutes of any incoming value
        tCIDLib::TEncodedTime enctIn;
        if (m_strInValue.bToCard8(enctIn, tCIDLib::ERadices::Auto))
        {
            tCIDLib::TCard4 c4Hours = tCIDLib::TCard4(enctIn / kCIDLib::enctOneHour);
            enctIn -= (c4Hours * kCIDLib::enctOneHour);
            tCIDLib::TCard4 c4Mins = tCIDLib::TCard4(enctIn / kCIDLib::enctOneMinute);
            enctIn -= (c4Mins * kCIDLib::enctOneMinute);
            tCIDLib::TCard4 c4Secs = tCIDLib::TCard4(enctIn / kCIDLib::enctOneSecond);

            if (c4Hours > 72)
                c4Hours = 72;
            if (c4Mins > 60)
                c4Mins = 60;
            if (c4Secs > 60)
                c4Secs = 60;

            m_pwndNumSpin->SetValue(c4Hours);
            m_pwndNumSpin2->SetValue(c4Mins);
            m_pwndNumSpin3->SetValue(c4Secs);
        }
    }
    return bRet;
}



// ---------------------------------------------------------------------------
//  TSelLimitValBaseDlg: Hidden Constructors
// ---------------------------------------------------------------------------
TSelLimitValBaseDlg::TSelLimitValBaseDlg() :

    m_eType(tCQCKit::EFldTypes::String)
    , m_i4Min(0)
    , m_pfldlTarget(nullptr)
    , m_pwndBList(nullptr)
    , m_pwndCheck(nullptr)
    , m_pwndCloseButton(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSlider(nullptr)
    , m_pwndNumSpin(nullptr)
    , m_pwndNumSpin2(nullptr)
    , m_pwndNumSpin3(nullptr)
    , m_pwndTextSpin(nullptr)
    , m_pwndText(nullptr)
    , m_pwndValue(nullptr)
    , m_ridLoaded(kCQCGKit::ridDlg_WrtFldText)
{
}


// ---------------------------------------------------------------------------
//  TSelLimitValBaseDlg: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// The derived classes call this to do the dirty work
tCIDLib::TBoolean
TSelLimitValBaseDlg::bRunIt(const   TWindow&            wndOwner
                            , const TString&            strTitle
                            , const tCQCKit::EFldTypes  eType
                            , const TString&            strLimits
                            , const TPoint&             pntOrg
                            ,       TString&            strValue
                            , const TString&            strOrgVal
                            , const tCIDLib::TBoolean   bInValue)
{
    m_eType = eType;
    m_strInValue = strOrgVal;
    m_strLimits = strLimits;
    m_strTitle = strTitle;

    // Create a limits object based on the incoming limits string
    m_pfldlTarget = TCQCFldLimit::pfldlMakeNew
    (
        L"Selection Value", strLimits, eType
    );

    // Remember if this is an enumerated type
    if ((eType == tCQCKit::EFldTypes::String) && m_strLimits.bStartsWithI(L"enum:"))
        m_bIsEnumType = kCIDLib::True;
    else
        m_bIsEnumType = kCIDLib::False;

    // If no incoming value, ask the limit object for a default
    if (!bInValue)
        m_pfldlTarget->QueryDefVal(m_strInValue);

    switch(m_pfldlTarget->eOptimalRep())
    {
        case tCQCKit::EOptFldReps::Check :
            m_ridLoaded = kCQCGKit::ridDlg_WrtFldBool;
            break;

        case tCQCKit::EOptFldReps::Combo :
            m_ridLoaded = kCQCGKit::ridDlg_WrtFldList;
            break;

        case tCQCKit::EOptFldReps::SelDialog :
            m_ridLoaded = kCQCGKit::ridDlg_WrtFldBList;
            break;

        case tCQCKit::EOptFldReps::Slider :
            m_ridLoaded = kCQCGKit::ridDlg_WrtFldSlider;
            break;

        case tCQCKit::EOptFldReps::Spin :
            // If numeric we load one, else if enumerated we load another
            if (m_bIsEnumType)
                m_ridLoaded = kCQCGKit::ridDlg_WrtFldSpin;
            else
                m_ridLoaded = kCQCGKit::ridDlg_WrtNumFldSpin;
            break;

        case tCQCKit::EOptFldReps::Text :
            m_ridLoaded = kCQCGKit::ridDlg_WrtFldText;
            break;

        case tCQCKit::EOptFldReps::Time :
            m_ridLoaded = kCQCGKit::ridDlg_WrtFldTime;
            break;

        default :
            facCQCGKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGKitErrs::errcFldW_BadFldRep
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TInteger(tCIDLib::i4EnumOrd(m_pfldlTarget->eOptimalRep()))
            );
            break;
    };

    tCIDLib::TCard4 c4Res;
    if (pntOrg.bAtOrg())
        c4Res = c4RunDlg(wndOwner, facCQCGKit(), m_ridLoaded);
    else
        c4Res = c4RunDlg(wndOwner, facCQCGKit(), m_ridLoaded, pntOrg);

    return (c4Res == kCIDCtrls::widOK);
}


// ---------------------------------------------------------------------------
//  TSelLimitValBaseDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TSelLimitValBaseDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCIDCtrls::widCancel)
    {
        EndDlg(kCIDCtrls::widCancel);
    }
     else if (wnotEvent.widSource() == kCIDCtrls::widOK)
    {
        //
        //  Get the current value of the active value widget and try to write
        //  it to the field.
        //
        TTextStringOutStream strmOut(2048UL);
        if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldBool)
        {
            if (m_pwndCheck->bCheckedState())
                strmOut << kCQCKit::pszFld_True;
            else
                strmOut << kCQCKit::pszFld_False;
        }
         else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldList)
        {
            const tCIDLib::TCard4 c4At = m_pwndList->c4CurItem();
            TString strVal;
            if (c4At != kCIDLib::c4MaxCard)
                m_pwndList->ValueAt(c4At, strVal);
            strmOut << strVal;
        }
         else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldBList)
        {
            const tCIDLib::TCard4 c4At = m_pwndBList->c4CurItem();
            TString strVal;
            if (c4At != kCIDLib::c4MaxCard)
                m_pwndBList->ValueAt(c4At, strVal);
            strmOut << strVal;
        }
         else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldSlider)
        {
            strmOut << m_pwndSlider->i4CurValue();
        }
         else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldSpin)
        {
            TString strVal;
            m_pwndTextSpin->QueryCurText(strVal);
            strmOut << strVal;
        }
         else if (m_ridLoaded == kCQCGKit::ridDlg_WrtNumFldSpin)
        {
            strmOut << tCIDLib::TInt4(m_pwndNumSpin->i4CurValue());
        }
         else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldText)
        {
            strmOut << m_pwndText->strWndText();
        }
         else if (m_ridLoaded == kCQCGKit::ridDlg_WrtFldTime)
        {
            //
            //  We have to get multiple values out and create the final
            //  100-ns time stamp result.
            //
            const tCIDLib::TCard4 c4Hours = m_pwndNumSpin->i4CurValue();
            const tCIDLib::TCard4 c4Mins = m_pwndNumSpin2->i4CurValue();
            const tCIDLib::TCard4 c4Secs = m_pwndNumSpin3->i4CurValue();

            tCIDLib::TEncodedTime enctRes
            (
                (c4Hours * kCIDLib::enctOneHour)
                + (c4Mins * kCIDLib::enctOneMinute)
                + (c4Secs * kCIDLib::enctOneSecond)
            );

            // Format it hex with correct radix prefix
            strmOut << L"0x" << TCardinal64(enctRes, tCIDLib::ERadices::Hex);
        }
         else
        {
            // <TBD> This is bad
        }

        // Ask the limits object to validate the value
        strmOut.Flush();
        if (m_pfldlTarget->bValidate(strmOut.strData()))
        {
            // Let the derived class see it. It will dismiss if appropriate
            try
            {
                ValueSelected(strmOut.strData());
            }

            catch(TError& errToCatch)
            {
                TExceptDlg dlgErr
                (
                    *this
                    , strWndText()
                    , L"Value processing caused an unhandled error"
                    , errToCatch
                );
            }
        }
         else
        {
            TErrBox msgbError
            (
                strWndText(), L"The value is outside of valid limits"
            );
            msgbError.ShowIt(*this);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses
TSelLimitValBaseDlg::eSliderHandler(TSliderChangeInfo& wnotEvent)
{
    if ((wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::Change)
    ||  (wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::Track))
    {
        // Update the value display to reflect the new slider value
        TString strVal;
        strVal.AppendFormatted(m_pwndSlider->i4CurValue());
        m_pwndValue->strWndText(strVal);
    }
    return tCIDCtrls::EEvResponses::Handled;
}




// ---------------------------------------------------------------------------
//  CLASS: TSelLimitValDlg
// PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TSelLimitValDlg: Constructors and destructor
// ---------------------------------------------------------------------------
TSelLimitValDlg::TSelLimitValDlg()
{
}

TSelLimitValDlg::~TSelLimitValDlg()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TSelLimitValDlg::bRunDlg(const  TWindow&            wndOwner
                        , const TString&            strTitle
                        , const tCQCKit::EFldTypes  eType
                        , const TString&            strLimits
                        , const TPoint&             pntOrg
                        ,       TString&            strValue
                        , const TString&            strOrgVal)
{
    // Just call the underlying class' runner
    const tCIDLib::TBoolean bRes = bRunIt
    (
        wndOwner
        , strTitle
        , eType
        , strLimits
        , pntOrg
        , strValue
        , strOrgVal
        , kCIDLib::True
    );

    if (bRes)
        strValue = m_strValue;

    return bRes;
}

tCIDLib::TBoolean
TSelLimitValDlg::bRunDlg(const  TWindow&            wndOwner
                        , const TString&            strTitle
                        , const tCQCKit::EFldTypes  eType
                        , const TString&            strLimits
                        , const TPoint&             pntOrg
                        ,       TString&            strValue)
{
    // Just call the underlying class' runner
    const tCIDLib::TBoolean bRes = bRunIt
    (
        wndOwner
        , strTitle
        , eType
        , strLimits
        , pntOrg
        , strValue
        , TString::strEmpty()
        , kCIDLib::False
    );

    if (bRes)
        strValue = m_strValue;

    return bRes;
}


// ---------------------------------------------------------------------------
//  Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TSelLimitValDlg::ValueSelected(const TString& strValue)
{
    // We are just selecting a value, so assume it's good
    m_strValue = strValue;
    EndDlg(kCIDCtrls::widOK);
}




// ---------------------------------------------------------------------------
//  CLASS: TWrtFldDlg
// PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TWrtFldDlg: Constructors and destructor
// ---------------------------------------------------------------------------
TWrtFldDlg::TWrtFldDlg(const TCQCSecToken& sectUser) :

    m_sectUser(sectUser)
{
}

TWrtFldDlg::~TWrtFldDlg()
{
}


// ---------------------------------------------------------------------------
//  TWrtFldDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TWrtFldDlg::bRunDlg(const   TWindow&        wndOwner
                    , const TString&        strTitle
                    , const TCQCFldDef&     flddTarget
                    , const TString&        strMoniker
                    , const TPoint&         pntOrg
                    ,       TString&        strValue
                    , const TString&        strOrgVal)
{
    // Store the incoming field info for later
    m_flddTarget = flddTarget;
    m_strMoniker = strMoniker;

    // Just call the underlying class' runner
    const tCIDLib::TBoolean bRes = bRunIt
    (
        wndOwner
        , strTitle
        , flddTarget.eType()
        , flddTarget.strLimits()
        , pntOrg
        , strValue
        , strOrgVal
        , kCIDLib::True
    );

    if (bRes)
        strValue = m_strValue;

    return bRes;
}

tCIDLib::TBoolean
TWrtFldDlg::bRunDlg(const   TWindow&    wndOwner
                    , const TString&    strTitle
                    , const TCQCFldDef& flddTarget
                    , const TString&    strMoniker
                    , const TPoint&     pntOrg
                    ,       TString&    strValue)
{
    // Store the incoming field info for later
    m_flddTarget = flddTarget;
    m_strMoniker = strMoniker;

    //
    //  No provided previous value, so let's try t get a default value if we can. We'll
    //  try to read the value of the field.
    //
    tCIDLib::TBoolean bInitVal = kCIDLib::False;
    TString strInitVal;
    if (tCIDLib::bAllBitsOn(m_flddTarget.eAccess(), tCQCKit::EFldAccess::Read))
    {
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker));
        tCIDLib::TCard4 c4SerialNum = 0;
        tCQCKit::EFldTypes eType;

        try
        {
            orbcAdmin->bReadFieldByName
            (
                c4SerialNum, m_strMoniker, m_flddTarget.strName(), strValue, eType
            );

            bInitVal = kCIDLib::True;
        }

        catch(...)
        {
            TErrBox msgbError
            (
                strWndText(), facCQCGKit().strMsg(kGKitErrs::errcFldW_GetCurVal)
            );
            msgbError.ShowIt(*this);
        }
    }

    // Just call the underlying class' runner
    const tCIDLib::TBoolean bRes = bRunIt
    (
        wndOwner
        , strTitle
        , flddTarget.eType()
        , flddTarget.strLimits()
        , pntOrg
        , strValue
        , strInitVal
        , bInitVal
    );

    if (bRes)
        strValue = m_strValue;

    return bRes;
}


// ---------------------------------------------------------------------------
//  TWrtFldDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TWrtFldDlg::ValueSelected(const TString& strValue)
{
    // Let's try to write the value
    try
    {
        tCQCKit::TCQCSrvProxy orbcAdmin
        (
            facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker)
        );

        orbcAdmin->WriteFieldByName
        (
            m_strMoniker
            , m_flddTarget.strName()
            , strValue
            , m_sectUser
            , tCQCKit::EDrvCmdWaits::DontCare
        );

        // It worked, so let's store it and exit with success
        m_strValue = strValue;
        EndDlg(kCIDCtrls::widOK);
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facCQCGKit().strMsg(kGKitErrs::errcFldW_WriteFailed)
            , errToCatch
        );
    }
}

