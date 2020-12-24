//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Thermostat_.hpp
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
//  Some CC impl classes that implement the various possible components of a
//  thermostat. For whatever reason they broke it up into a lot of separate CCs, mode
//  than really is warranted IMO. So modes and operating modes (states) are separate
//  CCs. Then we have one that handles high and low set points, and one  for current
//  temp. So up to seven CC impl objects if the thermo has both high and low set points,
//  and six CC impl classes. The current temp is done via multi-level sensor, we have
//  to implement the others here.
//
//  If all of them are supported on a given thermostat then we can consider it V2
//  compatible. Else we treat it as a non-standard thermostat. At some point the V2
//  standard will be updated to allow for a single set point.
//
//  We don't have the same sorts of constraints here that some CCs do, of having to
//  have separate state and field based variations of each, which is good given how
//  many there are. All of them are field managing types.
//
//  For thermostats, the unit name is really the sub-unit name if V2, since each field
//  has as pre-defined name part. So if the unit is FFloor, then we end up with names
//  like?
//
//      THERM#FFloor~CurrentTemp
//      THERM#FFloor~LowSetPnt
//
//  and so forth. The owning unit decides if we are V2 or not, and creates the field
//  names himself, so we can be whatever he needs. He is responsible to set up us
//  correctly if ne creates V2 field names.
//
//  We also assume that thermostats are never multi-channel.
//
//  Set Point Range
//
//  For the set point CC, we will set reasonable default min/max values based on the
//  indicated scale. But these can also be set by the owning unit, or via extra info
//  set on our CC. If set in the CC it is:
//
//      LowRange=min max
//      HighRange=min max
//
//  Where min and max are decimal radix values appropriate for the actual device. THese
//  are always in F scale, since the scale is selected by the user in most cases via
//  option. So we need to know what these are in order to convert to the actual range
//  when we set up the field.
//
//  Read/Write Access
//
//  Unless overridden by CC extra info or the owning unit, we take the read flags set
//  by the base CC impl. If we are in an always on unit, we'll force the write flag
//  on for those CCs that can handle writes.
//
//  Modes/States
//
//  The mode oriented ones take a CC specific extra info value that indicates what
//  modes or states are supported by the particular thermostat. These are bitmapped
//  values where the 0th bit is the first mode, 1st bit is the second mode, and so
//  on. We'll use these to generate enumerated limits for the fields. So they will be
//  one of these:
//
//      Modes=0xXX
//
//  Of course they could be set manually by the owning unit for more generic handlers.
//
//  For the states, we don't set any limit, and will just report what we get, with
//  an 'Unknown' for any we don't understand.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoMode
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplThermoMode : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplThermoMode
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
        );

        TZWCCImplThermoMode(const TZWCCImplThermoMode&) = delete;

        ~TZWCCImplThermoMode();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TBoolean bStringFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            ,       tCQCKit::ECommResults&  eRes
        )   override;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const override;

        tCIDLib::TVoid StoreFldIds() override;


        // -------------------------------------------------------------------
        //  Public, non-static methods
        // -------------------------------------------------------------------
        const TString& strFldName() const
        {
            return m_strFldName;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   override;

        tCIDLib::TVoid EnteredErrState() override;

        tCIDLib::TVoid SendValueQuery() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_Mode
        //      An enumerated string field that is written to to set set the mode
        //      of the thermostat.
        //
        //  m_c4ModeMask
        //      We store any mode mask we get from the extra info. We can use this
        //      to check for valid values.
        //
        //  m_strFldName
        //      For convenience we store our built up field name for ongoing use.
        //
        //  m_strLimits
        //      During prepare we look up the modes defined in the device info file
        //      and build up the limit string for our field.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Mode;
        tCIDLib::TCard4     m_c4ModeMask;
        TString             m_strFldName;
        TString             m_strLimits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplThermoMode, TZWCCImpl)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoFanMode
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplThermoFanMode : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplThermoFanMode
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
        );

        TZWCCImplThermoFanMode(const TZWCCImplThermoFanMode&) = delete;

        ~TZWCCImplThermoFanMode();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TBoolean bStringFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            ,       tCQCKit::ECommResults&  eRes
        )   override;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const override;

        tCIDLib::TVoid StoreFldIds() override;


        // -------------------------------------------------------------------
        //  Public, non-static methods
        // -------------------------------------------------------------------
        const TString& strFldName() const
        {
            return m_strFldName;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   override;


        tCIDLib::TVoid EnteredErrState() override;

        tCIDLib::TVoid SendValueQuery() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_Mode
        //      An enumerated string field that is written to to set set the mode
        //      of the thermostat fan.
        //
        //  m_c4ModeMask
        //      We store any mode mask we get from the extra info. We can use this
        //      to check for valid values.
        //
        //  m_strFldName
        //      For convenience we store our built up field name for ongoing use.
        //
        //  m_strLimits
        //      During prepare we look up the modes defined in the device info file
        //      and build up the limit string for our field.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4ModeMask;
        tCIDLib::TCard4     m_c4FldId_Mode;
        TString             m_strFldName;
        TString             m_strLimits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplThermoFanMode, TZWCCImpl)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoFanState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplThermoFanState : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplThermoFanState
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
        );

        TZWCCImplThermoFanState(const TZWCCImplThermoFanState&) = delete;

        ~TZWCCImplThermoFanState();


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


        // -------------------------------------------------------------------
        //  Public, non-static methods
        // -------------------------------------------------------------------
        const TString& strFldName() const
        {
            return m_strFldName;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   override;

        tCIDLib::TVoid EnteredErrState() override;

        tCIDLib::TVoid SendValueQuery() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_State
        //      A string field that reflects the fan state.
        //
        //  m_strFldName
        //      For convenience we store our built up field name for ongoing use.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldId_State;
        TString                 m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplThermoFanState, TZWCCImpl)
};


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplThermoState : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplThermoState
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
        );

        TZWCCImplThermoState(const TZWCCImplThermoState&) = delete;

        ~TZWCCImplThermoState();


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


        // -------------------------------------------------------------------
        //  Public, non-static methods
        // -------------------------------------------------------------------
        const TString& strFldName() const
        {
            return m_strFldName;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   override;

        tCIDLib::TVoid EnteredErrState() override;

        tCIDLib::TVoid SendValueQuery() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_State
        //      An string field that holds the current operating state.
        //
        //  m_strFldName
        //      For convenience we store our built up field name for ongoing use.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4FldId_State;
        TString         m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplThermoState, TZWCCImpl)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoSP
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplThermoSP : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplThermoSP
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
            , const tCIDLib::TBoolean       bHighSP
            , const tCIDLib::TBoolean       bFScale
        );

        TZWCCImplThermoSP(const TZWCCImplThermoSP&) = delete;

        ~TZWCCImplThermoSP();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIntFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewVal
            ,       tCQCKit::ECommResults&  eRes
        )   override;

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const override;

        tCIDLib::TVoid StoreFldIds() override;


        // -------------------------------------------------------------------
        //  Public, non-static methods
        // -------------------------------------------------------------------
        const TString& strFldName() const
        {
            return m_strFldName;
        }

        tCIDLib::TVoid SetSPRange
        (
            const   tCIDLib::TInt4          i4MinF
            , const tCIDLib::TInt4          i4MaxF
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   override;

        tCIDLib::TVoid EnteredErrState() override;

        tCIDLib::TVoid SendValueQuery() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bFScale
        //      Our creator tells us whether our thermo is set up for F or C scale,
        //      which we use to set appropriate limits on our field.
        //
        //  m_bHighSP
        //      We handle high or low set point, depending on what they tell us
        //      we are to do.
        //
        //  m_c4FldId_SP
        //      An int field that represents the set point value.
        //
        //  m_i4MaxSPValF
        //  m_i4MinSPValF
        //      These are the min/max values used to set the range for our field.
        //      We default them to reasonable values based on scale, but the owning
        //      unit can set them, and we can get the values from extra info.
        //
        //      THESE ARE IN F scale always, since the user can select a scale and
        //      the extra info cannot know which it will be. We will convert as
        //      required to set the field range.
        //
        //  m_strFldName
        //      For convenience we store our built up field name for ongoing use.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bFScale;
        tCIDLib::TBoolean   m_bHighSP;
        tCIDLib::TCard4     m_c4FldId_SP;
        tCIDLib::TInt4      m_i4MaxSPValF;
        tCIDLib::TInt4      m_i4MinSPValF;
        TString             m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplThermoSP, TZWCCImpl)
};

#pragma CIDLIB_POPPACK
