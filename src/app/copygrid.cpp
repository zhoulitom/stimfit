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

// copygrid.cpp
// Derived from wxGrid to allow copying to clipboard
// 2007-12-27, Christoph Schmidt-Hieber, University of Freiburg

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/grid.h"
#include "wx/clipbrd.h"


#include "./app.h"
#include "./frame.h"
#include "./doc.h"
#include "./view.h"
#include "./graph.h"
#include "./copygrid.h"


IMPLEMENT_CLASS(wxStfGrid, wxGrid)

BEGIN_EVENT_TABLE(wxStfGrid, wxGrid)
	EVT_MENU(wxID_COPYINTABLE,wxStfGrid::Copy)
	EVT_MENU(wxID_VIEW_MEASURE,wxStfGrid::ViewCrosshair)
	EVT_MENU(wxID_VIEW_BASELINE,wxStfGrid::ViewBaseline)
	EVT_MENU(wxID_VIEW_BASESD,wxStfGrid::ViewBaseSD)
	EVT_MENU(wxID_VIEW_PEAKZERO,wxStfGrid::ViewPeakzero)
	EVT_MENU(wxID_VIEW_PEAKBASE,wxStfGrid::ViewPeakbase)
	EVT_MENU(wxID_VIEW_RT2080,wxStfGrid::ViewRT2080)
	EVT_MENU(wxID_VIEW_T50,wxStfGrid::ViewT50)
	EVT_MENU(wxID_VIEW_RD,wxStfGrid::ViewRD)
	EVT_MENU(wxID_VIEW_SLOPERISE,wxStfGrid::ViewSloperise)
	EVT_MENU(wxID_VIEW_SLOPEDECAY,wxStfGrid::ViewSlopedecay)
	EVT_MENU(wxID_VIEW_LATENCY,wxStfGrid::ViewLatency)
	EVT_MENU(wxID_VIEW_CURSORS,wxStfGrid::ViewCursors)
	EVT_GRID_CELL_RIGHT_CLICK(wxStfGrid::OnRClick) 
	EVT_GRID_LABEL_RIGHT_CLICK(wxStfGrid::OnLabelRClick) 
	EVT_KEY_DOWN( wxStfGrid::OnKeyDown )
END_EVENT_TABLE()

wxStfGrid::wxStfGrid(
	wxWindow* parent, 
	wxWindowID id, 
	const wxPoint& pos, 
	const wxSize& size, 
	long style, 
	const wxString& name
) : wxGrid(parent,id,pos,size,style,name),
	selection(wxT(""))
{
	m_context.reset(new wxMenu());
	m_context->Append(wxID_COPYINTABLE, wxT("Copy selection"));
	
	m_labelContext.reset(new wxMenu());
	m_labelContext->AppendCheckItem(wxID_VIEW_MEASURE,wxT("Crosshair"));
	m_labelContext->AppendCheckItem(wxID_VIEW_BASELINE,wxT("Baseline"));
	m_labelContext->AppendCheckItem(wxID_VIEW_BASESD,wxT("Base SD"));
	m_labelContext->AppendCheckItem(wxID_VIEW_PEAKZERO,wxT("Peak (from 0)"));
	m_labelContext->AppendCheckItem(wxID_VIEW_PEAKBASE,wxT("Peak (from base)"));
	m_labelContext->AppendCheckItem(wxID_VIEW_RT2080,wxT("RT (20-80%)"));
	m_labelContext->AppendCheckItem(wxID_VIEW_T50,wxT("t50"));
	m_labelContext->AppendCheckItem(wxID_VIEW_RD,wxT("Rise/Decay"));
	m_labelContext->AppendCheckItem(wxID_VIEW_SLOPERISE,wxT("Slope (rise)"));
	m_labelContext->AppendCheckItem(wxID_VIEW_SLOPEDECAY,wxT("Slope (decay)"));
	m_labelContext->AppendCheckItem(wxID_VIEW_LATENCY,wxT("Latency"));
	m_labelContext->AppendSeparator();
	m_labelContext->AppendCheckItem(wxID_VIEW_CURSORS,wxT("Cursors"));
}

