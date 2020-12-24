//
// FILE NAME: CQCIR_ReceiverCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2003
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
//  This is the header file for the CQCIR_ReceiverCfg.cpp file, which
//  implements the TIRReceiverCfg class. This class represents the data that
//  an IR receiver server side driver persistes to disk and reloads on
//  startup.
//
//  Mostly its just storing the mapped events. Each event has a key which is
//  a stringified version of the value sent to us by the IR reciever to
//  represent the IR event, and the other info is the standard CQC event
//  stuff which tells the driver what macro or field write action to invoke
//  when we see the event string.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TIRReceiverCfg
//  PREFIX: irrc
// ---------------------------------------------------------------------------
class CQCIREXPORT TIRReceiverCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TEventList = TKeyedHashSet<TCQCStdKeyedCmdSrc,TString,TStringKeyOps>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIRReceiverCfg();

        TIRReceiverCfg(const TIRReceiverCfg&) = default;

        ~TIRReceiverCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIRReceiverCfg& operator=(const TIRReceiverCfg&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Add
        (
            const   TCQCStdKeyedCmdSrc&     csrcToAdd
        );

        tCIDLib::TBoolean bAddOrUpdate
        (
            const   TCQCStdKeyedCmdSrc&     csrcToAdd
        );

        tCIDLib::TBoolean bIsEmpty() const;

        tCIDLib::TCard4 c4ElemCount() const;

        TEventList::TCursor cursEvents() const;

        const TCQCStdKeyedCmdSrc* pcsrcFindByKey
        (
            const   TString&                strKeyToCheck
        );

        tCIDLib::TVoid RemoveKey
        (
            const   TString&                strKeyToRemove
        );

        tCIDLib::TVoid Reset();



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colEvents
        //      This is a collection of mapped events.
        // -------------------------------------------------------------------
        TEventList  m_colEvents;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRReceiverCfg,TObject)
};

#pragma CIDLIB_POPPACK


