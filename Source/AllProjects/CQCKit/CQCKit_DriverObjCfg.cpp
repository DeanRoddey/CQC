//
// FILE NAME: CQCKit_DriverObjCfg.cpp
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
//  This file implements the TCQCDriverObjCfg class.
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
RTTIDecls(TCQCDriverObjCfg,TCQCDriverCfg)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCKit_DriverObjCfg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version for the driver object config class.
        //
        //  Format history:
        //      1.  Original Version (not supported anymore)
        //      2.  Changed from storing prompt values in the named value map
        //          class instead of just simple key/value pairs.
        //      3.  Added event triggers in 1.4.25.
        //      4.  Added the m_bPaused flag in 4.5.9. Also removed the ancient
        //          check for paths using / style slashes, which we long ago used
        //          but later got rid of.
        //      5.  We are storing disabled field triggers which makes no sense. So
        //          we remove them during streaming in if pre-V5.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 5;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCDriverObjCfg
//  PREFIX: cqcdc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDriverObjCfg: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TCQCDriverObjCfg::eComp(const   TCQCDriverObjCfg&   cqcdc1
                        , const TCQCDriverObjCfg&   cqcdc2)
{
    return cqcdc1.m_strMoniker.eCompare(cqcdc2.m_strMoniker);
}

tCIDLib::ESortComps
TCQCDriverObjCfg::eIComp(const  TCQCDriverObjCfg&   cqcdc1
                        , const TCQCDriverObjCfg&   cqcdc2)
{
    return cqcdc1.m_strMoniker.eCompareI(cqcdc2.m_strMoniker);
}

const TString& TCQCDriverObjCfg::strKey(const TCQCDriverObjCfg& cqcdcSrc)
{
    return cqcdcSrc.strMoniker();
}


// ---------------------------------------------------------------------------
//  TCQCDriverObjCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDriverObjCfg::TCQCDriverObjCfg() :

    m_bPaused(kCIDLib::False)
    , m_pconncfgReal(new TCQCOtherConnCfg)
{
}

TCQCDriverObjCfg::TCQCDriverObjCfg(const TCQCDriverCfg& cqcdcSrc) :

    TCQCDriverCfg(cqcdcSrc)
    , m_bPaused(kCIDLib::False)
    , m_pconncfgReal(nullptr)
{
    // Duplicate the default conn config for now
    m_pconncfgReal = ::pDupObject<TCQCConnCfg>(&cqcdcSrc.conncfgDef());
}

TCQCDriverObjCfg::TCQCDriverObjCfg( const   TCQCDriverCfg&  cqcdcSrc
                                    , const TString&        strMoniker) :
    TCQCDriverCfg(cqcdcSrc)
    , m_bPaused(kCIDLib::False)
    , m_pconncfgReal(nullptr)
    , m_strMoniker(strMoniker)
{
    // Duplicate the default conn config for now
    m_pconncfgReal = ::pDupObject<TCQCConnCfg>(&cqcdcSrc.conncfgDef());
}

TCQCDriverObjCfg::TCQCDriverObjCfg(const TCQCDriverObjCfg& cqcdcSrc) :

    TCQCDriverCfg(cqcdcSrc)
    , m_bPaused(cqcdcSrc.m_bPaused)
    , m_colTriggers(cqcdcSrc.m_colTriggers)
    , m_nvmPromptVals(cqcdcSrc.m_nvmPromptVals)
    , m_pconncfgReal(nullptr)
    , m_strMoniker(cqcdcSrc.m_strMoniker)
{
    m_pconncfgReal = ::pDupObject<TCQCConnCfg>(cqcdcSrc.m_pconncfgReal);
}

TCQCDriverObjCfg::~TCQCDriverObjCfg()
{
    delete m_pconncfgReal;
}


