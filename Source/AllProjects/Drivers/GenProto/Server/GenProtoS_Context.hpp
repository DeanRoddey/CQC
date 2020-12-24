//
// FILE NAME: GenProtoS_Context.hpp
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
//  This is the header file for the GenProtoS_Context.cpp file, which
//  implements a class via which any called code can access various bits of
//  information via a single interface, so that it can be moved around
//  later without affecting the code that accesses it. This provide info
//  such as the available variables, prompt values passed to the driver via
//  user config, and other bits of info that might be referenced by the
//  code that builds or parsees messages.
//
//  So an expression context is passed in to the expression evaluation method
//  of the root expression node. It is then passed along through the nested
//  expression tree.
//
//  The information in the context is:
//
//  1.  A buffer to act as a target for building queries.
//  2.  A buffer to recieve reply data and from which expressions pull out
//      infomration.
//  3.  The value being written (which is only applicable for write comamnd
//      expressions), which is available as %WriteVal in write command
//      expressions.
//
// CAVEATS/GOTCHAS:
//
//  1)  For now, read/write is never done on the same context, so we have
//      one buffer internally. The ctor checks this and if we ever turn on
//      the ability to have them both, it will throw.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCtx
//  PREFIX: ctx
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoCtx : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoCtx
        (
            const   tGenProtoS::ESpecNodes  eNodes
            , const TString&                strName
        );

        ~TGenProtoCtx();


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard1 c1InputByte() const;

        tCIDLib::TCard1 c1InputByte
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TCard4 c4ReplyBytes() const;

        tCIDLib::TCard4 c4ReplyBytes
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4SendBytes() const;

        tCIDLib::TCard4 c4SendBytes
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid ChangeEncoding
        (
            const   TString&                strEncodingToSet
        );

        const TGenProtoExprVal& evalWrite() const;

        TGenProtoExprVal& evalWrite();

        TGenProtoExprVal& evalWrite
        (
            const   TGenProtoExprVal&       evalToSet
        );

        const TGenProtoInfo& gpinfoProto() const;

        const TGenProtoInfo& gpinfoProto
        (
            const   TGenProtoInfo&          gpinfoToSet
        );

        const TMemBuf& mbufReply() const;

        TMemBuf& mbufReply();

        const TMemBuf& mbufSend() const;

        TMemBuf& mbufSend();

        TGenProtoSDriver* psdrvThis();

        const TString& strProtoFile() const;

        const TString& strProtoFile
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetDriver
        (
                    TGenProtoSDriver* const psdrvToSet
        );

        TTextConverter& tcvtDevice();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented ctors and operators
        // -------------------------------------------------------------------
        TGenProtoCtx();
        TGenProtoCtx(const TGenProtoCtx&);
        tCIDLib::TVoid operator=(const TGenProtoCtx&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckAccess
        (
            const   tGenProtoS::ESpecNodes  eToCheck
            , const TString&                strMember
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c1InputByte
        //      The current byte read from the COM port that is being passed
        //      to the state machine.
        //
        //  m_c4DataBytes
        //      The number of bytes valid in the buffer. This will be zero
        //      if this context doesn't support access to the buffer.
        //
        //  m_evalWrite
        //      If this is a write command expression, then it can refer to
        //      the value being written. So that value must be set on the
        //      context before evaluation.
        //
        //  m_eAccess
        //      During expression parsing a special bitmapped enum is used
        //      to keep up with whether special expressions are allowed in that
        //      type of expression. We use the same enum to know if its ok
        //      for certain of our members to be accessed. So the ctor requires
        //      that we be set to allow particular field access.
        //
        //  m_gpinfoProto
        //      We get a copy of the global protocol info so that we can make
        //      it known to message processing code.
        //
        //  m_mbufData
        //      A memory buffer that's used to hold data send/reply data from
        //      the device. For now, since both send and reply is never done
        //      on the same context, we can use a single buffer.
        //
        //  m_ptcvtDevice
        //      The text encoding. This is used whenever any text is read from
        //      or written to the memory buffer. It is set to reflect the
        //      encoding we are given.
        //
        //  m_psdrvThis
        //      A pointer to the driver itself, since that is required for
        //      field read/write operations.
        //
        //  m_strName
        //      A name for use in error messages, so that we'll know what
        //      context the errors is in.
        //
        //  m_strPtotoFile
        //      We need to get this into expressions for error reporting.
        // -------------------------------------------------------------------
        tCIDLib::TCard1         m_c1InputByte;
        tCIDLib::TCard4         m_c4DataBytes;
        TGenProtoExprVal        m_evalWrite;
        tGenProtoS::ESpecNodes  m_eAccess;
        TGenProtoInfo           m_gpinfoProto;
        THeapBuf                m_mbufData;
        TGenProtoSDriver*       m_psdrvThis;
        TTextConverter*         m_ptcvtDevice;
        TString                 m_strName;
        TString                 m_strProtoFile;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoCtx, TObject)
};

#pragma CIDLIB_POPPACK

