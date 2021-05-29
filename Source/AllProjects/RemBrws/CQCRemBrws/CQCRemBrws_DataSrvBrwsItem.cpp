//
// FILE NAME: CQCRemBrws_DataSrvBrwsItem.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This is the implementation file for the data server client class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRemBrws_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TDSBrowseItem,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCRemBrws_DataSrvClient
    {
        // -----------------------------------------------------------------------
        //  The persistent format version for the browser item class. In actual fact they
        //  are never persisted. They are only flattened for transmission, but we use a
        //  format version for future flexibility.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 1;
    }
};





// ---------------------------------------------------------------------------
//   CLASS: TDSBrowseItem
//  PREFIX: dsbi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDSBrowseItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TDSBrowseItem::TDSBrowseItem() :

    m_bUserFlag(kCIDLib::False)
    , m_c4IdItem(0)
    , m_c4IdScope(0)
    , m_eFlags(tCQCRemBrws::EItemFlags::None)
    , m_eType(tCQCRemBrws::EDTypes::Count)
    , m_eMinRole(tCQCKit::EUserRoles::SystemAdmin)
{
}

TDSBrowseItem::TDSBrowseItem(const  TString                 strName
                            , const tCQCRemBrws::EDTypes    eType
                            , const tCIDLib::TCard4         c4IdScope
                            , const tCIDLib::TCard4         c4IdItem
                            , const tCQCKit::EUserRoles     eMinRole
                            , const tCQCRemBrws::EItemFlags eFlags) :

    m_bUserFlag(kCIDLib::False)
{
    Set(strName, eType, c4IdScope, c4IdItem, eMinRole, eFlags);
}

TDSBrowseItem::~TDSBrowseItem()
{
}


// ---------------------------------------------------------------------------
//  TDSBrowseItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Getters that all just check a bit in the flags member. In all of these we have special
//  concerns for the special system edit flag, which is used internally for us to edit
//  system resources in place.
//
tCIDLib::TBoolean TDSBrowseItem::bCanCopy() const
{
    // Special scopes never are copyable
    if (bIsSpecialScope())
        return kCIDLib::False;

    //
    //  Otherwise it should be fine. System files and scopes can be copied, and later
    //  pasted back into the user area.
    //
    return kCIDLib::True;
}


tCIDLib::TBoolean TDSBrowseItem::bCanCreateNew() const
{
    //
    //  Here we allow this in a scope or the root or a system scope (if in system
    //  edit mode), because we are creating something under it, not doing something
    //  to it.
    //
    return
    (
        tCIDLib::bAllBitsOn(m_eFlags, tCQCRemBrws::EItemFlags::CanNew)
        || (facCQCKit().bSysEdit()
        &&  ((m_eFlags == tCQCRemBrws::EItemFlags::SystemRoot)
        ||   (m_eFlags == tCQCRemBrws::EItemFlags::SystemScope)))
    );
}

tCIDLib::TBoolean TDSBrowseItem::bCanDelete() const
{
    return
    (
        tCIDLib::bAllBitsOn(m_eFlags, tCQCRemBrws::EItemFlags::CanDelete)
        || ((m_eFlags == tCQCRemBrws::EItemFlags::SystemScope) && facCQCKit().bSysEdit())
        || ((m_eFlags == tCQCRemBrws::EItemFlags::SystemItem) && facCQCKit().bSysEdit())
    );
}

tCIDLib::TBoolean TDSBrowseItem::bCanEdit() const
{
    return
    (
        tCIDLib::bAllBitsOn(m_eFlags, tCQCRemBrws::EItemFlags::CanEdit)
        || ((m_eFlags == tCQCRemBrws::EItemFlags::SystemItem) && facCQCKit().bSysEdit())
    );
}

tCIDLib::TBoolean TDSBrowseItem::bCanInvoke() const
{
    return
    (
        tCIDLib::bAllBitsOn(m_eFlags, tCQCRemBrws::EItemFlags::CanEdit)
        || ((m_eFlags == tCQCRemBrws::EItemFlags::SystemItem) && facCQCKit().bSysEdit())
    );
}

tCIDLib::TBoolean TDSBrowseItem::bCanPaste() const
{
    // If it's a user scope or a user root scope, then it's ok
    if ((m_eFlags == tCQCRemBrws::EItemFlags::UserScope)
    || (m_eFlags == tCQCRemBrws::EItemFlags::UserRoot))
    {
        return kCIDLib::True;
    }

    // If system edit is enable, and it's a system scope or system root scope, then ok
    if (facCQCKit().bSysEdit())
    {
        if ((m_eFlags == tCQCRemBrws::EItemFlags::SystemScope)
        ||  (m_eFlags == tCQCRemBrws::EItemFlags::SystemRoot))
        {
            return kCIDLib::True;
        }
    }

    // Otherwise, not doable
    return kCIDLib::False;
}

tCIDLib::TBoolean TDSBrowseItem::bCanRename() const
{
    return
    (
        tCIDLib::bAllBitsOn(m_eFlags, tCQCRemBrws::EItemFlags::CanRename)
        || ((m_eFlags == tCQCRemBrws::EItemFlags::SystemScope) && facCQCKit().bSysEdit())
        || ((m_eFlags == tCQCRemBrws::EItemFlags::SystemItem) && facCQCKit().bSysEdit())
    );
}

