//
// FILE NAME: CQCWebSrvC_EchoHandler_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/25/2015
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
//  This is the header for the CQCWebSrv_EchoHandler.cpp file, which implements
//  a specialized handler for the URL that is sent by the javascript that we provide
//  for folks to install on the Amazon web server to handle Echo voice input. We
//  watch for the special URL in the worker thread that is servicing incoming
//  requests. If it sees that URL, it will pass it to this handler.
//
//  We expect it to be a POST, with a JSON body. We parse out the JSON body to
//  decide what needs to be done. That is determined by a configuration file we
//  read in upon first access. We take the incoming info and use that to find the
//  appropriating mapping in the file. That tells us what to do.
//
//  The msg types we get are:
//
//  The messages look like this:
//
//      {
//          MyKey : a unique key for security reasons,
//          Type  : RunAction | SetValue,
//          Name  : name of the action to run or value to set,
//          Value : if applicable, the value to use
//      }
//
//  So we only have a couple of options, driven by the Type value. Currently we can
//  run an action (global action or macro, depending on how the config file maps it)
//  or set a value, which also runs a global action or macro, but passes in a value
//  to it.
//
//  The file is very simple, line oriented, and looks like this:
//
//      EchoConfig=
//          MyKey=xxxx [,xxx,...]
//      EndEchoConfig
//
//      RunMap
//          Action=\User\Echo\Home Theater
//              Home Theater, Theater
//          EndAction
//      EndRunMap
//
//      SetMap
//          Macro=\User\Echo\Set
//              Volume
//              Scene
//              Channel
//          EndMacro
//      EndSetMap
//
//      QueryMap
//          Macro=\User\Echo\Query
//              Current Channel, Channel
//              Temperature, Current Temperature, Current Temp
//          EndMacro
//      EndQueryMap
//
//  The reply format is also JSON and is currently just:
//
//      {
//          Reply : the reply text to speak
//      }
//
//  A 100 (Continue) status return means that more info is needed and so the reply
//  text is a query to get more info from the user. For any other code the implication
//  is that the session is ended.
//
//  Note that there can be more than one key, because this handler may be invoked by more
//  than one Echo, and this is used to allow the handler to distinguish which it is. For now
//  they have to create multiple skills, each with a different key. Later, if the Echo provides
//  a per-device id, that can be used.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TWSEchoCfgBlock
//  PREFIX: wsecb
// ---------------------------------------------------------------------------
class TWSEchoCfgBlock
{
    public :
        // --------------------------------------------------------------------
        //  Constructors and Destructor
        // --------------------------------------------------------------------
        TWSEchoCfgBlock();

        TWSEchoCfgBlock
        (
            const   TWSEchoCfgBlock&        wsecbSrc
        );

        TWSEchoCfgBlock(TWSEchoCfgBlock&&) = delete;

        ~TWSEchoCfgBlock();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWSEchoCfgBlock& operator=
        (
            const   TWSEchoCfgBlock&        wsecbSrc
        );

        TWSEchoCfgBlock& operator&(const TWSEchoCfgBlock&) = delete;
        TWSEchoCfgBlock& operator&(TWSEchoCfgBlock&&) = delete;


        // --------------------------------------------------------------------
        //  Public, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid AddPhraseList
        (
            const   tCIDLib::TStrList&      colToAdd
        );

        tCIDLib::TBoolean bMatches
        (
            const   TString&                strCheck
            ,       TString&                strPhraseFound
        )   const;

        tCIDLib::TBoolean bIsMacro() const;

        const TString& strPath() const;

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TBoolean       bIsMacro
            , const TString&                strPath
        );


    private :
        // --------------------------------------------------------------------
        //  Private data types
        // --------------------------------------------------------------------
        using TPhraseMap = TVector<tCIDLib::TStrList>;


        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_bIsMacro
        //      Indicates if this one is a macro or action based.
        //
        //  m_colPhrases
        //      The phrases that can be matched for this block. It's a list of lists
        //      though often they have a single value. The first one in each list
        //      is the one passed to the user's handler.
        //
        //  m_strPath
        //      The path to the global action or macro to run.
        // --------------------------------------------------------------------
        tCIDLib::TBoolean   m_bIsMacro;
        TPhraseMap          m_colPhrases;
        TString             m_strPath;
};


// ---------------------------------------------------------------------------
//   CLASS: TWSEchoCfg
//  PREFIX: wsec
// ---------------------------------------------------------------------------
class TWSEchoCfg
{
    public :
        // --------------------------------------------------------------------
        //  Constructors and Destructor
        // --------------------------------------------------------------------
        TWSEchoCfg();

        TWSEchoCfg(const TWSEchoCfg&) = delete;
        TWSEchoCfg(TWSEchoCfg&&) = delete;

        ~TWSEchoCfg();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWSEchoCfg& operator=(const TWSEchoCfg&) = delete;
        TWSEchoCfg& operator=(TWSEchoCfg&&) = delete;