void wxStfGrid::Copy(wxCommandEvent& WXUNUSED(event)) {
	if (!IsSelection()) {
		wxGetApp().ErrorMsg( wxT("Select cells first") );
		return;
	}
	// Write some text to the clipboard
	// These data objects are held by the clipboard, 
	// so do not delete them in the app.
	selection.Clear();
	bool newline=true;
	for (int nRow=0;nRow<GetNumberRows();++nRow) {
		bool selected=false;
		newline=true;
		for (int nCol=0;nCol<GetNumberCols();++nCol) {
			if (IsInSelection(nRow,nCol)) {
				// Add a line break if this is not the first line:
				if (newline && selection != wxT("") ) {
					selection << wxT("\n");
				}
				if (!newline) {
					selection << wxT("\t");
				}
				newline=false;
				try {
					selection << GetCellValue(nRow,nCol);
					selected=true;
				}
				catch (const std::out_of_range& e) {
					throw e;
				}
			}
		}
	}
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(
			new wxTextDataObject(selection)
		);
		wxTheClipboard->Close();
	}
}

void wxStfGrid::OnRClick(wxGridEvent& event) {
	event.Skip();
	PopupMenu(m_context.get());
}

void wxStfGrid::OnLabelRClick(wxGridEvent& event) {
	event.Skip();
	// Update checkmarks:
	m_labelContext->Check(wxID_VIEW_MEASURE,wxGetApp().GetActiveDoc()->GetViewCrosshair());
	m_labelContext->Check(wxID_VIEW_BASELINE,wxGetApp().GetActiveDoc()->GetViewBaseline());
	m_labelContext->Check(wxID_VIEW_BASESD,wxGetApp().GetActiveDoc()->GetViewBaseSD());
	m_labelContext->Check(wxID_VIEW_PEAKZERO,wxGetApp().GetActiveDoc()->GetViewPeakZero());
	m_labelContext->Check(wxID_VIEW_PEAKBASE,wxGetApp().GetActiveDoc()->GetViewPeakBase());
	m_labelContext->Check(wxID_VIEW_RT2080,wxGetApp().GetActiveDoc()->GetViewRT2080());
	m_labelContext->Check(wxID_VIEW_T50,wxGetApp().GetActiveDoc()->GetViewT50());
	m_labelContext->Check(wxID_VIEW_RD,wxGetApp().GetActiveDoc()->GetViewRD());
	m_labelContext->Check(wxID_VIEW_SLOPERISE,wxGetApp().GetActiveDoc()->GetViewSlopeRise());
	m_labelContext->Check(wxID_VIEW_SLOPEDECAY,wxGetApp().GetActiveDoc()->GetViewSlopeDecay());
	m_labelContext->Check(wxID_VIEW_LATENCY,wxGetApp().GetActiveDoc()->GetViewLatency());
	m_labelContext->Check(wxID_VIEW_CURSORS,wxGetApp().GetActiveDoc()->GetViewCursors());
	PopupMenu(m_labelContext.get());
}

void wxStfGrid::OnKeyDown(wxKeyEvent& event) {
	// Handle CTRL + 'c'
	event.Skip();
	switch (event.GetKeyCode()) {
		case 67:
		case 99: {
			if (event.ControlDown()) {
				wxCommandEvent dEvent;
				Copy(dEvent);
			}
			break;
		}
		default:
		    // pipe everything else to the graph
		    wxGetApp().GetActiveView()->GetGraph()->OnKeyDown(event);
	}
}

void wxStfGrid::ViewResults() {
	// Update checkmarks:
	m_labelContext->Check(wxID_VIEW_MEASURE,wxGetApp().GetActiveDoc()->GetViewCrosshair());
	m_labelContext->Check(wxID_VIEW_BASELINE,wxGetApp().GetActiveDoc()->GetViewBaseline());
	m_labelContext->Check(wxID_VIEW_BASESD,wxGetApp().GetActiveDoc()->GetViewBaseSD());
	m_labelContext->Check(wxID_VIEW_PEAKZERO,wxGetApp().GetActiveDoc()->GetViewPeakZero());
	m_labelContext->Check(wxID_VIEW_PEAKBASE,wxGetApp().GetActiveDoc()->GetViewPeakBase());
	m_labelContext->Check(wxID_VIEW_RT2080,wxGetApp().GetActiveDoc()->GetViewRT2080());
	m_labelContext->Check(wxID_VIEW_T50,wxGetApp().GetActiveDoc()->GetViewT50());
	m_labelContext->Check(wxID_VIEW_RD,wxGetApp().GetActiveDoc()->GetViewRD());
	m_labelContext->Check(wxID_VIEW_SLOPERISE,wxGetApp().GetActiveDoc()->GetViewSlopeRise());
	m_labelContext->Check(wxID_VIEW_SLOPEDECAY,wxGetApp().GetActiveDoc()->GetViewSlopeDecay());
	m_labelContext->Check(wxID_VIEW_LATENCY,wxGetApp().GetActiveDoc()->GetViewLatency());
	m_labelContext->Check(wxID_VIEW_CURSORS,wxGetApp().GetActiveDoc()->GetViewCursors());
	PopupMenu(m_labelContext.get());
}

