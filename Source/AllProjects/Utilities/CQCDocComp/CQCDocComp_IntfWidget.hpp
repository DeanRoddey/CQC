//
// FILE NAME: CQCDocComp_IntfWdg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/19/2016
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
//  This file defines a simple class that is used to hold information about an interface
//  widget. For events and RTVS, we just use key/value pairs, the key being the name
//  and the value being the description. For commands, we use the same class as the
//  action command classes, since it's the same stuff.
//
//  These are defined by .cqciwdg files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TDocIntfWidget
//  PREFIX: actt
// ---------------------------------------------------------------------------
class TDocIntfWidget : public TBasePage
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDocIntfWidget();

        TDocIntfWidget(const TDocIntfWidget&) = delete;

        ~TDocIntfWidget();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDocIntfWidget& operator=(const TDocIntfWidget&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseXML
        (
            const   TXMLTreeElement&        xtnodeRoot
            , const TString&                strName
            , const TTopic&                 topicPar
        )   override;

        tCIDLib::TVoid GenerateOutput
        (
                    TTextOutStream&         strmTar
        )   const override;


        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_colXXX
        //      Lists for the commands, events and RTVs this widget supports. Some of them
        //      will often be empty.
        //
        //  m_hnCmds
        //      Any overall description text for the commands block, it's optional.
        //
        //  m_hnDescr
        //      This is the overall description text for the widget, it's optional.
        //
        //  m_hnEvents
        //      Any overall description text for the events block.
        //
        //  m_hnRTVs
        //      Any overall description text for the RTVs block, it's optional.
        // -------------------------------------------------------------------
        TVector<TDocActionCmd>  m_colCmds;
        tCIDLib::TKVPList       m_colEvents;
        tCIDLib::TKVPList       m_colRTVs;
        THelpNode               m_hnCmds;
        THelpNode               m_hnDescr;
        THelpNode               m_hnEvents;
        THelpNode               m_hnRTVs;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseCmds
        (
            const   TXMLTreeElement&        xtnodeRoot
        );

        tCIDLib::TBoolean bParseItems
        (
            const   TXMLTreeElement&        xtnodeRoot
            ,       tCIDLib::TKVPList&      colToFill
            ,       THelpNode&              hnToFill
        );
};
