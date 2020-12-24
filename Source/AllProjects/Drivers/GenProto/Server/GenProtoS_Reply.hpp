//
// FILE NAME: GenProtoS_Reply.hpp
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
//  This is the header file for the GenProtoS_Reply.cpp file, which defines
//  the TGenProtoReply class. This class holds the info that we need to
//  recognize and invalidate responses from the device, and to parse out the
//  information in them.
//
//  They replies are named, and are referred to in later blocks in the
//  protocol file, which define exchanges between the driver and the device.
//  A hash map of these is kept by the driver data object which holds all of
//  the data resulting from the parse event.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


class TGenProtoSDriver;

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoReply
//  PREFIX: rep
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoReply : public TObject, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  We have a circular reference issue here, so we don't use the
        //  typedefs in the main header, and have to create our own.
        // -------------------------------------------------------------------
        typedef TVector<TGenProtoFldInfo>   TFldList;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TGenProtoReply&         repSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoReply
        (
            const   TString&                strKey
        );

        TGenProtoReply
        (
            const   TGenProtoReply&         repToCopy
        );

        ~TGenProtoReply();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoReply& operator=
        (
            const   TGenProtoReply&         repToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddStoreOp
        (
                    TGenProtoFldInfo* const pfldiTarget
            ,       TGenProtoExprNode* const pnodeValue
        );

        tCIDLib::TVoid AddStoreOp
        (
                    TGenProtoVarInfo* const pvariTarget
            ,       TGenProtoExprNode* const pnodeValue
            , const tCIDLib::TBoolean       bDeref
        );

        tCIDLib::TVoid AddValidationExpr
        (
                    TGenProtoExprNode* const pnodeToAdopt
        );

        tCIDLib::TBoolean bValidateReply
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::THashVal hshKey() const;

        const TString& strKey() const;

        tCIDLib::TVoid StoreValues
        (
                    TGenProtoCtx&           ctxThis
            ,       TFldList&               colFlds
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented ctors
        // -------------------------------------------------------------------
        TGenProtoReply();


        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef TRefVector<TGenProtoStoreOp>  TStoreOpList;
        typedef TRefVector<TGenProtoExprNode> TExprList;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colStoreOps
        //      Our list of storage operations that we do when our DoStoreOps()
        //      method is called. Each item pulls data out of the reply buffer
        //      and stores it in a field.
        //
        //  m_colValidate
        //      This is our vector of expression nodes that are used to
        //      validate the reply. They must all be boolean and if any of
        //      them return false, then the reply is assumed to be in error.
        //
        //  m_hshKey
        //      We pre-hash the key name to speed up searches.
        //
        //  m_strKey
        //      The key value for this item, which is the string portion that
        //      we map to.
        // -------------------------------------------------------------------
        TStoreOpList            m_colStoreOps;
        TExprList               m_colValidate;
        tCIDLib::THashVal       m_hshKey;
        TString                 m_strKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoReply,TObject)
        DefPolyDup(TGenProtoReply)
};


#pragma CIDLIB_POPPACK