void wxStfGrid::ViewCrosshair(wxCommandEvent& event) {
	event.Skip();
	// Toggle on or off:
	wxGetApp().GetActiveDoc()->SetViewCrosshair(m_labelContext->IsChecked(wxID_VIEW_MEASURE));
	SetCheckmark(wxT("ViewCrosshair"),wxID_VIEW_MEASURE);
}

void wxStfGrid::ViewBaseline(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewBaseline(m_labelContext->IsChecked(wxID_VIEW_BASELINE));
	SetCheckmark(wxT("ViewBaseline"),wxID_VIEW_BASELINE);
}

void wxStfGrid::ViewBaseSD(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewBaseSD(m_labelContext->IsChecked(wxID_VIEW_BASESD));
	SetCheckmark(wxT("ViewBaseSD"),wxID_VIEW_BASESD);
}

void wxStfGrid::ViewPeakzero(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewPeakZero(m_labelContext->IsChecked(wxID_VIEW_PEAKZERO));
	SetCheckmark(wxT("ViewPeakzero"),wxID_VIEW_PEAKZERO);
}

void wxStfGrid::ViewPeakbase(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewPeakBase(m_labelContext->IsChecked(wxID_VIEW_PEAKBASE));
	SetCheckmark(wxT("ViewPeakbase"),wxID_VIEW_PEAKBASE);
}

void wxStfGrid::ViewRT2080(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewRT2080(m_labelContext->IsChecked(wxID_VIEW_RT2080));
	SetCheckmark(wxT("ViewRT2080"),wxID_VIEW_RT2080);
}

void wxStfGrid::ViewT50(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewT50(m_labelContext->IsChecked(wxID_VIEW_T50));
	SetCheckmark(wxT("ViewT50"),wxID_VIEW_T50);
}

void wxStfGrid::ViewRD(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewRD(m_labelContext->IsChecked(wxID_VIEW_RD));
	SetCheckmark(wxT("ViewRD"),wxID_VIEW_RD);
}

void wxStfGrid::ViewSloperise(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewSlopeRise(m_labelContext->IsChecked(wxID_VIEW_SLOPERISE));
	SetCheckmark(wxT("ViewSloperise"),wxID_VIEW_SLOPERISE);
}

void wxStfGrid::ViewSlopedecay(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewSlopeDecay(m_labelContext->IsChecked(wxID_VIEW_SLOPEDECAY));
	SetCheckmark(wxT("ViewSlopedecay"),wxID_VIEW_SLOPEDECAY);
}

void wxStfGrid::ViewLatency(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewLatency(m_labelContext->IsChecked(wxID_VIEW_LATENCY));
	SetCheckmark(wxT("ViewLatency"),wxID_VIEW_LATENCY);
}

void wxStfGrid::ViewCursors(wxCommandEvent& event) {
	event.Skip();
	wxGetApp().GetActiveDoc()->SetViewCursors(m_labelContext->IsChecked(wxID_VIEW_CURSORS));
	SetCheckmark(wxT("ViewCursors"),wxID_VIEW_CURSORS);
}

void wxStfGrid::SetCheckmark(const wxString& RegEntry, int id) {
	// Toggle on or off:
	if (m_labelContext->IsChecked(id)) {
		wxGetApp().wxWriteProfileInt(wxT("Settings"),RegEntry,1);
	} else {
		wxGetApp().wxWriteProfileInt(wxT("Settings"),RegEntry,0);
	}
	// Update table:
	wxStfChildFrame* pChild=(wxStfChildFrame*)GetMainFrame()->GetActiveChild();
	pChild->UpdateResults();
}