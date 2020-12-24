//
// FILE NAME: ZWaveLevi2Sh_DevInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/23/2014
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
//  This is the header file for the ZWaveLevi2Sh_DevInfo.cpp file, which wraps the
//  info in the XML files that describe each Z-Wave unit. We parse that info in and
//  store it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWDevClass
//  PREFIX: zwdc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWDevClass
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eComp
        (
            const   TZWDevClass&            zwdc1
            , const TZWDevClass&            zwdc2
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWDevClass();

        TZWDevClass
        (
            const   TZWDevClass&            zwdcSrc
        );

        ~TZWDevClass();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWDevClass& operator=
        (
            const   TZWDevClass&            zwdcSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid QueryOptDef
        (
                    TZWOptionVal&           zwoptvTar
            , const TString&                strKey
            , const TString&                strDef
        )   const;

        tCIDLib::TVoid Reset
        (
            const   tZWaveLevi2Sh::ECClasses eClass
        );

        tCIDLib::TBoolean           m_bAllZeroInst;
        tCIDLib::TBoolean           m_bCanRead;
        tCIDLib::TBoolean           m_bCanWrite;
        tCIDLib::TBoolean           m_bPoll;
        tCIDLib::TBoolean           m_bSecure;
        tCIDLib::TCard4             m_c4Flags;
        tCIDLib::TCard4             m_c4InstId;
        tCIDLib::TCard4             m_c4PollMins;
        tCIDLib::TCard4             m_c4SetRespCls;
        tCIDLib::TCard4             m_c4SetRespCmd;
        TVector<TZWOptionVal>       m_colOpts;
        tZWaveLevi2Sh::ECClasses    m_eClass;
        tZWaveLevi2Sh::EMultiTypes  m_eMultiType;
        TString                     m_strDescr;
        TString                     m_strName;
};



// ---------------------------------------------------------------------------
//   CLASS: TZWDevInfo
//  PREFIX: zwdi
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWDevInfo
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TZWDevInfo&             zwdiSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWDevInfo();

        TZWDevInfo
        (
            const   TZWDevInfo&             zwdiSrc
        );

        ~TZWDevInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWDevInfo& operator=
        (
            const   TZWDevInfo&             zwdiSrc
        );



        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseClass
        (
            const   TXMLTreeElement&        xtnodeClass
        );

        tCIDLib::TVoid ParseFromXML
        (
            const   TString&                strFileName
            , const TXMLTreeElement&        xtnodeRoot
        );

        tCIDLib::TVoid Reset();



        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bAES128
        //  m_bBeam
        //  m_bNetSec
        //      Some flags indicating whether this device implements some optional
        //      features.
        //
        //  m_bLoaded
        //      Used by the containing unit objects to know if they have loaded up
        //      their device info yet, since they only store the device info file name,
        //      and load this info at runtime as required.
        //
        //  m_coClasses
        //      All of the classes declared for this device type. It allows us to
        //      deal with multi-instance devices as well, by indicating within any
        //      given class what those instances are, and providing optional pre-
        //      config of options. Even if not multi-instance, sometimes it's used
        //      to pre-configure default options for the single instance.
        //
        //  m_colAutoCfg
        //      The auto-config info. It's a list of key/values. The key is CfgParm
        //      Grp, or SceneCfg, to indicate the type of operation. The first
        //      value is the descriptive text. Then:
        //
        //          Grp : v2=group number
        //          CfgParm : v2=parm number v3=value/bytecnt
        //          SceneCfg : v2=Group id v3=scene id
        //
        //  m_eBaseClass
        //      The base command class for this device. This controls what command
        //      class basic reports are mapped to.
        //
        //  m_eGenType
        //  m_eSpecType
        //      The generric and specific unit types, which indicates the overall
        //      capabilities of the device. The specific type can be None, which means
        //      we just go by the generic type.
        //
        //  m_fcolGroups
        //      The groups that we want to make sure are associated with the VRCOP.
        //
        //  m_strFileName
        //      We store the file name we were loaded from, so that it can be used as
        //      a unique key.
        //
        //  m_strDevName
        //      The overall device name from the info file, mostly just for display
        //      to the user or in error messages.
        //
        //  m_strMake
        //  m_strModel
        //      This is the unique key for this device type. This is what the driver
        //      stores with unit configuration and uses to come back and find this
        //      info upon reload.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bAES128;
        tCIDLib::TBoolean           m_bBeam;
        tCIDLib::TBoolean           m_bLoaded;
        tCIDLib::TBoolean           m_bNetSec;
        tZWaveLevi2Sh::TAutoCfgList m_colAutoCfg;
        TVector<TZWDevClass>        m_colClasses;
        tZWaveLevi2Sh::ECClasses    m_eBaseClass;
        tZWaveLevi2Sh::EGenTypes    m_eGenType;
        tZWaveLevi2Sh::ESpecTypes   m_eSpecType;
        tCIDLib::TCardList          m_fcolGroups;
        TString                     m_strFileName;
        TString                     m_strDevName;
        TString                     m_strMake;
        TString                     m_strModel;
};


#pragma CIDLIB_POPPACK

