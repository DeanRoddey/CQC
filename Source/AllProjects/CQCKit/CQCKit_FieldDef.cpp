//
// FILE NAME: CQCKit_FieldDef.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/22/2001
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
//  This is the implementation file for the field description.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCFldDef,TObject)
RTTIDecls(TCQCDrvFldDef,TCQCFldDef)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCKit_FieldDef
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our field def current persistent format version
        //
        //  Version 2 -
        //      Added the semantic type enumeration for the V2 driver architecture.
        //
        //  Version 3 -
        //      Added the 'no store on write' member.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FDefFmtVersion = 3;


        // -----------------------------------------------------------------------
        //  Our driver field def current persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2DFDefFmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldDef
//  PREFIX: fldd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldDef: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TCQCFldDef::eComp(const TCQCFldDef& fldd1, const TCQCFldDef& fldd2)
{
    return fldd1.m_strName.eCompare(fldd2.m_strName);
}


tCIDLib::ESortComps
TCQCFldDef::eIComp(const TCQCFldDef& fldd1, const TCQCFldDef& fldd2)
{
    return fldd1.m_strName.eCompareI(fldd2.m_strName);
}


//
//  This is a special one that deals with V1 vs. V2 driver architecture. We sort
//  all V2 fields first and sort within them, and all V1 fields come after that
//  sorted within that range.
//
tCIDLib::ESortComps
TCQCFldDef::eICompV2(const  TCQCFldDef&         fldd1
                    , const TCQCFldDef&         fldd2)
{
    // Check for special stats fields
    const tCIDLib::TBoolean bF1Spec(fldd1.m_strName[0] == kCIDLib::chDollarSign);
    const tCIDLib::TBoolean bF2Spec(fldd2.m_strName[0] == kCIDLib::chDollarSign);

    // If not the same, then the one that is special is considered greater
    if (bF1Spec != bF2Spec)
    {
        if (bF1Spec)
            return tCIDLib::ESortComps::FirstGreater;
        return tCIDLib::ESortComps::FirstLess;
    }

    // If both are special, then just compare them
    if (bF1Spec && bF2Spec)
        return fldd1.m_strName.eCompareI(fldd2.m_strName);

    //
    //  At this point, neither can be special fields. Check for V2 fields
    const tCIDLib::TBoolean bF1V2(fldd1.m_strName.bContainsChar(kCIDLib::chPoundSign));
    const tCIDLib::TBoolean bF2V2(fldd2.m_strName.bContainsChar(kCIDLib::chPoundSign));

    //
    //  If they are both either V1 or V2, then we are just doing a standard comparison
    //  between the two names.
    //
    if (bF1V2 == bF2V2)
        return fldd1.m_strName.eCompareI(fldd2.m_strName);

    //
    //  They are mixed, so whichever one is V2 is considered less and the other is
    //  greater.
    //
    if (bF1V2)
        return tCIDLib::ESortComps::FirstLess;

    return tCIDLib::ESortComps::FirstGreater;
}


const TString& TCQCFldDef::strKey(const TCQCFldDef& flddSrc)
{
    return flddSrc.strName();
}


// ---------------------------------------------------------------------------
//  TCQCFldDef: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldDef::TCQCFldDef() :

    m_bAlwaysWrite(kCIDLib::False)
    , m_bFlag(kCIDLib::False)
    , m_bNoStoreOnWrite(kCIDLib::False)
    , m_bPrivate(kCIDLib::False)
    , m_bQueuedWrite(kCIDLib::False)
    , m_c4Id(0)
    , m_eAccess(tCQCKit::EFldAccess::Read)
    , m_eSemType(tCQCKit::EFldSTypes::Generic)
    , m_eType(tCQCKit::EFldTypes::Count)
{
}

TCQCFldDef::TCQCFldDef( const   TString&            strName
                        , const tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldAccess eAccess) :

    m_bAlwaysWrite(kCIDLib::False)
    , m_bFlag(kCIDLib::False)
    , m_bNoStoreOnWrite(kCIDLib::False)
    , m_bPrivate(kCIDLib::False)
    , m_bQueuedWrite(kCIDLib::False)
    , m_c4Id(0)
    , m_eAccess(eAccess)
    , m_eSemType(tCQCKit::EFldSTypes::Generic)
    , m_eType(eType)
    , m_strName(strName)
{
    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
}

