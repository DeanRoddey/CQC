//
// FILE NAME: GenProtoS_FldInfo.hpp
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
//  This is the header file for the GenProtoS_FldInfo.cpp file, which
//  defines the TGenProtoFldInfo class. This class includes a TCQCFldDef
//  member, which holds the basic information about a driver field, and adds
//  to that some extra info that we need to track for each field.
//
//  For instance, write commands are mapped to fields, so each field's entry
//  holds the write command that builds the message to send when that field
//  is written to.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


class TGenProtoReply;

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoFldInfo
//  PREFIX: fldi
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoFldInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoFldInfo
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCIDLib::TCard4         c4Id
            , const TString&                strLimits
            , const tCQCKit::EFldAccess     eAccess
            , const tCQCKit::EFldSTypes     eSemType
        );

        TGenProtoFldInfo
        (
            const   TGenProtoFldInfo&       fldiToCopy
        );

        ~TGenProtoFldInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoFldInfo& operator=
        (
            const   TGenProtoFldInfo&       fldiToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddAckInfo
        (
                    TGenProtoReply* const   prepAck
            ,       TGenProtoReply* const   prepNak
            , const tCIDLib::TCard4         c4Wait
        );

        tCIDLib::TVoid AddWriteCmdItem
        (
                    TGenProtoExprNode* const pnodeToAdopt
        );

        tCIDLib::TBoolean bHasWriteCmds() const;

        tCIDLib::TBoolean bIsReviveField() const;

        tCIDLib::TBoolean bIsReviveField
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsWriteable() const;

        tCIDLib::TVoid BuildWriteCmd
        (
                    TGenProtoCtx&           ctxToUse
        );

        tCIDLib::TCard4 c4MillisToWait() const;

        tCIDLib::TCard4 c4MinSendInterval() const;

        tCIDLib::TCard4 c4MinSendInterval
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4NextSendTime() const;

        const TCQCFldDef& flddInfo() const;

        TCQCFldDef& flddInfo();

        TGenProtoReply* prepAck();

        TGenProtoReply* prepNak();

        tCIDLib::TVoid SetLastSendTime
        (
            const   tCIDLib::TCard4         c4ToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors
        // -------------------------------------------------------------------
        TGenProtoFldInfo();


        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef TRefVector<TGenProtoExprNode>   TExprList;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bIsReviveField
        //      For 'dead if off' type devices (which is set in the ProtocolType
        //      setting), one field is marked as the 'revive' field. It must
        //      be write only, and is used to force the device back on, so
        //      it is not put into error state when we lose comm with the
        //      device.
        //
        //  m_c4LastSendTime
        //      The last time this message was sent.
        //
        //  m_c4MinSendInterval
        //      The minimum interval that has to pass before two messages of
        //      this type can be sent. It defaults to zero.
        //
        //  m_c4WaitMillis
        //      If we have ack/nak replies defined, this indicates how long
        //      to wait for one of them to arrive.
        //
        //  m_colWriteCmd
        //      The vector of expression nodes that defines how we build up
        //      a command to write to this field. We just evaluate each
        //      expression and put its results into a buffer.
        //
        //  m_flddInfo
        //      The field definition data that describes the characteristics
        //      of the field.
        //
        //  m_prepAck
        //  m_prepNak
        //      The optional ack/nak replies that will come back in response
        //      to the write command. They can be zero if none is defined, or
        //      just the ack can be defined and the nak zero. We don't own
        //      these so we can just shallow copy the pointers.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bIsReviveField;
        tCIDLib::TCard4     m_c4LastSendTime;
        tCIDLib::TCard4     m_c4MinSendInterval;
        tCIDLib::TCard4     m_c4WaitMillis;
        TExprList           m_colWriteCmd;
        TCQCFldDef          m_flddInfo;
        TGenProtoReply*     m_prepAck;
        TGenProtoReply*     m_prepNak;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoFldInfo,TObject)
};

#pragma CIDLIB_POPPACK