        // --------------------------------------------------------------------
        //  Public, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bCheckKey
        (
            const   TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bLoaded() const;

        tCIDLib::TBoolean bLoadMaps
        (
            const   tCIDLib::TBoolean       bForce
        );

        tCIDLib::TBoolean bMapPhrase
        (
            const   TString&                strIntentType
            , const TString&                strPhraseWeGot
            ,       TString&                strPreferredPhrase
            ,       TString&                strPath
            ,       tCIDLib::TBoolean&      bIsMacro
        )   const;


    private :
        // --------------------------------------------------------------------
        //  Private data members
        // --------------------------------------------------------------------
        using TCfgMap = TVector<TWSEchoCfgBlock>;


        // --------------------------------------------------------------------
        //  Private, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bCheckLine
        (
                    TTextInStream&          strmSrc
            , const TString&                strToCheck
        );

        tCIDLib::TBoolean bReadLine
        (
                    TTextInStream&          strmSrc
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bEndOK
        );

        tCIDLib::TVoid CheckKey
        (
                    TTextInStream&          strmSrc
            , const TString&                strToCheck
            ,       TString&                strValue
            , const tCIDLib::TBoolean       bEmptyOk
        );

        tCIDLib::TVoid CheckKey2
        (
                    TTextInStream&          strmSrc
            , const TString&                strToCheck1
            , const TString&                strToCheck2
            ,       TString&                strKey
            ,       TString&                strValue
            , const tCIDLib::TBoolean       bEmptyOk
        );

        tCIDLib::TVoid CheckLine
        (
                    TTextInStream&          strmSrc
            , const TString&                strToCheck
        );

        tCIDLib::TVoid LoadMap
        (
                    TTextInStream&          strmSrc
            ,       TCfgMap&                colToFill
            ,       tCIDLib::TStrList&      colTmpList
            , const TString&                strMapName
        );

        tCIDLib::TVoid Reset();


        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMapLoaded
        //      We fault in our mapping file upon first need, so as to avoid
        //      doing work that will never be used.
        //
        //  m_c4LineNum
        //      For use during parsing of the config file.
        //
        //  m_colKeys
        //      The keys from the file, which we expect to see in incoming msgs from
        //      the Echo server.
        //
        //  m_colQueryMap
        //  m_colRunMap
        //  m_colSetMap
        //      We have a map for each of the major types of requests from the Echo
        //      server.
        //
        //  m_mtxSync
        //      We have to sync access this object.
        //
        //  m_strPushback
        //      For use during parsing of the config file.
        // --------------------------------------------------------------------
        tCIDLib::TBoolean   m_bMapLoaded;
        tCIDLib::TCard4     m_c4LineNum;
        tCIDLib::TStrList   m_colKeys;
        TCfgMap             m_colQueryMap;
        TCfgMap             m_colRunMap;
        TCfgMap             m_colSetMap;
        TMutex              m_mtxSync;
        TString             m_strPushback;
};



// ---------------------------------------------------------------------------
//   CLASS: TWSEchoHandler
//  PREFIX: urlh
// ---------------------------------------------------------------------------
class TWSEchoHandler : public TWSURLHandler
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TWSEchoHandler();

        TWSEchoHandler(const TWSEchoHandler&) = delete;
        TWSEchoHandler(TWSEchoHandler&&) = delete;

        ~TWSEchoHandler();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWSEchoHandler& operator=(const TWSEchoHandler&) = delete;
        TWSEchoHandler& operator=(TWSEchoHandler&&) = delete;


        // --------------------------------------------------------------------
        //  Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TCard4 c4ProcessURL
        (
            const   TURL&                   urlReq
            , const TString&                strType
            , const tCIDLib::TKVPList&      colInHdrLines
            ,       tCIDLib::TKVPList&      colOutHdrLines
            , const tCIDLib::TKVPList&      colPQVals
            , const TString&                strEncoding
            , const TString&                strBoundary
            ,       THeapBuf&               mbufToFill
            ,       tCIDLib::TCard4&        c4ContLen
            ,       TString&                strContType
            ,       TString&                strRepText
        )   final;


    private :
        // --------------------------------------------------------------------
        //  Private, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bDoEchoOp
        (
            const   TString&                strIntentType
            , const TString&                strReportPhrase
            , const TString&                strPath
            , const tCIDLib::TBoolean       bIsMacro
            , const TString&                strEchoValue
            , const TString&                strEchoKey
            ,       TString&                strRepText
        );

        tCIDLib::TCard4 c4MakeReply
        (
            const   TString&                strText
            ,       TMemBuf&                mbufToFill
        );


        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_ctarGlobalVars
        //      We need a global variables object to pass in when doing actions. There
        //      is no actual carryover, they are reset each time. And we don't need it
        //      to be thread safe.
        //
        //  m_jprsData
        //      A JSON parser to parse out the incmoing data from the Amazon
        //      server.
        // --------------------------------------------------------------------
        TStdVarsTar m_ctarGlobalVars;
        TJSONParser m_jprsComm;


        // --------------------------------------------------------------------
        //  Private static methods
        //
        //  s_wsecMaps
        //      This is the mapping configuration that we load from the config file. It's
        //      a static object because it must be shared among all echo handler instances.
        //      Each thread creates its own handlers, but we need to have a single shared
        //      loaded configuration.
        // --------------------------------------------------------------------
        static TWSEchoCfg       s_wsecMaps;


        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TWSEchoHandler,TWSURLHandler)
};

#pragma CIDLIB_POPPACK


