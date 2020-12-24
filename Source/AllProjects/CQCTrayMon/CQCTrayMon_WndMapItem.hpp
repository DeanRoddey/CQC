//
// FILE NAME: CQCTrayMon_WndMapItem.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/15/2004
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
//  This is the header for the CQCTrayMon_WndMapitem.cpp file, which
//  implements the TAppCtrlWndMapItem class. This class represents a single
//  window in a given app under control. Each window is represented by a
//  'path', which looks kind of like a disk directory. The format is:
//
//      /toplevtext/id/id/id....
//
//  So it starts with a slash, then the text of the top level window under
//  which the target window lives (or it could be the target itself), followed
//  by the list of window ids of the child windows that get to the window
//  of interest. The ids are all formatted as 32 bit hex values into an
//  8 digit hex field, with leading zero fills, so a real world example
//  might be:
//
//      /Player Window/00008001/00000004
//
//  Which would find a top level window with the text "Play Window", then a
//  child of that window with the id 0x8001, and a child of that window
//  with the id 4.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TAppCtrlWndRec
//  PREFIX: acwr
// ---------------------------------------------------------------------------
class TAppCtrlWndRec : public TObject
{
    public  :
        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static const TString& strGetKey
        (
            const   TAppCtrlWndRec&         acwrSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppCtrlWndRec();

        TAppCtrlWndRec
        (
            const   TString&                strPath
            , const TString&                strClass
            , const tCIDCtrls::TWndId       widTarget
            , const tCIDCtrls::TWndHandle   hwndTarget
        );

        TAppCtrlWndRec
        (
            const   TAppCtrlWndRec&         acwrToCopy
        );

        ~TAppCtrlWndRec();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppCtrlWndRec& operator=
        (
            const   TAppCtrlWndRec&         acwrToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strClass() const;

        const TString& strClass
        (
            const   TString&                strToSet
        );

        const TString& strPath() const;

        tCIDCtrls::TWndHandle hwndTarget() const;

        tCIDCtrls::TWndId widTarget() const;

        tCIDLib::TVoid Set
        (
            const   tCIDCtrls::TWndId       widTarget
            , const tCIDCtrls::TWndHandle   hwndTarget
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_hwndTarget
        //      The handle of the target window
        //
        //  m_strClass
        //      The window class of that target window, which is often empty
        //      since the path is sufficiently unique.
        //
        //      format is explained above in the file comments.
        //  m_strPath
        //      The 'path' to the winodw represented by this window. The
        //      format is explained above in the file comments.
        //
        //  m_widTarget
        //      The window id of this window. If it's a top level window,
        //      this will be kCIDLib::c4MaxCard.
        // -------------------------------------------------------------------
        tCIDCtrls::TWndHandle   m_hwndTarget;
        TString                 m_strClass;
        TString                 m_strPath;
        tCIDCtrls::TWndId       m_widTarget;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppCtrlWndRec,TObject)
};


