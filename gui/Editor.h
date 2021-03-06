/* -*-c++-*-

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

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include <BoundingBox.h>
#include <RenderArea.h>
#include <Via.h>
#include <tr1/memory>

#include <GL/gl.h>
#include <GL/glu.h>
#include <gtkglmm.h>
#include <math.h>
#include <algorithm>

template<typename RendererType>
class GfxEditorTool {

private:

  RendererType & renderer;

public:

  GfxEditorTool(RendererType & _renderer) : renderer(_renderer) {}
  virtual ~GfxEditorTool() {}

  virtual void on_mouse_click(unsigned int real_x, unsigned int real_y, unsigned int button) = 0;
  virtual void on_mouse_double_click(unsigned int real_x, unsigned int real_y, unsigned int button) = 0;
  virtual void on_mouse_release(unsigned int real_x, unsigned int real_y, unsigned int button) = 0;
  virtual void on_mouse_motion(unsigned int real_x, unsigned int real_y) = 0;

protected:

  virtual RendererType & get_renderer() { return renderer; }

  virtual void start_tool_drawing() {
    renderer.start_tool();
  }

  virtual void stop_tool_drawing() {
    renderer.stop_tool();
  }

  virtual void set_renderer_lock(bool state) {
    GfxEditorTool<RendererType>::get_renderer().set_lock(state);
  }

};


// -------------------------------------------------------------------------------

template<typename RendererType>
class GfxEditorToolRectangle : public GfxEditorTool<RendererType> {

private:

  unsigned int start_x, start_y, stop_x, stop_y;
  bool drag_mode;
  degate::BoundingBox bbox;

public:

  GfxEditorToolRectangle(RendererType & renderer) :
    GfxEditorTool<RendererType>(renderer),
    drag_mode(false) {
  }

  void reset() {
    bbox = degate::BoundingBox();
    GfxEditorTool<RendererType>::start_tool_drawing();
    GfxEditorTool<RendererType>::stop_tool_drawing();
  }

  degate::BoundingBox get_bounding_box() const {
    return bbox;
  }

protected:

  bool in_drag_mode() const {
    return drag_mode;
  }

  void on_mouse_click(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(button == 1) {
      bbox = degate::BoundingBox(start_x, start_x, start_y, start_y);
      start_x = real_x;
      start_y = real_y;
      drag_mode = true;
      GfxEditorTool<RendererType>::set_renderer_lock(true);

    }
  }

  void on_mouse_release(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(button == 1) {
      drag_mode = false;
      GfxEditorTool<RendererType>::set_renderer_lock(false);
      if(start_x == real_x && start_y == real_y) reset();
    }
  }

  void on_mouse_double_click(unsigned int real_x, unsigned int real_y,
			     unsigned int button) {
    on_mouse_release(real_x, real_y, button);
  }

  void on_mouse_motion(unsigned int real_x, unsigned int real_y) {

    if(drag_mode) {
      stop_x = real_x;
      stop_y = real_y;

      GfxEditorTool<RendererType>::start_tool_drawing();
      //glPushAttrib (GL_LINE_BIT);

      glColor4ub(0xff, 0xff, 0xff, 0xff);
      glLineWidth(1);
      //glLineStipple(1, 0xf0f0);

      glBegin(GL_LINE_LOOP);
      glVertex2i(start_x, start_y);
      glVertex2i(real_x, start_y);
      glVertex2i(real_x, real_y);
      glVertex2i(start_x, real_y);
      glEnd();

      //glPopAttrib();

      GfxEditorTool<RendererType>::stop_tool_drawing();

      bbox = degate::BoundingBox(start_x, stop_x, start_y, stop_y);
    }
  }

};

template<typename RendererType>
class GfxEditorToolSelection : public GfxEditorToolRectangle<RendererType> {
private:

  sigc::signal<void, degate::BoundingBox const&>  signal_selection_activated_;
  sigc::signal<void>  signal_selection_revoked_;
  sigc::signal<void, unsigned int, unsigned int, unsigned int>  signal_mouse_clicked_;
  sigc::signal<void, unsigned int, unsigned int, unsigned int>  signal_mouse_double_clicked_;
  sigc::signal<void, degate::BoundingBox const&>  signal_selection_area_resized_;


public:

  GfxEditorToolSelection(RendererType & renderer) :
    GfxEditorToolRectangle<RendererType>(renderer) {
  }

  bool has_selection() const {
    degate::BoundingBox bbox = GfxEditorToolRectangle<RendererType>::get_bounding_box();
    return bbox.get_width() != 0 && bbox.get_height() != 0;
  }

  void reset_selection() {
    GfxEditorToolRectangle<RendererType>::reset();
    if(!signal_selection_revoked_.empty()) signal_selection_revoked_();
    GfxEditorToolRectangle<RendererType>::get_renderer().update_screen();
  }

  sigc::signal<void>& signal_selection_revoked() {
    return signal_selection_revoked_;
  }

  sigc::signal<void, degate::BoundingBox const& >& signal_selection_activated() {
    return signal_selection_activated_;
  }

  sigc::signal<void, degate::BoundingBox const& >& signal_selection_area_resized() {
    return signal_selection_area_resized_;
  }

  /**
   * Signal for a single, non-area-selecting mouse click.
   */
  sigc::signal<void, unsigned int, unsigned int, unsigned int> & signal_mouse_clicked() {
    return signal_mouse_clicked_;
  }

  /**
   * Signal for a single, non-area-selecting mouse click.
   */
  sigc::signal<void, unsigned int, unsigned int, unsigned int> & signal_mouse_double_clicked() {
    return signal_mouse_double_clicked_;
  }

