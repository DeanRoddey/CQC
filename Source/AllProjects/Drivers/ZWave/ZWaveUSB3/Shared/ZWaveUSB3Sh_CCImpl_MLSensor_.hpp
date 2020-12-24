//
// FILE NAME: ZWaveUSB3Sh_CCImpl_MLSensor_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/21/2017
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
//  This file implements the multi-level sensor Z-Wave command class. As usual
//  we have two classes, one that maintains an internal state and a derivative
//  that creates fields and works in terms of the base class' state.
//
//  Our extra info provides the following information:
//
//      Type=[Card, Int, Float]
//      RangeMin=[minimum range value]
//      RangeMax=[maximum range value]
//
//  It can be forced on us first, by the containing unit handler, in which case we
//  will not try to parse the extra info (which probably is not present in that
//  case.)
//
//  We are told the type and range to expect. We cannot force the unit to send us
//  a particular format. Well, we can set config, but it might not stay that way.
//  We just look at what we get and either it fits within the type/range we were
//  given, or we go to error state.
//
//  This one cannot derive from TZWCCImplLevel since the value may be Card, Int or
//  Float. So we have to deal with some of those details ourself in this case.
//
//  When we report a value change, we use the 'extra info' parameter to pass the
//  value scale that we got in the ML sensor report. This is something that derived
//  classes or the owning unit may need.
//
//  We take the default access flags set by the base CC impl class, which are generally
//  good for us. The owning unit or extra CC info can override this if desired.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLSensor
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplMLSensor : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bExtractInfo
        (
            const   TZWInMsg&               zwimSrc
            , const tCIDLib::TCard4         c4Offset
            ,       tCIDLib::TCard4&        c4Precision
            ,       tCIDLib::TCard4&        c4Scale
            ,       tCIDLib::TCard4&        c4Size
            ,       tCIDLib::TCard4&        c4RawValue
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplMLSensor
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplMLSensor(const TZWCCImplMLSensor&) = delete;

        ~TZWCCImplMLSensor();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExtraInfo() override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4Level() const
        {
            return m_c4Level;
        }

        tCQCKit::EFldTypes eValType() const
        {
            return m_eValType;
        }

        tCIDLib::TFloat8 f8Level() const
        {
            return m_f8Level;
        }

        tCIDLib::TInt4 i4Level() const
        {
            return m_i4Level;
        }

        tCIDLib::TVoid SetFormat
        (
            const   tCQCKit::EFldTypes      eValType
            , const tCIDLib::TFloat8        f8MinRange
            , const tCIDLib::TFloat8        f8MaxRange
            , const tZWaveUSB3Sh::ETScales  eScale = tZWaveUSB3Sh::ETScales::Actual
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

        tCIDLib::TVoid SendValueQuery() override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TFloat8 f8MinRange() const
        {
            return m_f8MinRange;
        }

        tCIDLib::TFloat8 f8MaxRange() const
        {
            return m_f8MaxRange;
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bError
        //      Keeps up with whether the last value we got was bad.
        //
        //  m_bFmtForced
        //      We had value digits/bytes info forced on us, so we won't try to look
        //      for extra info in our command class.
        //
        //  m_c4LastScale
        //      When we store a value, we store the scale value, which may be needed
        //      by the derived class.
        //
        //  m_c4Level
        //      If a card value type, then the last level we saw.
        //
        //  m_eTempScale
        //      If a temp scale, indicates what scale to store as the value. Defaults
        //      to Actual, which just stores whatever is received.
        //
        //  m_eValType
        //      This is the value type that the containing unit wants us to store.
        //      It can be Card, Int, or Float only.
        //
        //  m_f8Level
        //      If a float value type, then the last level we saw.
        //
        //  m_f8MinRange
        //  m_f8MaxRange
        //      The expected range of values. If outside this we go to error state.
        //
        //  m_i4Level
        //      If an int value type, then the last level we saw.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bError;
        tCIDLib::TBoolean       m_bFmtForced;
        tCIDLib::TCard4         m_c4Level;
        tZWaveUSB3Sh::ETScales  m_eTempScale;
        tCQCKit::EFldTypes      m_eValType;
        tCIDLib::TFloat8        m_f8Level;
        tCIDLib::TFloat8        m_f8MinRange;
        tCIDLib::TFloat8        m_f8MaxRange;
        tCIDLib::TInt4          m_i4Level;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplMLSensor, TZWCCImpl)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLSensorF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplMLSensorF : public TZWCCImplMLSensor
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplMLSensorF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strFldName
            , const tCQCKit::EFldSTypes     eSemType = tCQCKit::EFldSTypes::Generic
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplMLSensorF(const TZWCCImplMLSensorF&) = delete;

        ~TZWCCImplMLSensorF();


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
        //  m_c4FldId_Level
        //      This field may be Card4 or Float8, depending on the decimal digits
        //      required by the value.
        //
        //  m_eSemType
        //      The semantic field type for our field. Not much interest to us,
        //      but we need it for creating the field definition.
        //
        //  m_strFldName
        //      The name of the field. Has to include the whole field, including
        //      V2 stuff since we don't get involved in such decisions here and we
        //      will be used for both V2 and non-V2 stuff.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Level;
        tCQCKit::EFldSTypes m_eSemType;
        TString             m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplMLSensorF, TZWCCImplMLSensor)
};

#pragma CIDLIB_POPPACK

