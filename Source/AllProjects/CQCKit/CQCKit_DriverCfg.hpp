//
// FILE NAME: CQCKit_DriverCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/23/2000
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
//  This is the header for the CQCKit_DriverCfg.cpp file, which implements
//  the TCQCDriverCfg class. Instances of this class represent the base
//  configuration for a CQC device driver. Each driver is packaged along with
//  a 'manifest file', which is in XML format, that contains all of the
//  information that the installation server and other install/setup programs
//  need to understand the driver's layout. This class can parse those files
//  and provide access to the infromation in object format.
//
//  Note that these manifest files use a DTD that is hard coded into this
//  facility, in the CQCKit_DriverCfgDTD.cpp file. So manifest files must
//  use a PUBLIC ID for its DTD, using the correct public identifier. This
//  allows us to correctly map it to our internalized DTD. The DTD line must
//  be:
//
//  <!DOCTYPE CQCCfg:DriverCfg
//            PUBLIC "urn:charmedquark.com:CQC-DrvManifest.DTD" "CQCDriverCfg.DTD">
//
//  So a URN is provided, which this class will use to map it to the actual
//  internal DTD. The system indentifier is just a place holder that will be
//  displayed in errors, so that its obvious what the error is from.
//
//  There is a class derived from this one, called TCQCDriverObjCfg, which
//  extends this class to hold the information required for a particular
//  instance of a driver type. That class is used by CQCServer and CQCClient
//  programs to communicate about what drivers are loaded by a particular
//  CQCServer process, and is what is stored on the server side config server
//  to remember what drivers are stored.
//
//  We also define here a small class that is used to hold any optional cfg
//  info prompts in the manifest file. These are used to get the user to
//  provide configuration information needed by the driver in order to
//  install. It has it's limitations, and some drivers will just have to
//  wait for the user to run a client driver and do extensive configuration.
//  This is just to support those drivers, particularly macro based drivers,
//  which need small amounts of user info.
//
// CAVEATS/GOTCHAS:
//
//  1)  Server side connnection configuration, which indicates how the driver
//      connects to the device and the settings thereof, is polymorphic since
//      this object represents all possible devices.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TXMLTreeElement;
class TXMLEntitySrc;


