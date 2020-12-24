//
// FILE NAME: ZWaveLevi2Sh_Config.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2014
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
//  This file provides the implementation for the configuration command class
//  derivative.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCConfig, TZWCmdClass)




// ---------------------------------------------------------------------------
//   CLASS: TZWCCConfig
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCConfig: Cosntructors Destructor
// ---------------------------------------------------------------------------
TZWCCConfig::TZWCCConfig(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::Configuration, zwdcInst, COMMAND_CLASS_CONFIGURATION)
{
    //
    //  We are never readable or writeable, in the sense that those things mean to
    //  to the driver, i.e. no field values to read or write.
    //
    SetAccess(kCIDLib::False, kCIDLib::False);
}

TZWCCConfig::~TZWCCConfig()
{
}


// ---------------------------------------------------------------------------
//  TZWCCConfig: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCConfig::InitVals(const TZWaveUnit&,  MZWLeviSrvFuncs&)
{
    // Nothing to do in our case
}



// ---------------------------------------------------------------------------
//  TZWCCConfig: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Query the stuff associated with the indicate group in the indicated source unit
tCIDLib::TBoolean
TZWCCConfig::bQueryConfigParm(          TZWaveUnit&         unitSrc
                                , const tCIDLib::TCard4     c4ParmId
                                ,       tCIDLib::TCard4&    c4ToFill
                                ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    tCIDLib::TCardList fcolVals(16);
    fcolVals.c4AddElement(COMMAND_CLASS_CONFIGURATION);
    fcolVals.c4AddElement(CONFIGURATION_GET);
    fcolVals.c4AddElement(c4ParmId);

    tCIDLib::TBoolean bRet = kCIDLib::True;
    c4ToFill = 0;
    try
    {
        mzwsfLevi.LeviSendUnitMsg(unitSrc, *this, fcolVals);
        mzwsfLevi.bLeviWaitUnitMsg
        (
            unitSrc, *this, COMMAND_CLASS_CONFIGURATION, CONFIGURATION_REPORT, fcolVals
        );

        const tCIDLib::TCard4 c4Size = fcolVals[3] & 0x7;
        if (c4Size == 1)
        {
            c4ToFill = tCIDLib::TCard1(fcolVals[4]);
        }
         else if (c4Size == 2)
        {
            tCIDLib::TCard2 c2Val = tCIDLib::TCard1(fcolVals[4]);
            c2Val <<= 8;
            c2Val |= tCIDLib::TCard1(fcolVals[5]);
            c4ToFill = c2Val;
        }
         else
        {
            tCIDLib::TCard4 c4Val = tCIDLib::TCard1(fcolVals[4]);
            c4Val <<= 8;
            c4Val |= tCIDLib::TCard1(fcolVals[5]);
            c4Val <<= 8;
            c4Val |= tCIDLib::TCard1(fcolVals[6]);
            c4Val <<= 8;
            c4Val |= tCIDLib::TCard1(fcolVals[7]);
            c4ToFill = c4Val;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        bRet = kCIDLib::False;
    }
    return bRet;
}



//
//  Add the target unit's id (and optionally instance id) to the indicated group.
//  If the instance id is zero, then we do a standard association. Else we do a multi-
//  instance type. If we implement wakeup, we just queue the cmd for later sending.
//
tCIDLib::TBoolean
TZWCCConfig::bSetConfigParm(        TZWaveUnit&         unitTar
                            , const tCIDLib::TCard4     c4ParmId
                            , const tCIDLib::TCard4     c4ByteCnt
                            , const tCIDLib::TCard4     c4Value
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    tCIDLib::TBoolean   bRet = kCIDLib::True;
    tCIDLib::TCardList  fcolVals(16);

    fcolVals.c4AddElement(COMMAND_CLASS_CONFIGURATION);
    fcolVals.c4AddElement(CONFIGURATION_SET);
    fcolVals.c4AddElement(c4ParmId);
    fcolVals.c4AddElement(c4ByteCnt);

    // Put in the bytes in the appropriate format
    if (c4ByteCnt == 1)
    {
        fcolVals.c4AddElement(tCIDLib::TCard1(c4Value));
    }
     else if (c4ByteCnt == 2)
    {
        fcolVals.c4AddElement(tCIDLib::TCard1(c4Value >> 8));
        fcolVals.c4AddElement(tCIDLib::TCard1(c4Value));
    }
     else if (c4ByteCnt == 4)
    {
        fcolVals.c4AddElement(tCIDLib::TCard1(c4Value >> 24));
        fcolVals.c4AddElement(tCIDLib::TCard1(c4Value >> 16));
        fcolVals.c4AddElement(tCIDLib::TCard1(c4Value >> 8));
        fcolVals.c4AddElement(tCIDLib::TCard1(c4Value));
    }
     else
    {
        facZWaveLevi2Sh().LogMsg
        (
             CID_FILE
            , CID_LINE
            , L"Config parameter size must be 1, 2, or 4 byte"
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
        return kCIDLib::False;
    }

    if (unitTar.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp))
    {
        TZWQCmd zwqcNew(0, tZWaveLevi2Sh::ECClasses::Configuration, fcolVals);
        unitTar.QueueUpCmd(zwqcNew, mzwsfLevi);
    }
     else
    {
        try
        {
            mzwsfLevi.LeviSendUnitMsg(unitTar, *this, fcolVals);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            bRet = kCIDLib::False;
        }
    }
    return bRet;
}


