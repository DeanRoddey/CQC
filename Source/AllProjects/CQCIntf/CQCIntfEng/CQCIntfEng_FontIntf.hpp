//
// FILE NAME: CQCIntfEng_FontIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2002
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
//  A number of widget types share the characteristic of having displaying
//  text and needing font attributes and the configuration thereof. Those
//  widgets mix in this interface to get that functionality. The font attribute
//  configuration tab works in terms of this interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfFontIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfFontIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MCQCIntfFontIntf();

        MCQCIntfFontIntf
        (
            const   MCQCIntfFontIntf&       miwdgToCopy
        );

        virtual ~MCQCIntfFontIntf();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   MCQCIntfFontIntf&       miwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBold() const;

        tCIDLib::TBoolean bBold
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bItalic() const;

        tCIDLib::TBoolean bItalic
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bSameFont
        (
            const   MCQCIntfFontIntf&       miwdgSrc
        )   const;

        tCIDLib::TBoolean bNoTextWrap() const;

        tCIDLib::TBoolean bNoTextWrap
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4FontHeight() const;

        tCIDLib::TCard4 c4FontHeight
        (
            const   tCIDLib::TCard4         c4NewHeight
        );

        tCIDGraphDev::EFontDirs eDir() const;

        tCIDGraphDev::EFontDirs eDir
        (
            const   tCIDGraphDev::EFontDirs      eToSet
        );

        tCIDLib::EHJustify eHJustify() const;

        tCIDLib::EHJustify eHJustify
        (
            const   tCIDLib::EHJustify      eToSet
        );

        tCIDGraphDev::ETextFX eTextFX() const;

        tCIDGraphDev::ETextFX eTextFX
        (
            const   tCIDGraphDev::ETextFX       eToSEt
        );

        tCIDLib::EVJustify eVJustify() const;

        tCIDLib::EVJustify eVJustify
        (
            const   tCIDLib::EVJustify      eToSet
        );

        TGUIFont& gfontText
        (
                    TGraphDrawDev&          gdevTarget
        )   const;

        TPoint& pntOffset();

        const TPoint& pntOffset() const;

        TPoint& pntOffset
        (
            const   TPoint&                 pntToSet
        );

        tCIDLib::TVoid QueryFontAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
            , const TString* const          pstrCaption
            , const tCIDLib::TBoolean       bMultiLine = kCIDLib::True
        )   const;

        tCIDLib::TVoid QueryFontSelAttrs
        (
                    TFontSelAttrs&          fselToFill
        )   const;

        tCIDLib::TVoid ReadInFont
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid ResetFont();

        const TString& strFaceName() const;

        const TString& strFaceName
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid ScaleFont
        (
            const   tCIDLib::TFloat8        f8XScale
            , const tCIDLib::TFloat8        f8YScale
        );

        tCIDLib::TVoid SetFontAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid WriteOutFont
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Flags
        //      We don't keep a whole TFontSelAttrs object around. We just
        //      store separate fields and create the font on the fly. This
        //      one is used to store some of the bit fields associated with
        //      fonts, e.g. bold, italic, etc..., plus some general use
        //      ones such as 'no wrap'.
        //
        //  m_c4FontHeight
        //      We don't automatically scale the text to fit the area, since
        //      that wouldn't achieve the visual look often desired. So we
        //      provide separate control of the font height. It is the EM
        //      height in pixels.
        //
        //  m_eDir
        //      The font direction. If the caps flags of the widget indicate
        //      it can support anything other than standard 0 degrees, then
        //      this will be used to indicate the direction.
        //
        //  m_eEffect
        //      They can choose a text effect if they want. Any given
        //      widget can choose to support this or not. If not, they'll
        //      not set the caps flag and the font tab will disble the stuff
        //      that would allow this to be set.
        //
        //  m_eHJustify
        //  m_eVJustify
        //      These are used for text display mostly, and tell us how to
        //      justify the content within the area.
        //
        //  m_pgfontText
        //      A font that represents a font for our text settings. If this
        //      widget has no text, it won't ever get set up. It will also
        //      be deleted and reset to zero when the font attrs change, to
        //      force it to be recreated on the next call to gfontText().
        //
        //  m_pntOffset
        //      If m_eEffect is enabled, then some of the effects support an
        //      offset, to offset a drop shadow or reflection. Else it is
        //      ignored.
        //
        //  m_strFaceName
        //      Used to indicate the face name of the font to use when
        //      displaying text content.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4Flags;
        tCIDLib::TCard4         m_c4FontHeight;
        tCIDGraphDev::EFontDirs m_eDir;
        tCIDGraphDev::ETextFX  m_eEffect;
        tCIDLib::EHJustify      m_eHJustify;
        tCIDLib::EVJustify      m_eVJustify;
        mutable TGUIFont*       m_pgfontText;
        TPoint                  m_pntOffset;
        TString                 m_strFaceName;
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFontJan
//  PREFIX: jan
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFontJan : public TFontJanitor
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFontJan(        TGraphDrawDev* const    pgdevTarget
                        , const MCQCIntfFontIntf&       miwdgTarget) :

            TFontJanitor(pgdevTarget, &miwdgTarget.gfontText(*pgdevTarget))
        {
        }

        TCQCIntfFontJan(const TCQCIntfFontJan&) = delete;

        ~TCQCIntfFontJan()
        {
        }


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFontJan& operator=(const TCQCIntfFontJan&) = delete;
};

#pragma CIDLIB_POPPACK