TCQCFldDef::TCQCFldDef( const   TString&            strName
                        , const tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldAccess eAccess
                        , const tCQCKit::EFldSTypes eSemType) :

    m_bAlwaysWrite(kCIDLib::False)
    , m_bFlag(kCIDLib::False)
    , m_bNoStoreOnWrite(kCIDLib::False)
    , m_bPrivate(kCIDLib::False)
    , m_bQueuedWrite(kCIDLib::False)
    , m_c4Id(0)
    , m_eAccess(eAccess)
    , m_eSemType(eSemType)
    , m_eType(eType)
    , m_strName(strName)
{
    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
}

TCQCFldDef::TCQCFldDef( const   TString&            strName
                        , const tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldAccess eAccess
                        , const TString&            strLimits) :

    m_bAlwaysWrite(kCIDLib::False)
    , m_bFlag(kCIDLib::False)
    , m_bNoStoreOnWrite(kCIDLib::False)
    , m_bPrivate(kCIDLib::False)
    , m_bQueuedWrite(kCIDLib::False)
    , m_c4Id(0)
    , m_eAccess(eAccess)
    , m_eSemType(tCQCKit::EFldSTypes::Generic)
    , m_eType(eType)
    , m_strLimits(strLimits)
    , m_strName(strName)
{
    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
}

TCQCFldDef::TCQCFldDef( const   TString&            strName
                        , const tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldAccess eAccess
                        , const tCQCKit::EFldSTypes eSemType
                        , const TString&            strLimits) :

    m_bAlwaysWrite(kCIDLib::False)
    , m_bFlag(kCIDLib::False)
    , m_bNoStoreOnWrite(kCIDLib::False)
    , m_bPrivate(kCIDLib::False)
    , m_bQueuedWrite(kCIDLib::False)
    , m_c4Id(0)
    , m_eAccess(eAccess)
    , m_eSemType(eSemType)
    , m_eType(eType)
    , m_strLimits(strLimits)
    , m_strName(strName)
{
    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
}

TCQCFldDef::TCQCFldDef( const   TString&            strName
                        , const tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldAccess eAccess
                        , const tCIDLib::TCh* const pszLimits) :

    m_bAlwaysWrite(kCIDLib::False)
    , m_bFlag(kCIDLib::False)
    , m_bNoStoreOnWrite(kCIDLib::False)
    , m_bPrivate(kCIDLib::False)
    , m_bQueuedWrite(kCIDLib::False)
    , m_c4Id(0)
    , m_eAccess(eAccess)
    , m_eSemType(tCQCKit::EFldSTypes::Generic)
    , m_eType(eType)
    , m_strLimits(pszLimits)
    , m_strName(strName)
{
    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
}

TCQCFldDef::TCQCFldDef( const   TString&            strName
                        , const tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldAccess eAccess
                        , const tCQCKit::EFldSTypes eSemType
                        , const tCIDLib::TCh* const pszLimits) :

    m_bAlwaysWrite(kCIDLib::False)
    , m_bFlag(kCIDLib::False)
    , m_bNoStoreOnWrite(kCIDLib::False)
    , m_bPrivate(kCIDLib::False)
    , m_bQueuedWrite(kCIDLib::False)
    , m_c4Id(0)
    , m_eAccess(eAccess)
    , m_eSemType(eSemType)
    , m_eType(eType)
    , m_strLimits(pszLimits)
    , m_strName(strName)
{
    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
}

