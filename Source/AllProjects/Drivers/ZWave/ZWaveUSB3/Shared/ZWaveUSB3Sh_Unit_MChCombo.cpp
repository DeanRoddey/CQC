//
// FILE NAME: ZWaveUSB3Sh_MultiChCombo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2018
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
//  This is the implementation for a range of multi-channel combo modules.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros.
// ---------------------------------------------------------------------------
RTTIDecls(TMultiChComboUnit, TZWaveUnit)



// ---------------------------------------------------------------------------
//   CLASS: TMultiChComboUnit::TEPInfo
//  PREFIX: epi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMultiChComboUnit::TEPInfo: Constructors and destructor
// ---------------------------------------------------------------------------
TMultiChComboUnit::TEPInfo::~TEPInfo()
{
}

TMultiChComboUnit::TEPInfo::TEPInfo(const   tCIDLib::TCard1     c1EPId
                                    , const tCQCKit::EFldSTypes eSemType) :

    m_bV2Comp(kCIDLib::False)
    , m_c1EPId(c1EPId)
    , m_eSemType(eSemType)
{
}


// ---------------------------------------------------------------------------
//  TMultiChComboUnit::TEPInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Just cast off the constness to call the virtual versions
const TZWCCImpl* TMultiChComboUnit::TEPInfo::pzwcciFldC() const
{
    return const_cast<TEPInfo*>(this)->pzwcciFld();
}

const TZWCCImpl* TMultiChComboUnit::TEPInfo::pzwcciNotC() const
{
    return const_cast<TEPInfo*>(this)->pzwcciNot();
}



// For binary valued end points that derive from the common binary CC impl base class
class TEPInfoBin : public TMultiChComboUnit::TEPInfo
{
    public :
        // -----------------------------------------------------------
        //  Constructors and Destructor
        // -----------------------------------------------------------
        TEPInfoBin( const   tCIDLib::TCard1     c1EPId
                    , const tCQCKit::EFldSTypes eSemType) :

            TEPInfo(c1EPId, eSemType)
            , m_pzwcciFldBin(nullptr)
            , m_pzwcciNotBin(nullptr)
        {

        }

        ~TEPInfoBin()
        {
            delete m_pzwcciFldBin;
            delete m_pzwcciNotBin;
        }


        tCIDLib::TBoolean bOwnsFld(const tCIDLib::TCard4 c4FldId) const
        {
            return m_pzwcciFldBin->bOwnsFld(c4FldId);
        }

        EChRes eHandleChange(const tCIDLib::TCard4 c4ImplId) override
        {
            EChRes eRet = EChRes::NotMine;
            if (c4ImplId == m_pzwcciFldBin->c4ImplId())
            {
                m_pzwcciNotBin->bSetBinState
                (
                    m_pzwcciFldBin->bState(), tZWaveUSB3Sh::EValSrcs::Program
                );
                eRet = EChRes::Changed;
            }
             else if (m_pzwcciNotBin && (c4ImplId == m_pzwcciNotBin->c4ImplId()))
            {
                if (m_pzwcciFldBin->bSetBinState(m_pzwcciNotBin->bState(), tZWaveUSB3Sh::EValSrcs::Program))
                    eRet = EChRes::Changed;
                else
                    eRet = EChRes::NoChange;
            }
            return eRet;
        }

        TZWCCImpl* pzwcciFld() override
        {
            return m_pzwcciFldBin;
        }

        TZWCCImpl* pzwcciNot() override
        {
            return m_pzwcciNotBin;;
        }

        TZWCCImplBin*   m_pzwcciFldBin;
        TZWCCImplBin*   m_pzwcciNotBin;
};


// For level valued end points that derive from the common level CC impl base class
class TEPInfoLevel : public TMultiChComboUnit::TEPInfo
{
    public :
        TEPInfoLevel(const  tCIDLib::TCard1     c1EPId
                    , const tCQCKit::EFldSTypes eSemType) :

