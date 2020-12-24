//
// FILE NAME: CQCDocComp_FldDef.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/25/2016
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
//  The implementation of the simple field definition class used by drivers and device
//  class pages.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCDocComp.hpp"


// ---------------------------------------------------------------------------
//   CLASS: TDocFldDef
//  PREFIX: dfd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDocFldDef: Constructors and Destructor
// ---------------------------------------------------------------------------
TDocFldDef::TDocFldDef() :

    m_eSemType(tCQCKit::EFldSTypes::Count)
{
}

TDocFldDef::~TDocFldDef()
{
}


// ---------------------------------------------------------------------------
//  TDocFldDef: Public, non-virtual methods
// ---------------------------------------------------------------------------

// We return true if
tCIDLib::TBoolean TDocFldDef::bHasSemType() const
{
    // If it's generic or count value, we don't have one
    return
    (
        (m_eSemType != tCQCKit::EFldSTypes::Generic)
        && (m_eSemType != tCQCKit::EFldSTypes::Count)
    );
}


tCIDLib::TBoolean TDocFldDef::bParseXML(const TXMLTreeElement& xtnodeFld)
{
    // Get the stuff that is available as attributes on the fld node
    m_strName = xtnodeFld.xtattrNamed(L"Name").strValue();
    m_strType = xtnodeFld.xtattrNamed(L"Type").strValue();
    m_strAccess = xtnodeFld.xtattrNamed(L"Access").strValue();

    // The semantic type is optional
    TString strSemType;
    if (xtnodeFld.bAttrExists(L"SemType", strSemType))
    {
        // Make sure it's a valid semantic type
        m_eSemType = tCQCKit::eAltXlatEFldSTypes(strSemType);
        if (m_eSemType == tCQCKit::EFldSTypes::Count)
        {
            // It can be "[See Below]"
            if (strSemType != kCQCDocComp::strSeeBelow)
            {
                facCQCDocComp.strmErr()
                    << L"'" << strSemType << L"' is not a valid semantic field type"
                    << kCIDLib::NewEndLn;
                return kCIDLib::False;
            }
        }
    }

    //
    //  And process the contained description text. We can pass ourself since our content
    //  should meet the requirements of the HelpText content model. This means we don't
    //  have to make them put it in another nested HelpText element.
    //
    return TBasePage::bProcessHelpText(xtnodeFld, m_hnDescr);
}
