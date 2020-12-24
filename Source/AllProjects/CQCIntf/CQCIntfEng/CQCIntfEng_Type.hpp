//
// FILE NAME: CQCIntfEng_Type.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/09/2001
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
//  This is the non-class types header for the facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  Toolkit types namespace
// ---------------------------------------------------------------------------
namespace tCQCIntfEng
{
    // -----------------------------------------------------------------------
    //  Apps that contain our IV engine can sometimes have some modes that we need
    //  to know about. There app handler callback class (which the containing app
    //  has to implement) will return one of these values to indicate the state
    //  of those modes.
    // -----------------------------------------------------------------------
    enum class EAppFlags : tCIDLib::TCard4
    {
        None            = 0x00000000
        , Signage       = 0x00000001
        , DesMode       = 0x00000002
        , RemoteMode    = 0x00000004
    };


    // -----------------------------------------------------------------------
    //  Some stuff we need to do asynchronously, and we need the cooperation
    //  of the containing app. We send him a command via the app handler
    //  callback and he will post it to himself async and call us back.
    // -----------------------------------------------------------------------
    enum class EAsyncCmds
    {
        RunTOPopup

        , Count
    };


    // -----------------------------------------------------------------------
    //  Capabilities flags. Each widget class will set on the base widget
    //  class the flag for the capabilities it supports, which allows a
    //  few things to happen.
    //
    //  1.  The OptionalAct flag is used by widgets that support an action, but
    //      don't require one, so that the validation code in the action
    //      configuration tab won't warn them that they've not configured one.
    //
    //  2.  The NameReq flag indicates that the widget must have a name, set in
    //      the base attributes tab, because it is targeted by other widgets,
    //      such as overlays that are targeted by name or if it is going to be
    //      the target of widget commands.
    //
    //  The first set the capabilities of the widget (i.e. the things that
    //  can be done), but there is another set of flags in the widget class
    //  that represents the actual states of those settings that the caps
    //  flag indicate the widget can support.
    //
    //  The last set indicate capabilities that are not configured by the
    //  user, they are set by the widget and cannot be changed, such as
    //  whether that type of widget can accept the focus.
    // -----------------------------------------------------------------------
    enum class ECapFlags : tCIDLib::TCard4
    {
        None                = 0x00000000

        //
        //  Some flags that indicate whether certain functionality is implemented
        //  or required.
        //
        //      OptionalAct means that it's OK if the action is empty, no need to
        //      warn the user about that.
        //
        //      NameReq means that the widget must be given a name.
        //
        //      HitTrans indicates that the widget can optionally be transparent to
        //      hits, if it supports it. This tells the image mixin tab whether to
        //      enable the check box or not.
        //
        //      TrackBeyond - Most widgets want to not see gesture tracking beyond
        //      their area, but a small number may need to. This allows them to do
        //      so. The view will check for this and pass on out of area position
        //      info if its set.
        //
        , OptionalAct       = 0x00010000
        , NameReq           = 0x00020000
        , HitTrans          = 0x00040000
        , TrackBeyond       = 0x00080000

        //
        //  These indicate literal settings, not whether a settable thing is
        //  supported.
        //
        //      ActiveUpdate means that the widget doesn't use any external
        //      value to drive its display, but it needs to update regularly,
        //      such as an animation widget.
        //
        //      AlwaysOnTop means that the widget can't be drawn over, so it
        //      lets the redrawing code know not to bother to try, and
        //      generally will also imply Windowed.
        //
        //      TakesFocus means that this widget accepts focus, obviously.
        //
        //      Windowed means that the widget manages an actual real window
        //      (for something like the web browser widget.)
        //
        //      ValueUpdate means the widget drives its visual state by an
        //      external value such as a field or global variable.
        //
        //      Gestures means that incoming gestures should be passed to
        //      the widget.
        //
        //  ActiveUpdate and ValueUpdate could be used by the same widget but
        //  generally not. The reason for the distinction is that we update
        //  the former types much more often because they are things like
        //  animation widgets. Field/variable/etc.... based ones need to be
        //  updated less often. An example of using both would be the field
        //  or variable based dynamic marquee widget, which needs to both
        //  keep a value updated and actively scroll it.
        //
        , ActiveUpdate      = 0x00100000
        , AlwaysOnTop       = 0x00200000
        , TakesFocus        = 0x00400000
        , Windowed          = 0x01000000
        , ValueUpdate       = 0x02000000
        , DoesGestures      = 0x04000000

        // Convenient combos of the above
        , AnyUpdateFlags    = 0x02100000
    };


    // -----------------------------------------------------------------------
    //  When we do a recursive hit test, we sometimes want to find just
    //  regular widgets, or just containers, or either.
    // -----------------------------------------------------------------------
    enum class EHitFlags : tCIDLib::TCard2
    {
        Widgets             = 0x0001
        , Conts             = 0x0002
        , Both              = 0x0003
    };



    // -----------------------------------------------------------------------
    //  Some runtime flags for widgets. These control things that control how
    //  widgets react at runtime, but which the widgets don't control
    //  themselves (as they do with the ECapFlags above.) These are set on
    //  them by designer or viewer windows that own then.
    // -----------------------------------------------------------------------
    enum class ERunFlags : tCIDLib::TCard4
    {
        None                = 0x00000000
        , Dead              = 0x00000001
        , HasFocus          = 0x00000002
        , InGesture         = 0x00000004

        , All               = 0x00000007
    };


    // -----------------------------------------------------------------------
    //  This is used to mark a template as to how it is created, as a standard
    //  template or overlay, as a popup, or a popout.
    // -----------------------------------------------------------------------
    enum class ETmplTypes
    {
        Std
        , Popup
        , Popout
    };


    // -----------------------------------------------------------------------
    //  Returned from the eDoValidation() method of the template class.
    // -----------------------------------------------------------------------
    enum class EValTypes
    {
        Success
        , Warnings
        , Errors
    };


    // -----------------------------------------------------------------------
    //  The container class provides a method for the designer to use when it
    //  wants to move
    //
    //  We never move widgets or groups into or out of groups. We skip over
    //  groups as though they were a single z-order level.
    // -----------------------------------------------------------------------
    enum class EZOrder
    {
        ToBack
        , ToFront
        , Back
        , Forward

        , Count
    };
}

#pragma CIDLIB_POPPACK

