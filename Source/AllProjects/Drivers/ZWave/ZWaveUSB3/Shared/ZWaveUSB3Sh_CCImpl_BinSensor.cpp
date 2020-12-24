//
// FILE NAME: ZWaveUSB3Sh_CCImpl_BinSensor.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/11/2018
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
//  This is the implementation classs for the Z-Wave binary sensor command class
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
//  Magic marcos
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCImplBinSensor, TZWCCImplBin)
RTTIDecls(TZWCCImplBinSensorF, TZWCCImplBinSensor)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBinSensor
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBinSensor: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We take the default access set by the base CC impl class. If it needs to be
//  something else, the unit must set it. The defaults would almost always be
//  right for us, unless we are being used for notification purposes only, which
//  wouldn't be common.
//
TZWCCImplBinSensor::
TZWCCImplBinSensor(         TZWUnitInfo* const  punitiThis
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplBin(punitiThis, tZWaveUSB3Sh::ECClasses::BinSensor, c1EPId)
{
}

TZWCCImplBinSensor::~TZWCCImplBinSensor()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBinSensor: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCImplBinSensor::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValueQuery() called for non-readable bin sensor %(1)", strUnitName()
        );
        return;
    }

    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiOwner()
            , COMMAND_CLASS_SENSOR_BINARY
            , SENSOR_BINARY_GET
            , 0
            , tZWaveUSB3Sh::EMsgPrios::Async
            , kCIDLib::False
        );
        SendMsgAsync(zwomOut);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// ---------------------------------------------------------------------------
//  TZWCCImplBinSensor: Protected, inherited methods
// ---------------------------------------------------------------------------

// We just watch for a basic set or basic report operation
tZWaveUSB3Sh::ECCMsgRes
TZWCCImplBinSensor::eHandleCCMsg(const  tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_SENSOR_BINARY) && (c1Cmd == SENSOR_BINARY_REPORT))
    {
        // Get the state and write it, seeing if this is a change
        const tCIDLib::TCard1 c1Val = zwimIn.c1CCByteAt(2);
        if ((c1Val == 0) || (c1Val == 0xFF))
            bSetBinState(c1Val == 0xFF, tZWaveUSB3Sh::EValSrcs::Unit);
        else
            SetErrState();

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}





// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBinSensorF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBinSensorF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplBinSensorF::
TZWCCImplBinSensorF(        TZWUnitInfo* const      punitiThis
                    , const TString&                strFldName
                    , const tCQCKit::EFldSTypes     eSemType
                    , const tCIDLib::TCard1         c1EPId) :

    TZWCCImplBinSensor(punitiThis, c1EPId)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_eSemType(eSemType)
    , m_strFldName(strFldName)
{
}

TZWCCImplBinSensorF::~TZWCCImplBinSensorF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBinSensorF: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWCCImplBinSensorF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


tCIDLib::TVoid
TZWCCImplBinSensorF::QueryFldDefs(  tCQCKit::TFldDefList&   colAddTo
                                    , TCQCFldDef&           flddTmp) const
{
    // We just have our one boolean state field
    flddTmp.Set
    (
        m_strFldName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read, m_eSemType
    );
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplBinSensorF::StoreFldIds()
{
    // Just look up our one field using the name we were given
    m_c4FldId_State = c4LookupFldId(m_strFldName);
    SetFieldErr(m_c4FldId_State);
}


// ---------------------------------------------------------------------------
//  TZWCCImplBinSensorF: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCImplBinSensorF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_State);
    TParent::EnteredErrState();
}

tCIDLib::TVoid
TZWCCImplBinSensorF::ValueChanged(  const   tZWaveUSB3Sh::EValSrcs  eSrc
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          i4ExtraInfo)
{
    bWriteFld(m_c4FldId_State, bState());
    TParent::ValueChanged(eSrc, bWasInErr, i4ExtraInfo);
}