protected:


  void on_mouse_click(unsigned int real_x, unsigned int real_y,
		      unsigned int button) {
    GfxEditorToolRectangle<RendererType>::on_mouse_click(real_x, real_y, button);

    if(!signal_mouse_clicked_.empty())
      signal_mouse_clicked_(real_x, real_y, button);
  }

  void on_mouse_double_click(unsigned int real_x, unsigned int real_y,
			     unsigned int button) {
    GfxEditorToolRectangle<RendererType>::on_mouse_double_click(real_x, real_y, button);

    if(!signal_mouse_double_clicked_.empty())
      signal_mouse_double_clicked_(real_x, real_y, button);
  }

  void on_mouse_release(unsigned int real_x, unsigned int real_y, unsigned int button) {
    GfxEditorToolRectangle<RendererType>::on_mouse_release(real_x, real_y, button);

    if(button == 1) {

      if(has_selection()) {

	if(!signal_selection_activated_.empty())
	  signal_selection_activated_(GfxEditorToolRectangle<RendererType>::get_bounding_box());
      }
      /*
      else {
	if(!signal_mouse_clicked_.empty())
	  signal_mouse_clicked_(real_x, real_y, button);
      }
      */
      GfxEditorToolRectangle<RendererType>::get_renderer().update_screen();
    }
  }

  void on_mouse_motion(unsigned int real_x, unsigned int real_y) {
    GfxEditorToolRectangle<RendererType>::on_mouse_motion(real_x, real_y);

    if(has_selection() && GfxEditorToolRectangle<RendererType>::in_drag_mode()) {

	if(!signal_selection_area_resized_.empty())
	  signal_selection_area_resized_(GfxEditorToolRectangle<RendererType>::get_bounding_box());
    }
  }

};


// -------------------------------------------------------------------------------


template<typename RendererType>
class GfxEditorToolMove : public GfxEditorTool<RendererType> {

private:

  int start_x, start_y;
  bool drag_mode;

  void move(int x, int y) {
    GfxEditorTool<RendererType>::get_renderer().shift_viewport(start_x - x, start_y - y);
  }

