//
// FILE NAME: CQCGKit_ThisFacility.hpp
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
//  This file defines facility class for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCIDObjStore;

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCGKit
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCGKITEXPORT TFacCQCGKit : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCGKit();

        TFacCQCGKit(const TFacCQCGKit&) = delete;
        TFacCQCGKit(TFacCQCGKit&&) = delete;

        ~TFacCQCGKit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCGKit& operator=(const TFacCQCGKit&) = delete;
        TFacCQCGKit& operator=(TFacCQCGKit&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddStoreObj
        (
            const   TString&                strKey
            , const MStreamable&            strmblToWrite
            , const tCIDLib::TCard4         c4Reserve = 0
        );

        tCIDLib::TBoolean bAddUpdateStoreObj
        (
            const   TString&                strKey
            ,       tCIDLib::TCard4&        c4Version
            , const MStreamable&            strmblToWrite
            , const tCIDLib::TCard4         c4Reserve = 0
        );

        tCIDLib::TBoolean bChangePassword
        (
            const   TWindow&                wndOwner
            , const TString&                strUserName
            , const TString&                strOldPW
            , const TString&                strNewPW
            , const tCIDLib::TBoolean       bIsCQCPW
        );

        tCIDLib::TBoolean bDeleteStoreObj
        (
            const   TString&                strKey
        );

        tCIDLib::TBoolean bEditExpression
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            ,       TString&                strDevice
            ,       TString&                strField
            ,       TCQCFldDef&             fldSelected
            ,       TCQCExpression&         exprEdit
            , const tCIDLib::TBoolean       bChangeDescr = kCIDLib::True
        );

        tCIDLib::TBoolean bEditExpression2
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
            ,       TCQCExpression&         exprEdit
            , const tCIDLib::TBoolean       bChangeDescr = kCIDLib::True
        );

        tCIDLib::TBoolean bLogon
        (
            const   TWindow&                wndOwner
            ,       TCQCSecToken&           sectToFill
            ,       TCQCUserAccount&        uaccToFill
            , const tCQCKit::EUserRoles     eMinRole
            , const TString&                strAppTitle
            , const tCIDLib::TBoolean       bNoEnvLogon
            , const TString&                strIconName
        );

        tCIDLib::TBoolean bObjStoreAvail() const;

        tCIDLib::TBoolean bQueryFldValue
        (
            const   TWindow&                wndOwner
            , const TCQCFldDef&             flddTarget
            , const TString&                strMoniker
            , const TPoint&                 pntOrg
            ,       TString&                strValue
        );

        tCIDLib::TBoolean bQueryFldValue
        (
            const   TWindow&                wndOwner
            , const TCQCFldDef&             flddTarget
            , const TString&                strMoniker
            , const TPoint&                 pntOrg
            ,       TString&                strValue
            , const TString&                strOrgVal
        );

        tCIDLib::TBoolean bQueryLimitValue
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
            , const TPoint&                 pntOrg
            ,       TString&                strValue
        );

        tCIDLib::TBoolean bQueryLimitValue
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
            , const TPoint&                 pntOrg
            ,       TString&                strValue
            , const TString&                strOrgVal
        );

        tCIDLib::TBoolean bQueryVersion
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            ,       tCIDLib::TCard8&        c8ToFill
        );

        tCIDLib::TBoolean bReadFrameState
        (
            const   TString&                strKey
            ,       TCQCFrameState&         fstToFill
            , const TSize&                  szMin
            , const tCIDLib::TBoolean       bProvideDef = kCIDLib::True
        );

        tCIDLib::TBoolean bReadStoreObj
        (
            const   TString&                strKey
            ,       tCIDLib::TCard4&        c4Version
            ,       MStreamable&            strmblToFill
        );

        tCIDLib::TBoolean bStoreKeyExists
        (
            const   TString&                strKey
        )   const;

        TBitmap bmpLoadLogo
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bCacheIt = kCIDLib::True
        );

        tCIDLib::TCard4 c4QueryObjsInScope
        (
            const   TString&                strScope
            ,       tCIDLib::TStrCollect&   colToFill
        );

        tCIDLib::TCard4 c4UpdateStoreObj
        (
            const   TString&                strKey
            , const MStreamable&            strmblToWrite
        );

        tCIDLib::TVoid ChangePassword
        (
            const   TWindow&                wndOwner
            , const TString&                strUserName
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid DeleteStoreObj
        (
            const   TString&                strKey
        );

        tCIDLib::TVoid DeleteStoreScope
        (
            const   TString&                strScopeName
        );

        tCIDLib::ELoadRes eReadStoreObj
        (
            const   TString&                strKey
            ,       tCIDLib::TCard4&        c4Version
            ,       MStreamable&            strmblToFill
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        tCIDLib::TVoid InitObjectStore
        (
            const   TString&                strAppKey
            , const TString&                strUserName
            , const tCIDLib::TBoolean       bReset = kCIDLib::False
        );

        tCIDLib::TVoid LoadAppAttachInfo();

        tCIDLib::TVoid SetFldValue
        (
            const   TWindow&                wndOwner
            , const TCQCFldDef&             flddTarget
            , const TString&                strMoniker
            , const TPoint&                 pntOrg
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetFldValue
        (
            const   TWindow&                wndOwner
            , const TCQCFldDef&             flddTarget
            , const TString&                strMoniker
            , const TPoint&                 pntOrg
            , const TString&                strOrgVal
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetNoLocalStoreMode();

        tCIDLib::TVoid ShowDriverInfo
        (
            const   TWindow&                wndOwner
            , const TString&                strMoniker
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid ShowError
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const TString&                strMsg
            , const TError&                 errToShow
        );

        tCIDLib::TVoid ShowError
        (
            const   TWindow&                wndOwner
            , const TString&                strMsg
            , const TError&                 errToShow
        );

        tCIDLib::TVoid ShowError
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const TString&                strMsg
        );

        tCIDLib::TVoid ShowError
        (
            const   TWindow&                wndOwner
            , const TString&                strMsg
        );

        tCIDLib::TVoid StoreAppAttachInfo();

        tCIDLib::TVoid StoreFrameState
        (
            const   TFrameWnd&              wndToStore
            , const TString&                strKey
        );

        tCIDLib::TVoid TermObjectStore();



    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNoLocalStore
        //      Some programs don't want to use the local object store, but
        //      we don't want to try to have to special case every call to
        //      helper code (such all the stuff in the CQCIGKit facility),
        //      to avoid having that code throw an exception when it tries to
        //      access per user/program stored state data.
        //
        //      So we allow this flag to be set and all of our calls that allow
        //      access to stuff in the object store will just return a not
        //      found result instead of throwing.
        //
        //  m_mtxSync
        //      There's a possibility that, even in GUI apps, multiple threads
        //      could try to use the local object store (m_poseConfig), so
        //      we need to sync access to it.
        //
        //  m_poseConfig
        //      Various client programs need to store local data, and there
        //      are various bits of helper code that they call that need to
        //      store info across invocations. So, we allow them to create
        //      a private object store. It's a pointer since we only create
        //      if they ask. Otherwise it's null and will cause an exception
        //      if they attempt to use it.
        //
        //      Any app that needs this functionality must call InitObjStore()
        //      which tells us where to put it and what to name it. They
        //      will give an app specific key to differentiate it from the
        //      stores of other programs. It's also CQC user account specific.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bNoLocalStore;
        TMutex              m_mtxStore;
        TCIDObjStore*       m_poseConfig;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCGKit,TGUIFacility)
};

#pragma CIDLIB_POPPACK

