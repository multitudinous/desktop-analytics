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



using namespace gmtl;

/**
    * Tests if the given triangle and ray intersect with each other.
    *
    *  @param a,b,c - the triangle vertices (ccw ordering)
    *  @param ray - the ray
    *  @param u,v - tangent space u/v coordinates of the intersection
    *  @param t - an indicator of the intersection location 
    *  @post t gives you the intersection point:
    *         isect = ray.dir * t + ray.origin
    *  @return true if the ray intersects the triangle.
    *  @see from http://www.acm.org/jgt/papers/MollerTrumbore97/code.html
    */
   

   

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
