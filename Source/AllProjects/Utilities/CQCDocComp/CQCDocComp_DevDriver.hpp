//
// FILE NAME: CQCDocComp_DevDriver.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/05/2017
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
//  A special derivative of the base page for device drivers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TDevDriverPage
//  PREFIX: pg
// ---------------------------------------------------------------------------
class TDevDriverPage : public TBasePage
{
    public :
        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eSortByMake
        (
            const   TDevDriverPage&         pg1
            , const TDevDriverPage&         pg2
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDevDriverPage();

        TDevDriverPage(const TDevDriverPage&) = delete;

        ~TDevDriverPage();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDevDriverPage& operator=(const TDevDriverPage&) = delete;


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
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetLinkInfo
        (
            const   TString&                strDir
            , const TString&                strFile
        );


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bHaveSemType
        //      If this guy has any non-V2 fields, and any of them define a semantic
        //      type, we'll show that column. Else we'll just leave it out.
        //
        //  m_bV2Compat
        //      Indicates if this driver is V2 compatible or not.
        //
        //  m_fcolDevClasses
        //      The the device classes this driver implements, if any.
        //
        //  m_hnXXX
        //      The markup text of the various sections of the sections that are just
        //      open ended text. DCComments are optional comments that will be added to
        //      the otherwise automatically generated device classes section.
        //
        //  m_strConnType
        //      The connection type for this driver.
        //
        //  m_strDir
        //  m_strFile
        //      During parsing these are stored in order to allow to more easily later
        //      generate various indices that link to the driver files. We need these to
        //      generate the links. So the are the stuff we'd use in a link, .e.g
        //      Reference/Drivers/MediaRepos and iTunesTM, the Ref= and Id= stuff.
        //
        //  m_strCQCVersion
        //      The CQC version in which this driver became available, if available. If
        //      not it defaults to empty value.
        //
        //  m_strMake
        //  m_strModel
        //      The make/model of the driver. This is the unique key.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bHaveSemType;
        tCIDLib::TBoolean           m_bV2Compat;
        tCIDLib::TStrList           m_colDevClasses;
        tCQCDocComp::TFldDefList    m_colFldList;
        tCQCKit::TDevClassList      m_fcolDevClasses;
        THelpNode                   m_hnConnDets;
        THelpNode                   m_hnDesc;
        THelpNode                   m_hnDCComments;
        THelpNode                   m_hnEvTriggers;
        THelpNode                   m_hnFldComments;
        THelpNode                   m_hnQuirks;
        TString                     m_strConnType;
        TString                     m_strCQCVersion;
        TString                     m_strDir;
        TString                     m_strFile;
        TString                     m_strMake;
        TString                     m_strModel;
        TString                     m_strVersion;


        // -------------------------------------------------------------------
        //  Private, static data members
        //
        //  m_strXXX
        //      Some strings we look for a lot, so better to optimize by having
        //      them as objects.
        // -------------------------------------------------------------------
        static const TString    s_strAccess_CanReadMustWrite;
        static const TString    s_strAccess_CanWriteMustRead;
        static const TString    s_strAccess_Read;
        static const TString    s_strAccess_ReadWrite;
        static const TString    s_strAccess_Write;
        static const TString    s_strAttr_ConnType;
        static const TString    s_strAttr_CQCVersion;
        static const TString    s_strAttr_Make;
        static const TString    s_strAttr_Model;
        static const TString    s_strAttr_V2Compat;
        static const TString    s_strAttr_Version;
        static const TString    s_strConn_SerialOrIP;
        static const TString    s_strElem_DrvConnDets;
        static const TString    s_strElem_DrvDCComments;
        static const TString    s_strElem_DrvQuirks;
        static const TString    s_strElem_DrvEvTriggers;
        static const TString    s_strType_Other;
};

