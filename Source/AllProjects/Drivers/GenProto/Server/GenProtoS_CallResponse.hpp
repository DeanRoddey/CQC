//
// FILE NAME: GenProtoS_CallResponse.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the header file for the GenProtoS_CallResponse.cpp file, which
//  defines a simple class used to store the information that represents an
//  exchange with the device. It consists of a 'call', i.e. us sending the
//  device a query message, and an optional response, i.e. it sending us back
//  a message with the requested data. It also includes a 'wait for' value,
//  which indicates how long to wait for the reply before we just assume its
//  not coming.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCallRep
//  PREFIX: clrp
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoCallRep : public TObject, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoCallRep();

        TGenProtoCallRep
        (
                    TGenProtoQuery* const   pqryToSend
            ,       TGenProtoReply* const   prepToExpect
            , const tCIDLib::TCard4         c4MillisToWait
        );

        TGenProtoCallRep
        (
                    TGenProtoQuery* const   pqryToSend
            ,       TGenProtoReply* const   prepToExpect
            , const tCIDLib::TCard4         c4MillisToWait
            , const tCIDLib::TCard4         c4MillisPeriod
        );

        TGenProtoCallRep
        (
            const   TGenProtoCallRep&       clrpToCopy
        );

        ~TGenProtoCallRep();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoCallRep& operator=
        (
            const   TGenProtoCallRep&       clrpToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bExpectsReply() const;

        tCIDLib::TCard4 c4MillisPeriod() const;

        tCIDLib::TCard4 c4MillisToWait() const;

        tCIDLib::TCard4 c4NextPollTime() const;

        const TGenProtoQuery& qryToSend() const;

        TGenProtoQuery& qryToSend();

        const TGenProtoReply& repToExpect() const;

        TGenProtoReply& repToExpect();

        tCIDLib::TVoid ResetPollTime();

        tCIDLib::TVoid Set
        (
                    TGenProtoQuery* const   pqryToSend
            ,       TGenProtoReply* const   prepToExpect
            , const tCIDLib::TCard4         c4MillisToWait
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4MillisPeriod
        //      This is the period of time we wait between invoking this
        //      particular call/response. This is only used for those C/R
        //      objects defined within the polling block. Others don't need
        //      this and it will be zero.
        //
        //  m_c4MillisToWait
        //      The period of time to wait for the response. If there is no
        //      response defined, which is legal, then it will be zero.
        //
        //  m_c4NextPoll
        //      When used in a polling exchange, we have to keep up with how
        //      often each poll c/r in the polling list should be done. The
        //      ResetPollTime() method will take the current time and add our
        //      period time to it and store that here.
        //
        //  m_pqryToSend
        //      The query to send to the device. This one is required. If it
        //      is set, then the m_c4MillisToWait field must be non-zero.
        //
        //  m_prepToExpect
        //      The optional reply to expect. Sometimes they won't want to wait
        //      for a reply, since it might take a while, and just want the
        //      reply that comes eventually to be handled by the poll thread.
        //      If this is set, then the m_c4MillisToWait field indicate how
        //      long to wait before assuming the device isn't responding.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4MillisPeriod;
        tCIDLib::TCard4 m_c4MillisToWait;
        tCIDLib::TCard4 m_c4NextPoll;
        TGenProtoQuery* m_pqryToSend;
        TGenProtoReply* m_prepToExpect;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoCallRep,TObject)
        DefPolyDup(TGenProtoCallRep)
};

#pragma CIDLIB_POPPACK


