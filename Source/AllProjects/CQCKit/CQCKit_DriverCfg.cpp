//
// FILE NAME: CQCKit_DriverCfg.cpp
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
//  This file implements the TCQCDriverCfg class, and any related classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCDCfgPrompt,TObject)
RTTIDecls(TCQCDriverCfg,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_DriverCfg
    {
        // -----------------------------------------------------------------------
        //  Our current and previous format versions
        //
        //  Prompt format history:
        //      2.  Added support for optional flag.
        //      3.  Moved to the new named value map form for prompts and added
        //          the optional default value.
        //      4.  Added the new extra flag member
        //
        //  Driver config format history:
        //      2.  Moved from the TKeyValuePair class for prompt values to the
        //          named value map form so we can support a two level hierarchy.
        //      3.  Added the new device category list and the driver caps flags
        //      4.  Added stuff to support aysnc driver mode
        //      5.  Add the auto-sensing class path
        //      6.  Added a new dynamic type reference value in the server config
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtPromptVersion  = 4;
        constexpr tCIDLib::TCard2   c2FmtCfgVersion     = 6;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCDCfgPrompt
//  PREFIX: cqcdcp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDCfgPrompt: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDCfgPrompt::TCQCDCfgPrompt() :

    m_bFlag(kCIDLib::False)
    , m_bOptional(kCIDLib::False)
    , m_eType(tCQCKit::EPromptTypes::Count)
    , m_i4Max(0)
    , m_i4Min(0)
{
}

TCQCDCfgPrompt::TCQCDCfgPrompt( const   TString&                strName
                                , const TString&                strTitle
                                , const TString&                strDescrText
                                , const TString&                strLimits
                                , const TString&                strValNames
                                , const tCQCKit::EPromptTypes   eType
                                , const tCIDLib::TBoolean       bOptional
                                , const tCIDLib::TBoolean       bFlag) :
    m_bFlag(bFlag)
    , m_bOptional(bOptional)

    , m_eType(eType)
    , m_i4Max(0)
    , m_i4Min(0)
    , m_strDescrText(strDescrText)
    , m_strLimits(strLimits)
    , m_strName(strName)
    , m_strTitle(strTitle)
    , m_strValNames(strValNames)
{
    if (eType == tCQCKit::EPromptTypes::Range)
        StoreRange(strLimits);
}


// ---------------------------------------------------------------------------
//  TCQCDCfgPrompt: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCDCfgPrompt::operator==(const TCQCDCfgPrompt& cqcdcpSrc) const
{
    if (this == &cqcdcpSrc)
        return kCIDLib::True;

    return
    (
        (m_eType == cqcdcpSrc.m_eType)
        && (m_strName == cqcdcpSrc.m_strName)
        && (m_strDescrText == cqcdcpSrc.m_strDescrText)
        && (m_strLimits == cqcdcpSrc.m_strLimits)
        && (m_strTitle == cqcdcpSrc.m_strTitle)
        && (m_strValNames == cqcdcpSrc.m_strValNames)
        && (m_colDefaults == cqcdcpSrc.m_colDefaults)
    );
}


tCIDLib::TBoolean TCQCDCfgPrompt::operator!=(const TCQCDCfgPrompt& cqcdcpSrc) const
{
    return !operator==(cqcdcpSrc);
}


// ---------------------------------------------------------------------------
//  TCQCDCfgPrompt: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCDCfgPrompt::AddDefault(const TString& strKey, const TString& strValue)
{
    m_colDefaults.objPlace(strKey, strValue);
}


// Find the default value for a sub key if defined by the manifest
tCIDLib::TBoolean
TCQCDCfgPrompt::bDefaultFor(const TString& strSubKey, TString& strToFill) const
{
    const tCIDLib::TCard4 c4Count = m_colDefaults.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colDefaults[c4Index];

        if (kvalCur.strKey() == strSubKey)
        {
            strToFill = kvalCur.strValue();
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TCQCDCfgPrompt::bFlag() const
{
    return m_bFlag;
}


tCIDLib::TBoolean TCQCDCfgPrompt::bOptional() const
{
    return m_bOptional;
}


tCIDLib::TCard4 TCQCDCfgPrompt::c4DefCount() const
{
    return m_colDefaults.c4ElemCount();
}


tCQCKit::EPromptTypes TCQCDCfgPrompt::eType() const
{
    return m_eType;
}

tCQCKit::EPromptTypes
TCQCDCfgPrompt::eType(const tCQCKit::EPromptTypes eToSet)
{
    m_eType = eToSet;
    return m_eType;
}


tCIDLib::TInt4 TCQCDCfgPrompt::i4MaxValue() const
{
    return m_i4Max;
}


tCIDLib::TInt4 TCQCDCfgPrompt::i4MinValue() const
{
    return m_i4Min;
}


const TKeyValuePair&
TCQCDCfgPrompt::kvalDefaultAt(const tCIDLib::TCard4 c4At) const
{
    return m_colDefaults[c4At];
}


const TString& TCQCDCfgPrompt::strDescrText() const
{
    return m_strDescrText;
}

const TString& TCQCDCfgPrompt::strDescrText(const TString& strToSet)
{
    m_strDescrText = strToSet;
    return m_strDescrText;
}


const TString& TCQCDCfgPrompt::strLimits() const
{
    return m_strLimits;
}

const TString& TCQCDCfgPrompt::strLimits(const TString& strToSet)
{
    m_strLimits = strToSet;
    return m_strLimits;
}


const TString& TCQCDCfgPrompt::strName() const
{
    return m_strName;
}

const TString& TCQCDCfgPrompt::strName(const TString& strToSet)
{
    m_strName = strToSet;
    return m_strName;
}


const TString& TCQCDCfgPrompt::strTitle() const
{
    return m_strTitle;
}

const TString& TCQCDCfgPrompt::strTitle(const TString& strToSet)
{
    m_strTitle = strToSet;
    return m_strTitle;
}



const TString& TCQCDCfgPrompt::strValNames() const
{
    return m_strValNames;
}


tCIDLib::TVoid
TCQCDCfgPrompt::Set(const   TString&                strName
                    , const TString&                strTitle
                    , const TString&                strDescr
                    , const TString&                strLimits
                    , const TString&                strValNames
                    , const tCQCKit::EPromptTypes   eType
                    , const tCIDLib::TBoolean       bOptional
                    , const tCIDLib::TBoolean       bFlag)
{
    m_bFlag = bFlag;
    m_bOptional = bOptional;
    m_eType = eType;
    m_strDescrText = strDescr;
    m_strLimits = strLimits;
    m_strName = strName;
    m_strTitle = strTitle;
    m_strValNames = strValNames;
    m_colDefaults.RemoveAll();

    if (eType == tCQCKit::EPromptTypes::Range)
        StoreRange(strLimits);
}


// ---------------------------------------------------------------------------
//  TCQCDCfgPrompt: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDCfgPrompt::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Stream in the format version and make sure its good
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_DriverCfg::c2FmtPromptVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Looks ok, so lets read in the base version stuff
    strmToReadFrom  >> m_eType
                    >> m_i4Max
                    >> m_i4Min
                    >> m_strDescrText
                    >> m_strLimits
                    >> m_strTitle
                    >> m_strName;

    // If version 2 or later, then read in the optional flag, else default it
    if (c2FmtVersion >= 2)
        strmToReadFrom >> m_bOptional;
    else
        m_bOptional = kCIDLib::False;

    //
    //  If version 3 or later, then read in the value names, else default
    //  them. This is kind of ugly but we konw what the all the prompt types
    //  currently used are and what values they should have. So we have to
    //  look at the prompt names and set the values appopriately.
    //
    //  Also if version 3 or later, read in the default values, else clear it.
    //
    if (c2FmtVersion >= 3)
    {
        strmToReadFrom  >> m_strValNames
                        >> m_colDefaults;
    }
     else
    {
        if (m_strName == L"Binding")
        {
            m_strName = L"AppCtrl";
            m_strValNames = L"Path, Binding";
        }
         else if ((m_strName == L"Params")
              ||  (m_strName == L"Password")
              ||  (m_strName == L"LoginId"))
        {
            m_strValNames = L"Text";
        }
         else if ((m_strName == L"STBType")
              ||  (m_strName == L"Address"))
        {
            m_strValNames = L"Selected";
        }
         else if (m_strName == L"WorkingDir")
        {
            m_strValNames = L"Path";
        }
        m_colDefaults.RemoveAll();
    }

    if (c2FmtVersion >= 4)
        strmToReadFrom >> m_bFlag;
    else
        m_bFlag = kCIDLib::False;

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCDCfgPrompt::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_DriverCfg::c2FmtPromptVersion
                    << m_eType
                    << m_i4Max
                    << m_i4Min
                    << m_strDescrText
                    << m_strLimits
                    << m_strTitle
                    << m_strName

                    // Version 2 stuff
                    << m_bOptional

                    // Version 3 stuff
                    << m_strValNames
                    << m_colDefaults

                    // Version 4 stuff
                    << m_bFlag

                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCDCfgPrompt: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDCfgPrompt::StoreRange(const TString& strValue)
{
    // Break out the two range values
    TStringTokenizer stokVal(&strValue, L" ,");

    // There should be two tokens, a min and max
    TString strMin;
    TString strMax;
    if (!stokVal.bGetNextToken(strMin)
    ||  !stokVal.bGetNextToken(strMax))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadRangeLimit
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strName
        );
    }

    // Try to convert them
    try
    {
        m_i4Min = strMin.i4Val();
        m_i4Max = strMax.i4Val();
    }

    catch(...)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadRangeLimit
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strName
        );
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCDriverCfg
//  PREFIX: cqcdc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDriverCfg: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TCQCDriverCfg::eCompOnDisplayName(  const   TCQCDriverCfg&   cqcdc1
                                    , const TCQCDriverCfg&   cqcdc2)
{
    return cqcdc1.m_strDisplayName.eCompareI(cqcdc2.m_strDisplayName);
}

tCIDLib::ESortComps
TCQCDriverCfg::eCompOnMake( const   TCQCDriverCfg&   cqcdc1
                            , const TCQCDriverCfg&   cqcdc2)
{
    return cqcdc1.m_strMake.eCompareI(cqcdc2.m_strMake);
}


// ---------------------------------------------------------------------------
//  TCQCDriverCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDriverCfg::TCQCDriverCfg() :

    m_bAsyncMode(kCIDLib::False)
    , m_bAsyncFldOver(kCIDLib::True)
    , m_bEditableConn(kCIDLib::True)
    , m_c4ArchVersion(1)
    , m_c4CapFlags(0)
    , m_c4MajVersion(kCIDLib::c4MaxCard)
    , m_c4MinVersion(kCIDLib::c4MaxCard)
    , m_eCategory(tCQCKit::EDevCats::Miscellaneous)
    , m_eType(tCQCKit::EDrvTypes::CppLib)
    , m_pcolExtraFilesC(nullptr)
    , m_pcolExtraFilesS(nullptr)
    , m_pcolExtraFacs(nullptr)
    , m_pconncfgDef(new TCQCOtherConnCfg)
{
    m_fcolDevClasses.c4AddElement(tCQCKit::EDevClasses::None);
}

TCQCDriverCfg::TCQCDriverCfg(const TString& strSrcFile) :

    m_bAsyncMode(kCIDLib::False)
    , m_bAsyncFldOver(kCIDLib::True)
    , m_bEditableConn(kCIDLib::True)
    , m_c4ArchVersion(1)
    , m_c4CapFlags(0)
    , m_c4MajVersion(kCIDLib::c4MaxCard)
    , m_c4MinVersion(kCIDLib::c4MaxCard)
    , m_eCategory(tCQCKit::EDevCats::Miscellaneous)
    , m_eType(tCQCKit::EDrvTypes::CppLib)
    , m_pcolExtraFilesC(nullptr)
    , m_pcolExtraFilesS(nullptr)
    , m_pcolExtraFacs(nullptr)
    , m_pconncfgDef(new TCQCOtherConnCfg)
{
    ParseFrom(new TFileEntitySrc(strSrcFile));
}

TCQCDriverCfg::TCQCDriverCfg(const  TString&        strSrcName
                            , const TMemBuf&        mbufSrc
                            , const tCIDLib::TCard4 c4Count) :

    m_bAsyncMode(kCIDLib::False)
    , m_bAsyncFldOver(kCIDLib::True)
    , m_bEditableConn(kCIDLib::True)
    , m_c4ArchVersion(1)
    , m_c4CapFlags(0)
    , m_c4MajVersion(kCIDLib::c4MaxCard)
    , m_c4MinVersion(kCIDLib::c4MaxCard)
    , m_eCategory(tCQCKit::EDevCats::Miscellaneous)
    , m_eType(tCQCKit::EDrvTypes::CppLib)
    , m_pcolExtraFilesC(nullptr)
    , m_pcolExtraFilesS(nullptr)
    , m_pcolExtraFacs(nullptr)
    , m_pconncfgDef(new TCQCOtherConnCfg)
{
    ParseFrom(new TMemBufEntitySrc(strSrcName, mbufSrc, c4Count));
}

TCQCDriverCfg::TCQCDriverCfg(const TCQCDriverCfg& cqcdcSrc) :

    m_bAsyncMode(cqcdcSrc.m_bAsyncMode)
    , m_bAsyncFldOver(cqcdcSrc.m_bAsyncFldOver)
    , m_bEditableConn(cqcdcSrc.m_bEditableConn)
    , m_c4ArchVersion(cqcdcSrc.m_c4ArchVersion)
    , m_c4CapFlags(cqcdcSrc.m_c4CapFlags)
    , m_c4MajVersion(cqcdcSrc.m_c4MajVersion)
    , m_c4MinVersion(cqcdcSrc.m_c4MinVersion)
    , m_colPrompts(cqcdcSrc.m_colPrompts)
    , m_eCategory(cqcdcSrc.m_eCategory)
    , m_eType(cqcdcSrc.m_eType)
    , m_fcolDevClasses(cqcdcSrc.m_fcolDevClasses)
    , m_nvmFixed(cqcdcSrc.m_nvmFixed)
    , m_pcolExtraFilesC(nullptr)
    , m_pcolExtraFilesS(nullptr)
    , m_pcolExtraFacs(nullptr)
    , m_pconncfgDef(nullptr)
    , m_strAuthor(cqcdcSrc.m_strAuthor)
    , m_strAutoSenseClass(cqcdcSrc.m_strAutoSenseClass)
    , m_strClientExtraCfg(cqcdcSrc.m_strClientExtraCfg)
    , m_strClientLibName(cqcdcSrc.m_strClientLibName)
    , m_strContactEMail(cqcdcSrc.m_strContactEMail)
    , m_strDescription(cqcdcSrc.m_strDescription)
    , m_strDisplayName(cqcdcSrc.m_strDisplayName)
    , m_strDynRef(cqcdcSrc.m_strDynRef)
    , m_strMake(cqcdcSrc.m_strMake)
    , m_strModel(cqcdcSrc.m_strModel)
    , m_strServerExtraCfg(cqcdcSrc.m_strServerExtraCfg)
    , m_strServerLibName(cqcdcSrc.m_strServerLibName)
    , m_strVersion(cqcdcSrc.m_strVersion)
{
    // Polymorphically duplicate the connection config object
    m_pconncfgDef = ::pDupObject<TCQCConnCfg>(cqcdcSrc.m_pconncfgDef);

    // And copy the collections if any
    if (cqcdcSrc.m_pcolExtraFilesC)
        m_pcolExtraFilesC = new TBag<TString>(*cqcdcSrc.m_pcolExtraFilesC);

    if (cqcdcSrc.m_pcolExtraFilesS)
        m_pcolExtraFilesS = new TBag<TString>(*cqcdcSrc.m_pcolExtraFilesS);

    if (cqcdcSrc.m_pcolExtraFacs)
        m_pcolExtraFacs = new TBag<TCQCModuleInfo>(*cqcdcSrc.m_pcolExtraFacs);
}

TCQCDriverCfg::~TCQCDriverCfg()
{
    delete m_pconncfgDef;
    delete m_pcolExtraFilesC;
    delete m_pcolExtraFilesS;
    delete m_pcolExtraFacs;
}


// ---------------------------------------------------------------------------
//  TCQCDriverCfg: Public operators
// ---------------------------------------------------------------------------
TCQCDriverCfg& TCQCDriverCfg::operator=(const TCQCDriverCfg& cqcdcSrc)
{
    // Avoid self-assignment
    if (this == &cqcdcSrc)
        return *this;

    m_bAsyncMode        = cqcdcSrc.m_bAsyncMode;
    m_bAsyncFldOver     = cqcdcSrc.m_bAsyncFldOver;
    m_bEditableConn     = cqcdcSrc.m_bEditableConn;
    m_c4ArchVersion     = cqcdcSrc.m_c4ArchVersion;
    m_c4CapFlags        = cqcdcSrc.m_c4CapFlags;
    m_c4MajVersion      = cqcdcSrc.m_c4MajVersion;
    m_c4MinVersion      = cqcdcSrc.m_c4MinVersion;
    m_colPrompts        = cqcdcSrc.m_colPrompts;
    m_eCategory         = cqcdcSrc.m_eCategory;
    m_eType             = cqcdcSrc.m_eType;
    m_fcolDevClasses    = cqcdcSrc.m_fcolDevClasses;
    m_nvmFixed          = cqcdcSrc.m_nvmFixed;
    m_strAuthor         = cqcdcSrc.m_strAuthor;
    m_strAutoSenseClass = cqcdcSrc.m_strAutoSenseClass;
    m_strClientExtraCfg = cqcdcSrc.m_strClientExtraCfg;
    m_strClientLibName  = cqcdcSrc.m_strClientLibName;
    m_strContactEMail   = cqcdcSrc.m_strContactEMail;
    m_strDescription    = cqcdcSrc.m_strDescription;
    m_strDisplayName    = cqcdcSrc.m_strDisplayName;
    m_strDynRef         = cqcdcSrc.m_strDynRef;
    m_strMake           = cqcdcSrc.m_strMake;
    m_strModel          = cqcdcSrc.m_strModel;
    m_strServerExtraCfg = cqcdcSrc.m_strServerExtraCfg;
    m_strServerLibName  = cqcdcSrc.m_strServerLibName;
    m_strVersion        = cqcdcSrc.m_strVersion;

    //
    //  Polymorphically duplicate the connection config object, getting rid
    //  of any previous one first. There might not be one, so we have to check
    //  and leave it zeroed if not.
    //
    delete m_pconncfgDef;
    m_pconncfgDef = nullptr;
    m_pconncfgDef = ::pDupObject<TCQCConnCfg>(cqcdcSrc.m_pconncfgDef);

    //
    //  And update our collections. Since they are optional, we have to
    //  dance a bit here to do the right thing.
    //
    if (cqcdcSrc.m_pcolExtraFilesC)
    {
        // They have one, so we have to create one if we don't or just copy
        if (!m_pcolExtraFilesC)
            m_pcolExtraFilesC = new TBag<TString>(*cqcdcSrc.m_pcolExtraFilesC);
        else
            *m_pcolExtraFilesC = *cqcdcSrc.m_pcolExtraFilesC;
    }
     else
    {
        // They don't have one, so track ours if we have one
        delete m_pcolExtraFilesC;
        m_pcolExtraFilesC = nullptr;
    }

    if (cqcdcSrc.m_pcolExtraFilesS)
    {
        // They have one, so we have to create one if we don't or just copy
        if (!m_pcolExtraFilesS)
            m_pcolExtraFilesS = new TBag<TString>(*cqcdcSrc.m_pcolExtraFilesS);
        else
            *m_pcolExtraFilesS = *cqcdcSrc.m_pcolExtraFilesS;
    }
     else
    {
        // They don't have one, so trash ours if we have one
        delete m_pcolExtraFilesS;
        m_pcolExtraFilesS = nullptr;
    }

    if (cqcdcSrc.m_pcolExtraFacs)
    {
        // They have one, so we have to create one if we don't or just copy
        if (!m_pcolExtraFacs)
            m_pcolExtraFacs = new TBag<TCQCModuleInfo>(*cqcdcSrc.m_pcolExtraFacs);
        else
            *m_pcolExtraFacs = *cqcdcSrc.m_pcolExtraFacs;
    }
     else
    {
        // They don't have one, so trash ours if we have one
        delete m_pcolExtraFacs;
        m_pcolExtraFacs = nullptr;
    }
    return *this;
}


tCIDLib::TBoolean
TCQCDriverCfg::operator==(const TCQCDriverCfg& cqcdcSrc) const
{
    if (this == &cqcdcSrc)
        return kCIDLib::True;

    //
    //  Because of the way the CQC system works, we know that the make, model,
    //  and version are the primary determiners, so put them first. We don't
    //  have to compare strVersion, since we can compare the binary version
    //  values instead.
    //
    if ((m_strMake              != cqcdcSrc.m_strMake)
    ||  (m_strModel             != cqcdcSrc.m_strModel)
    ||  (m_c4MajVersion         != cqcdcSrc.m_c4MajVersion)
    ||  (m_c4MinVersion         != cqcdcSrc.m_c4MinVersion)
    ||  (m_eType                != cqcdcSrc.m_eType)
    ||  (m_eCategory            != cqcdcSrc.m_eCategory)
    ||  (m_c4ArchVersion        != cqcdcSrc.m_c4ArchVersion)
    ||  (m_c4CapFlags           != cqcdcSrc.m_c4CapFlags)
    ||  (m_fcolDevClasses       != cqcdcSrc.m_fcolDevClasses)
    ||  (m_strClientLibName     != cqcdcSrc.m_strClientLibName)
    ||  (m_strServerLibName     != cqcdcSrc.m_strServerLibName)
    ||  (m_bEditableConn        != cqcdcSrc.m_bEditableConn)
    ||  (m_strAuthor            != cqcdcSrc.m_strAuthor)
    ||  (m_strContactEMail      != cqcdcSrc.m_strContactEMail)
    ||  (m_strClientExtraCfg    != cqcdcSrc.m_strClientExtraCfg)
    ||  (m_strServerExtraCfg    != cqcdcSrc.m_strServerExtraCfg)
    ||  (m_strDescription       != cqcdcSrc.m_strDescription)
    ||  (m_strDisplayName       != cqcdcSrc.m_strDisplayName)
    ||  (m_strDynRef            != cqcdcSrc.m_strDynRef)
    ||  (m_bAsyncMode           != cqcdcSrc.m_bAsyncMode)
    ||  (m_bAsyncFldOver        != cqcdcSrc.m_bAsyncFldOver)
    ||  (m_colPrompts           != cqcdcSrc.m_colPrompts))
    {
        return kCIDLib::False;
    }

    if (m_nvmFixed != cqcdcSrc.m_nvmFixed)
        return kCIDLib::False;

    // Do the optional collections
    if (m_pcolExtraFilesC || cqcdcSrc.m_pcolExtraFilesC)
    {
        if (m_pcolExtraFilesC && cqcdcSrc.m_pcolExtraFilesC)
        {
            if (*m_pcolExtraFilesC != *cqcdcSrc.m_pcolExtraFilesC)
                return kCIDLib::False;
        }
         else
        {
            return kCIDLib::False;
        }
    }

    // Do the optional collections
    if (m_pcolExtraFilesS || cqcdcSrc.m_pcolExtraFilesS)
    {
        if (m_pcolExtraFilesS && cqcdcSrc.m_pcolExtraFilesS)
        {
            if (*m_pcolExtraFilesS != *cqcdcSrc.m_pcolExtraFilesS)
                return kCIDLib::False;
        }
         else
        {
            return kCIDLib::False;
        }
    }

    // Do the optional collections
    if (m_pcolExtraFacs || cqcdcSrc.m_pcolExtraFacs)
    {
        if (m_pcolExtraFacs && cqcdcSrc.m_pcolExtraFacs)
        {
            if (*m_pcolExtraFacs != *cqcdcSrc.m_pcolExtraFacs)
                return kCIDLib::False;
        }
         else
        {
            //
            //  If the one that's present is empty, they are really the same, so only
            //  return false if the one we have isn't empty.
            //
            if ((m_pcolExtraFacs && !m_pcolExtraFacs->bIsEmpty())
            ||  (cqcdcSrc.m_pcolExtraFacs && !cqcdcSrc.m_pcolExtraFacs->bIsEmpty()))
            {
                return kCIDLib::False;
            }
        }
    }

    //
    //  Do the connection config stuff. They have a polymorphic method to
    //  do this, which will check that they ar the same type and if so
    //  compare them. We do this last because, if the make/model and whatnot
    //  compare, then this should always be the same.
    //
    if (!m_pconncfgDef->bIsSame(*cqcdcSrc.m_pconncfgDef))
        return kCIDLib::False;

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCDriverCfg: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDriverCfg::NewConnCfg(TCQCConnCfg* const)
{
    // Default empty implementation
}


// ---------------------------------------------------------------------------
//  TCQCDriverCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return our async driver mode flags
tCIDLib::TBoolean TCQCDriverCfg::bAsyncMode() const
{
    return m_bAsyncMode;
}

tCIDLib::TBoolean TCQCDriverCfg::bAsyncFldOver() const
{
    return m_bAsyncFldOver;
}


// Return whether our conn config is editable
tCIDLib::TBoolean TCQCDriverCfg::bEditableConn() const
{
    return m_bEditableConn;
}


// See if there is a fixed prompt with this key/subkey. If so, return the value
tCIDLib::TBoolean
TCQCDriverCfg::bFindFixedVal(const  TString& strKey
                            , const TString& strSubKey
                            ,       TString& strToFill) const
{
    return m_nvmFixed.bQueryValue(strKey, strSubKey, strToFill);
}


// Indicate whether this driver supports the indicated capability
tCIDLib::TBoolean
TCQCDriverCfg::bHasCapability(const tCQCKit::EDrvCaps eToCheck) const
{
    return (m_c4CapFlags & tCIDLib::c4EnumOrd(eToCheck)) != 0;
}


// Indicate whether this driver has extra facilities involved
tCIDLib::TBoolean TCQCDriverCfg::bHasExtraFacs() const
{
    // If we never allocated a list, then we definitely have none
    if (!m_pcolExtraFacs)
        return kCIDLib::False;

    // Else return a negation of the empty check
    return !m_pcolExtraFacs->bIsEmpty();
}


// Indicate of this driver has any non-fixed prompts
tCIDLib::TBoolean TCQCDriverCfg::bHasPrompts() const
{
    return !m_colPrompts.bIsEmpty();
}


// See if this driver implements the passed class
tCIDLib::TBoolean
TCQCDriverCfg::bImplementsClass(const tCQCKit::EDevClasses eToCheck) const
{
    const tCIDLib::TCard4 c4Count = m_fcolDevClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_fcolDevClasses[c4Index] == eToCheck)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  In terms of being the same driver, if the make/model match, then they
//  are for the same device. We can't go by libraries, since the same
//  library can be used for multiple devices.
//
tCIDLib::TBoolean
TCQCDriverCfg::bSameDriver(const TCQCDriverCfg& cqcdcToCompare) const
{
    return  (m_strMake == cqcdcToCompare.m_strMake)
    &&      (m_strModel == cqcdcToCompare.m_strModel);
}


// Return the driver architecture version of this driver
tCIDLib::TCard4 TCQCDriverCfg::c4ArchVersion() const
{
    return m_c4ArchVersion;
}


// Return the number of fixed prompots
tCIDLib::TCard4 TCQCDriverCfg::c4FixedCount() const
{
    return m_nvmFixed.c4ElemCount();
}


// Return the major and minor version of the driver
tCIDLib::TCard4 TCQCDriverCfg::c4MajVersion() const
{
    return m_c4MajVersion;
}

tCIDLib::TCard4 TCQCDriverCfg::c4MinVersion() const
{
    return m_c4MinVersion;
}


// REturn the count of non-fixed prompts
tCIDLib::TCard4 TCQCDriverCfg::c4PromptCount() const
{
    return m_colPrompts.c4ElemCount();
}


// Return the class of the configuration types
const TClass& TCQCDriverCfg::clsConfigType() const
{
    return m_pconncfgDef->clsIsA();
}


TBag<TString>::TCursor TCQCDriverCfg::cursExtraFilesC() const
{
    if (!m_pcolExtraFilesC)
        m_pcolExtraFilesC = new TBag<TString>;
    return TBag<TString>::TCursor(m_pcolExtraFilesC);
}

TBag<TString>::TCursor TCQCDriverCfg::cursExtraFilesS() const
{
    if (!m_pcolExtraFilesS)
        m_pcolExtraFilesS = new TBag<TString>;
    return TBag<TString>::TCursor(m_pcolExtraFilesS);
}

TBag<TCQCModuleInfo>::TCursor TCQCDriverCfg::cursExtraFacs() const
{
    if (!m_pcolExtraFacs)
        m_pcolExtraFacs = new TBag<TCQCModuleInfo>;
    return TBag<TCQCModuleInfo>::TCursor(m_pcolExtraFacs);
}


TCQCDriverCfg::TPromptCursor TCQCDriverCfg::cursFixed() const
{
    return m_nvmFixed.cursItems();
}


TCQCDriverCfg::TValCursor
TCQCDriverCfg::cursFixedVals(const TString& strKey) const
{
    return m_nvmFixed.cursItemVals(strKey);
}


const TCQCConnCfg& TCQCDriverCfg::conncfgDef() const
{
    return *m_pconncfgDef;
}

TCQCConnCfg& TCQCDriverCfg::conncfgDef()
{
    return *m_pconncfgDef;
}


// Return the device category for this driver
tCQCKit::EDevCats TCQCDriverCfg::eCategory() const
{
    return m_eCategory;
}


// Return the driver type
tCQCKit::EDrvTypes TCQCDriverCfg::eType() const
{
    return m_eType;
}


// Return the list of device classes this driver supports
tCQCKit::TDevClassList TCQCDriverCfg::fcolDevClasses() const
{
    return m_fcolDevClasses;
}


// Return the list of fixed prompt definitions
const TCQCDriverCfg::TPromptVals& TCQCDriverCfg::nvmFixed() const
{
    return m_nvmFixed;
}


//
//  To avoid having to expose the XML headers to all our users (basically
//  all of CQC), we take a raw entry source instead of the usual templatized
//  counted pointer entity source ref, because the later cannot be forward
//  referenced. We'll put the passed source into an enty source ref and then
//  pass it on to the parser.
//
tCIDLib::TVoid TCQCDriverCfg::ParseFrom(const TXMLEntitySrc* pxsrcConfig)
{
    //
    //  Get the passed entity source into a source ref first, so that it's
    //  owned if anybody throws.
    //
    tCIDXML::TEntitySrcRef esrConfig(pxsrcConfig);

    try
    {
        TXMLTreeParser xtprsConfig;

        //
        //  Add an entity mapping so that the public URN that is used in the
        //  manifest files get mapped to our hard coded internal DTD text.
        //
        xtprsConfig.AddMapping
        (
            new TMemBufEntitySrc
            (
                L"CQCDrvManifest.DTD"
                , L"urn:charmedquark.com:CQC-DrvManifest.DTD"
                , TCQCDriverCfg::s_strEmbeddedDTDText
            )
        );

        //
        //  Ok, we have a file, so lets try to use the XML tree parser to get
        //  a representation of the file into memory. Tell it that we only
        //  want to see non-ignorable chars and tag information. That will
        //  save us a lot of filtering out comments and whitspace.
        //
        const tCIDLib::TBoolean bOk = xtprsConfig.bParseRootEntity
        (
            esrConfig
            , tCIDXML::EParseOpts::Validate
            , tCIDXML::EParseFlags::TagsNText
        );

        if (!bOk)
        {
            //
            //  Format the first error in the error list, into a string that
            //  we can pass as a replacement parameter to the real error we
            //  log.
            //
            const TXMLTreeParser::TErrInfo& erriFirst
                                            = xtprsConfig.colErrors()[0];

            TTextStringOutStream strmOut(512);
            strmOut << L"[" << erriFirst.strSystemId() << L"/"
                    << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                    << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

            //
            //  Throw an exception that indicates that the file was not valid
            //  XML. Pass along the first error that occured.
            //
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCfg_InvalidCfgXML
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strmOut.strData()
            );
        }

        //
        //  Ok, we now have the document with the validated system config
        //  data in it. So lets go through it and get the data out. We start
        //  by getting the root element node from the document.
        //
        const TXMLTreeElement& xtnodeRoot = xtprsConfig.xtdocThis().xtnodeRoot();

        // And call the private method that parses from the root node
        ParseFromNode(xtnodeRoot);

        // Let the derived types know we have a new conn config default
        NewConnCfg(m_pconncfgDef);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        if (!errToCatch.bLogged())
            TModule::LogEventObj(errToCatch);

        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_ManifestLoadFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , pxsrcConfig->strSystemId()
        );

        // It didn't work, so clean up anything we allocated, then rethrow
        delete m_pcolExtraFilesC;
        m_pcolExtraFilesC = nullptr;
        delete m_pcolExtraFilesS;
        m_pcolExtraFilesS = nullptr;
        delete m_pcolExtraFacs;
        m_pcolExtraFacs = nullptr;

        // Put back the deafult connection config
        delete m_pconncfgDef;
        m_pconncfgDef = new TCQCOtherConnCfg;

        throw;
    }

    catch(...)
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_ManifestLoadFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , pxsrcConfig->strSystemId()
        );

        // It didn't work, so clean up anything we allocated, then rethrow
        delete m_pcolExtraFilesC;
        m_pcolExtraFilesC = nullptr;
        delete m_pcolExtraFilesS;
        m_pcolExtraFilesS = nullptr;
        delete m_pcolExtraFacs;
        m_pcolExtraFacs = nullptr;

        // Put back the deafult connection config
        delete m_pconncfgDef;
        m_pconncfgDef = new TCQCOtherConnCfg;

        throw;
    }
}


