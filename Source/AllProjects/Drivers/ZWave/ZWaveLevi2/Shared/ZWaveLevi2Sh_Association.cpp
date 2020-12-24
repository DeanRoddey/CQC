//
// FILE NAME: ZWaveLevi2Sh_Association.cpp
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
//  This file provides the implementation for the association command class
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
RTTIDecls(TZWCCAssociation, TZWCmdClass)




// ---------------------------------------------------------------------------
//   CLASS: TZWCCAssociation
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCAssociation: Cosntructors Destructor
// ---------------------------------------------------------------------------
TZWCCAssociation::TZWCCAssociation(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::Association, zwdcInst, COMMAND_CLASS_ASSOCIATION)
{
    //
    //  We are never readable or writeable, in the sense that those things mean to
    //  to the driver, i.e. no field values to read or write.
    //
    SetAccess(kCIDLib::False, kCIDLib::False);
}

TZWCCAssociation::~TZWCCAssociation()
{
}


// ---------------------------------------------------------------------------
//  TZWCCAssociation: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCAssociation::InitVals(const TZWaveUnit&,  MZWLeviSrvFuncs&)
{
    // Nothing to do in our case
}



// ---------------------------------------------------------------------------
//  TZWCCAssociation: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCAssociation::bDelAllAssociation(       TZWaveUnit&         unitSrc
                                    , const tCIDLib::TCard4     c4InstId
                                    , const tCIDLib::TCard4     c4GroupId
                                    ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    tCIDLib::TBoolean   bRet = kCIDLib::True;
    tCIDLib::TCardList  fcolVals(16);

    if (eMultiType() == tZWaveLevi2Sh::EMultiTypes::Single)
    {
        CIDAssert(c4InstId == 0, L"Inst id should be zero for single instance class");

        fcolVals.c4AddElement(COMMAND_CLASS_ASSOCIATION);
        fcolVals.c4AddElement(ASSOCIATION_REMOVE);
        fcolVals.c4AddElement(c4GroupId);

        if (unitSrc.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp))
        {
            TZWQCmd zwqcNew(0, tZWaveLevi2Sh::ECClasses::Association, fcolVals);
            unitSrc.QueueUpCmd(zwqcNew, mzwsfLevi);
        }
         else
        {
            try
            {
                mzwsfLevi.LeviSendUnitMsg(unitSrc, *this, fcolVals);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
                bRet = kCIDLib::False;
            }
        }
    }
     else
    {
        bRet = kCIDLib::False;
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Multi-instance/class association is not yet implemented. Unit=%(1)"
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotSupported
            , unitSrc.strName()
        );
    }
    return bRet;

}


tCIDLib::TBoolean
TZWCCAssociation::bDelAssociation(          TZWaveUnit&         unitSrc
                                    , const tCIDLib::TCard4     c4InstId
                                    , const tCIDLib::TCard4     c4GroupId
                                    , const TZWaveUnit&         unitRemove
                                    ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    tCIDLib::TBoolean   bRet = kCIDLib::True;
    tCIDLib::TCardList  fcolVals(16);

    if (eMultiType() == tZWaveLevi2Sh::EMultiTypes::Single)
    {
        CIDAssert(c4InstId == 0, L"Inst id should be zero for single instance class");

        fcolVals.c4AddElement(COMMAND_CLASS_ASSOCIATION);
        fcolVals.c4AddElement(ASSOCIATION_REMOVE);
        fcolVals.c4AddElement(c4GroupId);
        fcolVals.c4AddElement(unitRemove.c4Id());

        if (unitSrc.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp))
        {
            TZWQCmd zwqcNew(0, tZWaveLevi2Sh::ECClasses::Association, fcolVals);
            unitSrc.QueueUpCmd(zwqcNew, mzwsfLevi);
        }
         else
        {
            try
            {
                mzwsfLevi.LeviSendUnitMsg(unitSrc, *this, fcolVals);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
                bRet = kCIDLib::False;
            }
        }
    }
     else
    {
        bRet = kCIDLib::False;
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Multi-instance/class association is not yet implemented. Unit=%(1)"
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotSupported
            , unitSrc.strName()
        );
    }
    return bRet;

}



//
//  Add the target unit's id (and optionally instance id) to the indicated group.
//  If the instance id is zero, then we do a standard association. Else we do a multi-
//  instance type. If we implement wakeup, we just queue the cmd for later sending.
//
tCIDLib::TBoolean
TZWCCAssociation::bDoAssociation(       TZWaveUnit&         unitSrc
                                , const tCIDLib::TCard4     c4InstId
                                , const TZWaveUnit&         unitTar
                                , const tCIDLib::TCard4     c4GroupId
                                ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    tCIDLib::TBoolean   bRet = kCIDLib::True;
    tCIDLib::TCardList  fcolVals(16);

    if (eMultiType() == tZWaveLevi2Sh::EMultiTypes::Single)
    {
        CIDAssert(c4InstId == 0, L"Inst id should be zero for single instance class");

        fcolVals.c4AddElement(COMMAND_CLASS_ASSOCIATION);
        fcolVals.c4AddElement(ASSOCIATION_SET);
        fcolVals.c4AddElement(c4GroupId);
        fcolVals.c4AddElement(unitTar.c4Id());

        if (unitSrc.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp))
        {
            TZWQCmd zwqcNew(0, tZWaveLevi2Sh::ECClasses::Association, fcolVals);
            unitSrc.QueueUpCmd(zwqcNew, mzwsfLevi);
        }
         else
        {
            try
            {
                mzwsfLevi.LeviSendUnitMsg(unitSrc, *this, fcolVals);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
                bRet = kCIDLib::False;
            }
        }
    }
     else
    {
        bRet = kCIDLib::False;
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Multi-instance/class association is not yet implemented. Unit=%(1)"
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotSupported
            , unitSrc.strName()
        );
    }
    return bRet;
}


// Query the stuff associated with the indicate group in the indicated source unit
tCIDLib::TBoolean
TZWCCAssociation::bQueryAssociation(        TZWaveUnit&         unitSrc
                                    , const tCIDLib::TCard4     c4GroupId
                                    ,       tCIDLib::TCardList& fcolVals
                                    ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    // Set up the message to send. We use the caller's list as a temp
    fcolVals.c4AddElement(COMMAND_CLASS_ASSOCIATION);
    fcolVals.c4AddElement(ASSOCIATION_GET);
    fcolVals.c4AddElement(c4GroupId);

    tCIDLib::TBoolean bRet = kCIDLib::True;
    try
    {
        mzwsfLevi.LeviSendUnitMsg(unitSrc, *this, fcolVals);
        mzwsfLevi.bLeviWaitUnitMsg
        (
            unitSrc, *this, COMMAND_CLASS_ASSOCIATION, ASSOCIATION_REPORT, fcolVals
        );

        // Remove the prefix stuff, leaving the actual id values
        fcolVals.Delete(0, 4);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        bRet = kCIDLib::False;
    }
    return bRet;
}

