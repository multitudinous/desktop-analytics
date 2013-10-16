/*************** <auto-copyright.pl BEGIN do not edit this line> **************
*
* jag3d is (C) Copyright 2011-2012 by Kenneth Mark Bryden and Paul Martz
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License version 2.1 as published by the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
*************** <auto-copyright.pl END do not edit this line> ***************/

#include "HighlightNodeByNamesVisitor.h"
#include <jagSG/Node.h>
#include <jagDraw/Node.h>
#include <jagDraw/BufferObject.h>
#include <jagDraw/VertexAttrib.h>
#include <jagBase/gmtlSupport.h>
#include <jagDraw/Error.h>
#include <jagBase/Profile.h>
#include <jagBase/LogMacros.h>
#include <boost/foreach.hpp>
#include <jagDraw/DrawCommand.h>
#include <jagDraw/Drawable.h>
#include <gmtl/gmtl.h>
#include <jagDraw/TriangleSurfer.h>
#include <gmtl/Xforms.h>
#include <cfloat>
#include <gmtl/Ray.h>
#include <gmtl/Intersection.h>
#include <gmtl/Vec.h>
#include "ToggleByNameVisitor.h"


using namespace gmtl;


   

   

namespace jagSG {
	

	
	


HighlightNodeByNamesVisitor::HighlightNodeByNamesVisitor( jagSG::NodePtr node, const std::vector<std::string>& nodeNames, bool addGlow, bool ignoreCase, gmtl::Vec4d glowColor )
	:Visitor("HIGHLIGHTBYNAMES")    
{
    reset();
	_nodeNames = nodeNames;
	belowHighlight = false;
	_addGlow = addGlow;
	_ignoreCase = ignoreCase;
	_glowColor = glowColor;
	
	/*for(int i = 1; i < 2; i++) {
		std::cout << nodeNames[i] << " TOGGLED" << std::endl;
		ToggleByNameVisitor tbn(node, nodeNames[i]);
	}*/
	    node->accept( *this );
}
HighlightNodeByNamesVisitor::HighlightNodeByNamesVisitor( const HighlightNodeByNamesVisitor& rhs )
  : Visitor( rhs )
    
{
    

    reset();
}
HighlightNodeByNamesVisitor::~HighlightNodeByNamesVisitor()
{
}



void HighlightNodeByNamesVisitor::reset()
{
    JAG3D_TRACE( "reset()" );
	
    resetCommandMap();
    resetMatrix();

    
}


}
