//
// FILE NAME: ZWaveUSB3Sh_Unit_MultiChCombo_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2018
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
//  This is a sort of generic handler that is designed to deal with a range of
//  multi-channel enabled modules that provide some combination of bits and pieces
//  in one device.
//
//  Ultimately, we'll never be flexible enough to deal with everything. But there are
//  lots of multi-channel sensor type devices that we can handle without making this
//  handler infinitely complex.
//
//  Because of the crazy complexity, we don't try to do this via device info file
//  configuration. Instead we just use the make/model set in the info file via a single
//  extra info value to decide internally within this handler what stuff to set up.
//  Ultimately it will get semi-ugly but any scheme will. We provide various private
//  helpers to minimize the work involved in the setup.
//
//  The make/model is basic info that is always set in the device info file, so it's
//  not extra info.
//
//  We do try to do a 'set and forget' scheme where we have complexity to get it set up;
//  but, once done, the info we leave in place is enough to service all of the other
//  required functionality.
//
//  For each end point we need to track
//
//      1.  End point number
//      2.  The primary CC (which we set up based on the make/model)
//      3.  A notification CC if this end point gets asyncs and it's via something
//          other than the primary CC. We set it up as appropriate based on the make
//          model. This is a value type, not a field type.
//      4.  A semantic type for the primary CC if applicable. We'll set it on the
//          CC but also keep it around for our use since there's no generic way to
//          get it back from the CCs.
//      5.  Is the primary CC V2 compliant, which we need to know for event trigger
//          purposes.
//      6.  We generate the field names for the CCs, which we do based on make/model
//          info. It's just set on the primary CC impl.
//
//  To keep things sane, we manage most of our CC impls, we don't set them on the base
//  class. Therefore we have to override the unit methods that provide that functionality.
//
//  End Point Tracking
//
//  We provide an end point info class that we use to track our end points. There is
//  a base class and a set of derivatives (internally within the cpp file) for specific
//  data types to avoid a lot of busy work related to forcing values between field and
//  notification classes.
//
//  End Point 0
//
//  The module may have some non-end point oriented functionality. Some of it is
//  just basic stuff like battery level (see Other CCs below.) But it may also
//  have a main sensor or switch or some such, plus some end points.
//
//  We treat these as end point 0. That works out well since any non-end point
//  oriented msgs will be given end point id of 0. And that way we can deal with
//  stuff the same as the other stuff without needing special case code.
//
//  Options
//
//  We need to set options at the overall unit level, so we have to look at the end
//  point types and decide what options to report. Mostly there will not be per-
//  end point options really. There won't ever be more than one of various types of
//  end points, since they make no sense. So we might need a temp range, but it makes
//  no sense to have multiple temp sensors in the same unit, they would always see
//  the same temp. Other stuff we know what the ranges will be, such as humidity or
//  luminance.
//
//  Other CCs
//
//  We still do of course deal with general CCs that are not end pointed, such as
//  battery or other stuff that applies to the whole unit. These we do just pass on
//  to the parent class for the usual processing. We only manually deal with the
//  end point ones.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  Since this guy (unusually) has CC impls that it does not set on the base unit
//      class, we have to override callbacks and distribute them to our CC impls, where
//      normally the base class would do that.
//
//  2)  Multiple end points may get asyncs via same class. This isn't an issue since
//      each will be assigned a different end point id. So each one can create its
//      own and the end point id in incoming msgs tells us which one it is and hence
//      where to route it.
//
//  3)  Sending out event triggers we can figure out via the V2 flag and the
//      semantic type of an end point.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TMultiChComboUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TMultiChComboUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  A small abstract class to manage the info about an end point. Derivatves
        //  are created internally for specific data types.
        // -------------------------------------------------------------------
        class TEPInfo
        {
            public :
                // -----------------------------------------------------------
                //  Public class types
                // -----------------------------------------------------------
                enum class EChRes
                {
                    NotMine
                    , NoChange
                    , Changed
                };


                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TEPInfo() = delete;

                TEPInfo(const TEPInfo&) = delete;

                ~TEPInfo();


                // -----------------------------------------------------------
                //  Public, virtual methods
                // -----------------------------------------------------------
                virtual tCIDLib::TBoolean bOwnsFld
                (
                    const   tCIDLib::TCard4     cFldId
                )   const = 0;

                virtual EChRes eHandleChange
                (
                    const   tCIDLib::TCard4     c4ImplId
                ) = 0;

                virtual TZWCCImpl* pzwcciFld() = 0;

                virtual TZWCCImpl* pzwcciNot() = 0;


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                const TZWCCImpl* pzwcciFldC() const;

                const TZWCCImpl* pzwcciNotC() const;


                // -----------------------------------------------------------
                //  Public class members
                //
                //  m_bV2Comp
                //      Indicates if our field is a V2 compliant one. This plut the
                //      semantic type indicates whether we should send event triggers
                //      upon change.
                //
                //  m_c1EPId
                //      The end point numeric id
                //
                //  m_eSemType
                //      The semantic type of our primary CC, just set to 'Generic' if
                //      nothing specific. This may be stored on one or both of the CC impls
                //      but it's not generically available, so we store it for our own
                //      use.
                // -----------------------------------------------------------
                tCIDLib::TBoolean   m_bV2Comp;
                tCIDLib::TCard1     m_c1EPId;
                tCQCKit::EFldSTypes m_eSemType;


            protected :
                // -----------------------------------------------------------
                //  Hidden constructors
                // -----------------------------------------------------------
                TEPInfo
                (
                    const   tCIDLib::TCard1     c1EPId
                    , const tCQCKit::EFldSTypes eSemType = tCQCKit::EFldSTypes::Generic
                );
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMultiChComboUnit() = delete;

        TMultiChComboUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TMultiChComboUnit(const TMultiChComboUnit&) = delete;

        ~TMultiChComboUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMultiChComboUnit& operator=(const TMultiChComboUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const final;

        tCIDLib::TBoolean bPrepare() final;

        tCIDLib::TVoid FormatCCsInfo
        (
                    TTextOutStream&         strmTar
        )   const final;

        tCIDLib::TVoid HandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   final;

        tCIDLib::TVoid ImplValueChanged
        (
            const   tCIDLib::TCard4         c4ImplId
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   final;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const final;

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              attrTmp
        )   const final;

        tCIDLib::TVoid Process() final;

        tCIDLib::TVoid StoreFldIds() final;

        tCIDLib::TVoid UnitIsOnline() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetupMM();

        TEPInfo* pepiAddBinSensor
        (
            const   tCIDLib::TCard1         c1EPId
            , const TZWUnitInfo&            unitiUs
            , const tCIDLib::TCh* const     pszSuffix
            , const tCIDLib::TCh* const     pszDesc
            , const tCQCKit::EFldSTypes     eSemType = tCQCKit::EFldSTypes::Generic
            , const tCIDLib::TBoolean       bAppendId = kCIDLib::False
        );

        TEPInfo* pepiAddTempSensor
        (
            const   tCIDLib::TCard1         c1EPId
            , const TZWUnitInfo&            unitiUs
            , const tCIDLib::TBoolean       bInternal
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colEndPts
        //      Our list of end point info objects. By ref since we stored derivatives
        //      of the base end point info class.
        // -------------------------------------------------------------------
        TRefVector<TEPInfo>     m_colEndPts;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMultiChComboUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
