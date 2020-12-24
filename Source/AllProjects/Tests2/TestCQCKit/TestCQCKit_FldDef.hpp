//
// FILE NAME: TestCQCKit_FldDef.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/11/2008
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
//  This is the header file for the field definition tests of the CQCKit test
//  suite. These classes are used to define the attributes of driver fields.
//  Drivers create a list of these and set them on the driver. They are also
//  used to report those attributes back to anyone who iterates the fields
//  of a driver. It doesn't do much. It just exists to store data.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_FldDef
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_FldDef : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_FldDef();

        TTest_FldDef(const TTest_FldDef&) = delete;
        TTest_FldDef(TTest_FldDef&&) = delete;

        ~TTest_FldDef();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tTestFWLib::ETestRes eRunTest
        (
                    TTextStringOutStream&   strmOutput
            ,       tCIDLib::TBoolean&      bWarning
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bTestVals
        (
                    TTextStringOutStream&   strmOut
            , const TTFWCurLn&              tfwclAt
            , const TCQCFldDef&             flddToTest
            , const TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCIDLib::TCh* const     pszLimits
            , const tCIDLib::TBoolean       bAlwaysWrite
            , const tCIDLib::TBoolean       bQueuedWrite
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCh* const     pszExtraCfg
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_FldDef,TTestFWTest)
};