// ---------------------------------------------------------------------------
//  TCQCDriverObjCfg: Public operators
// ---------------------------------------------------------------------------
TCQCDriverObjCfg&
TCQCDriverObjCfg::operator=(const TCQCDriverObjCfg& cqcdcSrc)
{
    if (this != &cqcdcSrc)
    {
        TParent::operator=(cqcdcSrc);

        // Polymorphically dup the connection config
        TCQCConnCfg* pconncfgTmp;
        pconncfgTmp = ::pDupObject<TCQCConnCfg>(cqcdcSrc.m_pconncfgReal);
        delete m_pconncfgReal;
        m_pconncfgReal = pconncfgTmp;

        // And copy over the other stuff
        m_bPaused       = cqcdcSrc.m_bPaused;
        m_colTriggers   = cqcdcSrc.m_colTriggers;
        m_nvmPromptVals = cqcdcSrc.m_nvmPromptVals;
        m_strMoniker    = cqcdcSrc.m_strMoniker;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCDriverObjCfg: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  The underlying config was updated, so we need to get our config set to
//  the default values.
//
tCIDLib::TVoid TCQCDriverObjCfg::NewConnCfg(TCQCConnCfg* const pconncfgNew)
{
    TCQCConnCfg* pconncfgTmp;
    pconncfgTmp = ::pDupObject<TCQCConnCfg>(pconncfgNew);
    delete m_pconncfgReal;
    m_pconncfgReal = pconncfgTmp;
}


// ---------------------------------------------------------------------------
//  TCQCDriverObjCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCDriverObjCfg::bFindPromptValue( const   TString&    strKey
                                    , const TString&    strSubKey
                                    ,       TString&    strValue) const
{
    FaultInPromptVals();
    return m_nvmPromptVals.bQueryValue(strKey, strSubKey, strValue);
}


// Get/set the paused flag
tCIDLib::TBoolean TCQCDriverObjCfg::bPaused() const
{
    return m_bPaused;
}

tCIDLib::TBoolean TCQCDriverObjCfg::bPaused(const tCIDLib::TBoolean bToSet)
{
    m_bPaused = bToSet;
    return m_bPaused;
}


tCIDLib::TBoolean
TCQCDriverObjCfg::bSameBase(const TCQCDriverCfg& cqcdcToCheck) const
{
    // Just use our parent's equality operator
    return TParent::operator==(cqcdcToCheck);
}


const TCQCDriverObjCfg::TTriggerList& TCQCDriverObjCfg::colTriggers() const
{
    return m_colTriggers;
}


const TCQCConnCfg& TCQCDriverObjCfg::conncfgReal() const
{
    return *m_pconncfgReal;
}

TCQCConnCfg& TCQCDriverObjCfg::conncfgReal()
{
    return *m_pconncfgReal;
}


TCQCDriverCfg::TPromptCursor TCQCDriverObjCfg::cursPromptVals() const
{
    FaultInPromptVals();
    return m_nvmPromptVals.cursItems();
}


TCQCDriverCfg::TValCursor
TCQCDriverObjCfg::cursPromptSubVals(const TString& strKey) const
{
    FaultInPromptVals();
    return m_nvmPromptVals.cursItemVals(strKey);
}


//
//  This method will look through the prompt values defined in the base
//  class (driven by the manifest), and add prompt value holders at our
//  level which will be filled in via the driver install wizard. This
//  makes sure that place holders are in place for any defined prompts,
//  and it also loads up default values into those place holders if they
//  are defined in the manifest.
//
//  So, after this, the driver wizard can be run and will be sure that
//  all values are present, even if empty, and any default values will
//  be there.
//
//  Normally, if any prompt values are present already, we do nothing since
//  they've already been set and we don't want to force any new ones that
//  might have been added since the driver was created, since it might not
//  understand them.
//
//  However, when the driver wizard is run in a 'reconfigure' mode, it will
//  update our base class with the latest info from the data server, so if
//  any new prompts have been added they will now be there in the base class
//  but we won't have any value storage for them yet. It will then call this
//  with the bForce flag set and we'll go through them again and for any
//  that don't already have values, we'll fault them in.
//
tCIDLib::TVoid
TCQCDriverObjCfg::FaultInPromptVals(const tCIDLib::TBoolean bForce) const
{
    //
    //  If the values have already been faulted in, then do nothing unless
    //  we are being forced to.
    //
    if (!m_nvmPromptVals.bIsEmpty() && !bForce)
        return;

    const tCIDLib::TCard4   c4Count = c4PromptCount();
    TString                 strVal;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCDCfgPrompt& cqcdcpCur = cqcdcpAt(c4Index);
        const TString& strName = cqcdcpCur.strName();
        const tCIDLib::TCard4 c4DefCount = cqcdcpCur.c4DefCount();

        //
        //  If we don't already have this key, then add an empty value
        //
        if (!m_nvmPromptVals.bKeyExists(strName))
            m_nvmPromptVals.AddItem(strName);

        //
        //  Loop through the values defined for this prompt. Any that
        //  don't exist, fault them in and given them their default value
        //  if they have one defined.
        //
        TStringTokenizer stokVals(&cqcdcpCur.strValNames(), L",");
        while (stokVals.bGetNextToken(strVal))
        {
            strVal.StripWhitespace();

            //
            //  If this one already exists, try the next one. Else we
            //  leave it with the current value.
            //
            if (m_nvmPromptVals.bSubKeyExists(strName, strVal))
                continue;

            // Create an empty value for this one
            m_nvmPromptVals.AddValue(strName, strVal, TString::strEmpty());

            // See if there's a default for this one
            for (tCIDLib::TCard4 c4DefInd = 0; c4DefInd < c4DefCount; c4DefInd++)
            {
                const TKeyValuePair& kvalCur = cqcdcpCur.kvalDefaultAt(c4DefInd);
                if (kvalCur.strKey() == strVal)
                {
                    m_nvmPromptVals.SetValue(strName, kvalCur.strKey(), kvalCur.strValue());
                    break;
                }
            }
        }
    }
}


