//
// FILE NAME: CQCTreeBrws_SrvBrwsIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/25/2015
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
//  This file holds the small amount of non-abstract functionality in the base
//  remote browsing interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCTreeBrwsIntf,TObject)




// ---------------------------------------------------------------------------
//   CLASS: TCQCTreeBrwsIntf
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCTreeBrwsIntf: Destructor
// ---------------------------------------------------------------------------
TCQCTreeBrwsIntf::~TCQCTreeBrwsIntf()
{
}



// ---------------------------------------------------------------------------
//  TCQCTreeBrwsIntf: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We implement the in place editor mixin, so that we can handle callbacks from
//  the IPE to validate an entered value. Currently it will only be used for an
//  in-place rename of an item in the tree.
//
//  We make sure that it doesn't include any invalid characters, and that it doesn't
//  match any other name in its parent scope.
//
//  If needed, derived browser classes can override this and provide more type
//  specific info.
//
tCIDLib::TBoolean
TCQCTreeBrwsIntf::bIPEValidate( const   TString&            strSrc
                                ,       TAttrData&          adatTar
                                , const TString&            strNewVal
                                ,       TString&            strErrMsg
                                ,       tCIDLib::TCard8&    ) const
{
    //
    //  Get a copy and strip leading/trailing whitespace. This will be our value that
    //  we use below to evaluate, and we'll put it back into the attribute as the actual
    //  value.
    //
    TString strRealVal = strNewVal;
    strRealVal.StripWhitespace();

    //
    //  Make sure it has nothing but alpha-num, and underscores and hyphens.
    //  That's enough to insure it'll be a legal file name.
    //
    const tCIDLib::TCard4 c4Len = strRealVal.c4Length();
    if (!c4Len)
    {
        strErrMsg = facCQCTreeBrws().strMsg(kTBrwsErrs::errcBrws_EmptyName);
        return kCIDLib::False;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
    {
        const tCIDLib::TCh chCur = strRealVal[c4Index];
        if (!TRawStr::bIsAlphaNum(chCur)
            && (chCur != kCIDLib::chSpace)
            && (chCur != kCIDLib::chHyphenMinus)
            && (chCur != kCIDLib::chUnderscore))
        {
            strErrMsg = facCQCTreeBrws().strMsg(kTBrwsErrs::errcBrws_BadName);
            return kCIDLib::False;
        }
    }

    //
    //  See if it's a dup of anything else at the same level. The special type
    //  value of the attribute is set to the original path, minus that target, so
    //  the parent path. We can use that to build up the path to check.
    //
    //  Only do this is the value has changed.
    //
    if (!strRealVal.bCompareI(adatTar.strValue()))
    {
        TString strPath(adatTar.strSpecType());
        facCQCRemBrws().AddPathComp(strPath, strRealVal);
        if (m_pwndBrowser->bPathExists(strPath))
        {
            strErrMsg = facCQCTreeBrws().strMsg(kTBrwsErrs::errcBrws_DupName);
            return kCIDLib::False;
        }
    }

    //
    //  It's good so store it. Our values are all just strings, so we don't have to
    //  any type specific stuff.
    //
    adatTar.SetString(strRealVal);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCTreeBrwsIntf: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  We handle this on behalf of derived classes if they don't override. We call a
//  helper to do the actual rename operation and to see if the target path can be
//  renamed, otherwise we can handle all of it here for everyone.
//
//  We use an in place editor and pass ourself as the validator (we implement his
//  callback mixin.) The derived class can also override the validation method and
//  do something different if needed.
//
tCIDLib::TBoolean
TCQCTreeBrwsIntf::bDoRename(const   TString&            strPath
                            ,       TTreeBrowseInfo&    wnotToSend)
{
    // If the derived class says it can't be renamed, then give up now
    if (!bCanRename(strPath))
        return kCIDLib::False;

    //
    //  Send a notification to the containing app to ask if it's OK, if not, give up.
    //  Default it to true so they only have to set it if they have it open, or if they
    //  don't handle it at all.
    //
    const tCIDLib::TBoolean bCanDoIt = bSendQuestionNot
    (
        tCQCTreeBrws::EEvents::CanRename
        , strPath
        , facCQCRemBrws().eXlatPathType(strPath, kCIDLib::True)
        , kCIDLib::True
    );

    if (!bCanDoIt)
    {
        TOkBox msgbFailed
        (
            L"This content is currently opened for viewing or editing. Please close "
            L"it and save any changes before you rename it."
        );
        msgbFailed.ShowIt(*m_pwndBrowser);
        return kCIDLib::False;
    }

    // Get the area of the target item. Tell it we want the whole width, not just the text
    TArea areaItem;
    if (!m_pwndBrowser->bQueryItemArea(strPath, areaItem, kCIDLib::False, kCIDLib::False))
        return kCIDLib::False;

    // Inflate it by one relative to the actual tree cell
    areaItem.Inflate(1, 1);

    // Enforce a minimum width so that they can type something useful.
    if (areaItem.c4Width() < 128)
        areaItem.c4Width(128);

    // Get the item display text
    TString strVal;
    m_pwndBrowser->QueryItemText(strPath, strVal);

    TAttrData adatEdit;
    adatEdit.Set
    (
        L"ItemName"
        , L"ItemName"
        , kCIDMData::strAttrLim_Required
        , tCIDMData::EAttrTypes::String
    );
    adatEdit.SetString(strVal);

    //
    //  Store the parent path as the special type value so that it'savailable in the
    //  validation callback.
    //
    TString strParPath;
    facCQCRemBrws().QueryParentPath(strPath, strParPath);
    adatEdit.SetSpecType(strParPath);

    TInPlaceEdit ipeVal;
    if (ipeVal.bProcess(*m_pwndBrowser, areaItem, adatEdit, *this, 0))
    {
        const tCIDLib::TBoolean bIsScope = m_pwndBrowser->bIsScope(strPath);
        if (bRenameItem(strParPath, strVal, adatEdit.strValue(), bIsScope))
        {
            //
            //  It worked, so fill in the notification that should be sent. We need
            //  to build up the full old and new paths to send.
            //
            TString strNew;
            strNew = strParPath;
            facCQCRemBrws().AddPathComp(strNew, adatEdit.strValue());
            wnotToSend = TTreeBrowseInfo
            (
                tCQCTreeBrws::EEvents::Renamed
                , strPath
                , strNew
                , facCQCRemBrws().eXlatPathType(strPath, kCIDLib::True)
                , *m_pwndBrowser
            );
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// The derived classes must call us here and pass on the user context, which we store for later
tCIDLib::TVoid TCQCTreeBrwsIntf::Initialize(const TCQCUserCtx& cuctxUser)
{
    m_cuctxUser = cuctxUser;
}


//
//  This is only need by browser objects that have bgn threads and need to post
//  stuff back to themselves to process in the GUI thread. Most of them don't
//  have bgn threads, so we provide a default implementation.
//
tCIDLib::TVoid
TCQCTreeBrwsIntf::ProcessChange(const   TString&
                                , const TString&
                                , const TString&)
{
}


//
//  Most browser objects don't need this. It's mostly just if they need to inform
//  some server they talk to that this thing has been changed and saved back to
//  whereever it's stored. The browser may need to inform some server that it needs
//  go back and re-read this new data. This is called after the admin client saves
//  a 'file'.
//
tCIDLib::TVoid TCQCTreeBrwsIntf::UpdateFile(const TString&)
{
}



// ---------------------------------------------------------------------------
//  TCQCTreeBrwsIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Provide access to the root's display name
const TString& TCQCTreeBrwsIntf::strName() const
{
    return m_strName;
}


// Provide access to the root path
const TString& TCQCTreeBrwsIntf::strRootPath() const
{
    return m_strRootPath;
}


// Provide access to this browser's display title
const TString& TCQCTreeBrwsIntf::strTitle() const
{
    return m_strTitle;
}


// The browser window will give us a pointer to himself before he creates us
tCIDLib::TVoid
TCQCTreeBrwsIntf::SetWindows(TCQCTreeBrowser* const pwndBrowser)
{
    m_pwndBrowser = pwndBrowser;
}


// Let our derived classes see the owning window and tree view
TCQCTreeBrowser& TCQCTreeBrwsIntf::wndBrowser() const
{
    return *m_pwndBrowser;
}




// ---------------------------------------------------------------------------
//  TCQCTreeBrwsIntf: Hidden constructors
// ---------------------------------------------------------------------------
TCQCTreeBrwsIntf::TCQCTreeBrwsIntf( const   TString&    strRootPath
                                    , const TString&    strName
                                    , const TString&    strTitle) :
    m_cuctxUser()
    , m_pwndBrowser(nullptr)
    , m_strName(strName)
    , m_strRootPath(strRootPath)
    , m_strTitle(strTitle)
{
}


// ---------------------------------------------------------------------------
//  TCQCTreeBrwsIntf: Protected, virtual methods
// ---------------------------------------------------------------------------

// If not overridden, we say it cannot be renamed
tCIDLib::TBoolean TCQCTreeBrwsIntf::bCanRename(const TString&) const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCTreeBrwsIntf::bRenameItem(  const   TString&            strParPath
                                , const TString&
                                , const TString&
                                , const tCIDLib::TBoolean)
{
    // If not overridden, then display an error
    TErrBox msgbErr
    (
        m_strTitle, TString::strConcat(L"This scope doesn't support renaming. Scope=", strParPath)
    );
    msgbErr.ShowIt(*m_pwndBrowser);
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCTreeBrwsIntf: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Send a tree browser notification out on behalf of the derived class, and return
//  the status flag that it comes back with. It's assumed this is one of the event
//  event types that asks a question.
//
tCIDLib::TBoolean
TCQCTreeBrwsIntf::bSendQuestionNot( const   tCQCTreeBrws::EEvents   eEvent
                                    , const TString&                strHPath
                                    , const tCQCRemBrws::EDTypes    eType
                                    , const tCIDLib::TBoolean       bDefault)
{
    // Build the notification to send
    TTreeBrowseInfo wnotQ
    (
        eEvent
        , strHPath
        , eType
        , *m_pwndBrowser
    );

    // Set the default in case they don't set it
    wnotQ.bFlag(bDefault);

    // Has to be sync, since we get back info here
    m_pwndBrowser->SendSyncNotify(wnotQ, TCQCTreeBrowser::nidBrowseEvent);
    return wnotQ.bFlag();
}

