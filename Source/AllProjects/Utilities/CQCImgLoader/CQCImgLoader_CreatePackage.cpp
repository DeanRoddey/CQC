//
// FILE NAME: CQCImgLoader_CreatePackage.cpp
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
//  to the creation of bulk image packs.
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



// ----------------------------------------------------------------------------
//  TFacCQCImgLoader: Private, non-virtual methods
// ----------------------------------------------------------------------------

tCIDLib::TVoid
TFacCQCImgLoader::RecursiveImgPack( const   TXMLTreeElement&    xtnodeScope
                                    ,       TCQCPackage&        packTar
                                    ,       TPathStr&           pathSrcRoot
                                    ,       THeapBuf&           mbufImg)
{
    TString strTmp1, strTmp2;

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

            // Build expand the source path for the next level and recurse
            TPathExpJan janSrc(&pathSrcRoot, strChild);
            RecursiveImgPack(xtnodeChild, packTar, janSrc.pathExp(), mbufImg);
        }
         else
        {
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
                    return;
            }
            TPoint pntTransAt(strTmp1.i4Val(), strTmp2.i4Val());

            // Load it up as an image
            {
                TPathStr pathImg(pathSrcRoot);
                pathImg.AddLevel(strName);
                TBinFileInStream strmSrc;
                strmSrc.Open
                (
                    pathImg
                    , tCIDLib::ECreateActs::OpenIfExists
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );
                strmSrc >> m_imgiLoad;
            }


            // Build up the name of the image in the repository
            TPathStr pathRepoName;
            pathRepoName.CopyInSubStr(pathSrcRoot, m_pathInFile.c4Length());
            pathRepoName.AddLevel(strName);
            pathRepoName.bRemoveExt();
            pathRepoName.bReplaceChar(kCIDLib::chBackSlash, kCIDLib::chForwardSlash);

            // Create a chunked file for this image
            m_chflImg.Reset();
            facCQCRemBrws().ChunkUpImage(m_imgiLoad, m_chflImg);

            // And add that as the image data for this file
            packTar.c4AddFile
            (
                tCQCKit::EPackFlTypes::PNGImage
                , pathRepoName
                , m_chflImg
                , kCIDLib::False
                , kCIDLib::False
            );
        }
    }
}



//
//  We assume that m_pathInFile points to the XML file that describes the
//  images to put into the package, and that m_pathOutFile is the path to
//  the package file to create. m_pathInFile points to the top level directory
//  of the package content. This file will have a Package.xml file in it and
//  under it will be System and/or User directories that represent the images
//  repository postions of the files. So by taking the image repository path
//  in the xml file and prepending this path, it will provide the path to
//  the source file.
//
tCIDLib::TVoid TFacCQCImgLoader::CreatePackage()
{
    //
    //  Make sure the output path exists, else ask if they want to create
    //  it. It can't be empty.
    //
    if (m_pathOutFile.bIsEmpty())
    {
        m_conOut << L"The /OutFile= parameter must be provided to indicate\n"
                    L"the output package file name." << kCIDLib::EndLn;
        return;
    }

    //
    //  See if the source file is there and parse it and see if all the source
    //  files exist.
    //
    TPathStr pathXML(m_pathInFile);
    pathXML.AddLevel(L"Package.Xml");
    TXMLTreeParser xtprsImages;
    try
    {
        // Try to parse the file and get a tree of the information
        tCIDXML::TEntitySrcRef esrSrc(new TFileEntitySrc(pathXML));

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

        // If any files weren't found, then give up
        if (!bFindSysImages(xtprsImages.xtdocThis().xtnodeRoot(), m_pathInFile))
            return;
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Error while verifying source files\n"
                 << errToCatch << kCIDLib::EndLn;
    }

    //
    //  If the target file doesn't exist, make sure they want to create it.
    //  If it does, make sure they want to overwrite it.
    //
    TPathStr pathPackFl(m_pathOutFile);
    pathPackFl.AppendExt(L"CQCImagePack");
    if (TFileSys::bExists(pathPackFl))
    {
        m_conOut << L"Output file '" << pathPackFl << L"' already exists\n"
                    L"Overwrite it?(Y/N): " << kCIDLib::FlushIt;
    }
     else
    {
        m_conOut << L"Output file '" << pathPackFl << L"' does not exist\n"
                    L"Create it?(Y/N): " << kCIDLib::FlushIt;
    }

    TString strTmp;
    m_conIn  >> strTmp;
    strTmp.ToUpper();
    if (strTmp != L"Y")
        return;

    // Make sure the path part exists
    TPathStr pathBase(m_pathOutFile);
    pathBase.bRemoveNameExt();
    TFileSys::MakePath(pathBase);

    //
    //  Ok, we are ready to do this. So create the package object and set it
    //  up with
    //
    TCQCPackage packOut
    (
        tCQCKit::EPackTypes::ImgBundle
        , facCQCKit().c8MakeVersion
          (
            kCQCKit::c4MajVersion
            , kCQCKit::c4MinVersion
            , kCQCKit::c4Revision
          )
    );


    try
    {
        //
        //  Provide a temp buffer for loading images. It just gets passed
        //  down through the recursive layers, to avoid the need to create
        //  a new one on every new scope entry.
        //
        //  We also need a copy of the source directory, since this guy
        //  changes it as it recurses and we need to keep the original
        //  member unchanged.
        //
        TPathStr pathSrcRoot(m_pathInFile);
        THeapBuf mbufImg(0x20000, kCIDLib::c4DefMaxBufferSz);
        RecursiveImgPack
        (
            xtprsImages.xtdocThis().xtnodeRoot()
            , packOut
            , pathSrcRoot
            , mbufImg
        );

        // Now we can create the actual package file
        TCQCPackage::MakePackage(pathPackFl, packOut);
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Error while packaging source files\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}


