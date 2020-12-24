//
// FILE NAME: CQCTrayMon_ListChangeEvent.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/15/2004
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
//  This is the header for the CQCTrayMon_ListChangeEvent.cpp file, which
//  implements the TAppListChangeEv class. Objects of this class are put into
//  a queue managed by the main frame window, by the protocol server object,
//  to tell the window about changes that it should display.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TAppListChangeEv
//  PREFIX: ace
// ---------------------------------------------------------------------------
class TAppListChangeEv : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppListChangeEv();

        TAppListChangeEv
        (
            const   TString&                strMoniker
            , const TString&                strTitle
            , const tCQCTrayMon::EListEvs   eEvent
        );

        TAppListChangeEv
        (
            const   TString&                strMoniker
            , const tCQCTrayMon::EListEvs   eEvent
        );

        TAppListChangeEv
        (
            const   TString&                strStatusMsg
        );

        TAppListChangeEv
        (
            const   TAppListChangeEv&       aceToCopy
        );

        ~TAppListChangeEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppListChangeEv& operator=
        (
            const   TAppListChangeEv&       aceToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCTrayMon::EListEvs eEvent() const;

        const TString& strMoniker() const;

        const TString& strMsgText() const;

        const TString& strTitle() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eEvent
        //      The event type that this object represents.
        //
        //  m_strMoniker
        //      The moniker of the application for which this change even was
        //      posted. This lets the window know which item in it's list box
        //      to update.
        //
        //  m_strTitle
        //      The title text of the affected application. For status text
        //      events, this is used for the status text message.
        // -------------------------------------------------------------------
        tCQCTrayMon::EListEvs   m_eEvent;
        TString                 m_strMoniker;
        TString                 m_strTitle;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppListChangeEv,TObject)
};