const TCQCDCfgPrompt&
TCQCDriverCfg::cqcdcpAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colPrompts[c4Index];
}

TCQCDCfgPrompt& TCQCDriverCfg::cqcdcpAt(const tCIDLib::TCard4 c4Index)
{
    return m_colPrompts[c4Index];
}


const TString& TCQCDriverCfg::strAuthor() const
{
    return m_strAuthor;
}


const TString& TCQCDriverCfg::strAutoSenseClass() const
{
    return m_strAutoSenseClass;
}


const TString& TCQCDriverCfg::strClientExtraCfg() const
{
    return m_strClientExtraCfg;
}


const TString& TCQCDriverCfg::strClientLibName() const
{
    return m_strClientLibName;
}


const TString& TCQCDriverCfg::strContactEMail() const
{
    return m_strContactEMail;
}


const TString& TCQCDriverCfg::strDescription() const
{
    return m_strDescription;
}


const TString& TCQCDriverCfg::strDisplayName() const
{
    return m_strDisplayName;
}


const TString& TCQCDriverCfg::strDynRef() const
{
    return m_strDynRef;
}


const TString&
TCQCDriverCfg::strFixedValue(const  TString& strKey
                            , const TString& strSubKey) const
{
    const TString* pstrRet = 0;
    try
    {
        pstrRet = &m_nvmFixed.objValue(strKey, strSubKey);
    }

    catch(const TError& errToCatch)
    {
        if (facCQCKit().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_FixedValNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strKey
            , strSubKey
        );
    }
    return *pstrRet;
}


