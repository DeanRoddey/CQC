//
// FILE NAME: CQCIntfEng_BooleanIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/20/2002
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
//  This file implements the boolean values mixin. Though we are a mixin, we
//  really don't provide any virtual methods, we provide concrete functionality
//  for the classes that mix us in.
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
#include    "CQCIntfEng_.hpp"



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_BooleanIntf
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        //
        //  Version 1
        //      Reset the version as of 1.4
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The key that we store our attributes under when asked to contribute
        //  to an attribute packet.
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const   pszAttrPackKey = L"Boolean Attributes";
    }
}


// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfBooleanIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------

//
//  Our QueryBoolAttrs method will format type/limits info into the user data of the
//  expression attributes it returns (if the containing class provides it.) But, in the
//  attribute editor, if the selected field changes and these boolean expression attributes
//  have to be updated. So we provide this method for him to use, so he doesn't have to
//  deal with these details. We can't use it internally since we don't know if we are
//  dealing with a field or not. It could be a variable, in which case we don't have that
//  sort of info at design time.
//
tCIDLib::TVoid
MCQCIntfBooleanIntf::FormatUserData(const TCQCFldDef& flddSrc, TString& strToFill)
{
    strToFill.Clear();
    TStringTokenizer::BuildQuotedCommaList
    (
        tCQCKit::strAltXlatEFldTypes(flddSrc.eType()), strToFill
    );

    TStringTokenizer::BuildQuotedCommaList(flddSrc.strLimits(), strToFill);
}



// ---------------------------------------------------------------------------
//  MCQCIntfBooleanIntf: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we set the true expression to just a simple 'is true' statement
//  so that we are in a state that is basically legal.
//
MCQCIntfBooleanIntf::MCQCIntfBooleanIntf() :

    m_exprFalse(L"False")
    , m_exprTrue
      (
        L"True"
        , tCQCKit::EExprTypes::Statement
        , tCQCKit::EStatements::IsTrue
        , TString::strEmpty()
        , kCIDLib::False
      )
{
}

MCQCIntfBooleanIntf::MCQCIntfBooleanIntf(const MCQCIntfBooleanIntf& miwdgToCopy) :

    m_exprFalse(miwdgToCopy.m_exprFalse)
    , m_exprTrue(miwdgToCopy.m_exprTrue)
{
}

MCQCIntfBooleanIntf::~MCQCIntfBooleanIntf()
{
}


// ---------------------------------------------------------------------------
//  MCQCIntfBooleanIntf: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid
MCQCIntfBooleanIntf::operator=(const MCQCIntfBooleanIntf& miwdgToAssign)
{
    if (this != &miwdgToAssign)
    {
        m_exprFalse = miwdgToAssign.m_exprFalse;
        m_exprTrue  = miwdgToAssign.m_exprTrue;
    }
}


// ---------------------------------------------------------------------------
//  MCQCIntfBooleanIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

// If the passed command is one of ours, do it
tCIDLib::TBoolean
MCQCIntfBooleanIntf::bDoBoolCmd(const   TCQCCmdCfg&     ccfgToDo
                                , const tCIDLib::TCard4 c4Index)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;

    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetBoolCompVal)
    {
        if (ccfgToDo.piAt(0).m_strValue.bCompareI(L"True Expression"))
            m_exprTrue.strCompVal(ccfgToDo.piAt(1).m_strValue);
        else
            m_exprFalse.strCompVal(ccfgToDo.piAt(1).m_strValue);

        bRes = kCIDLib::True;
    }
    return bRes;
}


tCIDLib::TBoolean
MCQCIntfBooleanIntf::bSameBool(const MCQCIntfBooleanIntf& miwdgSrc) const
{
    // Just compare our two expressions
    return ((m_exprFalse == miwdgSrc.m_exprFalse)
           && (m_exprTrue == miwdgSrc.m_exprTrue));
}


