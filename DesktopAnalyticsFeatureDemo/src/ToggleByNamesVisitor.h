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

#pragma once
#include <jagSG/IntersectVisitor.h>
#include <jagSG/Export.h>
#include <jagSG/Visitor.h>
#include <jagBase/Transform.h>
#include <jagDraw/Node.h>
#include <jagDraw/NodeContainer.h>
#include <jagDraw/DrawGraph.h>
#include <jagDraw/TransformCallback.h>
#include <jagDraw/UniformBlock.h>
#include <jagDraw/CommandMap.h>
#include <jagDraw/Drawable.h>
#include <jagDraw/Uniform.h>
#include <jagBase/ptr.h>
#include <gmtl/Ray.h>
#include <gmtl/Xforms.h>

#include <deque>


namespace jagSG {


	

/** \class IntersectVisitor IntersectVisitor.h <jagSG/IntersectVisitor.h>
\brief TBD
\details TBD
*/
class  ToggleByNamesVisitor : public Visitor
{
public:
	ToggleByNamesVisitor();
    ToggleByNamesVisitor( jagSG::NodePtr node, std::vector<std::string> names );
    ToggleByNamesVisitor( const ToggleByNamesVisitor& rhs );
    virtual ~ToggleByNamesVisitor();


   

    /** \brief TBD
    \details TBD */
    void reset();


    /** \brief TBD
    \details TBD */
    virtual void visit( jagSG::Node& node ) {
		bool hlthisnode = false;
		//std::cout << "nodeName:: " << node.getUserDataName() << std::endl;
		CommandMapStackHelper cmdh( *this, node.getCommandMap() );
		
		MatrixStackHelper msh( *this, node.getTransform() );
		
		bool foundName = false;

		//this is not the ideal way to do this
		BOOST_FOREACH(std::string name, _names) {
		if(node.getUserDataName().find(name) != std::string::npos ) {
			foundName = true;
		}	
		}
		if(foundName ) {
			node.setNodeMask(!node.getNodeMask());
		}
		checkMaskAndTraverse(node);
		if(hlthisnode)
			belowHighlight = false;
		//node.setMaxContexts(1);
	}


	
    

    


    

	

  

protected:
    
	void checkMaskAndTraverse(jagSG::Node& node) {
		node.traverse(*this);
	}
	
    unsigned int _currentID;
    bool belowHighlight;

   std::vector<std::string> _names;
};

typedef jagBase::ptr< jagSG::ToggleByNamesVisitor >::shared_ptr ToggleByNamesVisitorPtr;


// jagSG
}




