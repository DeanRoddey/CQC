//
// FILE NAME: ZWaveUSB3Sh_CCImpl_CSwitch_.hpp
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
//  A CC implementation for units that support color switches that represent the
//  separate color components as end points. So we are told the ids of our end
//  points and allow the color component values to be read and written.
//
//  We have one for the RGB bits. If it supports white then there's another for
//  white. Off/on support is provided separately as well.
//
//  We assume a V2 color switch here and handle all of the V2 stuff here. The
//  caller can provide us with a sub-unit part of the name, but we build the rest
//  of it ourself based on the unit name and known color switch fields, using the
//  sub-unit if provided.
//
//
//  When we report a value change, though we are reporting the overall RGB value
//  that is changing, we pass the component that changed as the extra info parameter
//  in case that is of use to derived classes or the containing unit.
//
//  We set default access of read/write in our ctor, since that is the most common
//  scenario. The extra info or owning unit can override if needed.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplCLghtRGB
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplCLghtRGB : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplCLghtRGB
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TCard1         c1WhiteComp
        );

        TZWCCImplCLghtRGB(const TZWCCImplCLghtRGB&) = delete;

        ~TZWCCImplCLghtRGB();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::ECommResults eSetColor
        (
            const   TString&                strValue
        );

        tCQCKit::ECommResults eSetComp
        (
            const   TString&                strValue
        );

        tCIDLib::TCard1 c1WhiteComp() const
        {
            return m_c1WhiteComp;
        }

        const TRGBClr& rgbColor() const
        {
            return m_rgbColor;
        }


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


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid QueryComp
        (
            const   tCIDLib::TCard1         c1CompId
        );

        tCIDLib::TVoid SetComp
        (
            const   tCIDLib::TCard1         c1CompId
            , const tCIDLib::TCard1         c1Val
        );

        tCIDLib::TVoid SetComps
        (
            const   tCIDLib::TCard1         c1Red
            , const tCIDLib::TCard1         c1Green
            , const tCIDLib::TCard1         c1Blue
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c1WhiteComp
        //      We can support the white component. This is driven by extra info. If
        //      it's not 0xFF, then it's the white color component id.
        //
        //  m_rgbColor
        //      Since have to expose an HSV color, that means any time any one color
        //      component changes, we have to combine it with the other three. So we
        //      keep the last values we'll seen. This is the composite last value
        //      we got. If it changes, we call RGBChanged().
        // -------------------------------------------------------------------
        tCIDLib::TCard1     m_c1WhiteComp;
        TRGBClr             m_rgbColor;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplCLghtRGB, TZWCCImpl)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplCLghtRGBF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplCLghtRGBF : public TZWCCImplCLghtRGB
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplCLghtRGBF
        (
                    TZWUnitInfo* const      punitiThis
            , const TString&                strSubUnit
            , const tCIDLib::TCard1         c1WhiteComp
        );

        TZWCCImplCLghtRGBF(const TZWCCImplCLghtRGBF&) = delete;

        ~TZWCCImplCLghtRGBF();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

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
        //  m_c4FldId_Color
        //      The standard V2 color lighting field, that supports HSV format
        //      control. So it's a string with the HSV values space separated.
        //
        //  m_c4FldId_SetComp
        //      A field that lets them pass a formatted string in the form "comp
        //      percent", e.g. red 50 or green 100, to see one of the individual
        //      color components.
        //
        //  m_strSubUnit
        //      If this is part of a sub-unit of a larger unit, the caller provides
        //      us with a sub-unit name to use in field name creation.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_Color;
        tCIDLib::TCard4     m_c4FldId_SetComp;
        TString             m_strSubUnit;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplCLghtRGBF, TZWCCImplCLghtRGB)
};

#pragma CIDLIB_POPPACK