tCQCIntfEng::EExprResults
MCQCIntfBooleanIntf::eCompareValue( const   TCQCFldValue&   fvNewValue
                                    , const TStdVarsTar&    ctarGlobalVars) const
{
    //
    //  Check the true first, if it is set. If it matches, we are done. If it
    //  does not, and there is no false, then we return false. If it is not
    //  set, then we check the false. If the false matches we set false, else
    //  we set true if there is no true. If both are set and neither matches,
    //  we return neither.
    //
    //  We have to have one of them or they wouldn't have been able to save
    //  the template.
    //
    tCQCIntfEng::EExprResults eTrueState = tCQCIntfEng::EExprResults::Neither;
    if (m_exprTrue.eType() != tCQCKit::EExprTypes::None)
    {
        try
        {
            if (m_exprTrue.bEvaluate(fvNewValue, &ctarGlobalVars))
                eTrueState = tCQCIntfEng::EExprResults::True;
            else
                eTrueState = tCQCIntfEng::EExprResults::False;
        }

        catch(...)
        {
            eTrueState = tCQCIntfEng::EExprResults::Error;
        }
    }

    tCQCIntfEng::EExprResults eRet = tCQCIntfEng::EExprResults::Neither;
    switch(eTrueState)
    {
        case tCQCIntfEng::EExprResults::True :
        {
            // We don't have to check anything else
            eRet = tCQCIntfEng::EExprResults::True;
            break;
        }

        case tCQCIntfEng::EExprResults::False :
        {
            //
            //  The true was there, but didn't match, so check the false
            //  statement.
            //
            tCQCIntfEng::EExprResults eFalseState = tCQCIntfEng::EExprResults::Neither;
            if (m_exprFalse.eType() != tCQCKit::EExprTypes::None)
            {
                try
                {
                    if (m_exprFalse.bEvaluate(fvNewValue, &ctarGlobalVars))
                        eFalseState = tCQCIntfEng::EExprResults::True;
                    else
                        eFalseState = tCQCIntfEng::EExprResults::False;
                }

                catch(...)
                {
                    eFalseState = tCQCIntfEng::EExprResults::Error;
                }
            }

            //
            //  If not present, or there and it matched, then return false
            //  since we either failed the true or matched the false. If it
            //  was there and didn't match, then neither matched, so we return
            //  neither.
            //
            if ((eFalseState == tCQCIntfEng::EExprResults::Neither)
            ||  (eFalseState == tCQCIntfEng::EExprResults::True))
            {
                eRet = tCQCIntfEng::EExprResults::False;
            }
             else if (eFalseState == tCQCIntfEng::EExprResults::Error)
            {
                eRet = tCQCIntfEng::EExprResults::Error;
            }
            break;
        }

        case tCQCIntfEng::EExprResults::Neither :
        {
            tCQCIntfEng::EExprResults eFalseState = tCQCIntfEng::EExprResults::Neither;
            if (m_exprFalse.eType() != tCQCKit::EExprTypes::None)
            {
                try
                {
                    if (m_exprFalse.bEvaluate(fvNewValue, &ctarGlobalVars))
                        eFalseState = tCQCIntfEng::EExprResults::True;
                    else
                        eFalseState = tCQCIntfEng::EExprResults::False;
                }

                catch(...)
                {
                    eFalseState = tCQCIntfEng::EExprResults::Error;
                }
            }

            //
            //  The true isn't set, so there has to be a false. So if the
            //  false is false, we return true. If the false is true, we
            //  return false. If it wasn't present, we'll fall through with
            //  the Neither value still set. If an error, we return error.
            //
            if (eFalseState == tCQCIntfEng::EExprResults::True)
                eRet = tCQCIntfEng::EExprResults::False;
            else if (eFalseState == tCQCIntfEng::EExprResults::False)
                eRet = tCQCIntfEng::EExprResults::True;
            else if (eFalseState == tCQCIntfEng::EExprResults::Error)
                eRet = tCQCIntfEng::EExprResults::Error;
            break;
        }

        case tCQCIntfEng::EExprResults::Error :
            // The true failed, so just return error
            eRet = tCQCIntfEng::EExprResults::Error;
            break;
    };
    return eRet;
}


// Provide constant access to our two expressions
const TCQCExpression& MCQCIntfBooleanIntf::exprFalse() const
{
    return m_exprFalse;
}

const TCQCExpression& MCQCIntfBooleanIntf::exprTrue() const
{
    return m_exprTrue;
}


tCIDLib::TVoid
MCQCIntfBooleanIntf::QueryBoolAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    //
    //  We do these as binary data, and just flatten the expressions into the
    //  attribute, since it's not convenient to have a textual representation.
    //
    colAttrs.objPlace
    (
        L"Expressions:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    //
    //  Ask the including class for the info to encode inot the attribute that will be
    //  used later by the expression editing dialog that this stuff drives. If they don't
    //  have any info we go with string, which just makes it generic.
    //
    tCQCKit::EFldTypes eType = tCQCKit::EFldTypes::String;
    TString strLimits;
    QueryBoolAttrLims(eType, strLimits);

    TString strLimInfo(64UL);
    TStringTokenizer::BuildQuotedCommaList(tCQCKit::strAltXlatEFldTypes(eType), strLimInfo);
    TStringTokenizer::BuildQuotedCommaList(strLimits, strLimInfo);

    TBinMBufOutStream strmOut(1024UL);
    strmOut << m_exprTrue << kCIDLib::FlushIt;
    TAttrData& adatTrue = colAttrs.objPlace
    (
        L"True"
        , kCQCIntfEng::strAttr_Bool_TrueExpr
        , strmOut.mbufData()
        , strmOut.c4CurPos()
        , TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    );
    adatTrue.SetSpecType(kCQCIntfEng::strAttrType_Expression);
    adatTrue.strUserData(strLimInfo);

    strmOut.Reset();
    strmOut << m_exprFalse << kCIDLib::FlushIt;
    TAttrData& adatFalse = colAttrs.objPlace
    (
        L"False"
        , kCQCIntfEng::strAttr_Bool_FalseExpr
        , strmOut.mbufData()
        , strmOut.c4CurPos()
        , TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    );
    adatFalse.SetSpecType(kCQCIntfEng::strAttrType_Expression);
    adatFalse.strUserData(strLimInfo);
}


