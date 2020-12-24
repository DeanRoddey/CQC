//
// FILE NAME: GenProtoS_MsgMatch.hpp
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
//  This is the header for the GenProtoS_MsgMatch.cpp file, which
//  defines the the TGenProtoMsgMatch class. This class represents one of the
//  blocks in the MsgMatches= block. Each one checks a message and sees if
//  it should be fielded by the reply associated with that match block.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMsgMatch
//  PREFIX: msgm
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoMsgMatch : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TGenProtoMsgMatch&      msgmSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoMsgMatch
        (
                    TGenProtoReply* const   prepToUse
            ,       TGenProtoExprNode* const pnodeToAdopt
        );

        ~TGenProtoMsgMatch();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMatches
        (
                    TGenProtoCtx&           ctxToUse
        );

        TGenProtoReply* prepMatch();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors
        // -------------------------------------------------------------------
        TGenProtoMsgMatch();
        TGenProtoMsgMatch(const TGenProtoMsgMatch&);
        tCIDLib::TVoid operator=(const TGenProtoMsgMatch&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pnodeMatch
        //      A pointer to the head node of the expression that we run to
        //      see if we match the message. We own it and clean it up when
        //      we are cleaned up.
        //
        //  m_prepToUse
        //      A pointer to the reply that we map to if successful. We don't
        //      own it, we just reference it.
        // -------------------------------------------------------------------
        TGenProtoExprNode*    m_pnodeMatch;
        TGenProtoReply*       m_prepToUse;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoMsgMatch,TObject)
};

#pragma CIDLIB_POPPACK


