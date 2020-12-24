//
// FILE NAME: CQCIntfEng_ValErrInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/21/2008
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
//  This file implements a class represents a validation error, a list of
//  which can be collected during validation of a template and presented
//  to the user.
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
#include    "CQCIntfEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfValErrInfo,TCQCObject)



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfValErrInfo
//  PREFIX: vei
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfValErrInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfValErrInfo::TCQCIntfValErrInfo() :

    m_bWarning(kCIDLib::True)
    , m_c4ErrId(kCIDLib::c4MaxCard)
    , m_c4WidgetId(0)
    , m_eType(tCQCIntfEng::EValErrTypes::Count)
{
}

TCQCIntfValErrInfo::
TCQCIntfValErrInfo( const   tCIDLib::TCard4     c4WidgetId
                    , const TString&            strErrText
                    , const tCIDLib::TBoolean   bWarning
                    , const TString&            strWidgetId) :

    m_bWarning(bWarning)
    , m_c4ErrId(facCIDLib().c4NextId())
    , m_c4WidgetId(c4WidgetId)
    , m_eType(tCQCIntfEng::EValErrTypes::Widget)
    , m_strErrText(strErrText)
    , m_strWidgetId(strWidgetId)
{
}

TCQCIntfValErrInfo::
TCQCIntfValErrInfo( const   tCQCIntfEng::EValErrTypes   eType
                    , const TString&                    strErrText
                    , const tCIDLib::TBoolean           bWarning
                    , const TString&                    strWidgetId) :

    m_bWarning(bWarning)
    , m_c4ErrId(facCIDLib().c4NextId())
    , m_c4WidgetId(kCIDLib::c4MaxCard)
    , m_eType(eType)
    , m_strErrText(strErrText)
    , m_strWidgetId(strWidgetId)
{
}

TCQCIntfValErrInfo::TCQCIntfValErrInfo(const TCQCIntfValErrInfo& veiSrc) :

    m_bWarning(veiSrc.m_bWarning)
    , m_c4ErrId(veiSrc.m_c4ErrId)
    , m_c4WidgetId(veiSrc.m_c4WidgetId)
    , m_eType(veiSrc.m_eType)
    , m_strErrText(veiSrc.m_strErrText)
    , m_strWidgetId(veiSrc.m_strWidgetId)
{
}

TCQCIntfValErrInfo::~TCQCIntfValErrInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfValErrInfo: Public operators
// ---------------------------------------------------------------------------
TCQCIntfValErrInfo&
TCQCIntfValErrInfo::operator=(const TCQCIntfValErrInfo& veiSrc)
{
    if (this != &veiSrc)
    {
        m_bWarning      = veiSrc.m_bWarning;
        m_c4ErrId       = veiSrc.m_c4ErrId;
        m_c4WidgetId    = veiSrc.m_c4WidgetId;
        m_eType         = veiSrc.m_eType;
        m_strErrText    = veiSrc.m_strErrText;
        m_strWidgetId   = veiSrc.m_strWidgetId;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfValErrInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCIntfValErrInfo::bWarning() const
{
    return m_bWarning;
}


tCIDLib::TCard4 TCQCIntfValErrInfo::c4ErrId() const
{
    return m_c4ErrId;
}


tCIDLib::TCard4 TCQCIntfValErrInfo::c4WidgetId() const
{
    return m_c4WidgetId;
}


tCQCIntfEng::EValErrTypes TCQCIntfValErrInfo::eType() const
{
    return m_eType;
}


const TString& TCQCIntfValErrInfo::strErrText() const
{
    return m_strErrText;
}


const TString& TCQCIntfValErrInfo::strWidgetId() const
{
    return m_strWidgetId;
}