const TString& TCQCDriverCfg::strMake() const
{
    return m_strMake;
}


const TString& TCQCDriverCfg::strModel() const
{
    return m_strModel;
}


const TString& TCQCDriverCfg::strServerExtraCfg() const
{
    return m_strServerExtraCfg;
}


const TString& TCQCDriverCfg::strServerLibName() const
{
    return m_strServerLibName;
}


const TString& TCQCDriverCfg::strVersion() const
{
    return m_strVersion;
}


// ---------------------------------------------------------------------------
//  TCQCDriverCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDriverCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    tCIDLib::TCard1 c1TmpMarker;

    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Stream in the format version and make sure its good
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_DriverCfg::c2FmtCfgVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Looks ok, so lets read it in
    strmToReadFrom  >> m_eType
                    >> m_bEditableConn
                    >> m_c4MajVersion
                    >> m_c4MinVersion;

    //
    //  If we are a version 1, then the fixed prompt values are still in
    //  TKeyValuePair format, so we have to create a temp collection to
    //  read them in, and covert them over to the new format.
    //
    if (c2FmtVersion > 1)
    {
        strmToReadFrom >> m_nvmFixed;
    }
     else
    {
        tCIDLib::TKVPList colTmp;
        strmToReadFrom >> colTmp;

        // Clean out the existing list before we start adding to it
        m_nvmFixed.Reset();

        //
        //  This is really ugly, but since we have changed the form of the
        //  prompts, we have to actually use or knowledge of the drivers and
        //  translate the prompts based on that, because we have to add a
        //  new piece of information (the overall prompt name.)
        //
        //  Luckily there are only two types of fixed prompts currently
        //  used.
        //
        if (!colTmp.bIsEmpty())
        {
            if (colTmp[0].strKey() == L"IRDevice")
            {
                //
                //  These are all IR drivers and have one that indicates the
                //  device that the generic IR driver controls.
                //
                m_nvmFixed.AddItem(L"IRDevice");
                m_nvmFixed.AddValue
                (
                    L"IRDevice"
                    , L"IRDevice"
                    , colTmp[0].strValue()
                );
            }
             else if (colTmp[0].strKey() == L"Controller")
            {
                // These are to tell the X-10 driver which controller to use.
                m_nvmFixed.AddItem(L"X10Device");
                m_nvmFixed.AddValue
                (
                    L"X10Device"
                    , L"Controller"
                    , colTmp[0].strValue()
                );
            }
        }
    }

    // Readin the prompts
    strmToReadFrom  >> m_colPrompts;

    //
    //  If before V3, default the architecture version to 1 and default
    //  the caps flags to zero, else read then in.
    //
    if (c2FmtVersion < 3)
    {
        m_c4ArchVersion = 1;
        m_c4CapFlags = 0;
    }
     else
    {
        strmToReadFrom >> m_c4ArchVersion
                       >> m_c4CapFlags;
    }

    strmToReadFrom  >> m_eCategory
                    >> m_strAuthor
                    >> m_strClientExtraCfg
                    >> m_strClientLibName
                    >> m_strContactEMail
                    >> m_strDescription
                    >> m_strDisplayName
                    >> m_strMake
                    >> m_strModel
                    >> m_strServerExtraCfg
                    >> m_strServerLibName
                    >> m_strVersion;

    //
    //  If it was version 1, then we have to see if there is a Path prompt.
    //  It's been combined into a single AppCtrl prompt that holds the binding
    //  and path values, so we need to remove it.
    //
    if (c2FmtVersion == 1)
    {
        const tCIDLib::TCard4 c4PCount = m_colPrompts.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PCount; c4Index++)
        {
            if (m_colPrompts[c4Index].strName() == L"Path")
            {
                m_colPrompts.RemoveAt(c4Index);
                break;
            }
        }
    }

    //
    //  Check for a frame marker that separates the simple stuff from
    //  the more complex members.
    //
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    //
    //  If V3 or greater, we need to read in the device classes, else
    //  put in a default 'None'.
    //
    m_fcolDevClasses.RemoveAll();
    if (c2FmtVersion > 2)
    {
        //
        //  The device classes are stored as strings for flexibility,
        //  and we just read themin and convert them to the enum values
        //  and store those away. First there's a count and it's XOR,
        //  so get that and validate it.
        //
        tCIDLib::TCard4 c4Count, c4XORCount;
        strmToReadFrom >> c4Count >> c4XORCount;
        if (c4Count != (c4XORCount ^ kCIDLib::c4MaxCard))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCfg_BadDevClassCnt
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // Read in these values and convert them and store them
        TString strDevCls;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmToReadFrom >> strDevCls;

            // If we manage to get a bogus class into the data, drop it
            const tCQCKit::EDevClasses eClass(tCQCKit::eXlatEDevClasses(strDevCls));
            if ((eClass != tCQCKit::EDevClasses::None)
            &&  (eClass != tCQCKit::EDevClasses::Count))
            {
                m_fcolDevClasses.c4AddElement(eClass);
            }
        }
    }
     else
    {
        m_fcolDevClasses.c4AddElement(tCQCKit::EDevClasses::None);
    }

    //
    //  Clean up any current connection config, and read in the new one,
    //  which must be done polymorphically.
    //
    TCQCConnCfg* pconncfgTmp;
    ::PolymorphicRead(pconncfgTmp, strmToReadFrom);
    delete m_pconncfgDef;
    m_pconncfgDef = pconncfgTmp;

    //
    //  Stream in the collections. If the weren't streamed, a 0 byte is
    //  streamed out, else an 1 followed by the collection.
    //
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    strmToReadFrom >> c1TmpMarker;
    if (c1TmpMarker)
    {
        // It was streamed, so stream in, create first if needed
        if (!m_pcolExtraFilesC)
            m_pcolExtraFilesC = new TBag<TString>;
        strmToReadFrom >> *m_pcolExtraFilesC;
    }
     else
    {
        // It wasn't streamed, so flush ours if we have one
        if (m_pcolExtraFilesC)
            m_pcolExtraFilesC->RemoveAll();
    }

    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    strmToReadFrom >> c1TmpMarker;
    if (c1TmpMarker)
    {
        // It was streamed, so stream in, create first if needed
        if (!m_pcolExtraFilesS)
            m_pcolExtraFilesS = new TBag<TString>;
        strmToReadFrom >> *m_pcolExtraFilesS;
    }
     else
    {
        // It wasn't streamed, so flush ours if we have one
        if (m_pcolExtraFilesS)
            m_pcolExtraFilesS->RemoveAll();
    }

    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    strmToReadFrom >> c1TmpMarker;
    if (c1TmpMarker)
    {
        // It was streamed, so stream in, create first if needed
        if (!m_pcolExtraFacs)
            m_pcolExtraFacs = new TBag<TCQCModuleInfo>;
        strmToReadFrom >> *m_pcolExtraFacs;
    }
     else
    {
        // It wasn't streamed, so flush ours if we have one
        if (m_pcolExtraFacs)
            m_pcolExtraFacs->RemoveAll();
    }

    // If at V4 or beyond, read in the async mode stuff, else default
    if (c2FmtVersion >= 4)
    {
        strmToReadFrom  >> m_bAsyncMode
                        >> m_bAsyncFldOver;
    }
     else
    {
        m_bAsyncMode = kCIDLib::False;
        m_bAsyncFldOver = kCIDLib::True;
    }

    // If V5 or beyond, read in the auto-sensing class path, else default it
    if (c2FmtVersion >= 5)
        strmToReadFrom >> m_strAutoSenseClass;
    else
        m_strAutoSenseClass.Clear();

    // If V6 or beyond, read in the dynamic type reference, else default it
    if (c2FmtVersion >= 6)
        strmToReadFrom >> m_strDynRef;
    else
        m_strDynRef.Clear();

    // And make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCDriverCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    //
    //  Stream out the easy stuff first, with a start object marker at the
    //  start and a frame marker at the end, to begin the non-easy bits.
    //  We also stream out a format version to allow for future automatic
    //  upgrade of data.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_DriverCfg::c2FmtCfgVersion
                    << m_eType
                    << m_bEditableConn
                    << m_c4MajVersion
                    << m_c4MinVersion
                    << m_nvmFixed
                    << m_colPrompts
                    << m_c4ArchVersion
                    << m_c4CapFlags
                    << m_eCategory
                    << m_strAuthor
                    << m_strClientExtraCfg
                    << m_strClientLibName
                    << m_strContactEMail
                    << m_strDescription
                    << m_strDisplayName
                    << m_strMake
                    << m_strModel
                    << m_strServerExtraCfg
                    << m_strServerLibName
                    << m_strVersion
                    << tCIDLib::EStreamMarkers::Frame;


    //
    //  The devices classes are written out in textual format for future
    //  flexibility.
    //
    const tCIDLib::TCard4 c4DevClsCnt = m_fcolDevClasses.c4ElemCount();
    strmToWriteTo   << c4DevClsCnt
                    << tCIDLib::TCard4(c4DevClsCnt ^ kCIDLib::c4MaxCard);

    TString strVal;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DevClsCnt; c4Index++)
    {
        strVal = tCQCKit::strXlatEDevClasses(m_fcolDevClasses[c4Index]);

        // Cut out the prefix
        strVal.CutUpTo(kCIDLib::chUnderscore);
        strmToWriteTo << strVal;
    }

    // Stream the connection config polymorphically
    ::PolymorphicWrite(m_pconncfgDef, strmToWriteTo);

    // And stream the collections if we have them
    strmToWriteTo << tCIDLib::EStreamMarkers::Frame;
    if (m_pcolExtraFilesC)
    {
        // Stream a 1 to indicate we had one, and the collection
        strmToWriteTo << tCIDLib::TCard1(1)
                      << *m_pcolExtraFilesC;
    }
     else
    {
        // We never allocated it, so stream a zero byte
        strmToWriteTo << kCIDLib::c1MinCard;
    }

    strmToWriteTo << tCIDLib::EStreamMarkers::Frame;
    if (m_pcolExtraFilesS)
    {
        // Stream a 1 to indicate we had one, and the collection
        strmToWriteTo << tCIDLib::TCard1(1)
                      << *m_pcolExtraFilesS;
    }
     else
    {
        // We never allocated it, so stream a zero byte
        strmToWriteTo << kCIDLib::c1MinCard;
    }

    strmToWriteTo << tCIDLib::EStreamMarkers::Frame;
    if (m_pcolExtraFacs)
    {
        // Stream a 1 to indicate we had one, and the collection
        strmToWriteTo << tCIDLib::TCard1(1)
                      << *m_pcolExtraFacs;
    }
     else
    {
        // We never allocated it, so stream a zero byte
        strmToWriteTo << kCIDLib::c1MinCard;
    }

    strmToWriteTo   // V4 stuff
                    << m_bAsyncMode
                    << m_bAsyncFldOver

                    // V5 stuff
                    << m_strAutoSenseClass

                    // V6 stuff
                    << m_strDynRef;


    // And finish off with an end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCDriverCfg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDriverCfg::ParseFixedCfg(const TXMLTreeElement& xtnodeData)
{
    // All the children will be individual prompts
    const tCIDLib::TCard4 c4PromptCnt = xtnodeData.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PromptCnt ; c4Index++)
    {
        // Each child should be a FixedVal element
        const TXMLTreeElement& xtnodeCur
                                = xtnodeData.xtnodeChildAtAsElement(c4Index);

        // Add an item with this name
        const TString& strItem = xtnodeCur.xtattrNamed(L"CQCCfg:Name").strValue();
        m_nvmFixed.AddItem(strItem);

        //
        //  And loop through the values within this fixed config item and
        //  add each one.
        //
        const tCIDLib::TCard4 c4ValCount = xtnodeCur.c4ChildCount();
        for (tCIDLib::TCard4 c4ValInd = 0; c4ValInd < c4ValCount; c4ValInd++)
        {
            const TXMLTreeElement& xtnodeVal
                                    = xtnodeCur.xtnodeChildAtAsElement(c4ValInd);

            const TString& strName = xtnodeVal.xtattrNamed(L"CQCCfg:Name").strValue();
            const TString& strValue = xtnodeVal.xtattrNamed(L"CQCCfg:Value").strValue();
            m_nvmFixed.AddValue(strItem, strName, strValue);
        }
    }
}


