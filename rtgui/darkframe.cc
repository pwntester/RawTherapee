/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <darkframe.h>
#include <options.h>
#include <guiutils.h>
#include <sstream>

using namespace rtengine;
using namespace rtengine::procparams;

DarkFrame::DarkFrame () : Gtk::VBox(), FoldableToolPanel(this)
{
	hbdf = Gtk::manage(new Gtk::HBox());
	darkFrameFile = Gtk::manage(new Gtk::FileChooserButton(M("TP_DARKFRAME_LABEL"), Gtk::FILE_CHOOSER_ACTION_OPEN));
	dfLabel = Gtk::manage(new Gtk::Label(M("GENERAL_FILE")));
	btnReset = Gtk::manage(new Gtk::Button());
	btnReset->set_image (*Gtk::manage(new Gtk::Image (Gtk::StockID("gtk-cancel"), Gtk::ICON_SIZE_BUTTON)));
	hbdf->pack_start(*dfLabel, Gtk::PACK_SHRINK, 4);
	hbdf->pack_start(*darkFrameFile);
	hbdf->pack_start(*btnReset, Gtk::PACK_SHRINK, 4);
	dfAuto = Gtk::manage(new Gtk::CheckButton((M("TP_DARKFRAME_AUTOSELECT"))));

	pack_start( *hbdf, Gtk::PACK_SHRINK, 4);
	pack_start( *dfAuto, Gtk::PACK_SHRINK, 4);

	dfautoconn = dfAuto->signal_toggled().connect ( sigc::mem_fun(*this, &DarkFrame::dfAutoChanged), true);
	dfFile = darkFrameFile->signal_file_set().connect ( sigc::mem_fun(*this, &DarkFrame::darkFrameChanged), true);
	btnReset->signal_clicked().connect( sigc::mem_fun(*this, &DarkFrame::darkFrameReset), true );
}

void DarkFrame::read(const rtengine::procparams::ProcParams* pp, const ParamsEdited* pedited)
{
	disableListener ();
	dfautoconn.block(true);

	if(pedited ){
		dfAuto->set_inconsistent(!pedited->raw.dfAuto );
	}
	if (Glib::file_test (pp->raw.dark_frame, Glib::FILE_TEST_EXISTS))
		darkFrameFile->set_filename (pp->raw.dark_frame);
	else if( !options.rtSettings.darkFramesPath.empty() )
		darkFrameFile->set_current_folder( options.rtSettings.darkFramesPath );
	hbdf->set_sensitive( !pp->raw.df_autoselect );

	lastDFauto = pp->raw.df_autoselect;

	dfAuto->set_active( pp->raw.df_autoselect );
	dfChanged = false;

	dfautoconn.block(false);
	enableListener ();
}

void DarkFrame::write( rtengine::procparams::ProcParams* pp, ParamsEdited* pedited)
{
	pp->raw.dark_frame = darkFrameFile->get_filename();
	pp->raw.df_autoselect = dfAuto->get_active();

	if (pedited) {
		pedited->raw.darkFrame = dfChanged;
		pedited->raw.dfAuto = !dfAuto->get_inconsistent();
	}

}

void DarkFrame::dfAutoChanged()
{
    if (batchMode) {
        if (dfAuto->get_inconsistent()) {
        	dfAuto->set_inconsistent (false);
        	dfautoconn.block (true);
        	dfAuto->set_active (false);
        	dfautoconn.block (false);
        }
        else if (lastDFauto)
        	dfAuto->set_inconsistent (true);

        lastDFauto = dfAuto->get_active ();
    }

	hbdf->set_sensitive( !dfAuto->get_active() );
    if (listener)
        listener->panelChanged (EvPreProcessAutoDF, dfAuto->get_active()?M("GENERAL_ENABLED"):M("GENERAL_DISABLED"));
}

void DarkFrame::darkFrameChanged()
{
	dfChanged=true;
    if (listener)
        listener->panelChanged (EvPreProcessDFFile, Glib::path_get_basename(darkFrameFile->get_filename()));
}

void DarkFrame::darkFrameReset()
{
	dfChanged=true;
	//darkFrameFile->set_current_name("");
	darkFrameFile->set_filename ("");

		if( !options.rtSettings.darkFramesPath.empty() )
	  	darkFrameFile->set_current_folder( options.rtSettings.darkFramesPath );

    if (listener)
        listener->panelChanged (EvPreProcessDFFile, M("GENERAL_NONE"));

}
