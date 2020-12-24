//
// FILE NAME: CQCIntfWEng_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/06/2009
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
//   CLASS: TFacCQCIntfWEng
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCINTFWENGEXPORT TFacCQCIntfWEng : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TInt4 i4ErrInvokeCode();

        static tCIDLib::TInt4 i4ErrSelCode();

        static tCIDLib::TVoid SetErrCBCodes
        (
            const   tCIDLib::TInt4          i4ErrSelId
            , const tCIDLib::TInt4          i4ErrInvokeId
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCIntfWEng();

        TFacCQCIntfWEng(const TFacCQCIntfWEng&) = delete;

        ~TFacCQCIntfWEng();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCIntfWEng& operator=(const TFacCQCIntfWEng&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNoInputMode() const;

        tCIDLib::TBoolean bNoInputMode
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bSaveTemplate
        (
            const   TCQCIntfTemplate&       iwdgToSave
            , const TWindow&                wndOwner
            , const TString&                strTitle
            , const tCIDLib::TBoolean       bCheckForOverwrite
            , const TCQCUserCtx&            cuctxToUse
        );

        [[nodiscard]] TCQCIntfViewWnd* pwndMakeNewInterface
        (
                    TWindow&                wndParent
            , const TCQCIntfTemplate&       iwdgTemplate
            , const tCIDCtrls::TWndId       widToUse
            ,       TDataSrvClient&         dsclToUse
            , const TString&                strTitle
            ,       MCQCIntfAppHandler* const pmiahToUse
            , const tCIDLib::TBoolean       bSuppressWarnings
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TVoid ShowErrors
        (
            const   TWindow&                wndParent
            ,       TWindow* const          pwndReport
            , const tCQCIntfEng::TErrList&  colErrs
            , const TString&                strTmplName
        );


    private :
        // -------------------------------------------------------------------
        //  Private, data members
        //
        //  m_bNoInput
        //      We support a no-input mode, for digital signage scenarios.
        //      We will just decline to pass any mouse events on to the
        //      view object. It defaults to false, to allow the almost always
        //      desired input mode.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bNoInput;


        // -------------------------------------------------------------------
        //  Private, static data members
        //
        //  s_i4ErrSelCode
        //  s_i4ErrInvokeCode
        //      The designer will can give use some codes we'll use to send
        //      notifications back to the editor window as the user cursors
        //      over or dbl clicks errors in the error list after a failed
        //      validation of a template. These default to zero which causes
        //      nothing to happen.
        // -------------------------------------------------------------------
        static tCIDLib::TInt4   s_i4ErrSelCode;
        static tCIDLib::TInt4   s_i4ErrInvokeCode;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCIntfWEng,TGUIFacility)

};

#pragma CIDLIB_POPPACK