tCIDLib::TVoid
TCQCDriverCfg::ParseFromNode(const TXMLTreeElement& xtnodeDriverCfg)
{
    TString strTmp;

    // Make sure that we got the node type we are expected
    AssertElemName(xtnodeDriverCfg, L"CQCCfg:DriverCfg")

    //
    //  Get the driver architecture version out and stored since it can
    //  control what's legal or not down the line.
    //
    const TString& strArchVer = xtnodeDriverCfg.xtattrNamed(L"CQCCfg:ArchVer").strValue();
    if (!strArchVer.bToCard4(m_c4ArchVersion)
    ||  !m_c4ArchVersion
    ||  (m_c4ArchVersion > kCQCKit::c4MaxArchVersion))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadDrvArchVer
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strArchVer
        );
    }

    // The first node must be the server config element
    const TXMLTreeElement& xtnodeSrvCfg
                                = xtnodeDriverCfg.xtnodeChildAtAsElement(0);
    AssertElemName(xtnodeSrvCfg, L"CQCCfg:ServerCfg")

    // And the second node must the common stuff
    const TXMLTreeElement& xtnodeCommCfg
                                = xtnodeDriverCfg.xtnodeChildAtAsElement(1);
    AssertElemName(xtnodeCommCfg, L"CQCCfg:CommonCfg")

    //
    //  Reset any members that would only get updated if an optional block
    //  is present. If those blocks aren't present, we have to be sure that
    //  we've reset these, or another parse will leave that stuff set.
    //
    delete m_pconncfgDef;
    m_pconncfgDef = 0;
    m_strClientExtraCfg.Clear();
    m_strClientLibName.Clear();
    m_colPrompts.RemoveAll();
    m_nvmFixed.Reset();
    if (m_pcolExtraFacs)
        m_pcolExtraFacs->RemoveAll();
    if (m_pcolExtraFilesC)
        m_pcolExtraFilesC->RemoveAll();
    if (m_pcolExtraFilesS)
        m_pcolExtraFilesS->RemoveAll();

    // Get the server info out
    if (xtnodeSrvCfg.xtattrNamed(L"CQCCfg:DrvType").strValue() == L"CppLib")
        m_eType = tCQCKit::EDrvTypes::CppLib;
    else if (xtnodeSrvCfg.xtattrNamed(L"CQCCfg:DrvType").strValue() == L"GenProto")
        m_eType = tCQCKit::EDrvTypes::GenProto;
    else
        m_eType = tCQCKit::EDrvTypes::Macro;
    m_strServerLibName  = xtnodeSrvCfg.xtattrNamed(L"CQCCfg:LibName").strValue();
    m_strServerExtraCfg = xtnodeSrvCfg.xtattrNamed(L"CQCCfg:ExtraCfg").strValue();
    m_strDynRef = xtnodeSrvCfg.xtattrNamed(L"CQCCfg:DynRef").strValue();

    // Parse out the driver capabilities flags
    {
        m_c4CapFlags = 0;
        const TString& strCaps = xtnodeCommCfg.xtattrNamed(L"CQCCfg:Caps").strValue();
        TStringTokenizer stokCaps(&strCaps, L", ");
        TString strCap;
        while (stokCaps.bGetNextToken(strTmp))
        {
            strCap = L"EDrvCap_";
            strCap.Append(strTmp);

            const tCQCKit::EDrvCaps eCap = tCQCKit::eXlatEDrvCaps(strCap);
            if (eCap != tCQCKit::EDrvCaps::Count)
                m_c4CapFlags |= tCIDLib::TCard4(eCap);
        }
    }

    //
    //  Get the common stuff out Get the display name first since we'll use it
    //  if we throw any errors decoding this stuff.
    //
    m_strDisplayName = xtnodeCommCfg.xtattrNamed(L"CQCCfg:DisplayName").strValue();
    m_strAuthor = xtnodeCommCfg.xtattrNamed(L"CQCCfg:Author").strValue();
    m_strContactEMail = xtnodeCommCfg.xtattrNamed(L"CQCCfg:Contact").strValue();
    m_strDescription = xtnodeCommCfg.xtattrNamed(L"CQCCfg:Description").strValue();
    m_strMake = xtnodeCommCfg.xtattrNamed(L"CQCCfg:Make").strValue();
    m_strModel = xtnodeCommCfg.xtattrNamed(L"CQCCfg:Model").strValue();
    m_bAsyncMode = xtnodeCommCfg.xtattrNamed(L"CQCCfg:AsyncMode").bValueAs();
    m_bAsyncFldOver = xtnodeCommCfg.xtattrNamed(L"CQCCfg:AsyncFldOver").bValueAs();

    // Translate the device category
    const TString& strDevCat = xtnodeCommCfg.xtattrNamed(L"CQCCfg:Category").strValue();
    m_eCategory = tCQCKit::eXlatEDevCats(strDevCat);
    if (m_eCategory == tCQCKit::EDevCats::Count)
    {
        m_eCategory = tCQCKit::EDevCats::Miscellaneous;
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadDevCat
            , tCIDLib::ESeverities::Warn
            , tCIDLib::EErrClasses::Format
            , strDevCat
            , m_strMake
            , m_strModel
        );
    }

    //
    //  Process the device classes. We can have one or more comma
    //  separated device category indicators. We only do this if it's a V2
    //  architcture driver,
    //
    m_fcolDevClasses.RemoveAll();
    if (m_c4ArchVersion == 1)
    {
        m_fcolDevClasses.c4AddElement(tCQCKit::EDevClasses::None);
    }
     else
    {
        const TString& strDevCats = xtnodeCommCfg.xtattrNamed(L"CQCCfg:DevClasses").strValue();
        m_fcolDevClasses.RemoveAll();
        TStringTokenizer stokDevCats(&strDevCats, L", ");
        TString strClass;
        while (stokDevCats.bGetNextToken(strClass))
        {
            const tCQCKit::EDevClasses eClass = tCQCKit::eXlatEDevClasses(strClass);
            if (eClass != tCQCKit::EDevClasses::Count)
            {
                m_fcolDevClasses.c4AddElement(eClass);
            }
             else
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcDevCls_BadDevClass
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::Format
                    , strClass
                    , m_strMake
                    , m_strModel
                );
            }
        }

        // If we didn't get any device classes, then set it to the default none.
        if (m_fcolDevClasses.bIsEmpty())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcDevCls_NoClassSet
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::Format
                , m_strMake
                , m_strModel
            );
            m_fcolDevClasses.c4AddElement(tCQCKit::EDevClasses::None);
        }
    }

    //
    //  The description string can be long, so they will often wrap it, and
    //  we'll therefore need to remove the extra whitespace. We only include
    //  literally spaces, so that we don't remove any formatting characters.
    //
    m_strDescription.Strip(L" ", tCIDLib::EStripModes::Full);

    //
    //  Get a temp ref to the version string and parse it out into the major
    //  and minor versions.
    //
    const TString& strVersion = xtnodeCommCfg.xtattrNamed(L"CQCCfg:Version").strValue();
    try
    {
        // Split the version name into its two parts
        TStringTokenizer stokVersion(&strVersion, L".");

        stokVersion.bGetNextToken(strTmp);
        m_c4MajVersion = strTmp.c4Val();
        stokVersion.bGetNextToken(strTmp);
        m_c4MinVersion = strTmp.c4Val();
    }

    catch(...)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadDriverVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strVersion
            , m_strDisplayName
        );
    }

    //
    //  Build the stringized version of the maj/min version. We don't use the
    //  original, because this way we can insure a consistent format of leading
    //  zeros and such.
    //
    TTextStringOutStream strmTmp(&m_strVersion);
    strmTmp << m_c4MajVersion << L"." << m_c4MinVersion << kCIDLib::FlushIt;
    m_strVersion = strmTmp.strData();

    //
    //  Ok, the rest is optional, so just loop through them and process
    //  what we find. We don't enforce any order here, and just assume that
    //  the XML parser did.
    //
    const tCIDLib::TCard4 c4ChildCount = xtnodeDriverCfg.c4ChildCount();
    for (tCIDLib::TCard4 c4ChildInd = 2; c4ChildInd < c4ChildCount; c4ChildInd++)
    {
        // Get the next available element
        const TXMLTreeElement& xtnodeCur
                        = xtnodeDriverCfg.xtnodeChildAtAsElement(c4ChildInd);

        //
        //  According to the type, get out the info and store it in the
        //  appropriate place
        //
        if (xtnodeCur.strQName() == L"CQCCfg:ClientCfg")
        {
            m_strClientLibName
                    = xtnodeCur.xtattrNamed(L"CQCCfg:LibName").strValue();
            m_strClientExtraCfg
                    = xtnodeCur.xtattrNamed(L"CQCCfg:ExtraCfg").strValue();
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:ConnCfg")
        {
            // The only child should be the actual connfiguration
            const TXMLTreeElement& xtnodeConnCfg
                                        = xtnodeCur.xtnodeChildAtAsElement(0);

            //
            //  Parse out the connection config. This will see what kind of
            //  config it is (e.g. Comm, USB, Firewire, etc...) and create
            //  the correct type of connection config object for it. Its the
            //  first subelement of the driver config node.
            //
            TCQCConnCfg* pconncfgTmp = 0;
            if (xtnodeConnCfg.strQName() == L"CQCCfg:SerialConn")
            {
                pconncfgTmp = new TCQCSerialConnCfg;
            }
             else if (xtnodeConnCfg.strQName() == L"CQCCfg:IPConn")
            {
                pconncfgTmp = new TCQCIPConnCfg;
            }
             else if (xtnodeConnCfg.strQName() == L"CQCCfg:OtherConn")
            {
                pconncfgTmp = new TCQCOtherConnCfg;
            }
             else if (xtnodeConnCfg.strQName() == L"CQCCfg:UPnPConn")
            {
                pconncfgTmp = new TCQCUPnPConnCfg;
            }
             else
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCfg_UnknownConnCfg
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , xtnodeConnCfg.strQName()
                );
            }

            // Let it parse its content. Use a janitor until it's known good
            TJanitor<TCQCConnCfg> janConnCfg(pconncfgTmp);
            pconncfgTmp->LoadFromXML(xtnodeConnCfg);

            // Set the ediable flag
            m_bEditableConn =
                (xtnodeCur.xtattrNamed(L"CQCCfg:Editable").strValue() == L"True");

            // Replace the current config with the new one
            delete m_pconncfgDef;
            m_pconncfgDef = janConnCfg.pobjOrphan();
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:OtherCfg")
        {
            ParseOtherCfg(xtnodeCur);
        }
    }
}


