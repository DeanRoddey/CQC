//
// FILE NAME: CQCBulkLoader_ThisFacility3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2010
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
//  This file handles some of the grunt work of the facility class, keeping
//  the main file a little more reasonable and readable. We have the stuff
//  here related to the validation of the configuration file data that we
//  have already loaded, and finishing the setup of the data based on what
//  we find out.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CQCBulkLoader.hpp"




// ---------------------------------------------------------------------------
//  TFacCQCBulkLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TFacCQCBulkLoader::bValidateCfg()
{
    // If there are any drivers, let's deal with those
    if (m_colDrivers.c4ElemCount())
    {
        m_strmOut << L"Validating drivers..." << kCIDLib::EndLn;
        if (!bValidateDrivers())
            return kCIDLib::False;

        //
        //  If we had drivers, then they are to be loaded on hosts that are
        //  running CQCServer. So we kept a unique list of hosts during
        //  the parse so that we can check to see that we can contact the
        //  CQC Server admin interface on each one.
        //
        if (!bValidateHosts())
            return kCIDLib::False;
    }

    // Validate the event info
    if (!bValidateEvents())
        return kCIDLib::False;

    // Validate the image info
    if (!bValidateImages())
        return kCIDLib::False;

    return kCIDLib::True;
}



tCIDLib::TBoolean TFacCQCBulkLoader::bValidateDrivers()
{
    // We need an installation server proxy for this
    tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

    const tCIDLib::TCard4   c4Count = m_colDrivers.c4ElemCount();
    TCQCDriverCfg           cqcdcCur;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TBLDrvInfo& drviCur = m_colDrivers[c4Index];

        //
        //  Let's see if we can download the driver manifest info for this one. This
        //  is the static part of the driver config from the manifest file
        //
        if (!orbcIS->bQueryDrvManifest(drviCur.strMake(), drviCur.strModel(), cqcdcCur))
        {
            m_strmOut << L"\n!!Could not find manifest info for driver make/model '"
                      << drviCur.strMake() << L"/" << drviCur.strModel()
                      << L"'" << kCIDLib::EndLn;
            return kCIDLib::False;
        }

        //
        //  Set it on the driver object, and get back a ref to the full
        //  driver instance configuration, of which the passed in manifest
        //  level info is the base class. This is what we'll fill in with
        //  the prompt info.
        //
        TCQCDriverObjCfg& cqcdcTar = drviCur.cqcdcSetManifest(cqcdcCur);

        // Store the moniker
        cqcdcTar.strMoniker(drviCur.strMoniker());

        //
        //  Based on the type of configuration this driver uses, we need to
        //  create a connection config object and set up the connection info
        //  and store it on the config object.
        //
        TCQCConnCfg* pconncfgCur = ::pDupObject<TCQCConnCfg>(cqcdcTar.conncfgDef());
        TJanitor<TCQCConnCfg> janConnCfg(pconncfgCur);

        if (!pconncfgCur->bParseConnInfo(drviCur.strConnInfo()))
        {
            m_strmOut << L"\n!!Connection info for driver '"
                      << drviCur.strMake() << L"' was not valid\n"
                      << kCIDLib::EndLn;
            return kCIDLib::False;
        }

        //
        //  Set it on the driver info object now. It doesn't adopt, it makes
        //  a copy. This is actually not what this method is for. It's for
        //  notifying the outer config class of a new default conn config
        //  being set during the manifest parsing phase in the base class.
        //  But we use it to force in a new connection configuration.
        //
        cqcdcTar.NewConnCfg(pconncfgCur);

        //
        //  Now if there are any prompts, we can go through and set them and
        //  make sure that they are valid based on the info we have.
        //
        const tCIDLib::TCard4 c4PCount = drviCur.c4PromptCount();
        for (tCIDLib::TCard4 c4PIndex = 0; c4PIndex < c4PCount; c4PIndex++)
        {
            const TBLPromptInfo& piCur = drviCur.piAt(c4PIndex);

            //
            //  Make sure that the driver defines this prompt key, and if
            //  the value we were given for the sub-key is correct. It will]
            //  set the prompt values on the driver config object
            //
            if (!bValidatePrompt(drviCur.strMoniker(), piCur, cqcdcTar))
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  We go through the list of events and make sure that the referenced paths exist.
//
//
//  We'll run through the events and make sure that all of the source events are
//  findable. We have a main method that loops through all of the defined trees, and
//  a recursive helper that handles the actual tree validation.
//
tCIDLib::TBoolean TFacCQCBulkLoader::bValidateEvents()
{
    const tCIDLib::TCard4 c4BlkCount = m_colEvents.c4ElemCount();

    // If there aren't any events, then nothing to do
    if (!c4BlkCount)
        return kCIDLib::True;

    //
    //  We need a path that we'll add to and trim back as we move through the hierarchy.
    //  The root block of each block of events will have the starting path.
    //
    TPathStr pathSrcDir;
    TString  strTarScope;
    for (tCIDLib::TCard4 c4BlkIndex = 0; c4BlkIndex < c4BlkCount; c4BlkIndex++)
    {
        const TBLEvScopeInfo& scpiRoot = m_colEvents[c4BlkIndex];

        // Make sure that this top level target is a path that starts with /User/
        if (!scpiRoot.strTarScope().bStartsWithI(m_strUserPref))
        {
            m_strmOut << L"!!The top level event scope '" << scpiRoot.strTarScope()
                      << L"' does not start with /User/\n" << kCIDLib::EndLn;
            return kCIDLib::False;
        }
        strTarScope = scpiRoot.strTarScope();

        // Looks reasonable to get the source path and validate this tree
        pathSrcDir = scpiRoot.strName();
        if (!bValidateEventTree(scpiRoot, pathSrcDir, strTarScope, scpiRoot.bIsSchedEv()))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TFacCQCBulkLoader::bValidateEventTree(  const   TBLEvScopeInfo&     scpiParent
                                        ,       TPathStr&           pathSrcDir
                                        ,       TString&            strTarScope
                                        , const tCIDLib::TBoolean   bTrigEv)
{
    // Make sure that the starting directory exists
    if (!TFileSys::bIsDirectory(pathSrcDir))
    {
        m_strmOut << L"!!Source event path '" << pathSrcDir
                  << L" does not exist\n" << kCIDLib::EndLn;
        return kCIDLib::False;
    }

    //
    //  OK, let's start enumerating this scope level. Save the current path length, so
    //  that we can cap it back to here when we get back up to this level again.
    //
    const tCIDLib::TCard4 c4SrcLen = pathSrcDir.c4Length();
    const tCIDLib::TCard4 c4TarLen = strTarScope.c4Length();

    // Do any sub-directories first
    const tCIDLib::TCard4 c4SubDirCnt = scpiParent.c4SubDirCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SubDirCnt; c4Index++)
    {
        const TBLEvScopeInfo& scpiCur = scpiParent.scpiAt(c4Index);

        //
        //  Build up the source and target paths for this level. AT this level they
        //  both just use the name to create similarly named trees at the source and
        //  target levels.
        //
        pathSrcDir.CapAt(c4SrcLen);
        pathSrcDir.AddLevel(scpiCur.strName());
        strTarScope.CapAt(c4TarLen);
        facCQCRemBrws().AddPathComp(strTarScope, scpiCur.strName());

        // And now recurse on this guy
        if (!bValidateEventTree(scpiCur, pathSrcDir, strTarScope, bTrigEv))
            return kCIDLib::False;
    }

    // Validate any events defined at this level
    TString strExt;
    const tCIDLib::TCard4 c4EvCnt = scpiParent.c4EvCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4EvCnt; c4Index++)
    {
        const TBLEventInfo& eviCur = scpiParent.eviAt(c4Index);

        pathSrcDir.CapAt(c4SrcLen);
        pathSrcDir.AddLevel(eviCur.strName());
        strTarScope.CapAt(c4TarLen);
        facCQCRemBrws().AddPathComp(strTarScope, eviCur.strName());

        // Append the correct extension to the source file
        pathSrcDir.AppendExt(facCQCRemBrws().strFlTypeExt(scpiParent.eType()));

        if (!TFileSys::bExists(pathSrcDir))
        {
            m_strmOut << L"!!Source Event '" << pathSrcDir
                      << L" does not exist\n" << kCIDLib::EndLn;
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}



//
//  We go through the list of hosts that any of the configured drivers asked
//  to be loaded on, and check that we can contact the CQC admin interface
//  on those hosts. Actually we just make sure that the hosts are all
//  registered in the name server.
//
tCIDLib::TBoolean TFacCQCBulkLoader::bValidateHosts()
{
    TDupList::TCursor cursHosts(&m_colHosts);
    if (cursHosts.bReset())
    {
        // We need a name server proxy for this
        TOrbObjId               ooidServer;
        TString                 strBinding;
        do
        {
            const TString& strCurHost = cursHosts.objRCur();

            strBinding = TCQCSrvAdminClientProxy::strAdminScope;
            strBinding.Append(kCIDLib::pszTreeSepChar);
            strBinding.Append(strCurHost);

            if (!facCIDOrbUC().bGetNSObject(strBinding, ooidServer))
            {
                m_strmOut << L"!!Target host '" << strCurHost
                          << L" does not appear to be running CQCServer\n"
                          << kCIDLib::EndLn;
                return kCIDLib::False;
            }
        }   while (cursHosts.bNext());
    }
    return kCIDLib::True;
}


//
//  We'll run through the images and make sure that all of the source
//  images are findable. We have a main method that loops through all of
//  the defined trees, and a recursive helper that handles the actual
//  tree validation.
//
tCIDLib::TBoolean TFacCQCBulkLoader::bValidateImages()
{
    const tCIDLib::TCard4 c4BlkCount = m_colImages.c4ElemCount();

    // If there aren't any images, then nothing to do
    if (!c4BlkCount)
        return kCIDLib::True;

    //
    //  We need a path that we'll add to and trim back as we move through
    //  the hierarchy. The root block of each block of images will have
    //  the starting path.
    //
    TPathStr pathSrcDir;
    TString  strTarScope;
    for (tCIDLib::TCard4 c4BlkIndex = 0; c4BlkIndex < c4BlkCount; c4BlkIndex++)
    {
        const TBLImgScopeInfo& scpiRoot = m_colImages[c4BlkIndex];

        // Make sure that the target scope starts with /User/ at this level
        if (!scpiRoot.strTarScope().bStartsWithI(m_strUserPref))
        {
            m_strmOut << L"!!The top level image scope '" << scpiRoot.strTarScope()
                      << L"' does not start with /User/\n" << kCIDLib::EndLn;
            return kCIDLib::False;
        }
        strTarScope = scpiRoot.strTarScope();

        // Looks reasonable to get the source path and validate this tree
        pathSrcDir = scpiRoot.strName();
        if (!bValidateImageTree(scpiRoot, pathSrcDir, strTarScope))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TFacCQCBulkLoader::bValidateImageTree(  const   TBLImgScopeInfo&    scpiParent
                                        ,       TPathStr&           pathSrcDir
                                        ,       TString&            strTarScope)
{
    // Make sure that the starting directory exists
    if (!TFileSys::bIsDirectory(pathSrcDir))
    {
        m_strmOut << L"!!Source image path '" << pathSrcDir
                  << L" does not exist\n" << kCIDLib::EndLn;
        return kCIDLib::False;
    }

    //
    //  OK, let's start enumerating this scope level. Save the current
    //  path lengths.
    //
    const tCIDLib::TCard4 c4SrcLen = pathSrcDir.c4Length();
    const tCIDLib::TCard4 c4TarLen = strTarScope.c4Length();

    // Do any sub-directories first
    const tCIDLib::TCard4 c4SubDirCnt = scpiParent.c4SubDirCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SubDirCnt; c4Index++)
    {
        const TBLImgScopeInfo& scpiCur = scpiParent.scpiAt(c4Index);

        //
        //  Build up the source and target paths for this level. AT this level they
        //  both just use the name to create similarly named trees at the source and
        //  target levels.
        //
        pathSrcDir.CapAt(c4SrcLen);
        pathSrcDir.AddLevel(scpiCur.strName());
        strTarScope.CapAt(c4TarLen);
        facCQCRemBrws().AddPathComp(strTarScope, scpiCur.strName());

        // And now recurse on this guy
        if (!bValidateImageTree(scpiCur, pathSrcDir, strTarScope))
            return kCIDLib::False;
    }

    // And do any images at this level
    TString strExt;
    const tCIDLib::TCard4 c4ImgCnt = scpiParent.c4ImgCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ImgCnt; c4Index++)
    {
        const TBLImageInfo& imgiCur = scpiParent.imgiAt(c4Index);

        pathSrcDir.CapAt(c4SrcLen);
        pathSrcDir.AddLevel(imgiCur.strName());
        strTarScope.CapAt(c4TarLen);
        facCQCRemBrws().AddPathComp(strTarScope, imgiCur.strName());

        if (!TFileSys::bExists(pathSrcDir))
        {
            m_strmOut << L"!!Source image '" << pathSrcDir
                      << L" does not exist\n" << kCIDLib::EndLn;
            return kCIDLib::False;
        }

        // Make sure it has a valid extension
        if (!pathSrcDir.bQueryExt(strExt)
        ||  (!strExt.bCompareI(L"JPEG")
        &&   !strExt.bCompareI(L"JPG")
        &&   !strExt.bCompareI(L"PNG")
        &&   !strExt.bCompareI(L"BMP")))
        {
            m_strmOut << L"!!Source image '" << pathSrcDir
                      << L" does not have a valid extension\n" << kCIDLib::EndLn;
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Does some validation on driver prompts. We make sure that the driver
//  defines such a prompt, and then test the subkey/value pairs.
//
tCIDLib::TBoolean
TFacCQCBulkLoader::bValidatePrompt( const   TString&            strMoniker
                                    , const TBLPromptInfo&      piTest
                                    ,       TCQCDriverObjCfg&   cqcdcTar)
{
    // First make sure that the driver has such a prompt
    const TCQCDCfgPrompt* pcqcdcpTest = pcqcdcpByKey
    (
        strMoniker, cqcdcTar, piTest.m_strKey
    );

    if (!pcqcdcpTest)
    {
        m_strmOut << L"\n!!Driver '" << strMoniker << L"' does not have a "
                     L"prompt named '" << piTest.m_strKey << L"'"
                  << kCIDLib::EndLn;
        return kCIDLib::False;
    }

    //
    //  Validate the sub-key values
    //
    //  <TBD> We need to provide some validation in the base driver config
    //  class via a static or something, move any validation in the driver
    //  install wizard to that, and use it here and in the driver install
    //  wizard.
    //
    //  For now we can't really do much validation and wouldn't wnat to
    //  replicate that functionality in more than one place anyway.
    //

    // It looks ok, so store the values
    const tCIDLib::TCard4 c4SubKeyCnt = piTest.m_colSubKeys.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SubKeyCnt; c4Index++)
    {
        const TKeyValuePair& kvalCur = piTest.m_colSubKeys[c4Index];
        try
        {
            cqcdcTar.SetPromptVal(piTest.m_strKey, kvalCur.strKey(), kvalCur.strValue());
        }

        catch(const TError& errToCatch)
        {
            m_strmOut << L"!!Failed to set prompt subkey '" << piTest.m_strKey
                      << L"." << kvalCur.strKey()
                      << L"' for driver '" << strMoniker
                      << L"'\n    " << errToCatch.strErrText()
                      << kCIDLib::EndLn;
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Looks through the prompts in the passed driver config and returns the
//  one with the indicated key. If not found, it throws.
//
const TCQCDCfgPrompt*
TFacCQCBulkLoader::pcqcdcpByKey(const   TString&        strMoniker
                                , const TCQCDriverCfg&  cqcdcSrc
                                , const TString&        strKey)
{
    const tCIDLib::TCard4 c4Count = cqcdcSrc.c4PromptCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(c4Index);
        if (cqcdcpCur.strName() == strKey)
            return &cqcdcpCur;
    }
    return 0;
}