            TEPInfo(c1EPId, eSemType)
            , m_pzwcciFldLev(nullptr)
            , m_pzwcciNotLev(nullptr)
        {
        }

        ~TEPInfoLevel()
        {
            delete m_pzwcciFldLev;
            delete m_pzwcciNotLev;
        }

        tCIDLib::TBoolean bOwnsFld(const tCIDLib::TCard4 c4FldId) const
        {
            return m_pzwcciFldLev->bOwnsFld(c4FldId);
        }

        EChRes eHandleChange(const tCIDLib::TCard4 c4ImplId) override
        {
            EChRes eRet = EChRes::NotMine;
            if (c4ImplId == m_pzwcciFldLev->c4ImplId())
            {
                m_pzwcciNotLev->bSetLevel(m_pzwcciFldLev->c4Level(), tZWaveUSB3Sh::EValSrcs::Program);
                eRet = EChRes::Changed;
            }
             else if (m_pzwcciNotLev && (c4ImplId == m_pzwcciNotLev->c4ImplId()))
            {
                if (m_pzwcciFldLev->bSetLevel(m_pzwcciNotLev->c4Level(), tZWaveUSB3Sh::EValSrcs::Program))
                    eRet = EChRes::Changed;
                else
                    eRet = EChRes::NoChange;
            }
            return eRet;
        }

        TZWCCImpl* pzwcciFld() override
        {
            return m_pzwcciFldLev;
        }

        TZWCCImpl* pzwcciNot() override
        {
            return m_pzwcciNotLev;
        }

        TZWCCImplLevel* m_pzwcciFldLev;
        TZWCCImplLevel* m_pzwcciNotLev;
};

//
//  Similar to above for for the multi-level sensor class. In this case, we assume
//  that there is only one class, no separate notification. In some cases it could
//  possibly be the Basic class, but generally that would be the main class as well.
//
class TEPInfoMLSensor : public TMultiChComboUnit::TEPInfo
{
    public :
        TEPInfoMLSensor(const   tCIDLib::TCard1     c1EPId
                        , const tCQCKit::EFldSTypes eSemType) :

            TEPInfo(c1EPId, eSemType)
            , m_pzwcciFldLev(nullptr)
        {
        }

        ~TEPInfoMLSensor()
        {
            delete m_pzwcciFldLev;
        }

        tCIDLib::TBoolean bOwnsFld(const tCIDLib::TCard4 c4FldId) const
        {
            return m_pzwcciFldLev->bOwnsFld(c4FldId);
        }

        EChRes eHandleChange(const tCIDLib::TCard4 c4ImplId) override
        {
            if (c4ImplId == m_pzwcciFldLev->c4ImplId())
                return EChRes::Changed;
            return EChRes::NotMine;
        }

        TZWCCImpl* pzwcciFld() override
        {
            return m_pzwcciFldLev;
        }

        TZWCCImpl* pzwcciNot() override
        {
            return nullptr;
        }

        TZWCCImplMLSensor* m_pzwcciFldLev;
};




// ---------------------------------------------------------------------------
//   CLASS: TMultiChComboUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMultiChComboUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TMultiChComboUnit::TMultiChComboUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_colEndPts(tCIDLib::EAdoptOpts::Adopt, 8)
{
}

TMultiChComboUnit::~TMultiChComboUnit()
{

}


// ---------------------------------------------------------------------------
//  TMultiChComboUnit: Public, inherited methods
// ---------------------------------------------------------------------------

// Just pass it on to our CC impls
tCIDLib::TBoolean TMultiChComboUnit::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    if (TParent::bOwnsFld(c4FldId))
        return kCIDLib::True;

    // Run through our end points and let them check
    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TEPInfo* pepiCur = m_colEndPts[c4Index];
        if (pepiCur->pzwcciFldC()->bOwnsFld(c4FldId))
            return kCIDLib::True;
    }

    return kCIDLib::False;
}


