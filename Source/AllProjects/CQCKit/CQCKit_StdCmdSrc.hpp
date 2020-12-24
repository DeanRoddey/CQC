//
// FILE NAME: CQCKit_StdCmdSrc.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/19/2005
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
//  The command source mixin is very generic and will be mixed into things like
//  interface widgets and so forth to allow them to become command sources in
//  addition to the other things that they do. But, in many cases, there is just
//  a need to store a set of blocks of commands under some sort of key such as
//  a name or keystroke or whatever. So for each mappable key we need to be able
//  to store a sequence of commands to invoke when that key is pressed.
//
//  There is a base command source class which is nothing but a wrapper around
//  the command source mixin. It is used for places where it needs to be possible
//  to make a copy of a command source's content, such as the CQC action editor
//  dialog. Since it has no other commands, it can stream in and out in the same
//  form that the command source mixin uses, so it's interchangeable.
//
//  So we have a another simple wrapper class, TCQCStdCmdSrc, which provides a few extra
//  bits that some applications might want to add to an action. NOTE THAT THE user id
//  is NOT included in object equality testing!
//
//  We can then derive other classes that provide a particular type of key
//  under which the block of commands will be stored. We do one standard one
//  here that just uses a text key, usually an MD5 hash based unique id. It
//  will be used in a number of places. Elsewhere there are other derivatives
//  for mappng to a key or whatever else is required.
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
//   CLASS: TBasicCmdSrc
//  PREFIX: csrc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TBasicCmdSrc :   public TObject
                                    , public MStreamable
                                    , public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBasicCmdSrc();

        TBasicCmdSrc
        (
            const   TBasicCmdSrc&           csrcSrc
        );

        ~TBasicCmdSrc();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBasicCmdSrc& operator=
        (
            const   TBasicCmdSrc&           csrcSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TBasicCmdSrc&           csrcSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TBasicCmdSrc&           csrcSrc
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom(TBinInStream& strmToReadFrom) override;

        tCIDLib::TVoid StreamTo(TBinOutStream& strmToWriteTo) const override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TBasicCmdSrc,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCStdCmdSrc
//  PREFIX: csrc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCStdCmdSrc

    : public TObject, public MStreamable, public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCStdCmdSrc();

        TCQCStdCmdSrc(const TCQCStdCmdSrc&) = default;
        TCQCStdCmdSrc(TCQCStdCmdSrc&&) = default;

        TCQCStdCmdSrc
        (
            const   TString&                strTitle
        );

        ~TCQCStdCmdSrc();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCStdCmdSrc& operator=(const TCQCStdCmdSrc&) = default;
        TCQCStdCmdSrc& operator=(TCQCStdCmdSrc&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCStdCmdSrc&          csrcSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCStdCmdSrc&          csrcSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNoGUI() const;

        tCIDLib::TBoolean bNoGUI
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bNoPropogate() const;

        tCIDLib::TBoolean bNoPropogate
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bUsed() const;

        tCIDLib::TBoolean bUsed
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4UserId() const;

        tCIDLib::TCard4 c4UserId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        const TString& strTitle() const;

        const TString& strTitle
        (
            const   TString&                strToSet
        );



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
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
        //  m_bNoGUI
        //  m_NoPropogate
        //      Some flags for use by the client code that utilizes these objects. These
        //      are often convenient for them to have to mark actions in particular ways.
        //
        //  m_bUsed
        //      These are sometimes stored in lists where all of the possible items are
        //      present, but only used when configured. So we provide a used flag for client
        //      code to use to mark them.
        //
        //  m_c4UserId
        //      A non-persistent value that can be used by client code, for things like
        //      mapping within list boxes and such. THIS IS NOT included in object equality
        //      testing, and it is reset to max card upon streaming in.
        //
        //  m_strTitle
        //      The human readonable title for this command source.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bNoGUI;
        tCIDLib::TBoolean   m_bNoPropogate;
        tCIDLib::TBoolean   m_bUsed;
        tCIDLib::TCard4     m_c4UserId;
        TString             m_strTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStdCmdSrc,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCStdKeyedCmdSrc
//  PREFIX: csrc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCStdKeyedCmdSrc : public TCQCStdCmdSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TCQCStdKeyedCmdSrc&     csrcSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCStdKeyedCmdSrc();

        TCQCStdKeyedCmdSrc
        (
            const   TString&                strTitle
            , const TString&                strId
        );

        TCQCStdKeyedCmdSrc(const TCQCStdKeyedCmdSrc&) = default;
        TCQCStdKeyedCmdSrc(TCQCStdKeyedCmdSrc&&) = default;

        ~TCQCStdKeyedCmdSrc();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCStdKeyedCmdSrc& operator=(const TCQCStdKeyedCmdSrc&) = default;
        TCQCStdKeyedCmdSrc& operator=(TCQCStdKeyedCmdSrc&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCStdKeyedCmdSrc&     csrcSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCStdKeyedCmdSrc&     csrcSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strId() const;

        const TString& strId
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
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
        //  m_strId
        //      The unique id for this command source.
        // -------------------------------------------------------------------
        TString m_strId;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStdKeyedCmdSrc,TCQCStdCmdSrc)
};

#pragma CIDLIB_POPPACK