TCQCFldDef::~TCQCFldDef()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldDef: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldDef::operator==(const TCQCFldDef& flddSrc) const
{
    // Do them in the order most likely to differ if not equal
    if ((m_strName          != flddSrc.m_strName)
    ||  (m_eSemType         != flddSrc.m_eSemType)
    ||  (m_eType            != flddSrc.m_eType)
    ||  (m_c4Id             != flddSrc.m_c4Id)
    ||  (m_strLimits        != flddSrc.m_strLimits)
    ||  (m_strExtraCfg      != flddSrc.m_strExtraCfg)
    ||  (m_bAlwaysWrite     != flddSrc.m_bAlwaysWrite)
    ||  (m_bPrivate         != flddSrc.m_bPrivate)
    ||  (m_bQueuedWrite     != flddSrc.m_bQueuedWrite)
    ||  (m_eAccess          != flddSrc.m_eAccess)
    ||  (m_bFlag            != flddSrc.m_bFlag)
    ||  (m_bNoStoreOnWrite  != flddSrc.m_bNoStoreOnWrite))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCFldDef::operator!=(const TCQCFldDef& flddToComp) const
{
    return !operator==(flddToComp);
}


// ---------------------------------------------------------------------------
//  TCQCFldDef: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the always write flag
tCIDLib::TBoolean TCQCFldDef::bAlwaysWrite() const
{
    return m_bAlwaysWrite;
}

tCIDLib::TBoolean TCQCFldDef::bAlwaysWrite(const tCIDLib::TBoolean bToSet)
{
    m_bAlwaysWrite = bToSet;
    return m_bAlwaysWrite;
}


// Get/set the general purpose flag
tCIDLib::TBoolean TCQCFldDef::bFlag() const
{
    return m_bFlag;
}

tCIDLib::TBoolean TCQCFldDef::bFlag(const tCIDLib::TBoolean bToSet)
{
    m_bFlag = bToSet;
    return m_bFlag;
}



// Returns true if we represent an enumerated string type field
tCIDLib::TBoolean TCQCFldDef::bIsEnumType() const
{
    if (m_eType != tCQCKit::EFldTypes::String)
        return kCIDLib::False;
    return m_strLimits.bStartsWithI(L"enum:");
}


// Returns true if we have a numeric field type
tCIDLib::TBoolean TCQCFldDef::bIsNumericType() const
{
    return  (m_eType == tCQCKit::EFldTypes::Card)
            || (m_eType == tCQCKit::EFldTypes::Float)
            || (m_eType == tCQCKit::EFldTypes::Int);
}


// A convenience to check the access for readable or writeable
tCIDLib::TBoolean TCQCFldDef::bIsReadable() const
{
    return tCIDLib::bAllBitsOn(m_eAccess,  tCQCKit::EFldAccess::Read);
}

tCIDLib::TBoolean TCQCFldDef::bIsWriteable() const
{
    return tCIDLib::bAllBitsOn(m_eAccess, tCQCKit::EFldAccess::Write);
}



// Get/set the no store on write flag
tCIDLib::TBoolean TCQCFldDef::bNoStoreOnWrite() const
{
    return m_bNoStoreOnWrite;
}

tCIDLib::TBoolean TCQCFldDef::bNoStoreOnWrite(const tCIDLib::TBoolean bToSet)
{
    m_bNoStoreOnWrite = bToSet;
    return m_bNoStoreOnWrite;
}


// Get/set the private flag
tCIDLib::TBoolean TCQCFldDef::bPrivate() const
{
    return m_bPrivate;
}

tCIDLib::TBoolean TCQCFldDef::bPrivate(const tCIDLib::TBoolean bToSet)
{
    m_bPrivate = bToSet;
    return m_bPrivate;
}


// Get/set the queued write flag
tCIDLib::TBoolean TCQCFldDef::bQueuedWrite() const
{
    return m_bQueuedWrite;
}

tCIDLib::TBoolean TCQCFldDef::bQueuedWrite(const tCIDLib::TBoolean bToSet)
{
    m_bQueuedWrite = bToSet;
    return m_bQueuedWrite;
}


//
//  A helper for the base driver class, which tells it if this field should, upon a
//  successfull return from the derived driver class from a field write, store the
//  new value. This is far and away the norm, but sometimes not.
//
//  1. If we are marked for queued writes, then obviously not since the transmission
//      to the device hasn't even happened yet.
//  2. If we are marked 'no store on write', which is an explicit indication that the
//      driver can't really determine the success when the write is done, and will
//      know later when some response comes back in.
//
//  So the caller should store it if we are not queued write and not 'no store'.
//
tCIDLib::TBoolean TCQCFldDef::bStoreClientWrite() const
{
    return (m_bQueuedWrite || m_bNoStoreOnWrite) == kCIDLib::False;
}


// Get/set the field id
tCIDLib::TCard4 TCQCFldDef::c4Id() const
{
    return m_c4Id;
}

tCIDLib::TCard4 TCQCFldDef::c4Id(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Id = c4ToSet;
    return m_c4Id;
}

tCQCKit::EFldAccess TCQCFldDef::eAccess() const
{
    return m_eAccess;
}


tCQCKit::EFldSTypes TCQCFldDef::eSemType() const
{
    return m_eSemType;
}

tCQCKit::EFldSTypes TCQCFldDef::eSemType(const tCQCKit::EFldSTypes eToSet)
{
    m_eSemType = eToSet;
    return m_eSemType;
}


tCQCKit::EFldTypes TCQCFldDef::eType() const
{
    return m_eType;
}

tCQCKit::EFldTypes TCQCFldDef::eType(const tCQCKit::EFldTypes eToSet)
{
    m_eType = eToSet;
    return m_eType;
}


//
//  If this is a numeric type field, return the formatted min and max values
//  it can contain. If not numeric, throw. We return them as formatted values
//  since the type can be different.
//
tCIDLib::TVoid
TCQCFldDef::QueryNumericRange(TString& strMinVal, TString& strMaxVal) const
{
    // Gen up a temporary limit object based on this field's limits
    TCQCFldLimit* pfldlTest = TCQCFldLimit::pfldlMakeNew(*this);
    TJanitor<TCQCFldLimit> janLimits(pfldlTest);

    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Card :
        {
            TCQCFldCardLimit* pfldlCard(static_cast<TCQCFldCardLimit*>(pfldlTest));
            strMinVal.SetFormatted(pfldlCard->c4Min());
            strMaxVal.SetFormatted(pfldlCard->c4Max());
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFldFloatLimit* pfldlCard(static_cast<TCQCFldFloatLimit*>(pfldlTest));
            strMinVal.SetFormatted(pfldlCard->f8Min());
            strMaxVal.SetFormatted(pfldlCard->f8Max());
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCFldIntLimit* pfldlCard(static_cast<TCQCFldIntLimit*>(pfldlTest));
            strMinVal.SetFormatted(pfldlCard->i4Min());
            strMaxVal.SetFormatted(pfldlCard->i4Max());
            break;
        }

        default :
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_NotNumeric
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
                , m_strName
            );
            break;
    };
}


