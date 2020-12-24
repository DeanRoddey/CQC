//
// FILE NAME: CQCDocComp_EvFilter.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/09/2016
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
//  This file defines a simple class that is used to hold information used for generating
//  triggered event filter reference documentation.
//
//  These are defined by .cqcevf files. For now there is only a single one, since it's
/// reasonable to have them all on the same page.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TDocEvFilter
//  PREFIX: evf
// ---------------------------------------------------------------------------
class TDocEvFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDocEvFilter();

        TDocEvFilter(const TDocEvFilter&) = default;

        ~TDocEvFilter();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDocEvFilter& operator=(const TDocEvFilter&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseXML
        (
            const   TXMLTreeElement&        xtnodeRoot
        );

        tCIDLib::TVoid GenerateOutput
        (
                    TTextOutStream&         strmTar
            , const TString&                strKey
        )   const;

        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_colParms
        //      A list of TKeyValuePair objects that represent the parameters. The key is the
        //      name and the value is a short description.
        //
        //  m_hnDescr
        //      This is the description text for the action.
        //
        //  m_strName
        //      The name of the action command.
        // -------------------------------------------------------------------
        tCIDLib::TKVPList   m_colParms;
        THelpNode           m_hnDescr;
        TString             m_strName;
};


// ---------------------------------------------------------------------------
//   CLASS: TDocEvFilterRef
//  PREFIX: evfr
// ---------------------------------------------------------------------------
class TDocEvFilterRef : public TBasePage
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDocEvFilterRef();

        TDocEvFilterRef(const TDocEvFilterRef&) = delete;

        ~TDocEvFilterRef();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDocEvFilterRef& operator=(const TDocEvFilterRef&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseXML
        (
            const   TXMLTreeElement&        xtnodeRoot
            , const TString&                strName
            , const TTopic&                 topicPar
        )   override;

        tCIDLib::TVoid GenerateOutput
        (
                    TTextOutStream&         strmTar
        )   const override;


        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_colFilters
        //      A list of TDocEvFilter objects that describe the filters available.
        //
        //  m_hnDescr
        //      This is the description text for the target.
        // -------------------------------------------------------------------
        TVector<TDocEvFilter>   m_colFilters;
        THelpNode               m_hnDescr;
};