//  We need to create our impl objects and prep them
tCIDLib::TBoolean TMultiChComboUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    // Based on the make/model set up our stuff
    return bSetupMM();
}


// Since we manage some of our own CCs, we have to override this
tCIDLib::TVoid TMultiChComboUnit::FormatCCsInfo(TTextOutStream& strmTar) const
{
    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TEPInfo* pepiCur = m_colEndPts[c4Index];
        pepiCur->pzwcciFldC()->FormatReport(strmTar);

        if (pepiCur->pzwcciNotC())
            pepiCur->pzwcciNotC()->FormatReport(strmTar);
    }
}


//
//  We have to manually handle distributing values to our CC impls. Since it could
//  be almost anything, we convert the incoming class to our enum and then run through
//  our end points for matches, either for the primary or notification CC.
//
tCIDLib::TVoid
TMultiChComboUnit::HandleCCMsg( const   tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimIn)
{
    TParent::HandleCCMsg(c1Class, c1Cmd, zwimIn);

    // See if we have the end point. If not encapsulated assume end point 0
    tCIDLib::TCard1 c1EP = 0;
    if (zwimIn.bWasEncapsulated())
        c1EP = zwimIn.c1SrcEPId();

    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    if (c1EP >= c4Count)
        return;

    TEPInfo* pepiTar = m_colEndPts[c1EP];
    pepiTar->pzwcciFld()->ProcessCCMsg(c1Class, c1Cmd, zwimIn);

    // If we have a notification CC, then let him see it
    if (pepiTar->pzwcciNot())
        pepiTar->pzwcciNot()->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
}


//
//  WE have to watch for changes from any notification CC impls, and push the current
//  value of that guy into the notification CC, and vice versa, to keep them in
//  sync. We will recurse back into here when we do that, but we ignore programmatic
//  changes so nothing will happen on the recursion.
//
//  We also have to handle event triggers here.
//
tCIDLib::TVoid
TMultiChComboUnit::ImplValueChanged(const   tCIDLib::TCard4         c4ImplId
                                    , const tZWaveUSB3Sh::EValSrcs  eSource
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          )
{
    //
    //  Ignore program source changes, we caused that ourself. When we force one
    //  motion impl with the new value from the other one that will call us back here
    //  but this will prevent actual recursion.
    //
    if (eSource == tZWaveUSB3Sh::EValSrcs::Program)
        return;

    //
    //  Find the end point that owns this CC impl and let him handle it. He'll
    //  do the work required to push the value between the field and notification
    //  CC impls as required.
    //
    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TEPInfo::EChRes eRes = m_colEndPts[c4Index]->eHandleChange(c4ImplId);

        //
        //  If there was a change, send an event trigger if relevant. If we are coming
        //  out of error state, we don't consider that a trigger change.
        //
        if ((eRes == TEPInfo::EChRes::Changed) && !bWasInErr)
        {

        }

        // If this guy was the one, then we are done
        if (eRes != TEPInfo::EChRes::NotMine)
            break;
    }
}



// Just pass it on to our CC impls
tCIDLib::TVoid
TMultiChComboUnit::QueryFldDefs(tCQCKit::TFldDefList& colAddTo, TCQCFldDef& flddTmp) const
{
    TParent::QueryFldDefs(colAddTo, flddTmp);
    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colEndPts[c4Index]->pzwcciFldC()->QueryFldDefs(colAddTo, flddTmp);
}