tCIDLib::TVoid TCQCDriverCfg::ParseOtherCfg(const TXMLTreeElement& xtnodeData)
{
    tCIDLib::EModFlags  eFlags;
    tCIDLib::EModTypes  eType;
    TString             strFilePath;

    // All the children have to be extra file and extra facility elements
    const tCIDLib::TCard4 c4ExtraCnt = xtnodeData.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ExtraCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur
                                = xtnodeData.xtnodeChildAtAsElement(c4Index);

        if (xtnodeCur.strQName() == L"CQCCfg:AutoSenseClass")
        {
            // The class path is the text content of this element
            m_strAutoSenseClass
                = xtnodeCur.xtnodeChildAtAsElement(0).xtnodeChildAtAsText(0).strText();
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:ExtraFiles")
        {
            //
            //  Its a file name to add to either the client or server side
            //  extra files list. If debugging, sanity check the parser.
            //
            const TString& strLoc
                        = xtnodeCur.xtattrNamed(L"CQCCfg:Location").strValue();
            strFilePath = xtnodeCur.xtattrNamed(L"CQCCfg:Path").strValue();

            // Just in case, make sure it has regular file type slashes
            strFilePath.bReplaceChar(L'/', L'\\');

            if (strLoc == L"Client")
            {
                // Allocate the list if not already
                if (!m_pcolExtraFilesC)
                    m_pcolExtraFilesC = new TBag<TString>;
                m_pcolExtraFilesC->objAdd(strFilePath);
            }
             else if (strLoc == L"Server")
            {
                // Allocate the list if not already
                if (!m_pcolExtraFilesS)
                    m_pcolExtraFilesS = new TBag<TString>;
                m_pcolExtraFilesS->objAdd(strFilePath);
            }
             else
            {
                #if CID_DEBUG_ON
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCfg_BadExtraFileLoc
                    , tCIDLib::ESeverities::ProcFatal
                    , tCIDLib::EErrClasses::Internal
                    , strLoc
                );
                #endif
            }
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:ExtraFacs")
        {
            // If we haven't allocate the collection yet, then do it
            if (!m_pcolExtraFacs)
                m_pcolExtraFacs = new TBag<TCQCModuleInfo>;

            // Get a ref to the name for later use
            const TString& strName
                        = xtnodeCur.xtattrNamed(L"CQCCfg:BaseName").strValue();

            // Set up the module type. If debugging, sanity check the parser
            const TString& strType
                        = xtnodeCur.xtattrNamed(L"CQCCfg:Type").strValue();
            if ((strType == L"SharedLib")
            ||  (strType == L"SharedObj"))
            {
                eType = tCIDLib::EModTypes::SharedLib;
            }
             else if (strType == L"Exe")
            {
                eType = tCIDLib::EModTypes::Exe;
            }
             else
            {
                // Make the compiler happy
                eType = tCIDLib::EModTypes::Exe;
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCfg_BadExtraFacType
                    , tCIDLib::ESeverities::ProcFatal
                    , tCIDLib::EErrClasses::Internal
                    , strType
                );
            }

            // And set up the flags
            eFlags = tCIDLib::EModFlags::None;
            if (xtnodeCur.xtattrNamed(L"CQCCfg:MsgFile").bValueAs())
                eFlags = tCIDLib::eOREnumBits(eFlags, tCIDLib::EModFlags::HasMsgFile);
            if (xtnodeCur.xtattrNamed(L"CQCCfg:ResFile").bValueAs())
                eFlags = tCIDLib::eOREnumBits(eFlags, tCIDLib::EModFlags::HasResFile);

            //
            //  Get a ref to the version and parse out the maj/min version vals.
            //  If it is empty, then assume current CQC version.
            //
            const TString& strVersion
                        = xtnodeCur.xtattrNamed(L"CQCCfg:Version").strValue();
            tCIDLib::TCard4 c4MajVer;
            tCIDLib::TCard4 c4MinVer;
            if (strVersion.bIsEmpty())
            {
                c4MajVer = kCQCKit::c4MajVersion;
                c4MinVer = kCQCKit::c4MinVersion;
            }
             else
            {
                TModule::ParseVersionStr(strVersion, c4MajVer, c4MinVer);
            }

            // And figure out which side wants this one
            const TString& strSides
                        = xtnodeCur.xtattrNamed(L"CQCCfg:UsedBy").strValue();
            tCIDLib::ECSSides eSides;
            if (strSides == L"Client")
                eSides = tCIDLib::ECSSides::Client;
            else if (strSides == L"Server")
                eSides = tCIDLib::ECSSides::Server;
            else if (strSides == L"Shared")
                eSides = tCIDLib::ECSSides::Both;
            else
            {
                // Make the compiler happy
                eSides = tCIDLib::ECSSides::Both;
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCfg_BadExtraFacSide
                    , tCIDLib::ESeverities::ProcFatal
                    , tCIDLib::EErrClasses::Internal
                    , strSides
                );
            }

            // And now we can add an element to the list
            m_pcolExtraFacs->objPlace(strName, eSides, eType, c4MajVer, c4MinVer, eFlags);
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgPrompts")
        {
            ParsePrompts(xtnodeCur);
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:FixedCfgVals")
        {
            ParseFixedCfg(xtnodeCur);
        }
         else
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCfg_UnknownExtraCfg
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , xtnodeCur.strQName()
            );
        }
    }
}


