/*

This file is part of the IC reverse engineering tool degate.

Copyright 2008, 2009, 2010 by Martin Schobert

Degate is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

Degate is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with degate. If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __PROJECTSETTINGSWIN_H__
#define __PROJECTSETTINGSWIN_H__

#include <gtkmm.h>

#include "Project.h"
#include "LogicModel.h"
#include "GladeFileLoader.h"

class ProjectSettingsWin : private GladeFileLoader {

 public:
  ProjectSettingsWin(Gtk::Window *parent, degate::Project_shptr project);

  virtual ~ProjectSettingsWin();

  bool run();

 private:
  degate::Project_shptr project;

  Gtk::Window * parent;
  Gtk::Entry * entry_project_name;
  Gtk::Entry * entry_project_description;
  Gtk::Entry * entry_lambda;
  Gtk::Entry * entry_wire_diameter;
  Gtk::Entry * entry_via_diameter;
  Gtk::Entry * entry_port_diameter;
  Gtk::Entry * entry_server_url;
  Gtk::Entry * entry_pixel_per_um;
  Gtk::Entry * entry_template_dimension;
  Gtk::Entry * entry_font_size;

  bool ok_clicked;

  // Signal handlers:
  virtual void on_ok_button_clicked();
  virtual void on_cancel_button_clicked();

  void set_color_for_button(Gtk::ColorButton * pCButton, degate::color_t c);
  degate::color_t get_color_for_button(std::string const & button_name) const;

};

#endif