//
//  This one we have to just scan through the end points and look at their types
//  and decide what attributes we will generate. We'll set some flags to prevent us
//  from triggering twice, since we assume that multiple end points of the same type
//  will honor the same options, i.e. we won't have one C and one F scale sensor in
//  same module.
//
tCIDLib::TVoid
TMultiChComboUnit::QueryUnitAttrs(  tCIDMData::TAttrList&   colToFill
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();

    tCIDLib::TBoolean bDidTemp = kCIDLib::False;
    tCIDLib::TBoolean bDidMotion = kCIDLib::False;
    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TEPInfo* pepiCur = m_colEndPts[c4Index];
        if (!bDidTemp)
        {
            if ((pepiCur->m_eSemType == tCQCKit::EFldSTypes::CurTemp)
            ||  (pepiCur->m_eSemType == tCQCKit::EFldSTypes::CurExtTemp))
            {
                // Do the standard temp options. There's a helper for this
                adatTmp.Set(L"Temp Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
                colToFill.objAdd(adatTmp);
                TZWUSB3CCThermo::QueryTempAttrs(unitiUs, colToFill, adatTmp);
                bDidTemp = kCIDLib::True;
            }
        }

        if (!bDidMotion)
        {
            if (pepiCur->m_eSemType == tCQCKit::EFldSTypes::MotionSensor)
            {
                adatTmp.Set(L"Motion Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
                colToFill.objAdd(adatTmp);
                adatTmp.Set
                (
                    L"Send Triggers"
                    , kZWaveUSB3Sh::strUOpt_SendTrigger
                    , tCIDMData::EAttrTypes::Bool
                );
                adatTmp.SetBool(unitiUs.bQueryOptVal(kZWaveUSB3Sh::strUOpt_SendTrigger, kCIDLib::True));
                colToFill.objAdd(adatTmp);
                bDidMotion = kCIDLib::True;
            }
        }
    }
}


// We have to override this and pss on to our CC impls
tCIDLib::TVoid TMultiChComboUnit::Process()
{
    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colEndPts[c4Index]->pzwcciFld()->Process();
}


// Just pass on to our CC impls
tCIDLib::TVoid TMultiChComboUnit::StoreFldIds()
{
    TParent::StoreFldIds();

    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colEndPts[c4Index]->pzwcciFld()->StoreFldIds();
}


// Just pass on to our CC impls
tCIDLib::TVoid TMultiChComboUnit::UnitIsOnline()
{
    TParent::UnitIsOnline();

    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colEndPts[c4Index]->pzwcciFld()->UnitIsOnline();
}



// ---------------------------------------------------------------------------
//  TMultiChComboUnit: Private, non-virtaal methods
// ---------------------------------------------------------------------------

//
//  Based on the make model we get from the device info
//
//  The helper methods that add CCs keep the default access bits set by the CCs.
//  If these are not correct, we need to adjust them after the fact. The defaults
//  are typically going to be right for us, since they deal with readability pretty
//  correctly, and none of our CCs are likely to ever be writeable.
//
tCIDLib::TBoolean TMultiChComboUnit::bSetupMM()
{
    TZWUnitInfo& unitiUs = unitiOwner();
    const TString& strMake = unitiUs.strMake();
    const TString& strModel = unitiUs.strModel();

    const tZWaveUSB3Sh::EZWMakes eMake = tZWaveUSB3Sh::eXlatEZWMakes(strMake);
    if (eMake == tZWaveUSB3Sh::EZWMakes::Fibaro)
    {
        if (strModel.bCompareI(L"FGDW-002"))
        {
            // A binary door/wnd, an external door/wnd, and an internal temp
            pepiAddBinSensor(0, unitiUs, L"_State", L"Door/Wnd");
            pepiAddBinSensor(1, unitiUs, L"_ExtState", L"Ext Door/Wnd");
            pepiAddTempSensor(2, unitiUs, kCIDLib::True);
        }
    }

    //
    //  Go back and assign faux CC ids to them all. There might be some on the parent
    //  class for standard stuff like battery, so start with a large number.
    //
    tCIDLib::TCard4 c4FauxId = 128;
    const tCIDLib::TCard4 c4Count = m_colEndPts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TEPInfo* pepiCur = m_colEndPts[c4Index];
        pepiCur->pzwcciFld()->c4ImplId(c4FauxId++);
        if (pepiCur->pzwcciNot())
            pepiCur->pzwcciNot()->c4ImplId(c4FauxId++);
    }

    // Return true if we understand the make model
    return !m_colEndPts.bIsEmpty();
}


