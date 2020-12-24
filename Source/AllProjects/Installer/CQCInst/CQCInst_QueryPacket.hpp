//
// FILE NAME: CQCInst_QueryPacket.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/15/2005
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
//  The installation happens in a background thread, but if that thread needs to
//  interact with the user, it needs to do so through the foreground thread thread. So
//  this class is used for that purpose. The background thread sets it up, then does a
//  synchronous message send to the message loop, which the foreground thread handles in
//  a window callback, does the requested action, and returns any status and output.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstQueryPacket
//  PREFIX: qpack
// ---------------------------------------------------------------------------
class TInstQueryPacket : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstQueryPacket();

        TInstQueryPacket
        (
            const   tCQCInst::EQueryOps     eOp
            , const tCIDCtrls::EMsgBoxTypes eMsgBoxType
            , const TString&                strMsg
        );

        TInstQueryPacket
        (
            const   TString&                strMsg
            , const TError&                 errToDisplay
        );

        TInstQueryPacket(const TInstQueryPacket&) = delete;
        TInstQueryPacket(TInstQueryPacket&&) = delete;

        ~TInstQueryPacket();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInstQueryPacket& operator=(const TInstQueryPacket&) = delete;
        TInstQueryPacket& operator=(TInstQueryPacket&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TError& errException() const;

        const TError& errException
        (
            const   TError&                 errToSet
        );

        tCIDCtrls::EMsgBoxTypes eMsgBoxType() const;

        tCIDCtrls::EMsgBoxRets eMsgBoxRet() const;

        tCIDCtrls::EMsgBoxRets eMsgBoxRet
        (
            const   tCIDCtrls::EMsgBoxRets  eToSet
        );

        tCQCInst::EQueryOps eOp() const;

        tCQCInst::EQueryRes eResult() const;

        tCQCInst::EQueryRes eResult
        (
            const   tCQCInst::EQueryRes     eToSet
        );

        const TString& strMsg() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_errException
        //      If the op is to show a fatal error, then this is the error
        //      info details. If coming back from an op and the m_eResult
        //      indicates an exception, this the exception that occured during
        //      the op handling.
        //
        //  m_eMsgBoxType
        //      If the operation indicates a message box, then this is the type of
        //      message box to display.
        //
        //  m_eOp
        //      The operation to do.
        //
        //  m_eMsgBoxRet
        //      If the operation is to display some message box, this is the return
        //      status of the message box, if relevant. For an OK box, there's no
        //      return status.
        //
        //  m_eResult
        //      The overall result of the query op being processed.
        //
        //  m_strMsg
        //      The message to display to the user.
        // -------------------------------------------------------------------
        TError                  m_errException;
        tCIDCtrls::EMsgBoxTypes m_eMsgBoxType;
        tCQCInst::EQueryOps     m_eOp;
        tCIDCtrls::EMsgBoxRets  m_eMsgBoxRet;
        tCQCInst::EQueryRes     m_eResult;
        TString                 m_strMsg;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstQueryPacket,TObject)
};

#pragma CIDLIB_POPPACK