tCIDLib::TVoid TCQCFldDef::Reset()
{
    m_bAlwaysWrite      = kCIDLib::False;
    m_bFlag             = kCIDLib::False;
    m_bNoStoreOnWrite   = kCIDLib::False;
    m_bPrivate          = kCIDLib::False;
    m_bQueuedWrite      = kCIDLib::False;
    m_c4Id              = 0;
    m_eAccess           = tCQCKit::EFldAccess::Read;
    m_eSemType          = tCQCKit::EFldSTypes::Generic;
    m_eType             = tCQCKit::EFldTypes::Count;

    m_strExtraCfg.Clear();
    m_strLimits.Clear();
    m_strName.Clear();
}


const TString& TCQCFldDef::strExtraCfg() const
{
    return m_strExtraCfg;
}

const TString& TCQCFldDef::strExtraCfg(const TString& strToSet)
{
    m_strExtraCfg = strToSet;
    return m_strExtraCfg;
}


const TString& TCQCFldDef::strLimits() const
{
    return m_strLimits;
}

const TString& TCQCFldDef::strLimits(const TString& strToSet)
{
    m_strLimits = strToSet;
    m_strLimits.StripWhitespace();
    return m_strLimits;
}


const TString& TCQCFldDef::strName() const
{
    return m_strName;
}

const TString& TCQCFldDef::strName(const TString& strToSet)
{
    m_strName = strToSet;
    m_strName.StripWhitespace();
    return m_strName;
}