const TCQCDriverCfg::TPromptVals& TCQCDriverObjCfg::nvmPromptVals() const
{
    FaultInPromptVals();
    return m_nvmPromptVals;
}


//
//  Sees if we have a trigger configured for the indicated field name. If
//  so returns a pointer, else null.
//
const TCQCFldEvTrigger*
TCQCDriverObjCfg::pfetFind(const TString& strFldName) const
{
    const tCIDLib::TCard4 c4Count = m_colTriggers.c4ElemCount();
    const TCQCFldEvTrigger* pfetRet = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldEvTrigger& fetCur = m_colTriggers[c4Index];
        if (fetCur.strFldName() == strFldName)
        {
            pfetRet = &fetCur;
            break;
        }
    }
    return pfetRet;
}


// Remove all defined triggers
tCIDLib::TVoid TCQCDriverObjCfg::RemoveAllTriggers()
{
    m_colTriggers.RemoveAll();
}


// Look for a trigger for the indicated field, and remove it if found
tCIDLib::TVoid TCQCDriverObjCfg::RemoveTrigger(const TString& strFldName)
{
    const tCIDLib::TCard4 c4Count = m_colTriggers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colTriggers[c4Index].strFldName() == strFldName)
        {
            m_colTriggers.RemoveAt(c4Index);
            break;
        }
    }
}


const TString& TCQCDriverObjCfg::strMoniker() const
{
    return m_strMoniker;
}

const TString& TCQCDriverObjCfg::strMoniker(const TString& strToSet)
{
    m_strMoniker = strToSet;
    return m_strMoniker;
}


const TString&
TCQCDriverObjCfg::strPromptValue(const  TString&    strKey
                                , const TString&    strSubKey) const
{
    FaultInPromptVals();
    return m_nvmPromptVals.objValue(strKey, strSubKey);
}

tCIDLib::TVoid
TCQCDriverObjCfg::SetPromptVal( const   TString&    strKey
                                , const TString&    strSubKey
                                , const TString&    strNewValue)
{
    FaultInPromptVals();
    m_nvmPromptVals.SetValue(strKey, strSubKey, strNewValue);
}