// Adds a bin sensor end point, with access and semantic type
TMultiChComboUnit::TEPInfo*
TMultiChComboUnit::pepiAddBinSensor(const   tCIDLib::TCard1         c1EPId
                                    , const TZWUnitInfo&            unitiUs
                                    , const tCIDLib::TCh* const     pszSuffix
                                    , const tCIDLib::TCh* const     pszDesc
                                    , const tCQCKit::EFldSTypes     eSemType
                                    , const tCIDLib::TBoolean       bAppendId)
{
    // Create a new binary type end point info object and store it
    TEPInfoBin* pepiRet = new TEPInfoBin(c1EPId, eSemType);
    m_colEndPts.Add(pepiRet);

    TString strDesc;
    TString strFldName;
    if (eSemType == tCQCKit::EFldSTypes::MotionSensor)
    {
        pepiRet->m_bV2Comp = kCIDLib::True;
        strFldName = tCQCKit::strAltXlatEDevClasses(tCQCKit::EDevClasses::MotionSensor);
        strFldName.Append(strName());
        strDesc = L"Motion";
    }
     else
    {
        strFldName = strName();
        if (pszSuffix)
            strFldName.Append(pszSuffix);
    }

    // If it's not the main (zero) end point, and they ask to, add the end point id
    if (bAppendId && (c1EPId != 0))
    {
        strFldName.Append(kCIDLib::chUnderscore);
        strFldName.AppendFormatted(c1EPId);
    }

    pepiRet->m_pzwcciFldBin = new TZWCCImplBinSensorF
    (
        &unitiOwner(), strFldName, eSemType, c1EPId
    );
    pepiRet->m_pzwcciFldBin->strDesc(pszDesc);

    return pepiRet;
}


//
//  Add's a temp sensor based on the MultiLevel Sensor type, no separate notification
//  class supported in this case.
//
TMultiChComboUnit::TEPInfo*
TMultiChComboUnit::pepiAddTempSensor(const  tCIDLib::TCard1     c1EPId
                                    , const TZWUnitInfo&        unitiUs
                                    , const tCIDLib::TBoolean   bInternal)
{
    TEPInfoMLSensor* pepiRet
    (
        new TEPInfoMLSensor
        (
            c1EPId
            , bInternal ? tCQCKit::EFldSTypes::CurTemp : tCQCKit::EFldSTypes::CurExtTemp
        )
    );
    m_colEndPts.Add(pepiRet);

    // We should have a temp scale option and min/max values
    const tCIDLib::TBoolean bFScale
    (
        unitiUs.strQueryOptVal(kZWaveUSB3Sh::strUOpt_TempScale, L"F") == L"F"
    );
    const tCIDLib::TFloat8 f8Min = unitiUs.i4QueryOptVal
    (
        kZWaveUSB3Sh::strUOpt_CurTempMin, bFScale ? -20 : -30
    );
    const tCIDLib::TFloat8 f8Max = unitiUs.i4QueryOptVal
    (
        kZWaveUSB3Sh::strUOpt_CurTempMax, bFScale ? 120 : 55
    );

    // It's never V2 here, that's only for thermostats, this is jsut a standalone sensor
    TString strFldName(unitiUs.strName());
    strFldName.Append(L"_Temp");

    TZWCCImplMLSensorF* pzwcciTemp = new TZWCCImplMLSensorF
    (
        &unitiOwner(), strFldName, pepiRet->m_eSemType, c1EPId
    );
    pepiRet->m_pzwcciFldLev = pzwcciTemp;

    // Set the format info and description
    pzwcciTemp->strDesc(L"Temperature");
    pzwcciTemp->SetFormat
    (
        tCQCKit::EFldTypes::Int
        , f8Min
        , f8Max
        , bFScale ? tZWaveUSB3Sh::ETScales::F : tZWaveUSB3Sh::ETScales::C
    );

    return pepiRet;
}

