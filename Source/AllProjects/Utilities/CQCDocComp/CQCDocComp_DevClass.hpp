//
// FILE NAME: CQCDocComp_DevClass.hpp
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
//  This file defines a class that holds the info parsed out of a single device class
//  file. It derives from the base page class and can be treated as one, but is more
//  specialized than just a regular help text type page. Most of the info is just data
//  which will be formatted out based on its known contents.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TDevClassPage : public TBasePage
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDevClassPage();

        TDevClassPage(const TDevClassPage&) = delete;

        ~TDevClassPage();

        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDevClassPage& operator=(const TDevClassPage&) = delete;


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
        //  Public data members
        //
        //  m_bHaveSemType
        //      During parsing, we remember if any fields have a semantic type. If not,
        //      we'll later leave that column out of the fields table.
        //
        //  m_bPowerReq
        //      Indicates if power management is required for this class or not.
        //
        //  m_colFldList
        //      The fields defined by this device class.
        //
        //  m_eSubUnit
        //      Indicates whether/how this device class deals with sub-units.
        //
        //  m_hnBDCmds
        //      The markup text for the backdoor commands section of the file, if present.
        //
        //  m_hnComments
        //      The markup text for the comments section of the file.
        //
        //  m_hnFldComments
        //      THe markup text for the field commands section of the file, if present.
        //
        //  m_hnPowerMng
        //      The markup text for the power management section of the file, if present.
        //
        //  m_hnTriggers
        //      The markup text for the triggers section of the file, if present.
        //
        //  m_strAsOf
        //      This is optional and indicates the CQC version at which this device class
        //      was defined.
        //
        //  m_strPrefix
        //      The device class prefix. The page title is the device class human
        //      readable name.
        //
        //  m_strSemType
        //      If the field must meet the requirements of a semantic field type, this
        //      indicates the type name.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bHaveSemType;
        tCIDLib::TBoolean           m_bPowerReq;
        tCQCDocComp::TFldDefList    m_colFldList;
        tCQCDocComp::ESubUnit       m_eSubUnit;
        THelpNode                   m_hnBDCmds;
        THelpNode                   m_hnComments;
        THelpNode                   m_hnFldComments;
        THelpNode                   m_hnPowerMng;
        THelpNode                   m_hnTriggers;
        TString                     m_strAsOf;
        TString                     m_strPrefix;
        TString                     m_strSemType;
};

