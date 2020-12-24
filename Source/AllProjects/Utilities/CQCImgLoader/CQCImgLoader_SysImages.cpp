//
// FILE NAME: CQCImgLoader_SysImages.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2006
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
//  This file holds the private methods of the facilty class that are related
//  to the generation of system images.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCImgLoader.hpp"




// ---------------------------------------------------------------------------
//  TFacCQCImgLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TFacCQCImgLoader::bRecurseSysImages(const   TXMLTreeElement&    xtnodeScope
                                    ,       TPathStr&           pathSrcRoot
                                    ,       TPathStr&           pathTarRoot)
{
    tCIDLib::TCard4 c4At;
    TString         strTmp1, strTmp2;

    //
    //  If we have a prefixes attribute, get it out and break it into it's
    //  individual values.
    //
    TVector<TString> colPrefixes;
    if (xtnodeScope.bAttrExists(L"IntfImg:Prefixes", c4At))
    {
        const TString& strPrefixes(xtnodeScope.xtattrAt(c4At).strValue());
        TStringTokenizer stokPref(&strPrefixes, L", ");
        TString strTok;
        while (stokPref.bGetNextToken(strTok))
            colPrefixes.objAdd(strTok);
    }

    //
    //  Iterate the chidren of this node. If we hit a scope, then we recurse
    //  on that node. If we hit images, we process them.
    //
    const tCIDLib::TCard4 c4ChildCount = xtnodeScope.c4ChildCount();
    for (tCIDLib::TCard4 c4ChildInd = 0; c4ChildInd < c4ChildCount; c4ChildInd++)
    {
        // Get the next element
        const TXMLTreeElement& xtnodeChild
        (
            xtnodeScope.xtnodeChildAtAsElement(c4ChildInd)
        );

        if (xtnodeChild.strQName() == L"IntfImg:Scope")
        {
            const TString& strChild
                        = xtnodeChild.xtattrNamed(L"IntfImg:Name").strValue();

            // If the target child dir doesn't exist, then create it
            if (!TFileSys::bExists(pathTarRoot, strChild))
                TFileSys::MakeSubDirectory(strChild, pathTarRoot);

            // Build expand the source and target and scope
            TPathExpJan janSrc(&pathSrcRoot, strChild);
            TPathExpJan janTar(&pathTarRoot, strChild);

            if (!bRecurseSysImages( xtnodeChild, janSrc.pathExp(), janTar.pathExp()))
                return kCIDLib::False;
        }
         else
        {
            //
            //  If we have a prefixes list, then we have to loop through
            //  this file for each prefix, else the name is literal and we
            //  do it once.
            //
            const TString& strName(xtnodeChild.xtattrNamed(L"IntfImg:Name").strValue());

            // Get the transparency info
            const tCIDLib::TBoolean bTrans
            (
                xtnodeChild.xtattrNamed(L"IntfImg:Trans").bValueAs()
            );

            const TString& strTransAt(xtnodeChild.xtattrNamed(L"IntfImg:TransAt").strValue());
            TStringTokenizer stokAt(&strTransAt, L", ");
            if (!stokAt.bGetNextToken(strTmp1) || !stokAt.bGetNextToken(strTmp2))
            {
                m_conOut << L"Image '" << strName << L"' in path '"
                         << pathSrcRoot << L"' has a bad transparency point. "
                         << L"Continue?(Y/N)" << kCIDLib::FlushIt;

                m_conIn >> strTmp1;
                strTmp1.ToUpper();
                if (strTmp1 != L"Y")
                    return kCIDLib::False;
            }
            TPoint pntTransAt(strTmp1.i4Val(), strTmp2.i4Val());

            if (colPrefixes.bIsEmpty())
            {
                GenAnImage
                (
                    pathSrcRoot
                    , pathTarRoot
                    , strName
                    , bTrans
                    , pntTransAt
                );
            }
             else
            {
                const tCIDLib::TCard4 c4PrefCount = colPrefixes.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PrefCount; c4Index++)
                {
                    TString strActualName(colPrefixes[c4Index]);
                    strActualName.Append(strName);
                    GenAnImage
                    (
                        pathSrcRoot
                        , pathTarRoot
                        , strActualName
                        , bTrans
                        , pntTransAt
                    );
                }
            }
        }
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TFacCQCImgLoader::GenAnImage(const  TPathStr&           pathSrcRoot
                            , const TPathStr&           pathTarRoot
                            , const TString&            strName
                            , const tCIDLib::TBoolean   bTrans
                            , const TPoint&             pntTransClrAt)
{
    // Build up the path to the image file
    TPathStr pathImg(pathSrcRoot);
    pathImg.AddLevel(strName);

    // Build up the name of the image in the repository
    TPathStr pathRepoName;
    pathRepoName.CopyInSubStr(pathTarRoot, m_pathBase.c4Length());
    pathRepoName.AddLevel(strName);
    pathRepoName.bRemoveExt();
    m_conOut << L"   Generating image: " << pathRepoName << kCIDLib::EndLn;

    // Build up the path to the target file
    TPathStr pathTar(pathTarRoot);
    pathTar.AddLevel(strName);

    //  Stream in the image and convert to PNG
    facCIDImgFact().bLoadToPNG(pathImg, m_imgiLoad, kCIDLib::True);

    // If it has color based transparency, set that on the image
    m_imgiLoad.bTransClr(bTrans);
    if (bTrans)
        m_imgiLoad.c4TransClr(m_imgiLoad.c4ClrAt(pntTransClrAt));

    // Use a helper to convert to the appropriate chunked file format
    facCQCRemBrws().ChunkUpImage(m_imgiLoad, m_chflImg, 1);

    //
    //  Now we can stream this object out to the target path. For this
    //  operation, the global pathOutFile value points to the top of the
    //  output tree. So we add the target image scope to this base path
    //  to create the output path.
    //
    pathImg = pathTarRoot;
    pathImg.AddLevel(strName);

    // In the target replace the original image extension with our image extension
    pathImg.bRemoveExt();
    facCQCRemBrws().AppendTypeExt(pathImg, tCQCRemBrws::EDTypes::Image);
    TBinFileOutStream strmTar
    (
        pathImg
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
    );
    strmTar << m_chflImg << kCIDLib::FlushIt;
}



//
//  We assume that pathInFile points to the XML file that describes the images
//  to generate, and that pathOutFile contains the top level directory of the
//  CQC source tree.
//
tCIDLib::TVoid TFacCQCImgLoader::GenSystemImages()
{
    //
    //  Make sure the output path exists, else ask if they want to create
    //  it. It can't be empty.
    //
    if (m_pathOutFile.bIsEmpty())
    {
        m_conOut << L"The /OutFile= parameter must be provided to indicate\n"
                    L"the CQCData directory" << kCIDLib::EndLn;
        return;
    }

    //
    //  Build the path to the actual output directory. We only generate System images.
    //  The user provided us an output 'file' which in our case should be the CQCData
    //  directory. So we need to provide the rest of the path.
    //
    TPathStr pathOutDir(m_pathOutFile);
    pathOutDir.AddLevels(L"DataServer", L"Images", L"System");

    if (!TFileSys::bExists(pathOutDir))
    {
        m_conOut << L"Output path '" << pathOutDir << L"' does not exist\n"
                    L"Create it?(Y/N): " << kCIDLib::FlushIt;
        TString strTmp;
        m_conIn  >> strTmp;
        strTmp.ToUpper();
        if (strTmp != L"Y")
            return;
        TFileSys::MakePath(pathOutDir);
    }

    if (!TFileSys::bIsDirectory(pathOutDir))
    {
        m_conOut << L"'" << pathOutDir << L"' is not a directory"
                 << kCIDLib::EndLn;
        return;
    }

    try
    {
        // Try to parse the file and get a tree of the information
        tCIDXML::TEntitySrcRef esrSrc(new TFileEntitySrc(m_pathInFile));

        TXMLTreeParser xtprsImages;
        const tCIDLib::TBoolean bOk = xtprsImages.bParseRootEntity
        (
            esrSrc
            , tCIDXML::EParseOpts::Validate
            , tCIDXML::EParseFlags::Tags
        );

        if (!bOk)
        {
            m_conOut << L"System image generation failed\n";
            const TXMLTreeParser::TErrInfo& erriFirst
                                            = xtprsImages.colErrors()[0];
            m_conOut << L"[" << erriFirst.strSystemId() << L"/"
                     << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                     << L"] " << erriFirst.strText() << kCIDLib::EndLn;
            return;
        }

        //
        //  Before we spend all the time to do this, run through and make sure
        //  that all of the source images can be found. If we find them, then
        //  go for it.
        //
        //
        //  Get the path that the input file is in, plus a System subdirectory,
        //  which is the top of the tree we will process.
        //
        TPathStr pathSrcTree(m_pathInFile);
        pathSrcTree.bRemoveLevel();
        pathSrcTree.AddLevel(L"System");

        if (bFindSysImages(xtprsImages.xtdocThis().xtnodeRoot(), pathSrcTree))
        {
            //
            //  Get the root element. Given this, we can now call a recursive
            //  helper method which will recurse the tree structure and do all
            //  of the actual work.
            //
            bRecurseSysImages
            (
                xtprsImages.xtdocThis().xtnodeRoot(), pathSrcTree, pathOutDir
            );
        }
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"System image generation failed\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}


