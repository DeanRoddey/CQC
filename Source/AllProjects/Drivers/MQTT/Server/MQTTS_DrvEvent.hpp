//
// FILE NAME: MQTTS_DrvEvent.hpp
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
//  Defines the driver event class that the main thread uses to post events to
//  the I/O thread.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//   CLASS: TMQTTDrvEvent
//  PREFIX: drvev
// ---------------------------------------------------------------------------
class TMQTTDrvEvent : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TMQTTDrvEvent();

        TMQTTDrvEvent(const TMQTTDrvEvent&) = delete;

        ~TMQTTDrvEvent();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTDrvEvent& operator=(const TMQTTDrvEvent&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset();


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_eEvent
        //      The type of event being reported.
        //
        //  m_strFldName
        //      If a field write, this is the name of the field.
        //
        //  m_strValue
        //      The formatted value that was writte.
        // -------------------------------------------------------------------
        tMQTTS::EDrvEvents  m_eEvent;
        TString             m_strFldName;
        TString             m_strValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTDrvEvent, TObject)
};


