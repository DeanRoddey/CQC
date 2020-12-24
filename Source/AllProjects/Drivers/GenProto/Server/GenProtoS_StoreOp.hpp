//
// FILE NAME: GenProtoS_StoreOp.hpp
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
//  This is the header file for the GenProtoS_StoreOp.cpp file, which
//  defines the TGenProtoStoreOp class, which is used in a number of places
//  where setting of fields or variables are allowed. In those cases, we
//  need to store a field or variable info object and the expression that
//  will be evaluated to get the value.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoStoreOp
//  PREFIX: sop
// ---------------------------------------------------------------------------
class TGenProtoStoreOp : public TObject, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructors
        // -------------------------------------------------------------------
        TGenProtoStoreOp() = delete;

        TGenProtoStoreOp
        (
                    TGenProtoFldInfo* const pfldiTarget
            ,       TGenProtoExprNode* const pnodeValue
        );

        TGenProtoStoreOp
        (
                    TGenProtoVarInfo* const pvariTarget
            ,       TGenProtoExprNode* const pnodeValue
            , const tCIDLib::TBoolean       bDeref
        );

        TGenProtoStoreOp
        (
            const   TGenProtoStoreOp&       sopToCopy
        );

        ~TGenProtoStoreOp();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   TGenProtoStoreOp&       sopToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsDeref() const;

        tCIDLib::TBoolean bDoStore
        (
                    TGenProtoCtx&           ctxThis
        );

        const TString& strTarget() const;

        const TString& strVarValue
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid SetField
        (
                    TGenProtoFldInfo* const pfldiTarget
        );


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        enum class EStoreTypes
        {
            Field
            , Variable
        };


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDeref
        //      Indicates whether our variable/const target is the actual
        //      target, or whether we should dereferenced it and treat it as
        //      the actual name of the target field. This allows for dynamic
        //      targeting of reply blocks.
        //
        //  m_eType
        //      Indicates whether the field or variable info object is the
        //      target of the store.
        //
        //  m_pfldiTarget
        //      This is a pointer to the field info object that we are to
        //      store to.
        //
        //  m_pnodeValue
        //      The top level expression node that we evaluate to create the
        //      value to store.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bDeref;
        EStoreTypes         m_eType;
        TGenProtoFldInfo*   m_pfldiTarget;
        TGenProtoExprNode*  m_pnodeValue;
        TGenProtoVarInfo*   m_pvariTarget;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoStoreOp, TObject)
        DefPolyDup(TGenProtoStoreOp)
};

#pragma CIDLIB_POPPACK