//
//  Add the indicated trigger. Keep in mind that it may be set to disabled, in
//  which case we are going to remove it.
//
tCIDLib::TVoid TCQCDriverObjCfg::SetTrigger(const TCQCFldEvTrigger& fetToAdd)
{
    const tCIDLib::TCard4 c4Count = m_colTriggers.c4ElemCount();
    TCQCFldEvTrigger* pfetTar = nullptr;
    const TString& strTar = fetToAdd.strFldName();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        TCQCFldEvTrigger& fetCur = m_colTriggers[c4Index];
        if (fetCur.strFldName() == strTar)
        {
            pfetTar = &fetCur;
            break;
        }
    }

    // If disabling, then remove this trigger. Else add or update
    if (fetToAdd.eType() == tCQCKit::EEvTrTypes::Unused)
    {
        if (pfetTar)
            m_colTriggers.RemoveAt(c4Index);
    }
     else
    {
        if (pfetTar)
            *pfetTar = fetToAdd;
        else
            m_colTriggers.objAdd(fetToAdd);
    }
}


// Set all triggers at once
tCIDLib::TVoid TCQCDriverObjCfg::SetTriggers(const TTriggerList& colToSet)
{
    m_colTriggers = colToSet;
}


tCIDLib::TVoid TCQCDriverObjCfg::UpdateBase(const TCQCDriverCfg& cqcdcToSet)
{
    // Call our parent's assignment operator to do this
    TParent::operator=(cqcdcToSet);
}



// ---------------------------------------------------------------------------
//  TCQCDriverObjCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDriverObjCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Call our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    //
    //  Check our version. Note that we don't support support 1 format any
    //  more.
    //
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if ((c2FmtVersion < 2) || (c2FmtVersion > CQCKit_DriverObjCfg::c2FmtVersion))
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

    // Read in the driver moniker and prompt values
    strmToReadFrom  >> m_strMoniker
                    >> m_nvmPromptVals;

    //
    //  Clean up any current connection config, and read in the new one,
    //  which must be done polymorphically.
    //
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    TCQCConnCfg* pconncfgTmp = nullptr;
    ::PolymorphicRead(pconncfgTmp, strmToReadFrom);
    delete m_pconncfgReal;
    m_pconncfgReal = pconncfgTmp;

    // If V3 or later, read in the event triggers, else just empty the list.
    if (c2FmtVersion >= 3)
        strmToReadFrom >> m_colTriggers;
    else
        m_colTriggers.RemoveAll();

    // If V4 or later, read in the paused flag, else default it
    if (c2FmtVersion >= 4)
        strmToReadFrom >> m_bPaused;
    else
        m_bPaused = kCIDLib::False;

    // If pre-V5, remove any disabled field triggers
    if (c2FmtVersion < 5)
    {
        tCIDLib::TCard4 c4Count = m_colTriggers.c4ElemCount();
        tCIDLib::TCard4 c4Index = 0;

        while (c4Index < c4Count)
        {
            if (m_colTriggers[c4Index].eType() == tCQCKit::EEvTrTypes::Unused)
            {
                m_colTriggers.RemoveAt(c4Index);
                c4Count--;
            }
             else
            {
                c4Index++;
            }
        }
    }

    // And make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCDriverObjCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Call our parent first
    TParent::StreamTo(strmToWriteTo);

    // And stream out our values, followed by an end object marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCKit_DriverObjCfg::c2FmtVersion
                    << m_strMoniker
                    << m_nvmPromptVals;

    // Stream the connection config polymorphically
    strmToWriteTo << tCIDLib::EStreamMarkers::Frame;
    ::PolymorphicWrite(m_pconncfgReal, strmToWriteTo);

    strmToWriteTo   // V3 stuff
                    << m_colTriggers

                    // V4 stuff
                    << m_bPaused;

    // And end it with an end object marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


