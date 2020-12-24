//
// FILE NAME: CQCDocComp.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/04/2015
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
//  This is the main header of this program. It brings in the underlying stuff we
//  need and our own headers for internal use.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

// ----------------------------------------------------------------------------
//  Includes.
// ----------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDEncode.hpp"
#include    "CQCKit.hpp"



// ----------------------------------------------------------------------------
//  Facility types
// ----------------------------------------------------------------------------
namespace tCQCDocComp
{
    // ------------------------------------------------------------------------
    //  The types of general markup nodes we support in those places where we
    //  allow open ended text content.
    // ------------------------------------------------------------------------
    enum ETypes
    {
        EType_None

        , EType_Bold
        , EType_Break
        , EType_Code
        , EType_DIV
        , EType_Image
        , EType_Indent
        , EType_InlineNote
        , EType_Link
        , EType_List
        , EType_ListItem
        , EType_Note
        , EType_Paragraph
        , EType_SecTitle
        , EType_SubSecTitle
        , EType_Superscript
        , EType_Table
        , EType_TableCol
        , EType_TableRow
        , EType_Text
    };


    // ------------------------------------------------------------------------
    //  Used in the device class to indicates sub-unit support
    // ------------------------------------------------------------------------
    enum ESubUnit
    {
        ESubUnit_Always
        , ESubUnit_Device
        , ESubUnit_Never
    };
}


// ----------------------------------------------------------------------------
//  Facility constants
// ----------------------------------------------------------------------------
namespace kCQCDocComp
{
    const TString       strSeeBelow(L"[See Below]");
}



// ---------------------------------------------------------------------------
//  Forward reference the topic class since it is part of a recursive structure.
// ---------------------------------------------------------------------------
class TTopic;


// ----------------------------------------------------------------------------
//  And our own headers
// ----------------------------------------------------------------------------
#include    "CQCDocComp_ErrorIds.hpp"
#include    "CQCDocComp_HelpNode.hpp"
#include    "CQCDocComp_FldDef.hpp"

namespace tCQCDocComp
{
    using TFldDefList = TVector<TDocFldDef>;
}

#include    "CQCDocComp_Pages.hpp"
#include    "CQCDocComp_ActionCmd.hpp"
#include    "CQCDocComp_CMLClass.hpp"
#include    "CQCDocComp_DevDriver.hpp"
#include    "CQCDocComp_EvFilter.hpp"
#include    "CQCDocComp_DevClass.hpp"
#include    "CQCDocComp_IntfWidget.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCDocComp
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCDocComp : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Public, static data members
        // -------------------------------------------------------------------
        static const TString    s_strExt_ActionTar;
        static const TString    s_strExt_CMLClass;
        static const TString    s_strExt_DevClass;
        static const TString    s_strExt_Driver;
        static const TString    s_strExt_EventFilter;
        static const TString    s_strExt_HelpPage;
        static const TString    s_strExt_IntfWidget;

        static const TString    s_strColSpan;
        static const TString    s_strExtra;
        static const TString    s_strExtra1;
        static const TString    s_strExtra2;
        static const TString    s_strFileExt;
        static const TString    s_strFileName;
        static const TString    s_strId;
        static const TString    s_strPageLink;
        static const TString    s_strPageMap;
        static const TString    s_strRef;
        static const TString    s_strRoot;
        static const TString    s_strSubDir;
        static const TString    s_strSubTopicMap;
        static const TString    s_strTitle;
        static const TString    s_strTopicPage;
        static const TString    s_strType;
        static const TString    s_strV2Compat;
        static const TString    s_strVirtual;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCDocComp();

        TFacCQCDocComp(const TFacCQCDocComp&) = delete;

        ~TFacCQCDocComp();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCDocComp& operator=(const TFacCQCDocComp&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        TTextOutStream& strmOut()
        {
            return *m_pstrmOut;
        }

        TTextOutStream& strmErr()
        {
            return m_strmErr;
        }

    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TPageList = TVector<TPageInfo>;
        using TPageRefList = TRefVector<TPageInfo>;
        using TTopicList = TVector<TSubTopicInfo>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGenerateDriverIndices();

        tCIDLib::TBoolean bGenerateIRModelIndex();

        tCIDLib::TBoolean bIterateTopics
        (
            const   TString&                strSrcPath
            , const TString&                strDirName
            , const TString&                strHelpPath
            ,       TTopic&                 topicToFill
        );

        tCIDLib::TBoolean bParsePageFile
        (
            const   TString&                strSrc
            , const TPageInfo&              pinfoSrc
            ,       TBasePage&              pgTar
            ,       TTopic&                 topicPar
            , const tCIDLib::TBoolean       bFileSrc
        );

        tCIDLib::TVoid CopyDir
        (
            const   TString&                strSrc
            , const TString&                strTar
        );

        tCIDLib::TVoid LoadDTD();

        tCIDLib::TVoid OutputStdHPHeader
        (
                    TTextOutStream&         strmTar
            , const TString&                strTitle
        );

        TTopic& topicFindByPath
        (
            const   TString&                strToFind
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bVerbose
        //      Defaults to false, can be set via the /Verbose command line option.
        //      That will cause it to dump out diagnostic stuff to help figure out
        //      issues in the help content.
        //
        //  m_colDriverPages
        //      We keep a list of all of the page info objects that refer to drivers. This
        //      is used later to generate some indices. This is a non-adopting by reference
        //      list, pointing at page info objects in m_colPages.
        //
        //  m_colPages
        //      We keep a list of all of the page info objects. The topics keep references
        //      to the entries here that were defined in their directories, and references
        //      are also in m_coldriverPages.
        //
        //  m_colTopics
        //      We keep a list of all of the topics, that we can use later to generate an
        //      overall topic index at the end.
        //
        //  m_pstrmOut
        //      If in verbose mode this is set to the standard output stream. Else it's
        //      s set to a string based text string so that we just eat the output
        //      (though still have it if an error occurs, so we can dump it out.)
        //
        //  m_strSrcPath
        //  m_strTarPath
        //      The source and target paths that we figure out when we first start up and
        //      put here for further reference.
        //
        //  m_strSrcSrcPath
        //      This the top level source path (m_strSrcPath) plus the Src path under
        //      it. I.e. the directory where the actual help content is.
        //
        //  m_strmErr
        //      The output stream for errors, which will show up even if we are not
        //      in verbose mode (where m_pstrmOut is nullptr.)
        //
        //  m_topicRoot
        //      The top-most topic object, which isn't a real one, it just provides the
        //      root for the hierarchy to kick start it.
        //
        //  m_xtprsToUse
        //      Our XML parser that we set up and reuse for all of the files.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bVerbose;
        TPageRefList        m_colDriverPages;
        TPageList           m_colPages;
        TTopicList          m_colTopics;
        TTextOutStream*     m_pstrmOut;
        TString             m_strSrcPath;
        TString             m_strSrcSrcPath;
        TString             m_strTarPath;
        TTextFileOutStream  m_strmErr;
        TTopic              m_topicRoot;
        TXMLTreeParser      m_xtprsToUse;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCDocComp,TFacility)
};


// ---------------------------------------------------------------------------
//  Export the facility object to any other modules. It's declared in the main
//  cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCDocComp facCQCDocComp;

