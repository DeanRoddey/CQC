//
// FILE NAME: CQCIntfEd_EdWrappers_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/18/2015
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
//  A number of attributes are edited via dialogs only, because they are too complex
//  to edit in the attribute editor window. In those cases, we just have a dummy
//  attribute that brings up a dialog. It directly edits the widget.
//
//  However in some cases like the mixins, we'd like to be able to save just that
//  mixin info and be able to undo the changes if they cancel out of the dialog.
//  So we create some simple classes that just exist to do the mixins.
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
//   CLASS: TIEImgListWrap
//  PREFIX: iew
// ---------------------------------------------------------------------------
class TIEImgListWrap : public MCQCIntfImgListIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIEImgListWrap();

        TIEImgListWrap(const TIEImgListWrap&) = delete;

        ~TIEImgListWrap();
};


// ---------------------------------------------------------------------------
//   CLASS: TIEXlatsWrap
//  PREFIX: iew
// ---------------------------------------------------------------------------
class TIEXlatsWrap : public MCQCIntfXlatIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIEXlatsWrap();

        TIEXlatsWrap(const TIEXlatsWrap&) = delete;

        ~TIEXlatsWrap();
};

#pragma CIDLIB_POPPACK

