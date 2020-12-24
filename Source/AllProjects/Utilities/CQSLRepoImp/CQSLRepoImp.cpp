//
// FILE NAME: CQSLRepoImp.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/15/2006
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
//  This is the main module for the program. This one is a utility that will
//  import J.River and DVD Profiler repository information into the CQSL
//  repository.
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
#include    "CQSLRepoImp.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc
(
        TThread&            thrThis
        , tCIDLib::TVoid*   pData
);


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"CQSLRepoImpMainThread", eMainThreadFunc))


// ---------------------------------------------------------------------------
//  Global data
// ---------------------------------------------------------------------------
tCIDLib::TBoolean   bTestMode = kCIDLib::False;
tCIDLib::TCard4     c4ErrCnt = 0;
tCIDLib::TCard4     c4MissingArtCnt = 0;
tCIDLib::TCard4     c4NoDiscCnt = 0;
tCIDLib::TCard4     c4SkippedCnt = 0;
TInConsole*         pconIn;
TOutConsole*        pconOut;



// ---------------------------------------------------------------------------
//  Local functions
// ---------------------------------------------------------------------------

//
//  This is the the thread function for the main thread.
//
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    // Point globals at our in/out streams
    TInConsole      conIn;
    TOutConsole     conOut;
    pconIn = &conIn;
    pconOut = &conOut;

    TString strTmp;
    try
    {
        //
        //  Ask CQCKit to load up environment info, which we'll need for
        //  almost anything.
        //
        TString strFailReason;
        if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
            return tCIDLib::EExitCodes::BadEnvironment;

        // Tell them what this is
        conOut << L"\n\nCQSL Repository Importer\n\n"
                  L"    <READ THIS>\n"
                  L"    This program should be used ONCE to load up your J.River and\n"
                  L"    DVD Profiler databases inot the CQSL Repository. If you run this\n"
                  L"    program twice, you will end up with all of the discs in the\n"
                  L"    repository twice and will probably have to delete it and start\n"
                  L"    over, or roll back a previous saved version to get rid of them.\n\n"
                  L"    - The media files will not be copied to the repository, so be sure\n"
                  L"    that the paths indicated in the source XML files are correct.\n\n"
                  L"    - Images will be imported into the target repository\n\n"
                  L"    - I can only import music from J.River and movies from DVD Profiler!\n\n"
                  L"    - I only work with DVD Profiler VERSION 3!\n\n"
               << kCIDLib::EndLn;

        if (!TRepoImpHelpers::bGetYNAnswer(L"Do you want to continue"))
            return tCIDLib::EExitCodes::Normal;

        // Initialize the client side of the ORB
        facCIDOrb().InitClient();
    }

    catch(const TError& errToCatch)
    {
        conOut  <<  L"An error occured during processing. "
                <<  L"\nError: " << errToCatch.strErrText()
                << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::FatalError;
    }

    //
    //  Since this is a demo and testing program, we'd like to catch
    //  all exceptions cleanly and report them. So put the whole thing
    //  in a try.
    //
    tCQSLRepoImp::TRepoClient   orbcRepoMgr;
    TString                     strLeaseId;
    try
    {
        //
        //  Ask them what repository driver we are going to be loading
        //  the metadata to and get a repository manager client back.
        //
        tCIDLib::TBoolean           bFileBased = kCIDLib::False;
        TString                     strRepoMoniker;
        TRepoImpHelpers::bGetRepo
        (
            L"Enter the moniker of the target repo driver"
            , strRepoMoniker
            , orbcRepoMgr

        );

        if (strRepoMoniker.bIsEmpty())
        {
            conOut  << L"\nNo target repository was provided. Exiting...\n"
                    << kCIDLib::EndLn;
            return tCIDLib::EExitCodes::Normal;
        }

        //
        //  Query the driver stats, both just to sanity check that we have
        //  a good connection, and to get the count of movie and music titles
        //  so that we can warn them if titles already exist.
        //
        tCIDLib::TCard4 c4MovieTitleCnt;
        tCIDLib::TCard4 c4MovieColCnt;
        tCIDLib::TCard4 c4MovieItemCnt;
        tCIDLib::TCard4 c4MovieArtCnt;
        tCIDLib::TCard4 c4MusicTitleCnt;
        tCIDLib::TCard4 c4MusicColCnt;
        tCIDLib::TCard4 c4MusicItemCnt;
        tCIDLib::TCard4 c4MusicArtCnt;
        TString         strRepoPath;
        orbcRepoMgr->QueryStats
        (
            c4MovieTitleCnt
            , c4MovieColCnt
            , c4MovieItemCnt
            , c4MovieArtCnt
            , c4MusicTitleCnt
            , c4MusicColCnt
            , c4MusicItemCnt
            , c4MusicArtCnt
            , strRepoPath
        );

        // Get the lease on the repository
        strLeaseId = TUniqueId::strMakeId();
        #if CID_DEBUG_ON
        if (!orbcRepoMgr->bGetLease(strLeaseId, kCIDLib::True))
        #else
        if (!orbcRepoMgr->bGetLease(strLeaseId, kCIDLib::False))
        #endif
        {
            conOut  << L"Some other repository manager seems to have the"
                       L"repository locked. Exiting..." << kCIDLib::EndLn;
            return tCIDLib::EExitCodes::Normal;
        }

        // Get the J.River XML file from them, if any
        TString strJRExpFile;
        TRepoImpHelpers::bGetFile
        (
            L"Path to J.River XML file", strJRExpFile, kCIDLib::True
        );

        // Get the DVD Profiler file from them, if any
        TString strDVDPExpFile;
        TRepoImpHelpers::bGetFile
        (
            L"Path to DVDP XML file", strDVDPExpFile, kCIDLib::True
        );

        // If we got neither file, tell them we can't do it
        if (strJRExpFile.bIsEmpty() && strDVDPExpFile.bIsEmpty())
        {
            conOut  << L"\nNo src file was provided to import. Exiting...\n"
                    << kCIDLib::EndLn;
            return tCIDLib::EExitCodes::Normal;
        }

        if (!strJRExpFile.bIsEmpty())
        {
            // If there are music sets in place, then warn them
            if (c4MusicTitleCnt)
            {
                if (!TRepoImpHelpers::bGetYNAnswer(L"There are existing music titles. Continue anyway?"))
                    return tCIDLib::EExitCodes::Normal;
            }
        }

        TString strMovieImgPath;
        if (!strDVDPExpFile.bIsEmpty())
        {
            // If there are movie sets in place, then warn them
            if (c4MovieTitleCnt)
            {
                if (!TRepoImpHelpers::bGetYNAnswer(L"There are existing movie titles. Continue anyway?"))
                    return tCIDLib::EExitCodes::Normal;
            }

            // For DVDP, we need to get the image path
            conOut  << L"Where is the DVD Profiler image directory? Point me at the\n"
                       L"the high quality images, not the thumbnails. I'll reduce them\n"
                       L"during the import, to end up with the best quality cover art\n"
                    << kCIDLib::EndLn;

            TRepoImpHelpers::bGetPath
            (
                L"DVDP Image Path", strMovieImgPath, kCIDLib::True
            );

            // Ask if it is file based or changer based
            bFileBased = !TRepoImpHelpers::bGetYNAnswer
            (
                "To help me out, are your DVDs in a changer?"
            );
        }

        //
        //  OK, we have the info we need. So let's show them the options
        //  and ask if they want to continue.
        //
        conOut << L"\nOK. I have the information I need, which is:\n"
               << L"\n     J.River XML: " << strJRExpFile
               << L"\n    DVD Prof XML: " << strDVDPExpFile
               << L"\n     DVDP Images: " << strMovieImgPath
               << L"\n      Repository: " << strRepoMoniker
               << L"\n  Changer Based?: " << (bFileBased ? L"No" : L"Yes")
               << L"\n\n";

        if (!TRepoImpHelpers::bGetYNAnswer(L"Continue with the load"))
            return tCIDLib::EExitCodes::Normal;

        // OK, let's do the DVD Profiler import phase if needed
        tCIDLib::TBoolean bResult = kCIDLib::False;
        bTestMode = kCIDLib::True;
        conOut << L"Doing a test parsing pass...\n" << kCIDLib::EndLn;
        if (!strDVDPExpFile.bIsEmpty())
        {
            // Do one pass in test mode
            TCQSLImpDVDPLoader dblDVDP;
            bResult = dblDVDP.bLoadIt
            (
                orbcRepoMgr
                , strDVDPExpFile
                , strMovieImgPath
                , strLeaseId
                , bFileBased
            );
        }

        if (!strJRExpFile.bIsEmpty())
        {
            // Do one pass in test mode
            TCQSLImpJRLoader dblJR;
            bResult = dblJR.bLoadIt(orbcRepoMgr, strJRExpFile, strLeaseId);
        }

        //
        //  Take us out of test mode and show the user the results and
        //  ask if they want to continue.
        //
        bTestMode = kCIDLib::False;
        conOut << L"\nTest Results:\n"
               << L"      Errors: " << c4ErrCnt << kCIDLib::NewLn
               << L" Missing Art: " << c4MissingArtCnt << kCIDLib::NewLn
               << L"    No Discs: " << c4NoDiscCnt << kCIDLib::NewLn
               << L"     Skipped: " << c4SkippedCnt << kCIDLib::DNewLn;

        if (TRepoImpHelpers::bGetYNAnswer(L"Continue with the import"))
        {
            // They do, so let's do it
            if (!strDVDPExpFile.bIsEmpty())
            {
                // Do one pass in test mode
                TCQSLImpDVDPLoader dblDVDP;
                bResult = dblDVDP.bLoadIt
                (
                    orbcRepoMgr
                    , strDVDPExpFile
                    , strMovieImgPath
                    , strLeaseId
                    , bFileBased
                );
            }

            if (!strJRExpFile.bIsEmpty())
            {
                TCQSLImpJRLoader dblJR;
                bResult = dblJR.bLoadIt
                (
                    orbcRepoMgr, strJRExpFile, strLeaseId
                );
            }
        }

        // Drop the lease now
        if (!strLeaseId.bIsEmpty())
            orbcRepoMgr->DropLease(strLeaseId);
    }

    //
    //  Catch any unhandled CIDLib runtime errors
    //
    catch(const TError& errToCatch)
    {
        conOut  <<  L"A CIDLib runtime error occured during processing. "
                <<  L"\nError: " << errToCatch.strErrText()
                << kCIDLib::NewLn << kCIDLib::EndLn;

        try
        {
            if (orbcRepoMgr.pobjData())
            {
                orbcRepoMgr->DropLease(strLeaseId);
                orbcRepoMgr.DropRef();
            }
        }

        catch(...)
        {
        }
        return tCIDLib::EExitCodes::FatalError;
    }

    // Catch a general exception
    catch(...)
    {
        conOut  << L"A general exception occured during processing\n"
                << kCIDLib::EndLn;

        try
        {
            if (orbcRepoMgr.pobjData())
            {
                orbcRepoMgr->DropLease(strLeaseId);
                orbcRepoMgr.DropRef();
            }
        }

        catch(...)
        {
        }
        return tCIDLib::EExitCodes::SystemException;
    }

    try
    {
        // Terminate the Orb support
        facCIDOrb().Terminate();
    }

    catch(const TError& errToCatch)
    {
        conOut  <<  L"An error occured during processing. "
                <<  L"\nError: " << errToCatch.strErrText()
                << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::FatalError;
    }
    return tCIDLib::EExitCodes::Normal;
}

