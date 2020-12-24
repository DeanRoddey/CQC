//
// FILE NAME: CQCDocComp_CMLClass.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/24/2016
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
//  A special derivative of the base page for CML classes. This holds all of the
//  info related to a CML class, until we can spit it all out. It actually
//  generates multiple pages, one for the class and one for each method.
//
//  We also have a small sub-type class for holding related to nested class like
//  enums, and nested literals. And one for methods, and groups of related methods that
//  share a common description.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TCMLMethodVar
{
    public :
        TCMLMethodVar();
        ~TCMLMethodVar();

        tCIDLib::TKValsList m_colParms;
        TString             m_strName;
        TString             m_strRetType;
};

class TCMLMethod
{
    public :
        TCMLMethod();
        ~TCMLMethod();

        TVector<TCMLMethodVar>  m_colVariants;
        THelpNode               m_hnDescr;
};

//
//  Methods are grouped according to their final, const, required, and so forth flags. So
//  we have a list of all methods of the class that have a given set of flags.
//
class TCMLMethodGrp
{
    public :
        TCMLMethodGrp();
        ~TCMLMethodGrp();

        // A set of flags for const, final, etc... Defined in the cpp file
        tCIDLib::TCard4     m_c4Flags;

        TVector<TCMLMethod> m_colMethods;
        THelpNode           m_hnDescr;
};


class TCMLSubType
{
    public :
        TCMLSubType();
        ~TCMLSubType();

        tCIDLib::TKValsList m_colValues;
        THelpNode           m_hnDescr;

        // Only used for enums
        TString             m_strName;

        // Only used for nested types
        TString             m_strType;
};


// ---------------------------------------------------------------------------
//   CLASS: TCMLClassPage
//  PREFIX: pg
// ---------------------------------------------------------------------------
class TCMLClassPage : public TBasePage
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCMLClassPage();

        ~TCMLClassPage();


        // -------------------------------------------------------------------
        //  Public, inherited classes
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
        //  Public class types
        // -------------------------------------------------------------------
        typedef TVector<TCMLMethodGrp>      TMethodList;
        typedef TVector<TCMLSubType>        TSubTypeList;


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bCopyable
        //  m_bFinal
        //      Boolean flags we get from the XML info.
        //
        //  m_colLiterals
        //  m_colTypes
        //      Sub-type lists for any nested literals and types.
        //
        //  m_colMethodGrps
        //      A list of method groups, each with a specific set of const, final, required,
        //      etc... attributes.
        //
        //  m_hnDesc
        //      The overall class description text.
        //
        //  m_strDottedPath
        //      The dotted format of the class' class path.
        //
        //  m_strParentPath
        //      The parent class path, in dotted notation
        //
        //  m_strSlashedPath
        //      The slashed path format of our class path, created from the
        //      dotted version.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bCopyable;
        tCIDLib::TBoolean   m_bFinal;
        TSubTypeList        m_colEnums;
        TSubTypeList        m_colLiterals;
        TMethodList         m_colMethodGrps;
        TSubTypeList        m_colTypes;
        THelpNode           m_hnDesc;
        TString             m_strDottedPath;
        TString             m_strSlashedPath;
        TString             m_strParentPath;
        mutable TString     m_strTmpFmt;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseEnums
        (
            const   TXMLTreeElement&        xtnodeEnums
        );

        tCIDLib::TBoolean bParseNestedTypes
        (
            const   TXMLTreeElement&        xtnodeTypes
        );

        tCIDLib::TBoolean bParseLiterals
        (
            const   TXMLTreeElement&        xtnodeLits
        );

        tCIDLib::TBoolean bParseMethodGrp
        (
            const   TXMLTreeElement&        xtnodeMGrp
            ,       TCMLMethodGrp&          methgToFill
        );

        tCIDLib::TVoid OutputMethodGrp
        (
                    TTextOutStream&         strmTar
            , const TString&                strSecTitle
            , const TCMLMethodGrp&          methgSrc
        )   const;
};

