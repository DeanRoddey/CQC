//
// FILE NAME: GenProtoS_Query.hpp
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
//  This is the header file for the GenProtoS_Query.cpp file, which defines
//  the TGenProtoQuery class. This class holds the info that we need to build
//  up a query command to send to the controlled device. It is composed of
//  some info data and a set of expressions, each of which is evaluated and
//  the result placed into a memory buffer, which is then transmitted.
//
//  They queries are named, and are referred to in later blocks in the
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

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoQuery
//  PREFIX: qry
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoQuery : public TObject, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TGenProtoQuery&         qrySrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoQuery
        (
            const   TString&                strKey
        );

        TGenProtoQuery
        (
            const   TGenProtoQuery&         qryToCopy
        );

        ~TGenProtoQuery();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoQuery& operator=
        (
            const   TGenProtoQuery&         qryToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddQueryExpr
        (
                    TGenProtoExprNode* const pnodeToAdopt
        );

        tCIDLib::TVoid BuildQueryCmd
        (
                    TGenProtoCtx&           ctxToUse
        );

        tCIDLib::THashVal hshKey() const;

        const TString& strKey() const;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented ctors
        // -------------------------------------------------------------------
        TGenProtoQuery();


        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef TRefVector<TGenProtoExprNode> TExprList;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colQuery
        //      This is our vector of expression nodes that are used to build
        //      up the query command.
        //
        //  m_hshKey
        //      We pre-hash the key to speed up searches.
        //
        //  m_strKey
        //      The key value for this item, which is the string portion that
        //      we map to.
        // -------------------------------------------------------------------
        TExprList           m_colExprs;
        tCIDLib::THashVal   m_hshKey;
        TString             m_strKey;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoQuery,TObject)
        DefPolyDup(TGenProtoQuery)
};


#pragma CIDLIB_POPPACK



