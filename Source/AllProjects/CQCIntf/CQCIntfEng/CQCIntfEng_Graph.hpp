//
// FILE NAME: CQCIntfEng_Graph.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/07/2011
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
//  This is the header for the CQCIntfEng_Graph.cpp file, which implements
//  the graph oriented widgets. We have a base class that just displays
//  the graph data, and derivatives that get that sample data from different
//  places.
//
//  The user gives us a min/max value for the graph. We use this to determine
//  where on the graph the samples go. Anything above or below this range is
//  not displayed, leaving a gap in the graph. We support either linear or
//  logarithmic graph scales.
//
//  We don't provide any visual background. We just display the graph lines.
//  Anything else would be too limiting visually. Instead we just let the
//  user provide whatever graphical background for the graph data they want.
//
//  One source of graph data is the Logic Server, so we have a derivative for
//  that. The logic server client facility provdes us with helpers to poll
//  this data and it caches the data as well and polling it in the background
//  for us. So we don't have to deal with cranking up our own polling threads
//  to do it, and since it's field data we can't use the polling engine that
//  the field related widgets use.
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
//   CLASS: TCQCIntfGraphBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfGraphBase : public TCQCIntfWidget
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfGraphBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDMath::EGraphScales eScale() const;

        tCIDMath::EGraphScales eScale
        (
            const   tCIDMath::EGraphScales  eToSet
        );

        tCIDLib::TFloat4 f4MaxVal() const;

        tCIDLib::TFloat4 f4MaxVal
        (
            const   tCIDLib::TFloat4        f4ToSet
        );

        tCIDLib::TFloat4 f4MinVal() const;

        tCIDLib::TFloat4 f4MinVal
        (
            const   tCIDLib::TFloat4        f4ToSet
        );

        tCIDLib::TVoid ForceUpdate();


    protected :
        // -------------------------------------------------------------------
        //  Protected data types
        // -------------------------------------------------------------------
        using TSampleList = TFundVector<tCIDLib::TFloat4>;


        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfGraphBase
        (
            const   TString&                strTypeName
            , const tCIDLib::TCard4         c4MaxSamples
        );

        TCQCIntfGraphBase
        (
            const   TCQCIntfGraphBase&      iwdgToCopy
        );

        TCQCIntfGraphBase& operator=
        (
            const   TCQCIntfGraphBase&      iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   override;

        tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddNewSamples
        (
            const   tCQLogicSh::TSampleList& fcolNew
            , const tCIDLib::TCard4         c4Count
            , const tCIDLib::TBoolean       bNoRedraw
        );

        tCIDLib::TVoid ResetGraph();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AdjustGraphSizePos
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        );

        tCIDLib::TVoid GenTestWave();



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaGraph
        //      The area we actually draw the graph output into, which is
        //      moved inwards depending on settings.
        //
        //  m_c4AvailPnts
        //      The number of points that got put into the graph points
        //      array the last time we interpolated the display points. This
        //      is how many points are available for display. Until the
        //      graph gets full, it will be less than the total number of
        //      display points we have to fill.
        //
        //  m_c4MaxSamples
        //      The caller tells us the size of the sample it provides, i.e.
        //      how many samples into the past it provides, and therefore
        //      how many we have to hold in order to hang onto them and redraw
        //      when required.
        //
        //  m_eScale
        //      The display scale the user selected. This is user config and
        //      persisted.
        //
        //  m_f4MaxVal
        //  m_f4MinVal
        //      The user configures what are the valid min/max values for
        //      the graph. This is what we use to calculate where the values
        //      lie within the graph area. Any values outside this range
        //      are not displayed, leaving gaps in the graph. These are
        //      psersided of course.
        //
        //  m_grdatSamples
        //      The sample we are displaying. The derived classes call us to
        //      add more samples to the list, and we update accordingly. This
        //      is purely runtime and not persisted.
        //
        //      The derived class tells us how many samples we need to hold,
        //      and we use that to size this list.
        //
        //  m_pntaDrawPnts
        //  m_pntaGraphPnts
        //      When new samples are added, we do an interpolation into
        //      m_pntaGraphPnts, to create a list of points that are scaled
        //      to the correct height and interpolated to our available
        //      set of pixels. This are 0,0 based. Every time we redraw it is
        //      possible that our origin moved, so we take these and use them
        //      to set up m_pntaDrawPnts with the actual points and draw from
        //      thos.
        // -------------------------------------------------------------------
        TArea                   m_areaGraph;
        tCIDLib::TCard4         m_c4AvailPnts;
        tCIDLib::TCard4         m_c4MaxSamples;
        tCIDMath::EGraphScales  m_eScale;
        tCIDLib::TFloat4        m_f4MaxVal;
        tCIDLib::TFloat4        m_f4MinVal;
        TGraphData              m_grdatSamples;
        TPointArray             m_pntaDrawPnts;
        TPointArray             m_pntaGraphPnts;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfGraphBase,TCQCIntfWidget)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfGraphLS
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfGraphLS : public TCQCIntfGraphBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfGraphLS();

        TCQCIntfGraphLS
        (
            const   TCQCIntfGraphLS&        iwdgToCopy
        );

        ~TCQCIntfGraphLS();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfGraphLS& operator=
        (
            const   TCQCIntfGraphLS&        iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid PostInit() override;

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

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strGraphName() const;

        const TString& strGraphName
        (
            const       TString&            strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const tCIDLib::TBoolean         bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4CfgSerialNum
        //      Each time we query graph data, we get a configuration serial
        //      number. We have to pass this back in subsequently, to make
        //      sure that our graph name and serial number are stil referencing
        //      the same data. We use the same one for both graphs since it
        //      applies equally.
        //
        //  m_c4SerialNum
        //      Our sample point serial numbers, which we get on each poll for
        //      sample data and have to pass back in again each time. This
        //      tells the server how many samples we are behind.
        //
        //  m_enctNextUpdate
        //      We don't use field based data, so the engine isn't going to
        //      be tellng us to update. So we set the active update flag
        //      and just check this time stamp to see if we should do another
        //      update. We check about once every 30 seconds, so this is set
        //      to 30 seconds in the future after each update.
        //
        //  m_fcolNewSamples
        //      This is what we query new samples into when we go back to
        //      the logic server. We then pass them down into the parent
        //      class if we get any back. This is viewing time only of course.
        //
        //  m_strGraphName
        //      The name of the logic server 'field' (though it's not really
        //      treated as a field) that we get our data from. This is
        //      persisted and how we get back in touch with the graph data
        //      at viewing time.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4CfgSerialNum;
        tCIDLib::TCard4         m_c4SerialNum;
        tCIDLib::TEncodedTime   m_enctNextUpdate;
        tCQLogicSh::TSampleList m_fcolNewSamples;
        TString                 m_strGraphName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfGraphLS,TCQCIntfGraphBase)
        DefPolyDup(TCQCIntfGraphLS)
};

#pragma CIDLIB_POPPACK


