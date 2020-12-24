//
// FILE NAME: CQCMedia_PLItemClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/15/2007
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
//  This is the header for the CQCMedia_PLItemClass.cpp, which implements
//  a CML level class that encapsulates a C++ class that represents a standard
//  media renderer playlist item (i.e. all the info about a media item that
//  a player caches away when it's queued up on the player for playback.
//
//  The primary purpose of this class is for CML based renderer drivers, so
//  that they can respond to the standard query for their playlist.
//
//  We also create a playlist manager class, which is a wrapper around the
//  C++ playlist manager. This allows for CML based renderer drivers without
//  having to expose a lot of interfaces that would otherwise be necessary
//  if they managed their own playlists.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCPLItemValue
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCPLItemValue : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPLItemValue
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
        );

        TCQCPLItemValue(const TCQCPLItemValue&) = delete;

        ~TCQCPLItemValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCPLItemValue& operator=(const TCQCPLItemValue&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDbgFormat
        (
                    TTextOutStream&         strmTarget
            , const TMEngClassInfo&
            , const tCIDMacroEng::EDbgFmts     eFormat
            , const tCIDLib::ERadices       eRadix
            , const TCIDMacroEngine&
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCMediaPLItem& mpliValue() const;

        TCQCMediaPLItem& mpliValue();

        const TCQCMediaPLItem& mpliValue
        (
            const   TCQCMediaPLItem&        mpliToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_mpliValue
        //      This is underlying C++ play list item object that we wrap.
        // -------------------------------------------------------------------
        TCQCMediaPLItem m_mpliValue;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPLItemValue,TMEngClassVal)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCPLItemInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCPLItemInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPLItemInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCQCPLItemInfo(const TCQCPLItemInfo&) = delete;

        ~TCQCPLItemInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCPLItemInfo& operator=(const TCQCPLItemInfo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Init
        (
                    TCIDMacroEngine&        meOwner
        )   override;

        TMEngClassVal* pmecvMakeStorage
        (
            const   TString&                strName
            ,       TCIDMacroEngine&        meOwner
            , const tCIDMacroEng::EConstTypes  eConst
        )   const override;



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInvokeMethod
        (
                    TCIDMacroEngine&        meOwner
            , const TMEngMethodInfo&        methiTarget
            ,       TMEngClassVal&          mecvInstance
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2EnumId_XXX
        //      We use some enumerated types in our interface, so we need
        //      to look up their ids.
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        //
        //  m_pmeciXXX
        //      We need to translate our enums between the CML and C++
        //      versions, so we need to look up and keep around their class
        //      info objects.
        // -------------------------------------------------------------------
        tCIDLib::TCard2     m_c2EnumId_LocTypes;
        tCIDLib::TCard2     m_c2EnumId_MediaTypes;
        tCIDLib::TCard2     m_c2MethId_DefCtor;

        tCIDLib::TCard2     m_c2MethId_FormatTotalTime;

        tCIDLib::TCard2     m_c2MethId_GetColArtist;
        tCIDLib::TCard2     m_c2MethId_GetColAspect;
        tCIDLib::TCard2     m_c2MethId_GetColCast;
        tCIDLib::TCard2     m_c2MethId_GetColCookie;
        tCIDLib::TCard2     m_c2MethId_GetColDescr;
        tCIDLib::TCard2     m_c2MethId_GetColLabel;
        tCIDLib::TCard2     m_c2MethId_GetColLeadActor;
        tCIDLib::TCard2     m_c2MethId_GetColName;
        tCIDLib::TCard2     m_c2MethId_GetColRating;
        tCIDLib::TCard2     m_c2MethId_GetColYear;

        tCIDLib::TCard2     m_c2MethId_GetItemArtist;
        tCIDLib::TCard2     m_c2MethId_GetItemCookie;
        tCIDLib::TCard2     m_c2MethId_GetItemName;
        tCIDLib::TCard2     m_c2MethId_GetItemSeconds;
        tCIDLib::TCard2     m_c2MethId_GetTitleCookie;
        tCIDLib::TCard2     m_c2MethId_GetTitleName;

        tCIDLib::TCard2     m_c2MethId_GetLocInfo;
        tCIDLib::TCard2     m_c2MethId_GetLocType;
        tCIDLib::TCard2     m_c2MethId_GetListItemId;
        tCIDLib::TCard2     m_c2MethId_GetMediaFormat;
        tCIDLib::TCard2     m_c2MethId_GetRepoMoniker;

        tCIDLib::TCard2     m_c2MethId_SetColArtist;
        tCIDLib::TCard2     m_c2MethId_SetColAspect;
        tCIDLib::TCard2     m_c2MethId_SetColCast;
        tCIDLib::TCard2     m_c2MethId_SetColCookie;
        tCIDLib::TCard2     m_c2MethId_SetColDescr;
        tCIDLib::TCard2     m_c2MethId_SetColLabel;
        tCIDLib::TCard2     m_c2MethId_SetColLeadActor;
        tCIDLib::TCard2     m_c2MethId_SetColName;
        tCIDLib::TCard2     m_c2MethId_SetColRating;
        tCIDLib::TCard2     m_c2MethId_SetColYear;

        tCIDLib::TCard2     m_c2MethId_SetItemArtist;
        tCIDLib::TCard2     m_c2MethId_SetItemCookie;
        tCIDLib::TCard2     m_c2MethId_SetItemName;
        tCIDLib::TCard2     m_c2MethId_SetItemSeconds;
        tCIDLib::TCard2     m_c2MethId_SetTitleCookie;
        tCIDLib::TCard2     m_c2MethId_SetTitleName;

        tCIDLib::TCard2     m_c2MethId_SetLocInfo;
        tCIDLib::TCard2     m_c2MethId_SetLocType;
        tCIDLib::TCard2     m_c2MethId_SetMediaFormat;
        tCIDLib::TCard2     m_c2MethId_SetRepoMoniker;

        TMEngEnumInfo*      m_pmeciLocTypes;
        TMEngEnumInfo*      m_pmeciMediaTypes;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPLItemInfo,TMEngClassInfo)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCPLMgrValue
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCPLMgrValue : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPLMgrValue
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
        );

        TCQCPLMgrValue(const TCQCPLMgrValue&) = delete;

        ~TCQCPLMgrValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCPLMgrValue& operator=(const TCQCPLMgrValue&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCMediaPLMgr& mplmValue() const;

        TCQCMediaPLMgr& mplmValue();

        const TCQCMediaPLMgr& mplmValue
        (
            const   TCQCMediaPLMgr&         mplmToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_mplmValue
        //      This is underlying C++ play list manager object that we wrap.
        // -------------------------------------------------------------------
        TCQCMediaPLMgr m_mplmValue;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPLMgrValue,TMEngClassVal)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCPLMgrInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCPLMgrInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPLMgrInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCQCPLMgrInfo(const TCQCPLMgrInfo&) = delete;

        ~TCQCPLMgrInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCPLMgrInfo& operator=(const TCQCPLMgrInfo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Init
        (
                    TCIDMacroEngine&        meOwner
        )   override;

        TMEngClassVal* pmecvMakeStorage
        (
            const   TString&                strName
            ,       TCIDMacroEngine&        meOwner
            , const tCIDMacroEng::EConstTypes  eConst
        )   const override;



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInvokeMethod
        (
                    TCIDMacroEngine&        meOwner
            , const TMEngMethodInfo&        methiTarget
            ,       TMEngClassVal&          mecvInstance
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddStrLiteral
        (
            const   TString&                strName
            , const TString&                strValue
        );

        tCIDLib::TCard4 c4CheckIParm
        (
                    TCQCPLMgrValue&         mecvTarget
            ,       TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ParmInd
        );

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
        )   const;

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
            , const MFormattable&           fmtblToken1
        )   const;

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
            , const TError&                 errCaught
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2EnumId_XXX
        //      We use some enumerated types in our interface, so we need
        //      to look up their ids.
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        //
        //  m_c4ErrIds_BadIndex
        //      We save the indices of the error enum values as we create
        //      them, so that we can easily throw them later without having
        //      to back and do a by name lookup of the value.
        //
        //  m_pmeciXXX
        //      We need to translate our enums between the CML and C++
        //      versions, so we need to look up and keep around their class
        //      info objects.
        // -------------------------------------------------------------------
        tCIDLib::TCard2     m_c2EnumId_Errors;
        tCIDLib::TCard2     m_c2EnumId_PLMgrActs;
        tCIDLib::TCard2     m_c2EnumId_PLMgrSelRes;
        tCIDLib::TCard2     m_c2EnumId_PLModes;
        tCIDLib::TCard2     m_c2MethId_DefCtor;
        tCIDLib::TCard2     m_c2MethId_AddMedia;
        tCIDLib::TCard2     m_c2MethId_GetCurCookie;
        tCIDLib::TCard2     m_c2MethId_GetCurIndex;
        tCIDLib::TCard2     m_c2MethId_GetCurItem;
        tCIDLib::TCard2     m_c2MethId_GetCurItemId;
        tCIDLib::TCard2     m_c2MethId_GetItemAt;
        tCIDLib::TCard2     m_c2MethId_GetItemCount;
        tCIDLib::TCard2     m_c2MethId_GetSerialNum;
        tCIDLib::TCard2     m_c2MethId_GetPLMode;
        tCIDLib::TCard2     m_c2MethId_GetVerbose;
        tCIDLib::TCard2     m_c2MethId_FormatCurPLItem;
        tCIDLib::TCard2     m_c2MethId_FormatPL;
        tCIDLib::TCard2     m_c2MethId_HasItemPlayed;
        tCIDLib::TCard2     m_c2MethId_IsEmpty;
        tCIDLib::TCard2     m_c2MethId_LoadFromRendPL;
        tCIDLib::TCard2     m_c2MethId_QueryStdRendFlds;
        tCIDLib::TCard2     m_c2MethId_RandomCatUpdate;
        tCIDLib::TCard2     m_c2MethId_RemoveById;
        tCIDLib::TCard2     m_c2MethId_RemoveItemAt;
        tCIDLib::TCard2     m_c2MethId_Reset;
        tCIDLib::TCard2     m_c2MethId_SelectById;
        tCIDLib::TCard2     m_c2MethId_SelectNewItem;
        tCIDLib::TCard2     m_c2MethId_SetMoniker;
        tCIDLib::TCard2     m_c2MethId_SetPLMode;
        tCIDLib::TCard2     m_c2MethId_SetVerbose;
        tCIDLib::TCard2     m_c2TypeId_FldDef;
        tCIDLib::TCard2     m_c2TypeId_FldList;
        tCIDLib::TCard4     m_c4ErrId_AddFailed;
        tCIDLib::TCard4     m_c4ErrId_EmptyList;
        tCIDLib::TCard4     m_c4ErrId_IndexErr;
        tCIDLib::TCard4     m_c4ErrId_ListLoad;
        TMEngEnumInfo*      m_pmeciErrors;
        TMEngVectorInfo*    m_pmeciFldList;
        TMEngEnumInfo*      m_pmeciPLMgrActs;
        TMEngEnumInfo*      m_pmeciPLMgrSelRes;
        TMEngEnumInfo*      m_pmeciPLModes;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPLMgrInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK



