//
// FILE NAME: CQCRemBrws_DataSrvBrowsItem.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/27/2015
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
//  This is a class that is used by the data server to return hierarchical data
//  info that is used in the client side tree browser. The pluggable browser object
//  that provides browsing of data server oriented info stores instances of these
//  on each item in its part of the tree, to keep up with what actions are possible
//  on the item, and to be able to check for changes and such. It has to be down
//  here before the data server needs to understand it as well, and this faciltiy
//  is used by the data server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TDSBrowseItem
//  PREFIX: dsbi
// ---------------------------------------------------------------------------
class CQCREMBRWSEXP TDSBrowseItem : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDSBrowseItem();

        TDSBrowseItem
        (
            const   TString                 strName
            , const tCQCRemBrws::EDTypes    eType
            , const tCIDLib::TCard4         c4IdScope
            , const tCIDLib::TCard4         c4IdItem
            , const tCQCKit::EUserRoles     eMinRole
            , const tCQCRemBrws::EItemFlags eFlags
        );

        TDSBrowseItem(const TDSBrowseItem&) = default;
        TDSBrowseItem(TDSBrowseItem&&) = default;

        ~TDSBrowseItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDSBrowseItem& operator=(const TDSBrowseItem&) = default;
        TDSBrowseItem& operator=(TDSBrowseItem&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanCopy() const;

        tCIDLib::TBoolean bCanCreateNew() const;

        tCIDLib::TBoolean bCanDelete() const;

        tCIDLib::TBoolean bCanEdit() const;

        tCIDLib::TBoolean bCanInvoke() const;

        tCIDLib::TBoolean bCanPaste() const;

        tCIDLib::TBoolean bCanRename() const;

        tCIDLib::TBoolean bIsSpecialScope() const;

        tCIDLib::TBoolean bIsScope() const;

        tCIDLib::TBoolean bIsSystemItem() const;

        tCIDLib::TBoolean bIsSystemScope() const;

        tCIDLib::TBoolean bIsUserItem() const;

        tCIDLib::TBoolean bIsUserScope() const;

        tCIDLib::TBoolean bLiveState() const;

        tCIDLib::TBoolean bLiveState
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bUserFlag() const;

        tCIDLib::TBoolean bUserFlag
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4BumpItemId();

        tCIDLib::TCard4 c4BumpScopeId();

        tCIDLib::TCard4 c4IdItem() const;

        tCIDLib::TCard4 c4IdItem
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4IdScope() const;

        tCIDLib::TCard4 c4IdScope
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCRemBrws::EItemFlags eFlags() const;

        tCQCRemBrws::EDTypes eType() const;

        tCQCKit::EUserRoles eMinRole() const;

        tCQCKit::EUserRoles eMinRole
        (
            const   tCQCKit::EUserRoles     eToSet
        );

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Set
        (
            const   TString                 strName
            , const tCQCRemBrws::EDTypes    eType
            , const tCIDLib::TCard4         c4IdScope
            , const tCIDLib::TCard4         c4IdItem
            , const tCQCKit::EUserRoles     eMinRole
            , const tCQCRemBrws::EItemFlags eFlags
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bUserFlag
        //      For use by the application. Currently, for events it's set if the event
        //      is paused, so that this info is available in the hierarchical data for
        //      the tree browser to use.
        //
        //  m_c4IdItem
        //      The id for this item itself, whether scope or item, so basically
        //      the id for changes in name.
        //
        //  m_c4IdScope
        //      If a scope, this is the id for scope content changes.
        //
        //  m_eFlags
        //      The flags set on this item, to indicate its type and the operations
        //      legal on it and so forth.
        //
        //  m_eMinRole
        //      The minimum user role required to edit this item, if it is even
        //      editable.
        //
        //  m_eType
        //      Though it will typically will be implied from where in a hierchical
        //      data structure this object is stored, we store the data type to
        //      make it much faster to determine the type.
        //
        //  m_strName
        //      The display name of this item.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bUserFlag;
        tCIDLib::TCard4         m_c4IdItem;
        tCIDLib::TCard4         m_c4IdScope;
        tCQCRemBrws::EItemFlags m_eFlags;
        tCQCKit::EUserRoles     m_eMinRole;
        tCQCRemBrws::EDTypes    m_eType;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDSBrowseItem,TObject)
};


#pragma CIDLIB_POPPACK



