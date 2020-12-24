//
// FILE NAME: ZWaveUSB3Sh_DevInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/03/2018
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
//  This is a small file that represents the information that we have available in
//  our device info files. It can also be used to store information gathered live
//  from the unit. The unit info class has one of these for the device info file
//  it loads. The internal device info cache stores objects of this type. It can
//  be keyed by the manufacturer ids, which is the unique identifier. Note that
//  two files could have the same make/model, since sometimes manufacturers put
//  out the same device with different ids (or so close to the same that it's not
//  something we can easily figure out.)
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWXInfoItem
//  PREFIX: xii
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWXInfoItem : public TObject, public MStreamable
{
    public :
        // ---------------------------------------------------------------------
        //  Constructors and Destructor
        // ---------------------------------------------------------------------
        TZWXInfoItem();

        TZWXInfoItem
        (
            const   TString&                strKey
            , const TString&                strValue
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWXInfoItem(const TZWXInfoItem&) = default;

        ~TZWXInfoItem();


        // ---------------------------------------------------------------------
        //  Public operators
        // ---------------------------------------------------------------------
        TZWXInfoItem& operator=(const TZWXInfoItem&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TZWXInfoItem&           xiiSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWXInfoItem&           xiiSrc
        )   const;


        // ---------------------------------------------------------------------
        //  Public data members
        //
        //    m_c1EPId
        //        If this is for a specific end point, this is the id. If not, it is
        //        0xFF.
        //
        //    m_strKey
        //        The key for this item
        //
        //    m_strValue
        //        The value for this item
        // ---------------------------------------------------------------------
        tCIDLib::TCard1     m_c1EPId;
        TString             m_strKey;
        TString             m_strValue;


    protected :
        // ---------------------------------------------------------------------
        //  Protected, inherited methods
        // ---------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const   override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWXInfoItem,TObject)
};

// ---------------------------------------------------------------------------
//  Slip in a list of extra info objects
// ---------------------------------------------------------------------------
namespace tZWaveUSB3Sh
{
    typedef TVector<TZWXInfoItem>   TXInfoList;
}




// ---------------------------------------------------------------------------
//   CLASS: TZWAssocInfo
//  PREFIX: associ
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWAssocInfo : public TObject, public MStreamable
{
    public :
        // -----------------------------------------------------------
        //  Constructors and Destructor
        // -----------------------------------------------------------
        TZWAssocInfo();

        TZWAssocInfo
        (
            const   TString&        strDesc
            , const tCIDLib::TCard1 c1Group
            , const tCIDLib::TCard1 c1EndPoint
        );

        TZWAssocInfo(const TZWAssocInfo&) = default;

        ~TZWAssocInfo();


        // -----------------------------------------------------------
        //  Public operators
        // -----------------------------------------------------------
        TZWAssocInfo& operator=(const TZWAssocInfo&) = default;

        tCIDLib::TBoolean operator==
        (
            const TZWAssocInfo&       associSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const TZWAssocInfo&       associSrc
        )   const;


        // -----------------------------------------------------------
        //  Public data members
        // -----------------------------------------------------------
        tCIDLib::TCard1 m_c1EndPoint;
        tCIDLib::TCard1 m_c1Group;
        TString         m_strDesc;


    protected :
        // -----------------------------------------------------------
        //  Protected, inherited methods
        // -----------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const   override;


    private :
        // -----------------------------------------------------------
        //  Magic macros
        // -----------------------------------------------------------
        RTTIDefs(TZWAssocInfo, TObject)
};

// ---------------------------------------------------------------------------
//  Slip in a list of association objects
// ---------------------------------------------------------------------------
namespace tZWaveUSB3Sh
{
    typedef TVector<TZWAssocInfo>   TAssocList;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWCfgParam
//  PREFIX: cfgp
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWCfgParam : public TObject, public MStreamable
{
    public :
        // -----------------------------------------------------------
        //  Constructors and Destructor
        // -----------------------------------------------------------
        TZWCfgParam();

        TZWCfgParam
        (
            const   TString&        strDesc
            , const tCIDLib::TCard1 c1ParamNum
            , const tCIDLib::TInt4  i4Value
            , const tCIDLib::TCard4 c4ByteCnt
        );

        TZWCfgParam(const TZWCfgParam&) = default;

        ~TZWCfgParam();


        // -----------------------------------------------------------
        //  Public operators
        // -----------------------------------------------------------
        TZWCfgParam& operator=(const TZWCfgParam&) = default;

        tCIDLib::TBoolean operator==
        (
            const TZWCfgParam&        cfgpSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const TZWCfgParam&        cfgpSrc
        )   const;


        // -----------------------------------------------------------
        //  Public data members
        // -----------------------------------------------------------
        tCIDLib::TCard1 m_c1ParamNum;
        tCIDLib::TCard4 m_c4ByteCnt;
        tCIDLib::TInt4  m_i4Value;
        TString         m_strDesc;


    protected :
        // -----------------------------------------------------------
        //  Protected, inherited methods
        // -----------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -----------------------------------------------------------
        //  Magic macros
        // -----------------------------------------------------------
        RTTIDefs(TZWCfgParam, TObject)
};

// ---------------------------------------------------------------------------
//  Slip in a list of config parameters objects
// ---------------------------------------------------------------------------
namespace tZWaveUSB3Sh
{
    typedef TVector<TZWCfgParam>    TCfgParamList;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWClassInfo
//  PREFIX: zwci
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWClassInfo : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWClassInfo();

        TZWClassInfo
        (
            const   tZWaveUSB3Sh::ECClasses eClass
        );

        TZWClassInfo
        (
            const   tZWaveUSB3Sh::ECClasses eClass
            , const tCIDLib::TCard4         c4Ver
            , const tCIDLib::TBoolean       bSecure
            , const tZWaveUSB3Sh::TXInfoList& colExtraInfo
        );

        TZWClassInfo(const TZWClassInfo&) = default;

        ~TZWClassInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWClassInfo& operator=(const TZWClassInfo&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TZWClassInfo&           zwciSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWClassInfo&           zwciSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_bSecure
        //      True if this class has been determined to be secure.
        //
        //  m_c4Version
        //      The version of this class if we could obtain it, else defaults to
        //      1.
        //
        //  m_colExtraInfo
        //      If we were set up from a device info file, then this is any extra
        //      info defined for this class. It will be passed to the CC impl
        //      objects to parameterize them.
        //
        //  m_eClass
        //      The particular class this object represents.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bSecure;
        tCIDLib::TCard4             m_c4Version;
        tZWaveUSB3Sh::TXInfoList    m_colExtraInfo;
        tZWaveUSB3Sh::ECClasses     m_eClass;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWClassInfo,TObject)
};

// ---------------------------------------------------------------------------
//  Slip in a list of class info objects
// ---------------------------------------------------------------------------
namespace tZWaveUSB3Sh
{
    typedef TVector<TZWClassInfo>       TCInfoList;
}




// ---------------------------------------------------------------------------
//   CLASS: TZWDevInfo
//  PREFIX: uniti
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWDevInfo :   public TObject
                                    , public MStreamable
                                    , public MFormattable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompByManIds
        (
            const   TZWDevInfo&             devi1
            , const TZWDevInfo&             devi2
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWDevInfo();

        TZWDevInfo(const TZWDevInfo&) = default;

        ~TZWDevInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWDevInfo& operator=(const TZWDevInfo&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TZWDevInfo&            deviSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWDevInfo&            deviSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFindExtraVal
        (
            const   TString&                strKey
            ,       TString&                strValue
        )   const;

        tCIDLib::TBoolean bFindExtraVal
        (
            const   tZWaveUSB3Sh::ECClasses eClass
            , const tCIDLib::TCard1         c1EndPnt
            , const TString&                strKey
            ,       TString&                strValue
        )   const;

        tCIDLib::TBoolean bIsValid() const;

        tCIDLib::TBoolean bSecureClass
        (
            const   tZWaveUSB3Sh::ECClasses eClass
        )   const;

        tCIDLib::TBoolean bSupportsClass
        (
            const   tZWaveUSB3Sh::ECClasses eClass
        )   const;

        tCIDLib::TBoolean bSupportsClass
        (
            const   tZWaveUSB3Sh::ECClasses eClass
            , const tCIDLib::TCard4         c4Version
        )   const;

        tCIDLib::TCard4 c4ClassVersion
        (
            const   tZWaveUSB3Sh::ECClasses eClass
            , const tCIDLib::TBoolean       bThrowIfNot
            , const TString&                strName
        )   const;

        tCIDLib::TCard4 c4FindCCIndex
        (
            const   tZWaveUSB3Sh::ECClasses eFind
        )   const;

        tCIDLib::TCard4 c4WakeupInt() const
        {
            return m_c4WakeupInt;
        }

        tCIDLib::TCard8 c8ManIds() const;

        const tZWaveUSB3Sh::TAssocList& colAssociations() const
        {
            return m_colAssociations;
        }

        const tZWaveUSB3Sh::TCfgParamList& colCfgParams() const
        {
            return m_colCfgParams;
        }

        const tZWaveUSB3Sh::TCInfoList& colCCs() const
        {
            return m_colCCs;
        }

        tCIDLib::TVoid FormatAutoCfg
        (
                    TTextOutStream&         strmTar
        )   const;

        tCIDLib::TVoid ParseFrom
        (
            const   TXMLTreeElement&        xtnodeDev
        );

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TBoolean       bRescan
        );

        const TString& strMake() const
        {
            return m_strMake;
        }

        const TString& strModel() const
        {
            return m_strModel;
        }

        const TString& strUnitClass() const
        {
            return m_strUnitClass;
        }

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TCard8         c8ManIds
            , const tZWaveUSB3Sh::TCInfoList& colClasses
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatTo
        (
                    TTextOutStream&         strmDest
        )   const   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4WakeupInt
        //      If it's a battery powered, non-listener, they can set a wakeup interval
        //      for us to set as part of the auto-config. Often the default wakeup is
        //      very long, which means it will be a very long time before the driver
        //      can get field info from the unit after a driver start. So it may be
        //      reduced, at some cost in battery life. If zero, it's not used.
        //
        //  m_c8ManIds
        //      The manufacturer/type/product ids combined in our standard way. This
        //      is the unique key.
        //
        //  m_colAssociations
        //      Any associations from the auto-config section of the device info.
        //
        //  m_colCfgParms
        //      Any configuration parameters from the auto-config section of the device
        //      info.
        //
        //  m_colCCs
        //      The command classes supported by this unit.
        //
        //  m_colExtraInfo
        //      Extra info that is unit type specific. This stuff allows generic handlers
        //      to be parameterized via the device info file. Both the overall unit and
        //      command classes can have their own. Each entry has an endpoint indicator
        //      as well, so that multiple instances of a CC can be independently
        //      targeted, whic is necessary if you have more than one of the same CC
        //      in a unit.
        //
        //  m_strMake
        //  m_strModel
        //      The make/model of unit represents by our device info file.
        //
        //  m_strUnitClass
        //      The name of the class that implements the handler for the particular
        //      unit represented by this config info.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4WakeupInt;
        tCIDLib::TCard8             m_c8ManIds;
        tZWaveUSB3Sh::TAssocList    m_colAssociations;
        tZWaveUSB3Sh::TCfgParamList m_colCfgParams;
        tZWaveUSB3Sh::TCInfoList    m_colCCs;
        tZWaveUSB3Sh::TXInfoList    m_colExtraInfo;
        TString                     m_strMake;
        TString                     m_strModel;
        TString                     m_strUnitClass;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWDevInfo, TObject)
};

#pragma CIDLIB_POPPACK