tCIDLib::TVoid TCQCFldDef::Set( const   TString&            strName
                                , const tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldAccess eAccess
                                , const TString&            strLimits
                                , const tCIDLib::TBoolean   bAlwaysWrite)
{
    m_bAlwaysWrite      = bAlwaysWrite;
    m_bFlag             = kCIDLib::False;
    m_bNoStoreOnWrite   = kCIDLib::False;
    m_bPrivate          = kCIDLib::False;
    m_bQueuedWrite      = kCIDLib::False;
    m_eAccess           = eAccess;
    m_eSemType          = tCQCKit::EFldSTypes::Generic;
    m_eType             = eType;
    m_strLimits         = strLimits;
    m_strName           = strName;
    m_c4Id              = 0;

    m_strExtraCfg.Clear();

    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
    m_strLimits.StripWhitespace();
}

tCIDLib::TVoid TCQCFldDef::Set( const   TString&            strName
                                , const tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldAccess eAccess
                                , const tCQCKit::EFldSTypes eSemType
                                , const TString&            strLimits
                                , const tCIDLib::TBoolean   bAlwaysWrite)
{
    m_bAlwaysWrite      = bAlwaysWrite;
    m_bFlag             = kCIDLib::False;
    m_bNoStoreOnWrite   = kCIDLib::False;
    m_bPrivate          = kCIDLib::False;
    m_bQueuedWrite      = kCIDLib::False;
    m_eAccess           = eAccess;
    m_eSemType          = eSemType;
    m_eType             = eType;
    m_strLimits         = strLimits;
    m_strName           = strName;
    m_c4Id              = 0;

    m_strExtraCfg.Clear();

    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
    m_strLimits.StripWhitespace();
}

tCIDLib::TVoid TCQCFldDef::Set( const   TString&            strName
                                , const tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldAccess eAccess
                                , const tCIDLib::TCh* const pszLimits
                                , const tCIDLib::TBoolean   bAlwaysWrite)
{
    m_bAlwaysWrite      = bAlwaysWrite;
    m_bFlag             = kCIDLib::False;
    m_bNoStoreOnWrite   = kCIDLib::False;
    m_bPrivate          = kCIDLib::False;
    m_bQueuedWrite      = kCIDLib::False;
    m_eAccess           = eAccess;
    m_eSemType          = tCQCKit::EFldSTypes::Generic;
    m_eType             = eType;
    m_strLimits         = pszLimits;
    m_strName           = strName;
    m_c4Id              = 0;

    m_strExtraCfg.Clear();

    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
    m_strLimits.StripWhitespace();
}

tCIDLib::TVoid TCQCFldDef::Set( const   TString&            strName
                                , const tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldAccess eAccess
                                , const tCQCKit::EFldSTypes eSemType
                                , const tCIDLib::TCh* const pszLimits
                                , const tCIDLib::TBoolean   bAlwaysWrite)
{
    m_bAlwaysWrite      = bAlwaysWrite;
    m_bFlag             = kCIDLib::False;
    m_bNoStoreOnWrite   = kCIDLib::False;
    m_bPrivate          = kCIDLib::False;
    m_bQueuedWrite      = kCIDLib::False;
    m_eAccess           = eAccess;
    m_eSemType          = eSemType;
    m_eType             = eType;
    m_strLimits         = pszLimits;
    m_strName           = strName;
    m_c4Id              = 0;

    m_strExtraCfg.Clear();

    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
    m_strLimits.StripWhitespace();
}

tCIDLib::TVoid TCQCFldDef::Set( const   TString&            strName
                                , const tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldAccess eAccess
                                , const tCIDLib::TBoolean   bAlwaysWrite)
{
    m_bAlwaysWrite      = bAlwaysWrite;
    m_bFlag             = kCIDLib::False;
    m_bNoStoreOnWrite   = kCIDLib::False;
    m_bPrivate          = kCIDLib::False;
    m_bQueuedWrite      = kCIDLib::False;
    m_eAccess           = eAccess;
    m_eSemType          = tCQCKit::EFldSTypes::Generic;
    m_eType             = eType;
    m_strName           = strName;
    m_c4Id              = 0;
    m_strLimits.Clear();

    m_strExtraCfg.Clear();

    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
    m_strLimits.StripWhitespace();
}

