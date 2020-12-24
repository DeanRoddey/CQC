//
// FILE NAME: CQCIGKit_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/18/2004
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


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIGKit
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCIGKITEXPORT TFacCQCIGKit : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCIGKit();

        TFacCQCIGKit(const TFacCQCIGKit&) = delete;
        TFacCQCIGKit(TFacCQCIGKit&&) = delete;

        ~TFacCQCIGKit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCIGKit& operator=(const TFacCQCIGKit&) = delete;
        TFacCQCIGKit& operator=(TFacCQCIGKit&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual method
        // -------------------------------------------------------------------
        tCIDLib::TVoid AboutDlg
        (
                    TWindow&                wndOwner
        );

        tCIDLib::TVoid AddFileToPack
        (
                    TDataSrvClient&         dsclToUse
            ,       TCQCPackage&            packTar
            , const tCQCKit::EPackFlTypes   eType
            , const TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eDType
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bEncrypt = kCIDLib::False
        );

        tCIDLib::TBoolean bCfgRemotePort
        (
            const   TWindow&                wndOwner
            ,       tCIDLib::TBoolean&      bEnable
            ,       TIPEndPoint&            ipepInOut
        );

        tCIDLib::TBoolean bCheckCQCVersion
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
        );

        tCIDLib::TBoolean bEditAction
        (
            const   TWindow&                wndOwner
            ,       MCQCCmdSrcIntf&         mcsrcEdit
            , const tCIDLib::TBoolean       bRequired
            , const TCQCUserCtx&            cuctxToUse
            ,       tCQCKit::TCmdTarList* const pcolExtraTars
            , const TString* const          pstrBasePath = nullptr
        );

        tCIDLib::TBoolean bEditKeyMap
        (
            const   TWindow&                wndOwner
            , const TString&                strAccount
            , const tCQCIGKit::EKMActModes  eMode
            , const TCQCUserCtx&            cuctxToUse
            ,       TCQCKeyMap&             kmToEdit
        );

        tCIDLib::TBoolean bEditTimeTokens
        (
            const   TWindow&                wndOwner
            ,       TString&                strToEdit
        );

        tCIDLib::TBoolean bGetLocInfo
        (
            const   TWindow&                wndOwner
            ,       tCIDLib::TFloat8&       f8Lat
            ,       tCIDLib::TFloat8&       f8Long
            , const tCIDLib::TBoolean       bForce
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bImportPackage
        (
            const   TWindow&                wndOwner
            , const TString&                strPackFile
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TBoolean bInvokeAction
        (
            const   TWindow&                wndOwner
            ,       TWindow&                wndModal
            , const MCQCCmdSrcIntf&         csrcToDo
            , const TString&                strEventId
            , const TString&                strTitle
            , const TCQCUserCtx&            cuctxToUse
            ,       MCQCCmdTracer* const    pcmdtDebug = nullptr
        );

        tCIDLib::TBoolean bInvokeKeyMap
        (
            const   tCIDCtrls::EExtKeys     eExtKey
            , const TWindow&                wndOwner
            ,       TWindow&                wndModal
            , const TString&                strTitle
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
            , const TCQCUserCtx&            cuctxToUse
            ,       MCQCCmdTracer* const    pcmdtDebug = nullptr
        );

        tCIDLib::TBoolean bInvokeKeyMap
        (
            const   tCIDCtrls::EExtKeys     eExtKey
            , const TWindow&                wndOwner
            ,       TWindow&                wndModal
            , const TCQCKeyMap&             kmData
            , const TString&                strTitle
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
            , const TCQCUserCtx&            cuctxToUse
            ,       MCQCCmdTracer* const    pcmdtDebug = nullptr
        );

        tCIDLib::TBoolean bQueryKeyMapData
        (
            const   TWindow&                wndOwner
            ,       TCQCKeyMap&             kmToFill
            , const TCQCUserCtx&            cuctxToUse
            , const TString&                strUserName
        )   const;

        tCIDLib::TBoolean bManagePackage
        (
            const   TWindow&                wndParent
            ,       TCQCPackage&            packTar
            , const TString&                strInitPath
            , const tCIDLib::TBoolean       bEncrypt
            , const tCQCRemBrws::EDTypes    eDType
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TBoolean bManageVars
        (
            const   TWindow&                wndOwner
            , const TString&                strUserName
            ,       TString&                strSelected
            ,       tCIDLib::TBoolean&      bByValue
            , const tCIDLib::TBoolean       bByNameOnly
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TBoolean bSelectDevSub
        (
            const   TWindow&                wndParent
            ,       TString&                strDevToFill
            ,       TString&                strSubToFill
            , const tCQCKit::EDevClasses    eDevClass
            , const TString&                strSuffix
            , const TCQCFldCache&           cfcSrc
        );

        tCIDLib::TBoolean bSelectDriver
        (
            const   TWindow&                wndOwner
            ,       TString&                strDriver
            , const tCQCKit::EDevCats       eDevCatFilt = tCQCKit::EDevCats::Count
            , const TString&                strMakeFilt = TString::strEmpty()
            , const TString&                strModelFilt = TString::strEmpty()
        );

        tCIDLib::TBoolean bSelectDriver
        (
            const   TWindow&                wndOwner
            , const TCQCFldCache&           cfcFldInfo
            ,       TString&                strDriver
        );

        tCIDLib::TBoolean bSelectField
        (
            const   TWindow&                wndOwner
            ,       TCQCFldCache&           cfcFldInfo
            ,       TString&                strDriver
            ,       TString&                strField
        );

        tCIDLib::TBoolean bSelectField
        (
            const   TWindow&                wndOwner
            ,       TCQCFldCache&           cfcFldInfo
            ,       TString&                strDriver
            ,       TString&                strField
            ,       tCIDLib::TBoolean&      bByValue
            , const tCIDLib::TBoolean       bNameOnly
        );

        tCIDLib::TBoolean bSelectField
        (
            const   TWindow&                wndOwner
            ,       TCQCFldFilter* const    pffiltToAdopt
            ,       TString&                strDriver
            ,       TString&                strField
        );

        tCIDLib::TBoolean bSelectField
        (
            const   TWindow&                wndOwner
            ,       TCQCFldFilter* const    pffiltToAdopt
            ,       TString&                strDriver
            ,       TString&                strField
            ,       tCIDLib::TBoolean&      bByValue
            , const tCIDLib::TBoolean       bNameOnly
        );

        tCIDLib::TBoolean bSelectField
        (
            const   TWindow&                wndOwner
            ,       TCQCFldFilter* const    pffiltToAdopt
            ,       TString&                strField
        );

        tCIDLib::TBoolean bSelectField
        (
            const   TWindow&                wndOwner
            ,       TCQCFldFilter* const    pffiltToAdopt
            ,       TString&                strField
            ,       tCIDLib::TBoolean&      bByValue
            , const tCIDLib::TBoolean       bNameOnly
        );

        tCIDLib::TBoolean bSelectIRCmd
        (
            const   TWindow&                wndOwner
            , const TString&                strDriver
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bWaitForCQC
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const TString&                strBinding
            , const tCIDLib::TCard4         c4WaitFor
        );

        TCQCDriverClient* pwndLoadNewDriver
        (
                    TWindow&                wndParent
            , const TArea&                  areaInit
            , const tCIDCtrls::TWndId       widToUse
            , const TCQCDriverObjCfg&       cqcdcInfo
            , const TCQCUserCtx&            cuctxToUse
            , const tCIDLib::TBoolean       bTestMode
        );

        const TString& strTopOfCheatSheet() const;

        tCIDLib::TVoid SetKeyMapData
        (
            const   TWindow&                wndOwner
            , const TCQCKeyMap&             kmToSet
            , const TCQCUserCtx&            cuctxToUse
            , const TString&                strUserName
        );

        tCIDLib::TVoid StoreVarCheatSheet
        (
            const   TWindow&                wndOwner
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TVoid UploadImage
        (
                    TDataSrvClient&         dsclUp
            ,       TPNGImage&              imgToUpload
            , const TString&                strTargetFile
            ,       TBinMBufOutStream&      strmImg
            ,       TBinMBufOutStream&      strmThumb
            , const tCIDLib::TBoolean       bNoDataCache
            , const tCIDLib::TKVPFList&     colXMeta
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TVoid UploadImage
        (
                    TDataSrvClient&         dsclUp
            , const TString&                strSrcFile
            , const TPoint&                 pntTransAt
            , const tCIDLib::TBoolean       bTrans
            , const TString&                strTargetFile
            ,       TBinMBufOutStream&      strmImg
            ,       TBinMBufOutStream&      strmThumb
            , const tCIDLib::TBoolean       bNoDataCache
            , const tCIDLib::TKVPFList&     colXMeta
            , const TCQCUserCtx&            cuctxToUse
        );

    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLoadKeyMapData
        (
            const   TWindow&                wndOwner
            , const TCQCUserCtx&            cuctxToUse
        )   const;

        tCIDLib::TVoid LoadVarCheatSheet
        (
            const   TWindow&                wndOwner
            , const TString&                strUserName
            , const TCQCUserCtx&            cuctxToUse
        ) const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bVarsLoaded
        //      Used to lazily fault in the user's variables cheat sheet
        //      data only if requested.
        //
        //  m_c4KeyMapVer
        //      The version for the keymap data in the object store. We keep
        //      this in order to be a lot more efficient, since we can quickly
        //      check if changes have been made each time that the key map data
        //      is referenced.
        //
        //  m_cavlCheatSheet
        //      We download the caller's variable list cheat sheet. We only
        //      download it if some code asks to access it. We only store it
        //      back if asked to.
        //
        //      Note that the logged in user can never change while the
        //      program is running and editing only ever happens in interactive
        //      GUI programs. So it's safe to just keep it around, and that
        //      avoids a lot of overhead downloading it every time the user
        //      wants to select a variable.
        //
        //  m_ctarGlobals
        //      A global variables object that we pass in to all of the
        //      key mapped action invocations. Since they are always invoked
        //      via us, it's just easiest for us to provide this.
        //
        //  m_enctNextKMQ
        //      We will only ask for check for new key map data every so
        //      often, so each time we do a check, we set this for that
        //      far forward. If the current time isn't beyond this time,
        //      we don't check again, we just use what we have.
        //
        //  m_kmData
        //      The key map data that we get from the object store. We check
        //      upon each access to see if it's been changed, and reload it
        //      if needed.
        //
        //  m_strUser
        //      The logged in user whose info we have in the keymaps and cheatsheet
        //      and so forth.
        // -------------------------------------------------------------------
        mutable tCIDLib::TBoolean       m_bVarsLoaded;
        mutable tCIDLib::TCard4         m_c4KeyMapVer;
        tCIDLib::TCard4                 m_c4StateId;
        mutable TCQCActVarList          m_cavlCheatSheet;
        TStdVarsTar                     m_ctarGlobals;
        mutable tCIDLib::TEncodedTime   m_enctNextKMQ;
        mutable TCQCKeyMap              m_kmData;
        mutable TString                 m_strUser;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCIGKit,TGUIFacility)
};

#pragma CIDLIB_POPPACK