// ---------------------------------------------------------------------------
//   CLASS: TCQCDCfgPrompt
//  PREFIX: cqcdcp
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCDCfgPrompt : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TDefList = TVector<TKeyValuePair>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDCfgPrompt();

        TCQCDCfgPrompt
        (
            const   TString&                strValName
            , const TString&                strTitle
            , const TString&                strDescrText
            , const TString&                strLimits
            , const TString&                strValNames
            , const tCQCKit::EPromptTypes   eType
            , const tCIDLib::TBoolean       bOptional
            , const tCIDLib::TBoolean       bFlag
        );

        TCQCDCfgPrompt(const TCQCDCfgPrompt&) = default;
        TCQCDCfgPrompt(TCQCDCfgPrompt&&) = default;

        ~TCQCDCfgPrompt() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDCfgPrompt& operator=(const TCQCDCfgPrompt&) = default;
        TCQCDCfgPrompt& operator=(TCQCDCfgPrompt&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCDCfgPrompt&         cqcdcpSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCDCfgPrompt&         cqcdcpSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddDefault
        (
            const   TString&                strKey
            , const TString&                strValue
        );

        tCIDLib::TBoolean bDefaultFor
        (
            const   TString&                strSubKey
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bFlag() const;

        tCIDLib::TBoolean bOptional() const;

        tCIDLib::TCard4 c4DefCount() const;

        tCIDLib::TInt4 i4MaxValue() const;

        tCIDLib::TInt4 i4MinValue() const;

        tCQCKit::EPromptTypes eType() const;

        tCQCKit::EPromptTypes eType
        (
            const   tCQCKit::EPromptTypes   eToSet
        );

        const TKeyValuePair& kvalDefaultAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TString& strDescrText() const;

        const TString& strDescrText
        (
            const   TString&                strToSet
        );

        const TString& strTitle() const;

        const TString& strTitle
        (
            const   TString&                strToSet
        );

        const TString& strLimits() const;

        const TString& strLimits
        (
            const   TString&                strToSet
        );

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );

        const TString& strValNames() const;

        tCIDLib::TVoid Set
        (
            const   TString&                strNameToSet
            , const TString&                strTitleToSet
            , const TString&                strDescrToSet
            , const TString&                strLimits
            , const TString&                strValNames
            , const tCQCKit::EPromptTypes   eType
            , const tCIDLib::TBoolean       bOptional
            , const tCIDLib::TBoolean       bFlag
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreRange
        (
            const   TString&                strValue
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bFlag
        //      A general purpose flag that prompts can use to store an extra
        //      flag. It only means something to the prompt object that handles
        //      that type of prompt.
        //
        //  m_bOptional
        //      Some prompts will allow the user not to enter anything, in
        //      which case this is set to indicate it's optional. It is
        //      defaulted to False, so they are only optional if explicitly
        //      indicated.
        //
        //  m_colDefaults
        //      Any default values that were defined for this prompt. It is
        //      a vector of key/value pairs, where the key is the name of
        //      the sub-key and the value is the default value.
        //
        //  m_eType
        //      The type of the prompt, which tells us how to interpret the
        //      other fields.
        //
        //  m_i4Max
        //  m_i4Min
        //      Min/max limits for a range based prompt. The entered value
        //      must be numeric and within this range, inclusive.
        //
        //  m_strDescrText
        //      The descriptive text to show to the user, to give them extra
        //      info about what to provide.
        //
        //  m_strLimits
        //      Used to store limits info such as reg ex for text prompt,
        //      or list of items for a list prompt, etc...
        //
        //  m_strName
        //      The name of the prompt. When this information is stored in
        //      the config repository and when it's passed to the driver,
        //      each value needs to be identified.
        //
        //  m_strTitle
        //      The title of the panel that will be displayed to get this
        //      value.
        //
        //  m_strValNames
        //      A comma separated list of the values that this prompt will
        //      contain. Most just contain one, but they can contain multiple
        //      values.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bFlag;
        tCIDLib::TBoolean       m_bOptional;
        tCIDLib::TBoolean       m_bValidate;
        TDefList                m_colDefaults;
        tCQCKit::EPromptTypes   m_eType;
        tCIDLib::TInt4          m_i4Max;
        tCIDLib::TInt4          m_i4Min;
        TString                 m_strDescrText;
        TString                 m_strLimits;
        TString                 m_strName;
        TString                 m_strTitle;
        TString                 m_strValNames;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDCfgPrompt,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCDriverCfg
//  PREFIX: cqcdc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCDriverCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TPromptList =  TVector<TCQCDCfgPrompt>;
        using TPromptVals =  TNamedValMap<TString>;
        using TValCursor =  TPromptVals::TValCursor;
        using TPromptCursor =  TPromptVals::TItemCursor;
        using TPair =  TPromptVals::TPair;


        // -------------------------------------------------------------------
        //  Some pre-fab comparators on different fields
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompOnDisplayName
        (
            const   TCQCDriverCfg&          cqcdc1
            , const TCQCDriverCfg&          cqcdc2
        );

        static tCIDLib::ESortComps eCompOnMake
        (
            const   TCQCDriverCfg&          cqcdc1
            , const TCQCDriverCfg&          cqcdc2
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDriverCfg();

        TCQCDriverCfg
        (
            const   TString&                strSrcFile
        );

        TCQCDriverCfg
        (
            const   TString&                srcName
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4DataSz
        );

        TCQCDriverCfg
        (
            const   TCQCDriverCfg&          cqcdcSrc
        );

        TCQCDriverCfg(TCQCDriverCfg&&) = delete;

        ~TCQCDriverCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDriverCfg& operator=
        (
            const   TCQCDriverCfg&          cqcdcSrc
        );

        TCQCDriverCfg& operator=(TCQCDriverCfg&&) = delete;

        tCIDLib::TBoolean operator==
        (
            const   TCQCDriverCfg&          cqcdcSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCDriverCfg&          cqcdcSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid NewConnCfg
        (
                    TCQCConnCfg* const      pconncfgNew
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAsyncMode() const;

        tCIDLib::TBoolean bAsyncFldOver() const;

        tCIDLib::TBoolean bEditableConn() const;

        tCIDLib::TBoolean bFindFixedVal
        (
            const   TString&                strKey
            , const TString&                strSubKey
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bHasCapability
        (
            const   tCQCKit::EDrvCaps       eToCheck
        )   const;

        tCIDLib::TBoolean bHasExtraFacs() const;

        tCIDLib::TBoolean bHasPrompts() const;

        tCIDLib::TBoolean bImplementsClass
        (
            const   tCQCKit::EDevClasses    eToCheck
        )   const;

        tCIDLib::TBoolean bSameDriver
        (
            const   TCQCDriverCfg&          cqcdcToCompare
        )   const;

        tCIDLib::TCard4 c4ArchVersion() const;

        tCIDLib::TCard4 c4FixedCount() const;

        tCIDLib::TCard4 c4MajVersion() const;

        tCIDLib::TCard4 c4MinVersion() const;

        tCIDLib::TCard4 c4PromptCount() const;

        const TClass& clsConfigType() const;

        TBag<TString>::TCursor cursExtraFilesC() const;

        TBag<TString>::TCursor cursExtraFilesS() const;

        TBag<TCQCModuleInfo>::TCursor cursExtraFacs() const;

        TPromptCursor cursFixed() const;

        TValCursor cursFixedVals
        (
            const   TString&                strKey
        )   const;

        const TCQCConnCfg& conncfgDef() const;

        TCQCConnCfg& conncfgDef();

        tCQCKit::EDevCats eCategory() const;

        tCQCKit::EDrvTypes eType() const;

        tCQCKit::TDevClassList fcolDevClasses() const;

        const TPromptVals& nvmFixed() const;

        tCIDLib::TVoid ParseFrom
        (
            const   TXMLEntitySrc* const    pxsrcConfig
        );

        const TCQCDCfgPrompt& cqcdcpAt
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        TCQCDCfgPrompt& cqcdcpAt
        (
            const   tCIDLib::TCard4         c4Index
        );

        const TString& strAuthor() const;

        const TString& strAutoSenseClass() const;

        const TString& strClientExtraCfg() const;

        const TString& strClientLibName() const;

        const TString& strContactEMail() const;

        const TString& strDescription() const;

        const TString& strDisplayName() const;

        const TString& strDynRef() const;

        const TString& strFixedValue
        (
            const   TString&                strKey
            , const TString&                strSubKey
        )   const;

        const TString& strMake() const;

        const TString& strModel() const;

        const TString& strServerExtraCfg() const;

        const TString& strServerLibName() const;

        const TString& strVersion() const;

        tCIDLib::TVoid SetFixedValue
        (
            const   TString&                strKey
            , const TString&                strSubKey
            , const TString&                strNewValue
        );


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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseFixedCfg
        (
            const   TXMLTreeElement&            xtnodeData
        );

        tCIDLib::TVoid ParseFromNode
        (
            const   TXMLTreeElement&            xtnodeDriverCfg
        );

        tCIDLib::TVoid ParseOtherCfg
        (
            const   TXMLTreeElement&            xtnodeData
        );

        tCIDLib::TVoid ParsePrompts
        (
            const   TXMLTreeElement&            xtnodeData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bEditableConn
        //      Indicates whether the user should be allowed to edit the
        //      connection config or not.
        //
        //  c4ArchVersion
        //      The driver architecture version. This was added in the run
        //      up to CQC version 4.0, to allow for the co-existence of new
        //      and old drivers. It will be defaulted for 1 for old style
        //      drivers, and set to 2 for new ones.
        //
        //  m_c4CapFlags
        //      The driver capabilities flags, set from the server side
        //      part of the manifest info. This is a bitmask indexed by
        //      the tCQCKit::EDrvCaps enum.
        //
        //  m_c4MajVersion
        //  m_c4MinVersion
        //      The version of the driver that was configured. This will be
        //      used to load the driver DLL/SharedLib via the standard CIDLib
        //      mechanisms which support versioned DLL/SharedLib names. The
        //      client and server drivers must have the same version number,
        //      so we only need to store one set of values.
        //
        //  m_colPrompts
        //      Holds any config prompts that are defined in the manifest,
        //      and used to get config info from the user at load time.
        //
        //  m_eCategory
        //      The CQC device category that the device controlled by this
        //      driver belongs in.
        //
        //  m_eType
        //      The type of driver. This indicates how to interpret the
        //      other fields. Most are always valid, but some are only used
        //      for particular types of drivers.
        //
        //  m_fcolDevClasses
        //      The list of device classes that this driver implements.
        //
        //  m_nvmFixed
        //      Holds any fixed config items from the manifest file. These
        //      are like the prompts, but no user input is required. The
        //      are set by the manifest file with known values.
        //
        //  m_pcolExtraFilesC
        //  m_pcolExtraFilesS
        //      These are lists of extra files that should be copied over when
        //      an instance of this driver is pushed out to a CQCServer or
        //      CQCClient. They will be placed in the same directory as the
        //      facility.
        //
        //  m_pcolExtraFacs
        //      A list of shared facilities that this driver shares between
        //      the client and server sides. They and their associated files
        //      will be copied to a common directory that is in the path of
        //      both the CQCServer and CQCClient. If they are both installed
        //      on the same machine, they will be shared.
        //
        //  m_pconncfgDef
        //      The default connection configuration info that was read from
        //      the manifest file. This is not changed. The derived class
        //      that holds the per-instance config has another copy that
        //      is initialized to the same as the default, but which then
        //      can be modified. This is a polymorphic member of course.
        //
        //  m_strAuthor
        //      Indicates the author of the driver. The DTD will default it
        //      to Charmed Quark Systems, Ltd if none is provided explicitly
        //      in the manifest.
        //
        //  m_strAutoSenseClass
        //      The optional auto-sensing class path.
        //
        //  m_strClassPath
        //      If this is a macro based driver, then this will indicate the
        //      macro class path of the driver class. Else, it is not used.
        //
        //  m_strClientExtraCfg
        //      The optional extra config, which is just text that gets
        //      passed straight through to the client driver instance. What
        //      it means is up to the driver developer.
        //
        //  m_strClientLibName
        //      This is the client side library name. Its the base (portable)
        //      part of the name. The full name will be built up using this
        //      plus the version information, to create the actual platform
        //      specific name.
        //
        //  m_strContactEMail
        //      The contact e-mail for this driver. The DTD will default it
        //      to the CQS support address if not provided explicitly in
        //      the manifest file.
        //
        //  m_strDescription
        //      This is the description of the driver, which shouldn't be
        //      too long, but can be more than one line long. It just describes
        //      the driver for display to humans. This is used both for the
        //      client and server side oriented displays.
        //
        //  m_strDisplayName
        //      This the driver name that is shown to the user. The library
        //      names can be cryptic and aren't optimum for users. This should
        //      be short and to the point, usually 64 chars or less.
        //
        //  m_strDynRef
        //      A special dynamic class reference can be set in the server side
        //      config. This is only used by CML drivers. The value will be set as
        //      the special dynamic reference value on the engine that runs that
        //      driver. So it can be used to import a particular class based on
        //      this manifest setting. It's used to support a range of models with
        //      a single main class and a set of manifest driven helper classes.
        //
        //  m_strMake
        //  m_strModel
        //      The make and model that was configured. This allows a single
        //      driver to support multiple makes of hardware from a company
        //      or multiple models of the same line.
        //
        //  m_strServerExtraCfg
        //      The optional extra config, which is just text that gets
        //      passed straight through to the server driver instance. What
        //      it means is up to the driver developer.
        //
        //  m_strServerLibName
        //      This is the server side library name. Its the base (portable)
        //      part of the name. The full name will be built up using this
        //      plus the version information, to create the actual platform
        //      specific name.
        //
        //  m_strVersion
        //      We build a string version of the version for convenience, to
        //      avoid making everyone format it.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean               m_bAsyncMode;
        tCIDLib::TBoolean               m_bAsyncFldOver;
        tCIDLib::TBoolean               m_bEditableConn;
        tCIDLib::TCard4                 m_c4ArchVersion;
        tCIDLib::TCard4                 m_c4CapFlags;
        tCIDLib::TCard4                 m_c4MajVersion;
        tCIDLib::TCard4                 m_c4MinVersion;
        TPromptList                     m_colPrompts;
        tCQCKit::EDevCats               m_eCategory;
        tCQCKit::EDrvTypes              m_eType;
        tCQCKit::TDevClassList          m_fcolDevClasses;
        TPromptVals                     m_nvmFixed;
        mutable TBag<TString>*          m_pcolExtraFilesC;
        mutable TBag<TString>*          m_pcolExtraFilesS;
        mutable TBag<TCQCModuleInfo>*   m_pcolExtraFacs;
        TCQCConnCfg*                    m_pconncfgDef;
        TString                         m_strAuthor;
        TString                         m_strAutoSenseClass;
        TString                         m_strClientExtraCfg;
        TString                         m_strClientLibName;
        TString                         m_strContactEMail;
        TString                         m_strDescription;
        TString                         m_strDisplayName;
        TString                         m_strDynRef;
        TString                         m_strMake;
        TString                         m_strModel;
        TString                         m_strServerExtraCfg;
        TString                         m_strServerLibName;
        TString                         m_strVersion;


        // -------------------------------------------------------------------
        //  Private, static members
        //
        //  s_strEmbeddedDTDText
        //      This is the text for the embedded DTD that is used when a
        //      driver config XML file is parsed.
        // -------------------------------------------------------------------
        static const TString            s_strEmbeddedDTDText;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDriverCfg,TObject)
};

#pragma CIDLIB_POPPACK