tCIDLib::TVoid TCQCFldDef::Set( const   TString&            strName
                                , const tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldAccess eAccess
                                , const tCQCKit::EFldSTypes eSemType
                                , const tCIDLib::TBoolean   bAlwaysWrite)
{
    m_bAlwaysWrite      = bAlwaysWrite;
    m_bFlag             = kCIDLib::False;
    m_bNoStoreOnWrite   = kCIDLib::False;
    m_bPrivate          = kCIDLib::False;
    m_bQueuedWrite      = kCIDLib::False;
    m_eAccess           = eAccess;
    m_eSemType          = eSemType;
    m_eType             = eType;
    m_strName           = strName;
    m_c4Id              = 0;
    m_strLimits.Clear();

    m_strExtraCfg.Clear();

    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();
    m_strLimits.StripWhitespace();
}


// ---------------------------------------------------------------------------
//  TCQCFldDef: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCFldDef::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_FieldDef::c2FDefFmtVersion))
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

    strmToReadFrom  >> m_bAlwaysWrite
                    >> m_bPrivate
                    >> m_bQueuedWrite
                    >> m_c4Id
                    >> m_eAccess
                    >> m_eType
                    >> m_strExtraCfg
                    >> m_strLimits
                    >> m_strName;

    // If V2 or beyond, read in the semantic type, else default it
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_eSemType;
    else
        m_eSemType = tCQCKit::EFldSTypes::Generic;

    // If V3 or beyond, read in the no store on write flag
    if (c2FmtVersion > 2)
        strmToReadFrom >> m_bNoStoreOnWrite;
    else
        m_bNoStoreOnWrite = kCIDLib::False;

    // And the end object marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Reset runtime only stuff
    m_bFlag = kCIDLib::False;
}


tCIDLib::TVoid TCQCFldDef::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_FieldDef::c2FDefFmtVersion
                    << m_bAlwaysWrite
                    << m_bPrivate
                    << m_bQueuedWrite
                    << m_c4Id
                    << m_eAccess
                    << m_eType
                    << m_strExtraCfg
                    << m_strLimits
                    << m_strName

                    // V2 stuff
                    << m_eSemType

                    // V3 stuff
                    << m_bNoStoreOnWrite

                    << tCIDLib::EStreamMarkers::Frame;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvFldDef
//  PREFIX: fldd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDrvFldDef: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDrvFldDef::TCQCDrvFldDef(const  TString&    strMoniker
                            , const TCQCFldDef& flddSrc) :
    TCQCFldDef(flddSrc)
    , m_strMoniker(strMoniker)
{
    // Can't have any whitespace
    m_strMoniker.StripWhitespace();
}

// ---------------------------------------------------------------------------
//  TCQCDrvFldDef: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCDrvFldDef::operator==(const TCQCDrvFldDef& dflddSrc) const
{
    return TParent::operator==(dflddSrc)
           && (m_strMoniker == dflddSrc.m_strMoniker);
}

tCIDLib::TBoolean TCQCDrvFldDef::operator!=(const TCQCDrvFldDef& dflddSrc) const
{
    return !operator==(dflddSrc)
           || (m_strMoniker != dflddSrc.m_strMoniker);
}


// ---------------------------------------------------------------------------
//  TCQCDrvFldDef: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TString& TCQCDrvFldDef::strMoniker() const
{
    return m_strMoniker;
}

const TString& TCQCDrvFldDef::strMoniker(const TString& strToSet)
{
    m_strMoniker = strToSet;
    return m_strMoniker;
}


// ---------------------------------------------------------------------------
//  TCQCDrvFldDef: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDrvFldDef::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_FieldDef::c2DFDefFmtVersion))
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

    // Do our parent class
    TParent::StreamFrom(strmToReadFrom);

    // And do our stuff
    strmToReadFrom  >> m_strMoniker;

    // And the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCDrvFldDef::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_FieldDef::c2DFDefFmtVersion;

    // Do our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    // And do our stuff
    strmToWriteTo   << m_strMoniker
                    << tCIDLib::EStreamMarkers::EndObject;
}