// Add any of our supported commands to the list
tCIDLib::TVoid
MCQCIntfBooleanIntf::QueryBoolCmds(         TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    //
    //  Set one of the expression's comp values. Only allowed in the preload
    //  context.
    //
    if (eContext == tCQCKit::EActCmdCtx::Preload)
    {
        TCQCCmd cmdSetCompVal
        (
            kCQCIntfEng::strCmdId_SetBoolCompVal
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetBoolCompVal)
            , 2
        );

        cmdSetCompVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Target)
                , TString(L"False Expression, True Expression")
            )
        );

        cmdSetCompVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet), tCQCKit::ECmdPTypes::Text
            )
        );
        colCmds.objAdd(cmdSetCompVal);
    }
}


// Supports the search and replace system
tCIDLib::TVoid
MCQCIntfBooleanIntf::ReplaceBool(const  TString&            strSrc
                                , const TString&            strTar
                                , const tCIDLib::TBoolean   bRegEx
                                , const tCIDLib::TBoolean   bFull
                                ,       TRegEx&             regxFind)
{
    //
    //  We apply it to the comp values of our two expressions, if they have
    //  a value.
    //
    TString strTmp;
    if (m_exprFalse.bHasCompVal())
    {
        strTmp = m_exprFalse.strCompVal();
        facCQCKit().bDoSearchNReplace(strSrc, strTar, strTmp, bRegEx, bFull, regxFind);
        m_exprFalse.strCompVal(strTmp);
    }

    if (m_exprTrue.bHasCompVal())
    {
        strTmp = m_exprTrue.strCompVal();
        facCQCKit().bDoSearchNReplace(strSrc, strTar, strTmp, bRegEx, bFull, regxFind);
        m_exprTrue.strCompVal(strTmp);
    }
}


// Resets us back to defaults
tCIDLib::TVoid MCQCIntfBooleanIntf::ResetBool()
{
    m_exprFalse.Set
    (
        tCQCKit::EExprTypes::None
        , tCQCKit::EStatements::IsEqual
        , TString::strEmpty()
        , kCIDLib::False
    );

    m_exprTrue.Set
    (
        tCQCKit::EExprTypes::None
        , tCQCKit::EStatements::IsEqual
        , TString::strEmpty()
        , kCIDLib::False
    );
}


// Stream our data in from the passed stream
tCIDLib::TVoid MCQCIntfBooleanIntf::ReadInBool(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_BooleanIntf::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCIntfBooleanIntf")
        );
    }

    // Read our stuff in
    strmToReadFrom  >> m_exprFalse
                    >> m_exprTrue;

    // And it should all end with another frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
MCQCIntfBooleanIntf::SetBoolAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    if ((adatNew.strId() == kCQCIntfEng::strAttr_Bool_FalseExpr)
    ||  (adatNew.strId() == kCQCIntfEng::strAttr_Bool_TrueExpr))
    {
        // These are flattened expressions
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());

        if (adatNew.strId() == kCQCIntfEng::strAttr_Bool_FalseExpr)
            strmSrc >> m_exprFalse;
        else
            strmSrc >> m_exprTrue;
    }
}


// Set up the false expression
tCIDLib::TVoid
MCQCIntfBooleanIntf::SetFalseVal(const  TString&              strVal
                                , const tCQCKit::EExprTypes   eType
                                , const tCQCKit::EStatements  eStatement)
{
    m_exprFalse.Set(eType, eStatement, strVal, kCIDLib::False);
}


// Set up the true expression
tCIDLib::TVoid
MCQCIntfBooleanIntf::SetTrueVal(const   TString&              strVal
                                , const tCQCKit::EExprTypes   eType
                                , const tCQCKit::EStatements  eStatement)
{
    m_exprTrue.Set(eType, eStatement, strVal, kCIDLib::False);
}


// Stream out data out to the passed stream
tCIDLib::TVoid
MCQCIntfBooleanIntf::WriteOutBool(TBinOutStream& strmToWriteTo) const
{
    // Frame our content, and stream out out a version format value
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEng_BooleanIntf::c2FmtVersion
                    << m_exprFalse
                    << m_exprTrue
                    << tCIDLib::EStreamMarkers::Frame;
}


// ---------------------------------------------------------------------------
//  MCQCIntfBooleanIntf: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// If not overridden, return basic default values
tCIDLib::TVoid
MCQCIntfBooleanIntf::QueryBoolAttrLims(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    eType = tCQCKit::EFldTypes::String;
    strLimits.Clear();
}