tCIDLib::TVoid TCQCDriverCfg::ParsePrompts(const TXMLTreeElement& xtnodeData)
{
    // All the children will be individual prompts
    const tCIDLib::TCard4 c4PromptCnt = xtnodeData.c4ChildCount();
    TCQCDCfgPrompt cqcdcpCur;
    TString strLimits;
    TString strText;
    TString strValNames;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PromptCnt ; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur
                                = xtnodeData.xtnodeChildAtAsElement(c4Index);

        // ALl of them have to have a Name attribute
        const TString& strName = xtnodeCur.xtattrNamed(L"CQCCfg:Name").strValue();

        // The optional attribute we can do generically
        tCIDLib::TCard4 c4At;
        tCIDLib::TBoolean bOptional = kCIDLib::False;
        if (xtnodeCur.bAttrExists(L"CQCCfg:Optional", c4At))
            bOptional = xtnodeCur.xtattrAt(c4At).bValueAs();

        // The flag attribute is false unless explicitly set below
        tCIDLib::TBoolean bFlag = kCIDLib::False;

        tCQCKit::EPromptTypes eType;
        strLimits.Clear();
        if (xtnodeCur.strQName() == L"CQCCfg:CfgACBPrompt")
        {
            //
            //  We use the limits string to store the search criteria, and the
            //  flag attribute to store the path only indicator.
            //
            eType = tCQCKit::EPromptTypes::TrayMonBinding;
            strValNames = L"Binding, Path";
            strLimits = xtnodeCur.xtattrNamed(L"CQCCfg:SearchCrit").strValue();
            bFlag = (xtnodeCur.xtattrNamed(L"CQCCfg:PathOnly").strValue() == L"Yes");
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgADPrompt")
        {
            eType = tCQCKit::EPromptTypes::AudioDev;
            strValNames = L"DevName";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgBPrompt")
        {
            // We use the limits just to store the check box text
            eType = tCQCKit::EPromptTypes::Bool;
            strLimits = xtnodeCur.xtattrNamed(L"CQCCfg:CheckText").strValue();
            strValNames = L"State";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgFPrompt")
        {
            // We use the limits to store the search criteria
            eType = tCQCKit::EPromptTypes::File;
            strValNames = L"File";
            strLimits = xtnodeCur.xtattrNamed(L"CQCCfg:SearchCrit").strValue();
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgLPrompt")
        {
            // We use the limits to store the item list to select from
            eType = tCQCKit::EPromptTypes::List;
            strLimits = xtnodeCur.xtattrNamed(L"CQCCfg:ItemList").strValue();
            strValNames = L"Selected";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgMLPrompt")
        {
            eType = tCQCKit::EPromptTypes::MultiList;

            //
            //  This guy supports a scheme where a numbered list can be
            //  generated, so check for a NumRange attr.
            //
            if (xtnodeCur.bAttrExists(L"CQCCfg:NumRange", c4At))
            {
                //
                //  The value should be in the form "f,l", i.e. the first and
                //  last numeric values to use.
                //
                strText = xtnodeCur.xtattrAt(c4At).strValue();
                if (!strText.bSplit(strLimits, L','))
                {
                    facCQCKit().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcCfg_BadMLRange
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , strName
                    );
                }

                tCIDLib::TCard4 c4First = 0;
                tCIDLib::TCard4 c4Last = 0;
                try
                {
                    c4First = strText.c4Val(tCIDLib::ERadices::Dec);
                    c4Last = strLimits.c4Val(tCIDLib::ERadices::Dec);
                }

                catch(...)
                {
                    facCQCKit().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcCfg_BadMLRange
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , strName
                    );
                }

                // The ItemList value contains the pattern
                strText = xtnodeCur.xtattrNamed(L"CQCCfg:ItemList").strValue();

                // Make sure it has a %(1) token in it
                if (!strText.bTokenExists(L'1'))
                {
                    facCQCKit().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcCfg_BadMLRange
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , strName
                    );
                }

                // Looks ok, so generate the values
                strLimits.Clear();
                for (tCIDLib::TCard4 c4MLInd = c4First; c4MLInd <= c4Last; c4MLInd++)
                {
                    strValNames = strText;
                    strValNames.eReplaceToken(TCardinal(c4MLInd), L'1');
                    if (!strLimits.bIsEmpty())
                        strLimits.Append(L",");
                    strLimits.Append(strValNames);
                }
            }
             else
            {
                // Nothing special, the values are explicitly provided
                strLimits = xtnodeCur.xtattrNamed(L"CQCCfg:ItemList").strValue();
            }
            strValNames = L"Selections";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgNMLPrompt")
        {
            eType = tCQCKit::EPromptTypes::NamedMList;
            strValNames = L"Selections";

            // The limits string is used to hold the max number of items
            strLimits = xtnodeCur.xtattrNamed(L"CQCCfg:Count").strValue();
            tCIDLib::TCard4 c4Cnt;
            if (!strLimits.bToCard4(c4Cnt, tCIDLib::ERadices::Dec))
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCfg_BadCount
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , strName
                );
            }

            // The extra flag holds the 'allow spaces in names' indicator
            bFlag = xtnodeCur.xtattrNamed(L"CQCCfg:Spaces").strValue() == L"Yes";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgODBCPrompt")
        {
            eType = tCQCKit::EPromptTypes::ODBCSrc;
            strValNames = L"SrcName";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgPPrompt")
        {
            //
            //  Store the fully qualified path flag in the limits field, since
            //  we only have one general purpose flag we can store.
            //
            eType = tCQCKit::EPromptTypes::Path;
            bFlag = xtnodeCur.xtattrNamed(L"CQCCfg:FQPath").strValue() == L"Yes";
            strLimits = xtnodeCur.xtattrNamed(L"CQCCfg:FQPath").strValue();
            strValNames = L"Path";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgTPrompt")
        {
            // We use the limits string to store the regular expression
            eType = tCQCKit::EPromptTypes::Text;
            strLimits = xtnodeCur.xtattrNamed(L"CQCCfg:RegEx").strValue();
            strValNames = L"Text";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgRPrompt")
        {
            // Try to convert both values, to make sure that they numbers
            const TString& strLow = xtnodeCur.xtattrNamed(L"CQCCfg:LowVal").strValue();
            const TString& strHigh = xtnodeCur.xtattrNamed(L"CQCCfg:HighVal").strValue();
            try
            {
                tCIDLib::IgnoreRet(strLow.i4Val());
                tCIDLib::IgnoreRet(strHigh.i4Val());
            }

            catch(...)
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCfg_BadRangeLimit
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , strName
                );
            }

            eType = tCQCKit::EPromptTypes::Range;
            strLimits = strLow;
            strLimits.Append(kCIDLib::chComma);
            strLimits.Append(strHigh);
            strValNames = L"Value";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgRepoPrompt")
        {
            eType = tCQCKit::EPromptTypes::RepoDriver;
            strValNames = L"Moniker";
        }
         else if (xtnodeCur.strQName() == L"CQCCfg:CfgTTSVPrompt")
        {
            eType = tCQCKit::EPromptTypes::TTSVoice;
            strValNames = L"VoiceName";
        }
         else
        {
            // Make the compiler happy
            eType = tCQCKit::EPromptTypes::Count;
        }

        if (eType == tCQCKit::EPromptTypes::Count)
        {
            if (facCQCKit().bLogFailures())
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCfg_UnknownPromptType
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , xtnodeCur.strQName()
                );
            }
        }
         else
        {
            //
            //  The first child should be a PromptText element whose only
            //  child should be the text node with the prompt text. Do a full
            //  strip on it, to get rid of any new lines and extra space.
            //
            strText = xtnodeCur.xtnodeChildAtAsElement(0).xtnodeChildAtAsText(0).strText();
            strText.StripWhitespace(tCIDLib::EStripModes::Full);

            // Set up all of the stuff we got on a temp object
            cqcdcpCur.Set
            (
                strName
                , xtnodeCur.xtattrNamed(L"CQCCfg:Title").strValue()
                , strText
                , strLimits
                , strValNames
                , eType
                , bOptional
                , bFlag
            );

            //
            //  And some can have an optional default values, so any that are
            //  defined. Any child nodes after the 0th one (which was the
            //  text node) are default values nodes.
            //
            //
            const tCIDLib::TCard4 c4ChildCount = xtnodeCur.c4ChildCount();
            for (tCIDLib::TCard4 c4CInd = 1; c4CInd < c4ChildCount; c4CInd++)
            {
                const TXMLTreeElement& xtnodeDef = xtnodeCur.xtnodeChildAtAsElement(c4CInd);
                cqcdcpCur.AddDefault
                (
                    xtnodeDef.xtattrNamed(L"CQCCfg:ValName").strValue()
                    , xtnodeDef.xtattrNamed(L"CQCCfg:Default").strValue()
                );
            }
            m_colPrompts.objAdd(cqcdcpCur);
        }
    }
}


