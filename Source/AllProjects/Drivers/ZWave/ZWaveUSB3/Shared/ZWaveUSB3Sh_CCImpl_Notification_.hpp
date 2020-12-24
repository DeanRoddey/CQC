//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Notification_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/03/2018
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
//  Some CC impl classes that implement useful aspects of the Notification Z-Wave
//  command class. It is used for various types of sensors. These are all read
//  only since notifications are only for getting status. If the status of the
//  thing is readable some other way, the unit class that creates us must get
//  that info and set it on us.
//
//  The whole issue of notifications and alarms are a joke in Z-Wave, and it's
//  practically impossible to tease out all of the info, because version 1 of
//  ALARM had no defined codes, so it's completely randomly selected values that
//  you have to go find (and usually cannot.) Version had defined codes, but then
//  it was layered over with the NOTIFICATION class as they tried to create some
//  sort of coherence.
//
//  So we use our command class version so we can watch for the appropriate types
//  of msgs and ids and such.
//
//  If a V1 alarm type, the notification id is not provided since there wasn't
//  such a concept there.
//
//  If we accept a new state, we will also, if possible, store the notification
//  event parameter data, which our owning unit can query if desired. For V3
//  and beyond that's obvious. For V1, we assume that last byte is that. The unit
//  will have to know if it is useful.
//
//  Extra Info
//
//  If notifications are to be used, then extra data must be set on the notification
//  CC to indiate the id and the off/on event ids. The notification id may be 0 if it
//  an old style ALARM type.
//
//      NotId   : Used to match the incoming msg to see if it's one that applies to this
//                instance. Where we get this from in the message is driven by NIdType.
//      NIdType : Indicates what parts of the report make up the notification id, since
//                there are a number of schemes at this point. See below for what this
//                means. This is one of the tZWaveUSB3Sh::ENotIdTypes values, so see
//                that enum for details. If not explicitly set, defaults to NotType.
//
//      OffEvs  : A space separated list of numbers that represent the off values
//      OnEvs   : A space separated list of numbers that represent the on values
//      EvType  : Indicates what field of the report we check the events against, since
//                it won't always be the new style event, even if it's not a V1 alarm
//                type. See below for the values. Only used for non-V1 alarm types.
//
//  If not set explicitly, it defaults to NotType, for the new style scheme.
//
//  Similarly we need to know what to compare the off/on events to. Usually it is the new
//  event id for non-V1 or the old level for V1. But, for non-V1 msgs we may need to be
//  told what to check. The values are defined by the tZWaveUSB3Sh::ENotEvTypes enum
//  so see that enum definition.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  For our field based type, if we were told we can send triggers, and the field
//      name and semantic type indicate one should be sent, then we do that. The
//      semantic type indicates what type of trigger.
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplNotifyState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplNotifyState : public TZWCCImplBin
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplNotifyState
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplNotifyState(const TZWCCImplNotifyState&) = delete;

        ~TZWCCImplNotifyState();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPrepare() override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetNotIds
        (
            const   tZWaveUSB3Sh::EBinSensorTypes eType
        );

        tCIDLib::TCard4 c4LastEventParam() const;

        tCIDLib::TVoid SetNotIds
        (
            const   tCIDLib::TCard2         c2NotId
            , const tCIDLib::TByteList      fcolOffEvs
            , const tCIDLib::TByteList      fcolOnEvs
        );

        tCIDLib::TVoid SetNotIds
        (
            const   tZWaveUSB3Sh::ENotIdTypes eNotIdType
            , const tZWaveUSB3Sh::ENotEvTypes eEvType
            , const tCIDLib::TCard2         c2NotId
            , const tCIDLib::TByteList      fcolOffEvs
            , const tCIDLib::TByteList      fcolOnEvs
        );


    protected :
        // -------------------------------------------------------------------
        //  protected, inherited methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bForceIds
        //  m_bForceTypes
        //      In some cases we want to be driven by an owning unit as to what types
        //      and ids we use. When those methods are called to set them, we will set
        //      these flags and that will prevent bPrepare() from trying to parse this
        //      info from the notification class extra info (which shouldn't exist in
        //      that case, and we would consider that an error if not forced.)
        //
        //  m_c2NotId
        //      The notification id we use if we are on V2 or above. See the class
        //      comments, it may be a single msg field or a pair.
        //
        //  m_c4LastEventParam
        //      If we store a new value, we will if available store the event parameters.
        //      If none, we set it to zero. The owning unit can come get this
        //
        //  m_c4Version
        //      See the class comments above, this is our alarm/notification class
        //      version.
        //
        //  m_eEvType
        //  m_eNotIdType
        //      See class notes above, indicates what makes up the notification id and
        //      the off/on events. Only used for non-V1 alarm msgs.
        //
        //  m_fcolOffEvs
        //  m_fcolOnEvs
        //      The event ids that represent off and on.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bForceIds;
        tCIDLib::TBoolean           m_bForceTypes;
        tCIDLib::TCard2             m_c2NotId;
        tCIDLib::TCard4             m_c4LastEventParam;
        tCIDLib::TCard4             m_c4Version;
        tZWaveUSB3Sh::ENotEvTypes   m_eEvType;
        tZWaveUSB3Sh::ENotIdTypes   m_eIdType;
        tCIDLib::TByteList          m_fcolOffEvs;
        tCIDLib::TByteList          m_fcolOnEvs;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplNotifyState, TZWCCImplBin)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplNotifyStateF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplNotifyStateF : public TZWCCImplNotifyState
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplNotifyStateF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
            , const tCQCKit::EFldSTypes     eSemType = tCQCKit::EFldSTypes::Generic
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplNotifyStateF(const TZWCCImplNotifyStateF&) = delete;

        ~TZWCCImplNotifyStateF();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const override;

        tCIDLib::TVoid StoreFldIds() override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid EnteredErrState() override;

        tCIDLib::TVoid ValueChanged
        (
            const   tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_State
        //      Our state field.
        //
        //  m_eSemType
        //      The semantic field type for our field. Not much interest to us, but
        //      we need it for creating the field definition.
        //
        //  m_strFldName
        //      The name of the field. Has to include the whole field, including
        //      V2 stuff since we don't get involved in such decisions here and we
        //      will be used for both V2 and non-V2 stuff.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_State;
        tCQCKit::EFldSTypes m_eSemType;
        TString             m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplNotifyStateF, TZWCCImplNotifyState)
};
#pragma CIDLIB_POPPACK
