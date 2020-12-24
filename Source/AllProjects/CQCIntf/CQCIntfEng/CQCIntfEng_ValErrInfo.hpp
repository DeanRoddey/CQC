//
// FILE NAME: CQCIntfEng_ValErrInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/21/2008
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
//  This is the header for the CQCIntfEng_VarErrInfo.cpp file, which
//  implements the TCQCIntfValErrInfo class. Instances of this class represent
//  a validation failure. When a template is loaded or manually validated,
//  a collection of these is passed in and for each validation error found,
//  an instance is placed in the collection so that the errors can be
//  reported at the end and the offending widgets located.
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
//   CLASS: TCQCIntfValErrInfo
//  PREFIX: vei
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfValErrInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfValErrInfo();

        TCQCIntfValErrInfo
        (
            const   tCIDLib::TCard4         c4UniqueId
            , const TString&                strErrText
            , const tCIDLib::TBoolean       bWarning
            , const TString&                strWidgetId
        );

        TCQCIntfValErrInfo
        (
            const   tCQCIntfEng::EValErrTypes eType
            , const TString&                strErrText
            , const tCIDLib::TBoolean       bWarning
            , const TString&                strWidgetId
        );

        TCQCIntfValErrInfo
        (
            const   TCQCIntfValErrInfo&     veiToCopy
        );

        ~TCQCIntfValErrInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfValErrInfo& operator=
        (
            const   TCQCIntfValErrInfo&     veiToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bWarning() const;

        tCIDLib::TCard4 c4ErrId() const;

        tCIDLib::TCard4 c4WidgetId() const;

        tCQCIntfEng::EValErrTypes eType() const;

        const TString& strErrText() const;

        const TString& strWidgetId() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bStatus
        //      Each error can be warning or failure, and this indicates which. In some
        //      cases we won't show warning messages, only failures, and we need to be
        //      able to display to the user which type is which in any case.
        //
        //  m_c4ErrId
        //      A unique ongoing id is assigned to each error. This is used to by display
        //      code to map selections in a list box back to the error that occurred.
        //
        //  m_c4WidgetId
        //      The unique id of the widget, so that we can get back to it again if the
        //      user wants to correct the problem. This is only valid if the type indicates
        //      its a widget oriented error.
        //
        //  m_eType
        //      Indicates what type of error it is, i.e. what area of the template it is
        //      in, which explains how to interpret the other info.
        //
        //  m_strErrText
        //      The text of the error as filled in by the widget that failed validation.
        //
        //  m_strWidgetId
        //      The text widget id of the widget, if it has one and this is a widget
        //      oriented error. Else it's empty.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bWarning;
        tCIDLib::TCard4             m_c4ErrId;
        tCIDLib::TCard4             m_c4WidgetId;
        tCQCIntfEng::EValErrTypes   m_eType;
        TString                     m_strErrText;
        TString                     m_strWidgetId;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfValErrInfo,TObject)
};

#pragma CIDLIB_POPPACK

