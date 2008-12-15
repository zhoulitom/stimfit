// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

/*! \file cursorsdlg.h
 *  \author Christoph Schmidt-Hieber
 *  \date 2008-01-16
 *  \brief Declares wxStfCursorsDlg.
 */

#ifndef _CURSORSDLG_H
#define _CURSORSDLG_H

/*! \addtogroup wxstf
 *  @{
 */

#include "./../../core/stimdefs.h"
#include "wx/bookctrl.h"

class wxStfDoc;

//! Cursor settings non-modal dialog
class StfDll wxStfCursorsDlg : public wxDialog 
{
    DECLARE_EVENT_TABLE()

private:
    wxNotebookPage* CreateMeasurePage();
    wxNotebookPage* CreatePeakPage();
    wxNotebookPage* CreateBasePage();
    wxNotebookPage* CreateDecayPage();
    wxFlexGridSizer* CreateCursorInput(
            wxPanel* nbPage,
            wxWindowID textC1,
            wxWindowID textC2,
            wxWindowID comboU1,
            wxWindowID comboU2,
            std::size_t c1,
            std::size_t c2
    );

    int ReadCursor(wxWindowID textId, bool isTime) const;
    void UpdateUnits(wxWindowID comboId, bool& setTime, wxWindowID textID);
    bool cursorMIsTime,
    cursor1PIsTime,cursor2PIsTime,
    cursor1BIsTime,cursor2BIsTime,
    cursor1DIsTime,cursor2DIsTime;
    wxStfDoc* actDoc;
    wxNotebook* m_notebook;

    void OnPageChanged( wxNotebookEvent& event );
    void OnComboBoxUM( wxCommandEvent& event );
    void OnComboBoxU1P( wxCommandEvent& event );
    void OnComboBoxU2P( wxCommandEvent& event );
    void OnComboBoxU1B( wxCommandEvent& event );
    void OnComboBoxU2B( wxCommandEvent& event );
    void OnComboBoxU1D( wxCommandEvent& event );
    void OnComboBoxU2D( wxCommandEvent& event );
    void OnRadioAll( wxCommandEvent& event );
    void OnRadioMean( wxCommandEvent& event );
    void OnPeakcalcexec( wxCommandEvent& event );

    //! Only called when a modal dialog is closed with the OK button.
    /*! \return true if all dialog entries could be read successfully
     */
    bool OnOK();

public:
    //! Constructor
    /*! \param parent Pointer to parent window.
     *  \param id Window id.
     *  \param title Dialog title.
     *  \param pos Initial position.
     *  \param size Initial size.
     *  \param style Dialog style.
     */
    wxStfCursorsDlg(
            wxWindow* parent,
            int id = wxID_ANY,
            wxString title = wxT("Cursor settings"),
            wxPoint pos = wxDefaultPosition,
            wxSize size = wxDefaultSize,
            int style = wxCAPTION
    );
    
    //! Called upon ending a modal dialog.
    /*! \param retCode The dialog button id that ended the dialog
     *         (e.g. wxID_OK)
     */
    virtual void EndModal(int retCode);
    
    //! Called when data should be transferred from the non-modal dialog (e.g. when OK is pressed)
    /*! Note that a non-modal dialog won't be destroyed when OK is clicked,
     *  it will only disappear from sight. This function will then apply the current
     *  cursor settings and update the results table.
     *  \return The return value of the base class version wxWindow::TransferDataFromWindow()
     */
    virtual bool TransferDataFromWindow();

    //! Get the measurement cursor x-position
    /*! \return The measurement cursor x-position in units of sampling points. 
     */
    int GetCursorM() const;

    //! Get the left peak cursor x-position
    /*! \return The left peak cursor x-position in units of sampling points. 
     */
    int GetCursor1P() const;

    //! Get the right peak cursor x-position
    /*! \return The right peak cursor x-position in units of sampling points. 
     */
    int GetCursor2P() const;

    //! Get the left base cursor x-position
    /*! \return The left base cursor x-position in units of sampling points. 
     */
    int GetCursor1B() const;

    //! Get the right base cursor x-position
    /*! \return The right base cursor x-position in units of sampling points. 
     */
    int GetCursor2B() const;

    //! Get the left fit cursor x-position
    /*! \return The left fit cursor x-position in units of sampling points. 
     */
    int GetCursor1D() const;

    //! Get the right fit cursor x-position
    /*! \return The right fit cursor x-position in units of sampling points. 
     */
    int GetCursor2D() const;

    //! Gets the number of points used for the binned average during peak detection.
    /*! \return The number of points used for the binned average during peak detection.
     */
    int GetPeakPoints() const;

    //! Sets the number of points used for the binned average during peak detection.
    /*! \param peakPoints The number of points used for the binned average during peak detection.
     */
    void SetPeakPoints(int peakPoints);

    //! Gets the direction of peak calculations.
    /*! \return The current direction of peak calculations.
     */
    stf::direction GetDirection() const;

    //! Sets the direction of peak calculations.
    /*! \return The new direction of peak calculations.
     */
    void SetDirection(stf::direction direction);

    //! Indicates whether the right peak cursor should always be at the end of the trace.
    /*! \return true if the peak cursor should always be at the end of the trace.
     */
    bool GetPeakAtEnd() const;

    //! Indicates whether to always start a fit at the current peak position.
    /*! \return true if the fit should always be started at the current peak position.
     */
    bool GetStartFitAtPeak() const;

    //! Updates the cursor entries.
    void UpdateCursors();

    //! Retrieve the current cursor notebook page.
    /*! \return The cursor corresponding to the currently selected notebook page.
     */
    stf::cursor_type CurrentCursor() const;

    //! Get the slope at which the baseline should be fixed.
    /*! \return The slope at which the baseline should be fixed.
     */
    double GetSlope() const;

    //! Set the units of the slope.
    /*! \param units The units of the slope.
     */
    void SetSlopeUnits(const wxString& units);

    //! Indicates whether the baseline should be fixed to a certain slope.
    /*! \return true if the baseline should be fixed, false otherwise.
     */
    bool GetBaseToSlope() const;

    //! Indicates whether an additional vertical ruler should be drawn through the baseline.
    /*! \return true if an additional ruler should be drawn.
     */
    bool GetRuler() const;

    //! Sets the currently active document.
    /*! \param actDoc_ A pointer to the currently active document.
     */
    void SetActiveDoc(wxStfDoc* actDoc_) { actDoc = actDoc_; }
};

/* @} */

#endif
