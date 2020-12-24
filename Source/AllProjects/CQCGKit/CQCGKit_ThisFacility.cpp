//
// FILE NAME: CQCGKit_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/11/2001
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
//  This is the implementation file for the facility class. WE have a local
//  class or two to help with info we track on behalf of the client.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGKit_.hpp"
#include    "CQCGKit_ChangePWDlg_.hpp"
#include    "CQCGKit_DrvInfoDlg_.hpp"
#include    "CQCGKit_EdExprDlg_.hpp"
#include    "CQCGKit_GetVersionDlg_.hpp"
#include    "CQCGKit_LogonDlg_.hpp"
#include    "CQCGKit_WriteFldDlg_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCGKit,TFacility)




// ---------------------------------------------------------------------------
// Local data, types and constants
// ---------------------------------------------------------------------------
namespace CQCGKit_ThisFacility
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The key that we store app attach info under
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const pszOSKey_AppAttach = L"/AppAttachInfo/List";
    }
}


// ---------------------------------------------------------------------------
//  We have a set of PNG images that can be loaded by name. We load them
//  up and convert to a bitmap and return it. We also cache them so that
//  we don't have to reload them constantly.
// ---------------------------------------------------------------------------
class TBmpCacheItem
{
    public :
        static const TString& strKey(const TBmpCacheItem& itemSrc)
        {
            return itemSrc.m_strName;
        }

        TBmpCacheItem(const TString& strName, TBitmap& bmpItem) :
            m_bmpItem(bmpItem)
            , m_strName(strName)
        {
        }

        TBitmap m_bmpItem;
        TString m_strName;
};



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCGKit
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCGKit: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCGKit::TFacCQCGKit() :

    TGUIFacility
    (
        L"CQCGKit"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
        , TString(L"CQC")
    )
    , m_bNoLocalStore(kCIDLib::False)
    , m_poseConfig(nullptr)
{
}


