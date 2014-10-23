// define this to use XPMs everywhere (by default, BMPs are used under Win)
// BMPs use less space, but aren't compiled into the executable on other platforms
#define __WXMSW__ 1
#ifdef __WXMSW__
    #define USE_XPM_BITMAPS 0
#else
    #define USE_XPM_BITMAPS 1
#endif
#include <combo.h>
#if USE_XPM_BITMAPS && defined(__WXMSW__) && !wxUSE_XPM_IN_MSW
    #error You need to enable XPM support to use XPM bitmaps with toolbar!
#endif // USE_XPM_BITMAPS

// If this is 1, the sample will test an extra toolbar identical to the
// main one, but not managed by the frame. This can test subtle differences
// in the way toolbars are handled, especially on Mac where there is one
// native, 'installed' toolbar.
#define USE_UNMANAGED_TOOLBAR 0

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "mondrian.xpm"
#endif

#if USE_XPM_BITMAPS
    #include "bitmaps/new.xpm"
    #include "bitmaps/open.xpm"
    #include "bitmaps/save.xpm"
    #include "bitmaps/copy.xpm"
    #include "bitmaps/cut.xpm"
    #include "bitmaps/preview.xpm"  // paste XPM
    #include "bitmaps/print.xpm"
    #include "bitmaps/help.xpm"
#endif // USE_XPM_BITMAPS

enum Positions
{
    TOOLBAR_LEFT,
    TOOLBAR_TOP,
    TOOLBAR_RIGHT,
    TOOLBAR_BOTTOM
};
#include <EL_DocView.h>
// ----------------------------------------------------------------------------
// classes
// ----------------------------------------------------------------------------

// Define a new application
class EL_App : public wxApp
{
public:
    bool OnInit();
};

// Define a new frame
class EL_Frame: public wxFrame
{
public:
    EL_Frame(wxFrame *parent,
            wxWindowID id = wxID_ANY,
            const wxString& title = _T("wxToolBar Sample"),
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE|wxCLIP_CHILDREN|wxNO_FULL_REPAINT_ON_RESIZE);

    void PopulateToolbar(wxToolBarBase* toolBar);
    void RecreateToolbar();

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void OnSize(wxSizeEvent& event);

    void OnToggleToolbar(wxCommandEvent& event);
    void OnToggleAnotherToolbar(wxCommandEvent& event);
    void OnToggleHorizontalText(wxCommandEvent& WXUNUSED(event));

    void OnToggleToolbarSize(wxCommandEvent& event);
    void OnChangeOrientation(wxCommandEvent& event);
    void OnToggleToolbarRows(wxCommandEvent& event);
    void OnToggleTooltips(wxCommandEvent& event);
    void OnToggleCustomDisabled(wxCommandEvent& event);

//    void OnEnablePrint(wxCommandEvent& WXUNUSED(event)) { DoEnablePrint(); }
//    void OnDeletePrint(wxCommandEvent& WXUNUSED(event)) { DoDeletePrint(); }
//    void OnInsertPrint(wxCommandEvent& event);
    void OnChangeToolTip(wxCommandEvent& event);
    void OnToggleHelp(wxCommandEvent& WXUNUSED(event)) { DoToggleHelp(); }
    void OnToggleRadioBtn(wxCommandEvent& event);

    void OnToolbarStyle(wxCommandEvent& event);
    void OnToolbarCustomBitmap(wxCommandEvent& event);

    void OnToolLeftClick(wxCommandEvent& event);
    void OnToolRightClick(wxCommandEvent& event);

    void OnCombo(wxCommandEvent& event);
    void OnAuiNotebook(wxAuiNotebookEvent& event);
	void OnViewSelectionChanged(EL_DocViewEvent & event);
    void OnUpdateCopyAndCut(wxUpdateUIEvent& event);
    void OnUpdateToggleHorzText(wxUpdateUIEvent& event);
    void OnUpdateToggleRadioBtn(wxUpdateUIEvent& event)
        { event.Enable( m_tbar != NULL ); }

private:
//    void DoEnablePrint();
//    void DoDeletePrint();
    void DoToggleHelp();
    bool DoFileOpen();
    void LayoutChildren();
    wxTreeCtrl* CreateTreeCtrl(wxWindow *  parent);
    bool                m_smallToolbar,
                        m_horzText,
                        m_useCustomDisabled,
                        m_showTooltips;
    size_t              m_rows;             // 1 or 2 only

    // the number of print buttons we have (they're added/removed dynamically)
//    size_t              m_nPrint;

    // store toolbar position for future use
    Positions           m_toolbarPosition;

    wxScrolledWindow *m_left, *m_right;
	wxSplitterWindow* m_topsplitter;
    wxAuiNotebook      *notebook;
	wxAuiNotebook      *notebookproperty;
    wxTreeCtrl         *m_graphicsTree;
	wxTreeCtrl         *m_colorTree;
	EL_LayerSelectCombo   *layerselect;
	EL_ColorSelectCombo   *colorselect;
	wxTextCtrl         *conerpower;
	wxTextCtrl         *cutpower;
	wxTextCtrl         *cutspeed;
	wxTextCtrl         *cutaddspeed;
	wxTextCtrl         *sclptpower;
	wxTextCtrl         *sclptspeed;
	wxTextCtrl         *sclptaddspeed;
	wxTextCtrl         *sclptdensity;   //µÒøÃ√‹∂»
	wxTextCtrl         *sclptXfix;      //∫·œÚ≤π≥•¡ø
    wxPanel            *m_panel;
    wxToolBar          *m_extraToolBar;

    wxToolBar          *m_tbar;

    // the path to the custom bitmap for the test toolbar tool
    wxString            m_pathBmp;

    DECLARE_EVENT_TABLE()
};