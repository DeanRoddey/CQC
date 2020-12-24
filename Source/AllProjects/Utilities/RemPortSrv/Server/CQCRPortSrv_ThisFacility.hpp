//
// FILE NAME: CQCRPortSrv_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
//  This is the header for the facility class for the remote port server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCRPortSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCRPortSrv : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCRPortSrv();

        ~TFacCQCRPortSrv();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryCommPorts
        (
                    tCIDLib::TStrList&      colToFill
        );

        TRefVector<TCQCSerPortInfo> colPorts() const;

        tCIDLib::TVoid CloseAllPorts();

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid LoadList
        (
                    TCheckedMCListWnd&      wndPortList
        );

        TCQCSerPortInfo* pspiFind
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        TCQCSerPortInfo* pspiFindByPath
        (
            const   TString&                strPath
        );

        const TString& strTitle() const;

        tCIDLib::TVoid SetAllowedPorts
        (
            const   tCIDLib::TCardList&     fcolCookes
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented ctors and operators
        // -------------------------------------------------------------------
        TFacCQCRPortSrv(const TFacCQCRPortSrv&);
        tCIDLib::TVoid operator=(const TFacCQCRPortSrv&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colPorts
        //      For each port that we open on behalf of a client, we keep a tracking
        //      object on our side. We give them back a cookie and they pass it back
        //      in on subsequent port operations. We use that to look up the item in
        //      this list.
        //
        //      We make this guy thread safe. For atomic ops it's save itself, or we can
        //      lock it for multiple ops.
        //
        //  m_ippnListen
        //      The IP port on which we listen for connections. We just pass
        //      this on to the server side ORB init. We set a default if not
        //      proivded.
        //
        //  m_strTitle
        //      Used in the title bar and popups and such, so we just load it
        //      up once and avoid having redundant hard coded strings.
        //
        //  m_wndMainFrame
        //      The main frame window.
        // -------------------------------------------------------------------
        TRefVector<TCQCSerPortInfo>     m_colPorts;
        TString                         m_strTitle;
        TMainFrameWnd                   m_wndMainFrame;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCRPortSrv,TGUIFacility)
};



