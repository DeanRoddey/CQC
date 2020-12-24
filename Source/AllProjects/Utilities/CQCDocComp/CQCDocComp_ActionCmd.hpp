//
// FILE NAME: CQCDocComp_ActionCmd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2016
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
//  This file defines a simple class that is used to hold information about an action command.
//  These are fairly simple but we still want to track them in a fully semantic way so that we
//  can control how they are displayed. So they have their own semantic markup and page
//  extension so that we can know which files hold them.
//
//  We have one class that contains the overall info on the action target defined by that file.
//  And it contains a list of action commands.
//
//  These are defined by .cqcact files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TDocActionCmd
//  PREFIX: actc
// ---------------------------------------------------------------------------
class TDocActionCmd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDocActionCmd();

        TDocActionCmd(const TDocActionCmd&) = default;

        ~TDocActionCmd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDocActionCmd& operator=(const TDocActionCmd&) = delete;


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
        //  m_bRetStatus
        //      This action command returns a true/false result, so it can be used in If/Else
        //      type command statements.
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
        tCIDLib::TBoolean   m_bRetStatus;
        tCIDLib::TKVPList   m_colParms;
        THelpNode           m_hnDescr;
        TString             m_strName;
};


// ---------------------------------------------------------------------------
//   CLASS: TDocActionTar
//  PREFIX: actt
// ---------------------------------------------------------------------------
class TDocActionTar : public TBasePage
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDocActionTar();

        TDocActionTar(const TDocActionTar&) = delete;

        ~TDocActionTar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDocActionTar& operator=(const TDocActionTar&) = delete;


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
        //  m_colCmds
        //      A list of TDocActionCmd objects that describe the commands for this action
        //      target.
        //
        //  m_hnDescr
        //      This is the description text for the target.
        // -------------------------------------------------------------------
        TVector<TDocActionCmd>  m_colCmds;
        THelpNode               m_hnDescr;
};