tCIDLib::TBoolean TDSBrowseItem::bIsScope() const
{
    return tCIDLib::bAllBitsOn(m_eFlags, tCQCRemBrws::EItemFlags::IsScope);
}

//
//  A special scope, which we consider to be the ones marked non-typed, or root. THat's the
//  ones above the typed part of the hierarchy, and System and User ones.
//
tCIDLib::TBoolean TDSBrowseItem::bIsSpecialScope() const
{
    return
    (
        tCIDLib::bAllBitsOn(m_eFlags, tCQCRemBrws::EItemFlags::NonTyped)
        || tCIDLib::bAllBitsOn(m_eFlags, tCQCRemBrws::EItemFlags::Root)
    );
}


tCIDLib::TBoolean TDSBrowseItem::bIsSystemItem() const
{
    return (m_eFlags == tCQCRemBrws::EItemFlags::SystemItem);
}


// A non-special system scope
tCIDLib::TBoolean TDSBrowseItem::bIsSystemScope() const
{
    return (m_eFlags == tCQCRemBrws::EItemFlags::SystemScope);
}


// A non-special user scope
tCIDLib::TBoolean TDSBrowseItem::bIsUserItem() const
{
    return (m_eFlags == tCQCRemBrws::EItemFlags::UserItem);
}


tCIDLib::TBoolean TDSBrowseItem::bIsUserScope() const
{
    return (m_eFlags == tCQCRemBrws::EItemFlags::UserScope);
}


tCIDLib::TBoolean TDSBrowseItem::bUserFlag() const
{
    return m_bUserFlag;
}

tCIDLib::TBoolean TDSBrowseItem::bUserFlag(const tCIDLib::TBoolean bToSet)
{
    m_bUserFlag = bToSet;
    return m_bUserFlag;
}


// Bump the item or scope ids
tCIDLib::TCard4 TDSBrowseItem::c4BumpItemId()
{
    return ++m_c4IdItem;
}


tCIDLib::TCard4 TDSBrowseItem::c4BumpScopeId()
{
    return ++m_c4IdScope;
}


// Get or set the ids
tCIDLib::TCard4 TDSBrowseItem::c4IdItem() const
{
    return m_c4IdItem;
}

tCIDLib::TCard4 TDSBrowseItem::c4IdItem(const tCIDLib::TCard4 c4ToSet)
{
    m_c4IdItem = c4ToSet;
    return m_c4IdItem;
}

tCIDLib::TCard4 TDSBrowseItem::c4IdScope() const
{
    return m_c4IdScope;
}

tCIDLib::TCard4 TDSBrowseItem::c4IdScope(const tCIDLib::TCard4 c4ToSet)
{
    m_c4IdScope = c4ToSet;
    return m_c4IdScope;
}


tCQCRemBrws::EItemFlags TDSBrowseItem::eFlags() const
{
    return m_eFlags;
}


// Return the file type this item represents
tCQCRemBrws::EDTypes TDSBrowseItem::eType() const
{
    return m_eType;
}


// Get or set the minimum role for this item
tCQCKit::EUserRoles TDSBrowseItem::eMinRole() const
{
    return m_eMinRole;
}

tCQCKit::EUserRoles TDSBrowseItem::eMinRole(const tCQCKit::EUserRoles eToSet)
{
    m_eMinRole = eToSet;
    return m_eMinRole;
}


// Return the item/scope name of this item
const TString& TDSBrowseItem::strName() const
{
    return m_strName;
}

const TString& TDSBrowseItem::strName(const TString& strToSet)
{
    m_strName = strToSet;
    return m_strName;
}


// Reset the item after contructions
tCIDLib::TVoid
TDSBrowseItem::Set( const   TString                 strName
                    , const tCQCRemBrws::EDTypes    eType
                    , const tCIDLib::TCard4         c4IdScope
                    , const tCIDLib::TCard4         c4IdItem
                    , const tCQCKit::EUserRoles     eMinRole
                    , const tCQCRemBrws::EItemFlags eFlags)
{
    m_c4IdItem   = c4IdItem;
    m_c4IdScope  = c4IdScope;
    m_eFlags     = eFlags;
    m_eType      = eType;
    m_eMinRole   = eMinRole;
    m_strName    = strName;
}


// ---------------------------------------------------------------------------
//  TDSBrowseItem: Protected, non-virtaul methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TDSBrowseItem::StreamFrom(TBinInStream& strmToReadFrom)
{
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCRemBrws_DataSrvClient::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_bUserFlag
                    >> m_c4IdItem
                    >> m_c4IdScope
                    >> m_eFlags
                    >> m_eType
                    >> m_eMinRole
                    >> m_strName;

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TDSBrowseItem::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCRemBrws_DataSrvClient::c2FmtVersion

                    << m_bUserFlag
                    << m_c4IdItem
                    << m_c4IdScope
                    << m_eFlags
                    << m_eType
                    << m_eMinRole
                    << m_strName

                    << tCIDLib::EStreamMarkers::EndObject;
}