  void on_mouse_double_click(unsigned int real_x, unsigned int real_y, unsigned int button) {}

public:

  GfxEditorToolMove(RendererType & renderer) :
    GfxEditorTool<RendererType>(renderer),
    drag_mode(false) {
  }

protected:

  void on_mouse_click(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(button == 1) {
      start_x = real_x;
      start_y = real_y;
      drag_mode = true;
      GfxEditorTool<RendererType>::set_renderer_lock(true);
    }
  }

  void on_mouse_release(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(button == 1) {
      drag_mode = false;
      GfxEditorTool<RendererType>::set_renderer_lock(false);
      move(real_x, real_y);
    }
  }

  void on_mouse_motion(unsigned int real_x, unsigned int real_y) {
    if(drag_mode) move(real_x, real_y);
  }


};


// -------------------------------------------------------------------------------

template<typename RendererType>
class GfxEditorToolVia : public GfxEditorTool<RendererType> {

private:

  sigc::signal<void, unsigned int, unsigned int, unsigned int>  signal_mouse_clicked_;

  void on_mouse_double_click(unsigned int real_x, unsigned int real_y, unsigned int button) {}

public:

  GfxEditorToolVia(RendererType & renderer) :
    GfxEditorTool<RendererType>(renderer) {
  }

  /**
   * Signal for a single mouse click.
   */
  sigc::signal<void, unsigned int, unsigned int, unsigned int> & signal_mouse_clicked() {
    return signal_mouse_clicked_;
  }

protected:

  void on_mouse_click(unsigned int real_x, unsigned int real_y, unsigned int button) { }
  void on_mouse_motion(unsigned int real_x, unsigned int real_y) {}

  void on_mouse_release(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(button == 1) {
      if(!signal_mouse_clicked_.empty())
	signal_mouse_clicked_(real_x, real_y, button);
    }
  }

};

// -------------------------------------------------------------------------------

template<typename RendererType>
class GfxEditorToolWire : public GfxEditorTool<RendererType> {

private:

  unsigned int start_x , start_y;
  bool have_start;

  bool shift_state;

  sigc::signal<void, unsigned int, unsigned int, unsigned int, unsigned int>  signal_wire_added_;
  sigc::signal<void, unsigned int, unsigned int, degate::Via::DIRECTION>  signal_via_added_;


  void angle_snap(int start_x, int start_y, int stop_x, int stop_y,
		  unsigned int * out_x, unsigned int * out_y) {
    assert(out_x != NULL && out_y != NULL);
    int delta_x = stop_x - start_x;
    int delta_y = stop_y - start_y;
    double r = sqrt(delta_x * delta_x + delta_y * delta_y);
    double theta = atan2(delta_y, delta_x); // -pi .. +pi
    theta = round(theta / (M_PI/4.0)) * (M_PI/4.0);

    int w = GfxEditorTool<RendererType>::get_renderer().get_virtual_width();
    int h = GfxEditorTool<RendererType>::get_renderer().get_virtual_height();
    *out_x = std::min(std::max((int)(start_x + r * cos(theta)), 0), w);
    *out_y = std::min(std::max((int)(start_y + r * sin(theta)), 0), h);
  }

public:

  GfxEditorToolWire(RendererType & renderer) :
    GfxEditorTool<RendererType>(renderer),
    have_start(false),
    shift_state(false) {
  }


  void set_shift_state(bool state) {
    shift_state = state;
  }

  /**
   * Signal for a single mouse click.
   */
  sigc::signal<void, unsigned int, unsigned int, unsigned int, unsigned int> & signal_wire_added() {
    return signal_wire_added_;
  }

  /**
   * Signal for a single mouse click.
   */
  sigc::signal<void, unsigned int, unsigned int, degate::Via::DIRECTION> & signal_via_added() {
    return signal_via_added_;
  }

protected:

