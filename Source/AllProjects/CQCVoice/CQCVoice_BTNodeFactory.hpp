//
// FILE NAME: CQCVoice_BTNodeFactory_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/07/2017
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
//  This is the header for our behavior tree node factory. We register this with the
//  CIDAI facility so that it can gen up our nodes when it parses our behavior tree
//  XML.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCVoiceBTNodeFact
//  PREFIX: nfact
// ---------------------------------------------------------------------------
class TCQCVoiceBTNodeFact : public TAIBTNodeFact
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCVoiceBTNodeFact();

        TCQCVoiceBTNodeFact(const TCQCVoiceBTNodeFact& ctxvSrc) = delete;

        ~TCQCVoiceBTNodeFact();


        // -------------------------------------------------------------------
        //  Public constructors
        // -------------------------------------------------------------------
        TCQCVoiceBTNodeFact& operator=(const TCQCVoiceBTNodeFact&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        TAIBTNode* pbtnodeNew
        (
            const   TString&                strPath
            , const TString&                strName
            , const TString&                strType
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCVoiceBTNodeFact, TAIBTNodeFact)
};

#pragma CIDLIB_POPPACK

