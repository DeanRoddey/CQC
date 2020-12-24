//
// FILE NAME: ZWaveLevi2Sh_UnitInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2014
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
//  This is the header file for the ZWaveLevi2Sh_UnitInfo.cpp file, which implements
//  the TZWaveUnit class. This class provides the core functionality that we handle
//  for all units. Then we add command class handlers to each instance in order to
//  deal with the declared functionality of the device based on those command classes.
//  We don't really pay much attention to the generic/specific types. It's all driven
//  by the command classes implemented.
//
// CAVEATS/GOTCHAS:
//
//  1)  ONLY PERSISTENT values are copied in the copy ctor and assignment operator.
//      We don't want to copy all of the dynamically generated stuff. That stuff
//      doesn't need to be streamed around. It's the device info and the command
//      class handlers that are generated based on the command class list in the
//      device info, and the options list which is only needed on the client side
//      to display to the user.
//
//      SO BE SURE you know the context of the user of the unit objects and on't
//      access stuff that hasn't been set up.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWaveUnit : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompById
        (
            const   TZWaveUnit&             unit1
            , const TZWaveUnit&             unit2
        );

        static tCIDLib::ESortComps eCompByName
        (
            const   TZWaveUnit&             unit1
            , const TZWaveUnit&             unit2
        );

        static TZWaveUnit* punitBestMatch
        (
            const   tZWaveLevi2Sh::EGenTypes eGenType
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveUnit();

        TZWaveUnit
        (
            const   tCIDLib::TCard4         c4Id
            , const tZWaveLevi2Sh::EGenTypes eType
        );

        TZWaveUnit
        (
            const   TZWaveUnit&             unitSrc
        );

        ~TZWaveUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveUnit& operator=
        (
            const   TZWaveUnit&             unitSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWaveUnit&             unitSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWaveUnit&             unitSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bActivePoll
        (
                    MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TBoolean bClassIsSecure
        (
            const   tZWaveLevi2Sh::ECClasses eToTest
        )   const;

        tCIDLib::TBoolean bDevInfoLoaded() const;

        tCIDLib::TBoolean bDisabled() const;

        tCIDLib::TBoolean bDisabled
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bFailed() const;

        tCIDLib::TBoolean bFailed
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bMissing() const;

        tCIDLib::TBoolean bMissing
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bHandleGenReport
        (
            const   tCIDLib::TCard4         c4InstId
            , const tCIDLib::TCardList&     fcolVals
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TBoolean bImplementsClass
        (
            const   tZWaveLevi2Sh::ECClasses eToTest
        )   const;

        tCIDLib::TBoolean bIsViable() const;

        tCIDLib::TBoolean bLoadDevInfo
        (
                    TXMLTreeParser&         xtprsTouse
        );

        tCIDLib::TBoolean bOptValIsFixed
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4CmdClassInd
        )   const;

        tCIDLib::TBoolean bQueryFldVals
        (
                    MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TBoolean bQueryOpts
        (
                    tZWaveLevi2Sh::TOptList& colToFill
        )   const;

        tCIDLib::TCard4 c4ClassInstCount
        (
            const   tZWaveLevi2Sh::ECClasses eToTest
        )   const;

        tCIDLib::TCard4 c4Id() const;

        tCIDLib::TCard4 c4Id
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        const tCIDLib::TStrList& colAllowedOptVals
        (
            const   TString&                strKey
        )   const;

        tZWaveLevi2Sh::ECClasses eBaseClass() const;

        tZWaveLevi2Sh::EGenTypes eGenType() const;

        tZWaveLevi2Sh::ESpecTypes eSpecType() const;

        tCQCKit::ECommResults eWriteBoolFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4ClassInd
        );

        tCQCKit::ECommResults eWriteCardFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4ClassInd
        );

        tCQCKit::ECommResults eWriteIntFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4Value
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4ClassInd
        );

        tCQCKit::ECommResults eWriteFloatFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8Value
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4ClassInd
        );

        tCQCKit::ECommResults eWriteStringFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4ClassInd
        );

        tCIDLib::TEncodedTime enctLastValue() const;

        tCIDLib::TVoid InitVals
        (
                    MZWLeviSrvFuncs&        mzwsfLevi
        );

        TZWCmdClass* pzwccFind
        (
            const   tZWaveLevi2Sh::ECClasses eClass
            , const tCIDLib::TCard4         c4InstId
        );

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        tCIDLib::TVoid QueueUpCmd
        (
            const   TZWQCmd&                qcmdToAdd
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TVoid ResetLastValTime();

        tCIDLib::TVoid ResetUnitType
        (
            const   tZWaveLevi2Sh::EGenTypes eNew
        );

        const TString& strDevInfoFile() const;

        const TString& strFindOptVal
        (
            const   TString&                strKey
        )   const;

        const TString& strMake() const;

        const TString& strModel() const;

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetDevInfoFile
        (
            const   TString&                strFileName
        );

        tCIDLib::TVoid SetOptVal
        (
            const   TString&                strOptKey
            , const TString&                strValKey
        );

        tCIDLib::TVoid StoreFldIds
        (
            const   MZWLeviSrvFuncs&        mzwsfLevi
        );

        const TZWDevInfo& zwdiThis() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual method
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckFldId
        (
            const   tCIDLib::TCard4         c4ToCheck
            , const tCIDLib::TCh* const     pszFldName
        )   const;

        tCIDLib::TBoolean bSetAssoc
        (
                    MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4TarId
            , const tCIDLib::TCard4         c4GroupId
        );

        tCIDLib::TBoolean bSetWakeupInt
        (
                    MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4VRCOPId
            , const tCIDLib::TCard4         c4Seconds
        );

        tCIDLib::TVoid CreateClassObjs();

        tCIDLib::TVoid SetDefName();



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDisabled
        //      The driver doesn't have to interact with all of the units available on
        //      the Z-Wave network. If the user doesn't need a given unit to be controlled,
        //      he can mark it disabled. So we always have the known units in the config,
        //      but we can ignore some if desired. This is persisted of course.
        //
        //  m_bFailed
        //      It was otherwise ok, but failed to create its fields or we couldn't load
        //      the device info. This is runtime but still persisted because we need it
        //      get to the client when he queries our config.
        //
        //  m_bMissing
        //      It wasn't found during the last network scan. This is runtime but still
        //      persisted because we need it get to the client when he queries our config.
        //
        //  m_c4Id
        //      The id of the unit, which is what we need to actually communicate with
        //      the unit at the Z-Wave level.
        //
        //  m_colCmdClasses
        //      The list of command classes that this unit implements. Each one is a
        //      derivative of the command base class. Only the classes implemented are
        //      added, but they arealways in the order of the command class enum.
        //
        //      This is only faulted in upon use in the driver. Otherwise they aren't
        //      needed. So they aren't streamed over to the client or back. They only
        //      hold runtime values. It is driven by the device info which is also
        //      only faulted in upon requirement, based on the device info file name
        //      below.
        //
        //  m_colOptVals
        //      The user selections for all of the options. There is one entry here for
        //      each option that the installed command classes implement. The key for
        //      each one is the private key for that option and the value is the key for
        //      the option value (both set by the command class for its internal use.)
        //      It is a keyed hash set keyed on the option key, for quickly finding
        //      particular options.
        //
        //  m_eGenType
        //      The generic type reported during the network scan. When the dev info is
        //      set on us, the generic type reported there must match this or it will be
        //      rejected. This is persisted.
        //
        //  m_enctLastValue
        //      We remember the last time we got a msg from the unit.
        //
        //  m_strDevInfoFile
        //      The name of the file that contains the device info for this particular
        //      unit. This is used (by the overall config class) to load back up the
        //      device info after the units are streamed back in. The user sets this.
        //
        //  m_strName
        //      The name that the user gave this unit. Persisted.
        //
        //  m_zwdiThis
        //      The device info. This is runtime only. The dev info file above is
        //      used to reload this information when the driver loads up the config.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bDisabled;
        tCIDLib::TBoolean           m_bFailed;
        tCIDLib::TBoolean           m_bMissing;
        tCIDLib::TCard4             m_c4Id;
        tZWaveLevi2Sh::TCmdIntfList m_colCmdClasses;
        tZWaveLevi2Sh::TOptValList  m_colOptVals;
        tZWaveLevi2Sh::TZWCmdQ      m_colCmdQ;
        tZWaveLevi2Sh::EGenTypes    m_eGenType;
        tCIDLib::TEncodedTime       m_enctLastValue;
        TString                     m_strDevInfoFile;
        TString                     m_strName;
        TZWDevInfo                  m_zwdiThis;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUnit,TObject)
};

#pragma CIDLIB_POPPACK


