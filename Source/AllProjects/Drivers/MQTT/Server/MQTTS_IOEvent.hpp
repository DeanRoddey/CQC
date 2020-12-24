//
// FILE NAME: MQTTS_IOEvent.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/04/2019
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
//  Defines the I/O event class that the I/O thread uses to post status info to
//  the main thread.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//   CLASS: TMQTTIOEvent
//  PREFIX: ioev
// ---------------------------------------------------------------------------
class TMQTTIOEvent : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TMQTTIOEvent();

        TMQTTIOEvent(const TMQTTIOEvent&) = delete;

        ~TMQTTIOEvent();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTIOEvent& operator=(const TMQTTIOEvent&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SetBadValEvent
        (
            const   TMQTTFldCfg&            mqfldcSrc
        );

        tCIDLib::TVoid SetFldWriteRes
        (
            const   TMQTTFldCfg&            mqfldcSrc
            , const tCQCKit::ECommResults   eResult
        );

        tCIDLib::TVoid SetNewValEvent
        (
            const   TMQTTFldCfg&            mqfldcSrc
            , const TString&                strNewFldVal
        );


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bV2Compat
        //      This is passed along for field writes since the driver needs to send
        //      out standard event triggers for V2 fields of appropriate types.
        //
        //  m_eCommRes
        //      For field writes we return a comm result value that the driver can just
        //      return, so he doesn't have to try to figure out what to return.
        //
        //  m_eEvent
        //      The type of event being reported.
        //
        //  m_eFldType
        //      Just a sanity check, to let the main thread confirm that the I/O
        //      thread is giving us data in the form we expect for the field.
        //
        //  m_eIOState
        //      If this is a state change event, this is the new state of the I/O
        //      thread.
        //
        //  m_eSemType
        //      If a publish event, we pass along the semantic type, which may be
        //      needed for event trigger sending purposes.
        //
        //  m_strBaseName
        //      If a field store event, we pass along the base name so that it can
        //      be put into any outgoing event trigger if one is sent.
        //
        //  m_strFldName
        //      So the driver can find the field when this is a field oriented
        //      event.
        //
        //  m_strValue
        //      We store all the values as text. The driver base class already
        //      has all of the code to handle translating to the target format
        //      and validating the value and all that. So we just save ourselves
        //      a lot of redundancy.
        //
        //      For field write failure events, we put an error msg in here.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bV2Compat;
        tCQCKit::ECommResults   m_eCommRes;
        tMQTTS::EIOEvents       m_eEvent;
        tCQCKit::EFldTypes      m_eFldType;
        tMQTTSh::EClStates      m_eIOState;
        tCQCKit::EFldSTypes     m_eSemType;
        TString                 m_strBaseName;
        TString                 m_strFldName;
        TString                 m_strValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTIOEvent, TObject)
};