TFacCQCGKit::~TFacCQCGKit()
{
    // Should get cleaned up through a term call, but just in case
    if (m_poseConfig)
    {
        try
        {
            delete m_poseConfig;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCGKit: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Add an object to the local store
tCIDLib::TVoid
TFacCQCGKit::AddStoreObj(const  TString&        strKey
                        , const MStreamable&    strmblToWrite
                        , const tCIDLib::TCard4 c4Reserve)
{
    // If in no local store mode, do nothing
    if (m_bNoLocalStore)
        return;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    m_poseConfig->AddObject(strKey, strmblToWrite, c4Reserve);
}


// Add or update an object in the local objec store
tCIDLib::TBoolean
TFacCQCGKit::bAddUpdateStoreObj(const   TString&            strKey
                                ,       tCIDLib::TCard4&    c4Vers
                                , const MStreamable&        strmblToWrite
                                , const tCIDLib::TCard4     c4Reserve)
{
    // If in no local store mode, do nothing, return false to make the caller happy
    if (m_bNoLocalStore)
        return kCIDLib::False;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    return m_poseConfig->bAddOrUpdate(strKey, c4Vers, strmblToWrite, c4Reserve);
}


//
//  Does the grunt work of changing a user's password. We get the user name of the
//  account to change, and the caller has to provide the old password for verification,
//  so that we can prove he knew it before allowing the change.
//
//  They indicate if the new password is the CQC password or the web password. THe old
//  password is always the CQC password.
//
tCIDLib::TBoolean
TFacCQCGKit::bChangePassword(const  TWindow&            wndOwner
                            , const TString&            strUserName
                            , const TString&            strOldCQCPW
                            , const TString&            strNewPW
                            , const tCIDLib::TBoolean   bIsCQCPW)
{
    try
    {
        tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

        //
        //  First we will got through a faux login exchange, to be absolutely
        //  sure that they provided the correct password. So ask the security
        //  server to give us a challenge. We use the name of the currently
        //  logged in user.
        //
        TCQCSecChallenge seccChangePW;
        if (!orbcSS->bLoginReq(strUserName, seccChangePW))
        {
            // Tell the user this is an unknown login name
            TErrBox msgbBadName
            (
                facCQCGKit().strMsg(kGKitErrs::errcLogon_UnknownUser, strUserName)
            );
            msgbBadName.ShowIt(wndOwner);
            return kCIDLib::False;
        }

        // Lets do a hash of the user's current password
        TMD5Hash mhashPW;
        TMessageDigest5 mdigPW;
        mdigPW.StartNew();
        mdigPW.DigestStr(strOldCQCPW);
        mdigPW.Complete(mhashPW);

        // And use that to validate the challenge
        seccChangePW.Validate(mhashPW);

        // And send that back to get a security token, assuming its legal
        tCQCKit::ELoginRes  eRes;
        TCQCSecToken        sectTmp;
        TCQCUserAccount     uaccTmp;
        if (!orbcSS->bGetSToken(seccChangePW, sectTmp, uaccTmp, eRes))
        {
            // Tell the user what went wrong
            TErrBox msgbBadPW(tCQCKit::strXlatELoginRes(eRes));
            msgbBadPW.ShowIt(wndOwner);
            return kCIDLib::False;
        }

        //
        //  Ok, they know the current password, so lets do the password change
        //  event. So create a key from the current password hash and then a
        //  Blowfish encrypter from that key. We'll use this to encrypt the
        //  new password before we send it over. The server knows the old PW
        //  hash and will us it to decrypt what we send.
        //
        TCryptoKey ckeyPW(mhashPW);
        TBlowfishEncrypter crypPW(ckeyPW);

        //
        //  We send different stuff based on the type of password that we are
        //  changing. So create an output stream and format the apprporiate
        //  stuff into it.
        //
        THeapBuf mbufPlain(4192UL);
        TBinMBufOutStream strmOut(&mbufPlain);

        if (bIsCQCPW)
        {
            mdigPW.StartNew();
            mdigPW.DigestStr(strNewPW);
            mdigPW.Complete(mhashPW);
            strmOut << mhashPW;
        }
         else
        {
            strmOut << strNewPW;
        }
        strmOut.Flush();

        // And encrypt it into a second buffer to send
        THeapBuf mbufCypher(4192UL);
        const tCIDLib::TCard4 c4EncBytes = crypPW.c4Encrypt
        (
            mbufPlain, mbufCypher, strmOut.c4CurSize()
        );

        // And finally, call the security server to change the PW
        orbcSS->ChangePassword
        (
            strUserName, bIsCQCPW, sectTmp, c4EncBytes, mbufCypher
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        //
        //  Watch for a particular error, which is that they tried to
        //  set the web password, but their account is not enabled for
        //  that. Display it just as a popup, not an error.
        //
        if (errToCatch.bCheckEvent( facCQCKit().strName()
                                    , kKitErrs::errcSec_NotWebEnabledAcc))
        {
            TOkBox msgbFailed(errToCatch.strErrText());
            msgbFailed.ShowIt(wndOwner);
        }
         else
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgFailed
            (
                wndOwner
                , wndOwner.strWndText()
                , facCQCGKit().strMsg(kGKitErrs::errcLogon_PWChangeFailed)
                , errToCatch
            );
        }
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


// Delete a local object store object if it exists
tCIDLib::TBoolean TFacCQCGKit::bDeleteStoreObj(const TString& strKey)
{
    // If in no local store mode, do nothing. Return false to make the caler happy
    if (m_bNoLocalStore)
        return kCIDLib::False;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    return m_poseConfig->bDeleteObjectIfExists(strKey);
}


// Wrappers around the expression editing dialogs
tCIDLib::TBoolean
TFacCQCGKit::bEditExpression(const  TWindow&            wndOwner
                            , const TString&            strTitle
                            ,       TString&            strDevice
                            ,       TString&            strField
                            ,       TCQCFldDef&         flddSelected
                            ,       TCQCExpression&     exprEdit
                            , const tCIDLib::TBoolean   bChangeDescr)
{
    TCQCEdExprDlg dlgEdit;
    return dlgEdit.bRunDlg
    (
        wndOwner, strTitle, strDevice, strField, flddSelected, exprEdit, bChangeDescr
    );
}

tCIDLib::TBoolean
TFacCQCGKit::bEditExpression2(  const   TWindow&            wndOwner
                                , const TString&            strTitle
                                , const tCQCKit::EFldTypes  eType
                                , const TString&            strLimits
                                ,       TCQCExpression&     exprEdit
                                , const tCIDLib::TBoolean   bChangeDescr)
{
    TCQCEdExprDlg2 dlgEdit;
    return dlgEdit.bRunDlg(wndOwner, strTitle, eType, strLimits, exprEdit, bChangeDescr);
}


// Indicate whether the object store has been initialized
tCIDLib::TBoolean TFacCQCGKit::bObjStoreAvail() const
{
    return (m_poseConfig != nullptr);
}


//
//  A wrapper around the logon dialog, so that we can avoid exposing its
//  header.
//
tCIDLib::TBoolean
TFacCQCGKit::bLogon(const   TWindow&            wndOwner
                    ,       TCQCSecToken&       sectToFill
                    ,       TCQCUserAccount&    uaccToFill
                    , const tCQCKit::EUserRoles eMinRole
                    , const TString&            strAppTitle
                    , const tCIDLib::TBoolean   bNoEnvLogon
                    , const TString&            strIconName)
{
    TCQCLogonDlg dlgLogon;
    return dlgLogon.bRunDlg
    (
        wndOwner
        , sectToFill
        , uaccToFill
        , eMinRole
        , strAppTitle
        , bNoEnvLogon
        , strIconName
    );
    return kCIDLib::True;
}


//
//  A wrapper around the limit value selection dialog, to avoid exposing the header. It
//  just passes the type and limits from the incoming field def. One takes a previous value
//  and ones doesn't.
//
tCIDLib::TBoolean
TFacCQCGKit::bQueryFldValue(const   TWindow&    wndOwner
                            , const TCQCFldDef& flddTarget
                            , const TString&    strMoniker
                            , const TPoint&     pntOrg
                            ,       TString&    strValue)
{
    TString strTitle
    (
        kGKitMsgs::midDlg_SelFldVal_Title, *this, strMoniker, flddTarget.strName()
    );

    TSelLimitValDlg dlgQVal;
    return dlgQVal.bRunDlg
    (
        wndOwner
        , strTitle
        , flddTarget.eType()
        , flddTarget.strLimits()
        , pntOrg
        , strValue
   );
}

tCIDLib::TBoolean
TFacCQCGKit::bQueryFldValue(const   TWindow&    wndOwner
                            , const TCQCFldDef& flddTarget
                            , const TString&    strMoniker
                            , const TPoint&     pntOrg
                            ,       TString&    strValue
                            , const TString&    strOrgVal)
{
    TString strTitle
    (
        kGKitMsgs::midDlg_SelFldVal_Title, *this, strMoniker, flddTarget.strName()
    );

    TSelLimitValDlg dlgQVal;
    return dlgQVal.bRunDlg
    (
        wndOwner
        , strTitle
        , flddTarget.eType()
        , flddTarget.strLimits()
        , pntOrg
        , strValue
        , strOrgVal
   );
}


//
//  A wrapper around the limit value selection dialog, to avoid exposing the header. This
//  one is for getting non-field values (though it could be used for fields as well, though
//  the above version is more likely.) One takes a previous value and the other doesn't.
//
tCIDLib::TBoolean
TFacCQCGKit::bQueryLimitValue(  const   TWindow&            wndOwner
                                , const TString&            strTitle
                                , const tCQCKit::EFldTypes  eType
                                , const TString&            strLimits
                                , const TPoint&             pntOrg
                                ,       TString&            strValue)
{
    TSelLimitValDlg dlgQVal;
    return dlgQVal.bRunDlg
    (
        wndOwner
        , strTitle
        , eType
        , strLimits
        , pntOrg
        , strValue
   );
}

tCIDLib::TBoolean
TFacCQCGKit::bQueryLimitValue(  const   TWindow&            wndOwner
                                , const TString&            strTitle
                                , const tCQCKit::EFldTypes  eType
                                , const TString&            strLimits
                                , const TPoint&             pntOrg
                                ,       TString&            strValue
                                , const TString&            strOrgVal)
{
    TSelLimitValDlg dlgQVal;
    return dlgQVal.bRunDlg
    (
        wndOwner
        , strTitle
        , eType
        , strLimits
        , pntOrg
        , strValue
        , strOrgVal
   );
}


// A wrapper around the version entry dailog, to avoid exposing its header
tCIDLib::TBoolean
TFacCQCGKit::bQueryVersion( const   TWindow&            wndOwner
                            , const TString&            strTitle
                            ,       tCIDLib::TCard8&    c8ToFill)
{
    TGetVersionDlg dlgGetVer;
    return dlgGetVer.bRun(wndOwner, strTitle, c8ToFill);
}


//
//  A helper to read in and verify a stored frame window state object, which are pretty
//  commonly stored so having a helper to do it is a big time saver. The caller gives us the
//  key to read, the frame state to fill, and a minimum size the target window can use.
//  Just in case the font changed or something, this let's use not return them an area that
//  is too small.
//
//  If none is available, then we can optionally set a default to let the system size and
//  position the frame, else we'll return false and not fill in the frame state.
//
//  We also make sure that it is fully contained within the virtual screen area. If not,
//  we'll force it back in so that it is (without violating its minimum size limits.)
//
tCIDLib::TBoolean
TFacCQCGKit::bReadFrameState(const  TString&            strKey
                            ,       TCQCFrameState&     fstToFill
                            , const TSize&              szMin
                            , const tCIDLib::TBoolean   bProvideDef)
{
    //
    //  If there's no local object store available, or we are in no local store state
    //  then just say no.
    //
    if (m_bNoLocalStore || (m_poseConfig == nullptr))
        return kCIDLib::False;

    // Lock and try to read it. If not found, then return false now
    try
    {
        TLocker lockrStore(&m_mtxStore);
        tCIDLib::TCard4 c4Version = 0;
        if (!m_poseConfig->bReadObject(strKey, c4Version, fstToFill))
        {
            if (bProvideDef)
            {
                fstToFill.SetArea(facCIDCtrls().areaDefWnd(), tCIDCtrls::EPosStates::Restored);
                return kCIDLib::False;
            }
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        fstToFill.SetArea(facCIDCtrls().areaDefWnd(), tCIDCtrls::EPosStates::Restored);
        return kCIDLib::False;
    }

    // Make sure it's at least the min size
    TArea areaAdjust = fstToFill.areaFrame();
    areaAdjust.TakeLarger(szMin);
    fstToFill.areaFrame(areaAdjust);

    //  Keep us within the available virtual screen area
    if (!TGUIFacility::areaVirtualScreen().bContainsArea(areaAdjust))
    {
        areaAdjust.ForceWithin(TGUIFacility::areaVirtualScreen(), kCIDLib::True);

        // Again make sure we don't violate the minimize size
        areaAdjust.TakeLarger(szMin);
        fstToFill.areaFrame(areaAdjust);
    }

    return kCIDLib::True;
}


// Read an object from the local obj store if it exists
tCIDLib::TBoolean
TFacCQCGKit::bReadStoreObj( const   TString&            strKey
                            ,       tCIDLib::TCard4&    c4Version
                            ,       MStreamable&        strmblToFill)
{
    // If in no local store mode, do nothing. Return false to say we didn't find it
    if (m_bNoLocalStore)
        return kCIDLib::False;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    return m_poseConfig->bReadObject(strKey, c4Version, strmblToFill);
}


// Indicates whether the indicated key exists in the local object store
tCIDLib::TBoolean TFacCQCGKit::bStoreKeyExists(const TString& strKey) const
{
    // If in no local store mode, return false to say we didn't find it
    if (m_bNoLocalStore)
        return kCIDLib::False;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    return m_poseConfig->bKeyExists(strKey);
}


//
//  Load up a logo image. The internal image content is reference counted so
//  this is not as heavy as it seems to return them by value.
//
TBitmap
TFacCQCGKit::bmpLoadLogo(const  TString&            strName
                        , const tCIDLib::TBoolean   bCacheIt)
{
    // And the cache collection
    static TKeyedHashSet<TBmpCacheItem, TString, TStringKeyOps> colCache
    (
        29, TStringKeyOps(), &TBmpCacheItem::strKey, tCIDLib::EMTStates::Safe
    );

    // See if it's in our cache first. If so, return that
    TBmpCacheItem* pitemCache = colCache.pobjFindByKey(strName);
    if (pitemCache)
        return pitemCache->m_bmpItem;

    // It's not so build the actual name of the file to load
    TPathStr pathActName(strPath());
    pathActName.AddLevel(L"LogoImages");
    pathActName.AddLevel(L"Logo_");
    pathActName.Append(strName);
    pathActName.AppendExt(L"png");

    TPNGImage imgNew;
    try
    {
        TBinFileInStream strmImg
        (
            pathActName
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        strmImg >> imgNew;
    }

    catch(...)
    {
        // It failed, so return a default bitmap
        return TBitmap();
    }

    // Create the bitmap now
    TGraphDesktopDev gdevTmp;
    TBitmap bmpRet(gdevTmp, imgNew);

    // Create the bitmap and put it into our cache, unless asked not to
    if (bCacheIt)
    {
        TBmpCacheItem itemNew(strName, bmpRet);
        colCache.objAdd(itemNew);
    }

    // And return the new bitmap
    return bmpRet;
}


// Ask now many objects are in a given scope of the local store
tCIDLib::TCard4
TFacCQCGKit::c4QueryObjsInScope(const   TString&                strScope
                                ,       TCollection<TString>&   colToFill)
{
    // If in no local store mode, return zero count
    if (m_bNoLocalStore)
        return 0;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    return m_poseConfig->c4QueryObjectsInScope(strScope, colToFill);
}


// Update an object in the local store. It must exist already
tCIDLib::TCard4
TFacCQCGKit::c4UpdateStoreObj(  const   TString&        strKey
                                , const MStreamable&    strmblToWrite)
{
    // If in no local store mode, do nothing
    if (m_bNoLocalStore)
        return 0;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    return m_poseConfig->c4UpdateObject(strKey, strmblToWrite);
}


//
//  Just a wrapper around the change password dialog, so that we can avoid
//  exposing the header.
//
tCIDLib::TVoid
TFacCQCGKit::ChangePassword(const   TWindow&        wndOwner
                            , const TString&        strUserName
                            , const TCQCSecToken&   sectUser)
{
    TCQCChangePWDlg dlgChangePW;
    dlgChangePW.Run(wndOwner, strUserName);
}


// Delete an object or an entire scope from the local object store
tCIDLib::TVoid TFacCQCGKit::DeleteStoreObj(const TString& strKey)
{
    // If in no local store mode, do nothing
    if (m_bNoLocalStore)
        return;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    m_poseConfig->DeleteObject(strKey);
}

tCIDLib::TVoid TFacCQCGKit::DeleteStoreScope(const TString& strScopeName)
{
    // If in no local store mode, do nothing
    if (m_bNoLocalStore)
        return;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    m_poseConfig->DeleteScope(strScopeName);
}


//
//  Read an object from the local object store, return the version info, and
//  optionally throwing if not found. Otherwise it returns a status that
//  indicates whether it was present or not. The version is in and out, so
//  if the incoming version is the same, it indicates no change and returns
//  nothing.
//
tCIDLib::ELoadRes
TFacCQCGKit::eReadStoreObj( const   TString&            strKey
                            ,       tCIDLib::TCard4&    c4Version
                            ,       MStreamable&        strmblToFill
                            , const tCIDLib::TBoolean   bThrowIfNot)
{
    // If in no local store mode, return not fund
    if (m_bNoLocalStore)
        return tCIDLib::ELoadRes::NotFound;

    CIDAssert(m_poseConfig != nullptr, L"The local object store was not initialized");
    TLocker lockrStore(&m_mtxStore);
    return m_poseConfig->eReadObject(strKey, c4Version, strmblToFill, bThrowIfNot);
}


//
//  Any GUI programs shuold gall this once the user has logged on. This
//  will allow for local storage of config data.
//
tCIDLib::TVoid
TFacCQCGKit::InitObjectStore(const  TString&            strAppKey
                            , const TString&            strUserName
                            , const tCIDLib::TBoolean   bReset)
{
    if (!m_poseConfig)
    {
        TLocker lockrStore(&m_mtxStore);
        if (!m_poseConfig)
        {
            // Build up the path to our object store
            TPathStr pathStore = facCQCKit().strUsersDir();
            pathStore.AddLevel(strUserName);
            pathStore.AddLevel(L"AppConfig");
            pathStore.AddLevel(strAppKey);

            // If the path doesn't exist, then create it
            if (!TFileSys::bExists(pathStore))
                TFileSys::MakePath(pathStore);

            // Create the object store object
            m_poseConfig = new TCIDObjStore;

            // Build the path to the actual file, which is used a couple times
            TPathStr pathFullPath(pathStore);
            pathFullPath.AddLevel(L"ConfigData.CIDObjStore");

            //
            //  If asked to reset, delete the file if it exists,
            //  which will make us create a new one.
            //
            if (bReset && TFileSys::bExists(pathFullPath))
            {
                try
                {
                    TFileSys::DeleteFile(pathFullPath);
                }

                catch(TError& errToCatch)
                {
                    if (facCQCGKit().bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kGKitErrs::errcState_CantReset
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathFullPath
                    );
                }
            }

            //
            //  And initialize it. The return value isn't success/failure,
            //  but whether it created a new store or opened an old one.
            //
            try
            {
                m_poseConfig->bInitialize(pathStore, L"ConfigData");
            }

            catch(const TError& errToCatch)
            {
                if (bShouldLog(errToCatch))
                    LogEventObj(errToCatch);

                ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGKitErrs::errcState_StoreInitFailed
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Init
                    , pathFullPath
                );
            }
        }
    }
}


//
//  Apps can invoke apps and track them. If they cycle, they want to reattach
//  to any of them that are left. The can store and later reload their
//  attach info using this and StoreAppAttachInfo. We use the same object
//  store as we manage for them to load/store other app specific config.
//
tCIDLib::TVoid TFacCQCGKit::LoadAppAttachInfo()
{
    // If in no local store mode, do nothing
    if (m_bNoLocalStore)
        return;

    try
    {
        if (m_poseConfig->bKeyExists(CQCGKit_ThisFacility::pszOSKey_AppAttach))
        {
            // We have some stored, so load it
            tCIDLib::TCard4 c4Ver = 0;
            TString strData;
            m_poseConfig->bReadObject
            (
                CQCGKit_ThisFacility::pszOSKey_AppAttach, c4Ver, strData
            );

            // Call CQCKit to set the list
            facCQCKit().SetAppAttachInfo(strData);
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }
}


//
//  A wrapper around the field write dialog, to avoid exposing the header. In this case,
//  it's in the mode that set the field to whatever value the user enters.
//
tCIDLib::TVoid
TFacCQCGKit::SetFldValue(const  TWindow&        wndOwner
                        , const TCQCFldDef&     flddTarget
                        , const TString&        strMoniker
                        , const TPoint&         pntOrg
                        , const TCQCSecToken&   sectUser)
{
    TString strTitle
    (
        kGKitMsgs::midDlg_SetFldVal_Title, *this, strMoniker, flddTarget.strName()
    );

    TWrtFldDlg  dlgSetVal(sectUser);
    TString     strNewVal;
    dlgSetVal.bRunDlg(wndOwner, strTitle, flddTarget, strMoniker, pntOrg, strNewVal);
}

tCIDLib::TVoid
TFacCQCGKit::SetFldValue(const  TWindow&        wndOwner
                        , const TCQCFldDef&     flddTarget
                        , const TString&        strMoniker
                        , const TPoint&         pntOrg
                        , const TString&        strOrgVal
                        , const TCQCSecToken&   sectUser)
{
    TString strTitle
    (
        kGKitMsgs::midDlg_SetFldVal_Title, *this, strMoniker, flddTarget.strName()
    );

    TWrtFldDlg  dlgSetVal(sectUser);
    TString     strNewVal;
    dlgSetVal.bRunDlg(wndOwner, strTitle, flddTarget, strMoniker, pntOrg, strNewVal, strOrgVal);
}


//
//  Set a mode flag that causes us to just return 'not found' results or
//  take no action on any local object store related calls, whereas normally
//  we'd throw if the object store is not initialized.
//
tCIDLib::TVoid TFacCQCGKit::SetNoLocalStoreMode()
{
    m_bNoLocalStore = kCIDLib::True;
}


//
//  A wrapper around the driver info dialog.
//
tCIDLib::TVoid
TFacCQCGKit::ShowDriverInfo(const TWindow& wndOwner, const TString& strMoniker, const TCQCSecToken& sectUser)
{
    TDrvInfoDlg dlgInfo(sectUser);
    dlgInfo.RunDlg(wndOwner, strMoniker);
}


//
//  Some convenience methods for showing errors, to keep it all in one place and
//  allow for checking for special cases and such. Where possible, everyone should
//  use these.
//
tCIDLib::TVoid
TFacCQCGKit::ShowError( const   TWindow&    wndOwner
                        , const TString&    strMsg
                        , const TError&     errToShow)
{
    ShowError(wndOwner, wndOwner.strWndText(), strMsg, errToShow);
}

tCIDLib::TVoid
TFacCQCGKit::ShowError( const   TWindow&    wndOwner
                        , const TString&    strTitle
                        , const TString&    strMsg
                        , const TError&     errToShow)
{
    if (errToShow.bCheckEvent(  facCQCKit().strName()
                                , kKitErrs::errcFld_ValueRejected))
    {
        TErrBox msgbErr(strTitle, errToShow.strErrText());
        msgbErr.ShowIt(wndOwner);
    }
     else if (errToShow.bCheckEvent(facCQCKit().strName()
                                    , kKitErrs::errcDrv_NotFound))
    {
        TErrBox msgbErr(strTitle, errToShow.strErrText());
        msgbErr.ShowIt(wndOwner);
    }
     else
    {
        TExceptDlg dlgbFail(wndOwner, strTitle, strMsg, errToShow);
    }
}


tCIDLib::TVoid
TFacCQCGKit::ShowError( const   TWindow&    wndOwner
                        , const TString&    strMsg)
{
    TErrBox msgbErr(wndOwner.strWndText(), strMsg);
    msgbErr.ShowIt(wndOwner);
}

tCIDLib::TVoid
TFacCQCGKit::ShowError( const   TWindow&    wndOwner
                        , const TString&    strTitle
                        , const TString&    strMsg)
{
    TErrBox msgbErr(strTitle, strMsg);
    msgbErr.ShowIt(wndOwner);
}


//
//  Stores away in the local object store info about any opened apps that
//  the containing app has invoked via our app invocation/tracking stuff.
//
tCIDLib::TVoid TFacCQCGKit::StoreAppAttachInfo()
{
    // If in no local store mode, do nothing
    if (m_bNoLocalStore)
        return;

    try
    {
        // As the CQCKit facility for the app attach info he has
        TString strOutData(0x10000UL);
        facCQCKit().QueryAppAttachInfo(strOutData);

        // And store it
        tCIDLib::TCard4 c4Ver = 0;
        m_poseConfig->bAddOrUpdate
        (
            CQCGKit_ThisFacility::pszOSKey_AppAttach
            , c4Ver
            , strOutData
            , 1024
        );
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


//
//  A helper that various GUI programs can use to store their frame state in the local,
//  user/application specific data store, in the standard format. We also provide a
//  bReadFrameState() to get this back.
//
//  We will only store the area if the window is not minimized, or maximized, i.e. we are
//  storing the restored state only. We always store the pos state.
//
tCIDLib::TVoid
TFacCQCGKit::StoreFrameState(const TFrameWnd& wndToStore, const TString& strKey)
{
    // Make sure it exists
    if (!wndToStore.bIsValid())
        return;

    // Create a frame state object and let it set itself up from the source window
    TCQCFrameState fstStore;
    fstStore.SetFrom(wndToStore);

    // And store it out
    try
    {
        tCIDLib::TCard4 c4Version = 0;
        facCQCGKit().bAddUpdateStoreObj(strKey, c4Version, fstStore, 64);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


//
//  The client app should call this late in the clean up process, if it
//  initialized the object store above.
//
tCIDLib::TVoid TFacCQCGKit::TermObjectStore()
{
    TLocker lockrStore(&m_mtxStore);
    if (m_poseConfig)
    {
        m_poseConfig->Close();
        delete m_poseConfig;
        m_poseConfig = nullptr;
    }
}