  void on_mouse_motion(unsigned int real_x, unsigned int real_y) {
    if(have_start) {

      unsigned int x, y;
      angle_snap(start_x, start_y, real_x, real_y, &x, &y);

      GfxEditorTool<RendererType>::start_tool_drawing();
      glColor4ub(0xff, 0xff, 0xff, 0xff);
      glLineWidth(1);

      glBegin(GL_LINES);
      glVertex2i(start_x, start_y);
      glVertex2i(x, y);
      glEnd();

      GfxEditorTool<RendererType>::stop_tool_drawing();

    }
  }

  void on_mouse_click(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if((button == 1 || button == 2) && have_start == false) {
      start_x = real_x;
      start_y = real_y;
      have_start = true;

      GfxEditorTool<RendererType>::set_renderer_lock(true);
    }
    else if(button == 3) {
      have_start = false;
      GfxEditorTool<RendererType>::start_tool_drawing();
      GfxEditorTool<RendererType>::stop_tool_drawing();
      GfxEditorTool<RendererType>::set_renderer_lock(false);
    }
  }

  void on_mouse_release(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(have_start) {

      if(button == 1 || button == 2) {

	unsigned int stop_x, stop_y;
	angle_snap(start_x, start_y, real_x, real_y, &stop_x, &stop_y);

	if(!signal_via_added_.empty() && button == 2) {
	  signal_via_added_(stop_x, stop_y,
			    shift_state ? degate::Via::DIRECTION_UP :
			    degate::Via::DIRECTION_DOWN);
	  GfxEditorTool<RendererType>::get_renderer().render_vias();
	}

	if(stop_x != start_x || stop_y != start_y) {

	  if(!signal_wire_added_.empty()) {
	    signal_wire_added_(start_x, start_y, stop_x, stop_y);
	    GfxEditorTool<RendererType>::get_renderer().render_wires();
	  }

	}

	start_x = stop_x;
	start_y = stop_y;
      }

    }
  }

  void on_mouse_double_click(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(button == 1 && have_start == true) {

      GfxEditorTool<RendererType>::start_tool_drawing();
      GfxEditorTool<RendererType>::stop_tool_drawing();

      have_start = false;
      GfxEditorTool<RendererType>::set_renderer_lock(false);

    }
  }

};

// -------------------------------------------------------------------------------



template<class RendererType>
class GfxEditor : public RendererType {

private:

  std::tr1::shared_ptr<GfxEditorTool<RendererType> > tool;

public:

  GfxEditor() {
    RendererType::signal_mouse_click().connect(sigc::mem_fun(*this, &GfxEditor::on_mouse_click));
    RendererType::signal_mouse_double_click().connect(sigc::mem_fun(*this, &GfxEditor::on_mouse_double_click));
    RendererType::signal_mouse_motion().connect(sigc::mem_fun(*this, &GfxEditor::on_mouse_motion));
    RendererType::signal_mouse_release().connect(sigc::mem_fun(*this, &GfxEditor::on_mouse_release));
  }

  virtual ~GfxEditor() {}

  void set_tool(std::tr1::shared_ptr<GfxEditorTool<RendererType> > tool) {
    this->tool = tool;
  }

  std::tr1::shared_ptr<GfxEditorTool<RendererType> > get_tool() {
    return tool;
  }

  std::tr1::shared_ptr<GfxEditorTool<RendererType> > get_tool() const {
    return tool;
  }

  void on_mouse_click(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(tool != NULL) tool->on_mouse_click(real_x, real_y, button);
  }

  void on_mouse_double_click(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(tool != NULL) tool->on_mouse_double_click(real_x, real_y, button);
  }

  void on_mouse_release(unsigned int real_x, unsigned int real_y, unsigned int button) {
    if(tool != NULL) tool->on_mouse_release(real_x, real_y, button);
  }

  void on_mouse_motion(unsigned int real_x, unsigned int real_y) {
    if(tool != NULL) tool->on_mouse_motion(real_x, real_y);
  }

};

#endif
