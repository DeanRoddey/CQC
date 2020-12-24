//
// FILE NAME: TestCQCKit_FldStorage.hpp
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
//  This is the header file for the field storage tests of the CQCKit test
//  suite. These classes are used to store field values inside drivers and
//  in some other places. They are based on a base class since any code that
//  needs to deal with field values generally needs to be able to deal wtih
//  different field types, and hence manage them polymorphically.
//
//  Since the only thing that changes in the derivatives is the way that the
//  data value is stored, we just do the bulk of tests on one variation, then
//  do a few per-type tests to test out just the differences.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_FldStoreBase
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_FldStoreBase : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_FldStoreBase();

        TTest_FldStoreBase
        (
            const   TString&                strName
            , const TString&                strDescr
            , const tCIDLib::TCard4         c4Level
        );

        TTest_FldStoreBase(const TTest_FldStoreBase&) = delete;
        TTest_FldStoreBase(TTest_FldStoreBase&&) = delete;

        ~TTest_FldStoreBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tTestFWLib::ETestRes eRunTest
        (
                    TTextStringOutStream&   strmOutput
            ,       tCIDLib::TBoolean&      bWarning
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bTestDefValues
        (
                    TTextStringOutStream&   strmOut
            , const TTFWCurLn&              tfwclAt
            , const TCQCFldStore&           cfsToTest
        );

        const TString& strMoniker() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strMoniker
        //      A moniker we use in the store classes
        // -------------------------------------------------------------------
        const TString   m_strMoniker;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_FldStoreBase,TTestFWTest)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_FldStoreBool
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_FldStoreBool : public TTest_FldStoreBase
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_FldStoreBool();

        TTest_FldStoreBool(const TTest_FldStoreBool&) = delete;
        TTest_FldStoreBool(TTest_FldStoreBool&&) = delete;

        ~TTest_FldStoreBool();


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
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_FldStoreBool,TTest_FldStoreBase)
};


