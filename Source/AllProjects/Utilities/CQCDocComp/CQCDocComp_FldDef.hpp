//
// FILE NAME: CQCDocComp_FldDef.hpp
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
//  This file defines a simple class that is used to hold information about a field. Both
//  drivers and device classes define fields the same way so they both use this class. It's
//  a name, a field type, an access type, and a description. The latter is our standard markup
//  text format, though often it's just text.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TDocFldDef
//  PREFIX: dfd
// ---------------------------------------------------------------------------
class TDocFldDef
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDocFldDef();

        TDocFldDef(const TDocFldDef&) = default;

        ~TDocFldDef();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDocFldDef& operator=(const TDocFldDef&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHasSemType() const;

        tCIDLib::TBoolean bParseXML
        (
            const   TXMLTreeElement&        xtnodeRoot
        );


        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_eSemType
        //      The semantic type of the field, if any. If the text is [See below] we
        //      will set it to the _Count value.
        //
        //  m_hnDescr
        //      This is the field description text.
        //
        //  m_strAccess
        //      The field access type
        //
        //  m_strName
        //      The name of the field, no V2 prefix, just the name. If it's in a device
        //      class the prefix is obvious. If it's in a driver, then it can't be a V2
        //      field.
        //
        //  m_strType
        //      The field data type.
        // -------------------------------------------------------------------
        tCQCKit::EFldSTypes m_eSemType;
        THelpNode           m_hnDescr;
        TString             m_strAccess;
        TString             m_strName;
        TString             m_strType;
};
