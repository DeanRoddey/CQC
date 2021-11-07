//
// FILE NAME: CQCIntfEng_SpecialImg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/18/2005
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
//  This is the header for the CQCIntfEng_SpecialImg.cpp file, which
//  implements an abstract class that a few types of widgets derive from.
//  These are widgets that don't use the standard image or image list mixins
//  because they automatically pick their own images dynamically, for instance
//  the Weather Channel and DirectTV widgets.
//
//  So this class is provided so that they can use a common configuration
//  tab dialog to allow for some of the control that would be available for
//  widgets that use the standard image mixins. For instance, we want to be
//  able to adjust the opacity of the images. This class doesn't have any
//  data of it's own, it just provides the virtual methods via which the
//  config tab can control aspects of the derived classes. So it is very
//  simple, because it doesn't handle any of the widget functioanlity really.
//
//  The tab will ask for a sample bitmap, so that it has somethign to
//  display to the user so that they can see the results of their adjustments.
//  This class is only really used at design time, so the widget should
//  provide some default sample image of the sort it would normally display.
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
//   CLASS: TCQCIntfSpecialImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfSpecialImg : public TCQCIntfWidget
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TCQCIntfSpecialImg() = delete;

        ~TCQCIntfSpecialImg();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TCard1 c1Opacity() const = 0;

        virtual tCIDLib::TCard1 c1Opacity
        (
            const   tCIDLib::TCard1         c1ToSet
        ) = 0;

        virtual TBitmap bmpSample() const = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfSpecialImg
        (
            const   tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
        );

        TCQCIntfSpecialImg
        (
            const   TCQCIntfSpecialImg&      iwdgToCopy
        );

        TCQCIntfSpecialImg& operator=
        (
            const   TCQCIntfSpecialImg&      iwdgToAssign
        );


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfSpecialImg,TCQCIntfWidget)
};

#pragma CIDLIB_POPPACK


