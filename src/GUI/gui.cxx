/**************************************************************************
 * gui.cxx
 *
 * Written 1998 by Durk Talsma, started Juni, 1998.  For the flight gear
 * project.
 *
 * Additional mouse supported added by David Megginson, 1999.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * $Id$
 **************************************************************************/


#include <config.h>

#include <simgear/compiler.h>

#include <string>

#include <simgear/structure/exception.hxx>
#include <simgear/structure/commands.hxx>

#include <simgear/misc/sg_path.hxx>
#include <simgear/props/props.hxx>
#include <simgear/props/props_io.hxx>

#include <Main/main.hxx>
#include <Main/globals.hxx>
#include <Main/locale.hxx>
#include <Main/fg_props.hxx>
#include <Viewer/WindowSystemAdapter.hxx>
#include <Viewer/CameraGroup.hxx>
#include <GUI/new_gui.hxx>

#include "gui.h"

#include <osg/GraphicsContext>
#include <osg/GLExtensions>

using namespace flightgear;

/* -------------------------------------------------------------------------
init the gui
_____________________________________________________________________*/

namespace
{

// Operation for querying OpenGL parameters. This must be done in a
// valid OpenGL context, potentially in another thread.

struct GeneralInitOperation : public GraphicsContextOperation
{
    GeneralInitOperation()
        : GraphicsContextOperation(std::string("General init"))
    {
    }
    void run(osg::GraphicsContext* gc)
    {
        SGPropertyNode* simRendering = fgGetNode("/sim/rendering");

        simRendering->setStringValue("gl-vendor", (char*) glGetString(GL_VENDOR));
        SG_LOG( SG_GENERAL, SG_INFO, glGetString(GL_VENDOR));

        simRendering->setStringValue("gl-renderer", (char*) glGetString(GL_RENDERER));
        SG_LOG( SG_GENERAL, SG_INFO, glGetString(GL_RENDERER));

        simRendering->setStringValue("gl-version", (char*) glGetString(GL_VERSION));
        SG_LOG( SG_GENERAL, SG_INFO, glGetString(GL_VERSION));

        // Old hardware without support for OpenGL 2.0 does not support GLSL and
        // glGetString returns NULL for GL_SHADING_LANGUAGE_VERSION.
        //
        // See http://flightgear.org/forums/viewtopic.php?f=17&t=19670&start=15#p181945
        const char* glsl_version = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);
        if( !glsl_version )
          glsl_version = "UNSUPPORTED";
        simRendering->setStringValue("gl-shading-language-version", glsl_version);
        SG_LOG( SG_GENERAL, SG_INFO, glsl_version);

        GLint tmp;
        glGetIntegerv( GL_MAX_TEXTURE_SIZE, &tmp );
        simRendering->setIntValue("max-texture-size", tmp);

        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tmp);
        simRendering->setIntValue("max-texture-units", tmp);

        const osg::GLExtensions* extensions = gc->getState()->get<osg::GLExtensions>();
        if (extensions->glVertexAttribDivisor) {
            SG_LOG( SG_GENERAL, SG_INFO, "VertexAttribDivisor supported");
        }
        
    }
};

}

/** Initializes GUI.
 * Returns true when done, false when still busy (call again). */
bool guiInit(osg::GraphicsContext* gc)
{
    static osg::ref_ptr<GeneralInitOperation> genOp;
    static bool didInit = false;
    
    if (didInit) {
        return true;
    }
    
    if (!genOp.valid())
    {
        // Pick some window on which to do queries.
        // XXX Perhaps all this graphics initialization code should be
        // moved to renderer.cxx?
        genOp = new GeneralInitOperation;
        if (gc) {
            gc->add(genOp.get());
        } else {
            WindowSystemAdapter* wsa = WindowSystemAdapter::getWSA();
            wsa->windows[0]->gc->add(genOp.get());
        }
        return false; // not ready yet
    }
    else
    {
        if (!genOp->isFinished())
            return false;

        genOp = 0;
        didInit = true;
        // we're done
        return true;
    }
}

void syncPausePopupState()
{
    bool paused = fgGetBool("/sim/freeze/master",true) || fgGetBool("/sim/freeze/clock",true);
    SGPropertyNode_ptr args(new SGPropertyNode);
    args->setStringValue("id", "sim-pause");
    if (paused && fgGetBool("/sim/view-name-popup")) {
      args->setStringValue("label", "Simulation is paused");
      globals->get_commands()->execute("show-message", args, nullptr);
    } else {
      globals->get_commands()->execute("clear-message", args, nullptr);
    }
}
