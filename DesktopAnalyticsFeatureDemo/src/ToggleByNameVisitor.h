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
class  ToggleByNameVisitor : public Visitor
{
public:
	ToggleByNameVisitor();
    ToggleByNameVisitor( jagSG::NodePtr node, std::string name );
    ToggleByNameVisitor( const ToggleByNameVisitor& rhs );
    virtual ~ToggleByNameVisitor();


   

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
		
		

		if(node.getUserDataName().find(_name) != std::string::npos || belowHighlight) {
			if(node.getUserDataName().find(_name) != std::string::npos) {
				hlthisnode = true;
				belowHighlight = true;
			}
			 jagDraw::UniformBlockPtr highlights( jagDraw::UniformBlockPtr(
            new jagDraw::UniformBlock( "Highlight" ) ) );

			 jagDraw::UniformSetPtr usp( jagDraw::UniformSetPtr(
        new jagDraw::UniformSet() ) );

		jagDraw::UniformPtr up1(new jagDraw::Uniform("hlo", true));
		//highlights->addUniform(up1);
		jagDraw::UniformPtr up2(new jagDraw::Uniform("hlc", gmtl::Vec4f(1,0,1,.5)));
		//highlights->addUniform(up2);
		jagDraw::CommandMapPtr cmp = node.getOrCreateCommandMap();
		//ubsp->insert(highlights);
		usp->insert(up1);
		usp->insert(up2);
		cmp->insert(usp);
		node.setCommandMap(cmp);
		}
		else {
			 jagDraw::UniformBlockPtr highlights( jagDraw::UniformBlockPtr(
            new jagDraw::UniformBlock( "Highlight" ) ) );

			 jagDraw::UniformSetPtr usp( jagDraw::UniformSetPtr(
        new jagDraw::UniformSet() ) );

		jagDraw::UniformPtr up1(new jagDraw::Uniform("hlo", false));
		//highlights->addUniform(up1);
		jagDraw::UniformPtr up2(new jagDraw::Uniform("hlc", gmtl::Vec4f(1,0,1,.5)));
		//highlights->addUniform(up2);
		jagDraw::CommandMapPtr cmp = node.getOrCreateCommandMap();
		//ubsp->insert(highlights);
		usp->insert(up1);
		usp->insert(up2);
		cmp->insert(usp);
		node.setCommandMap(cmp);
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

   std::string _name;
};

typedef jagBase::ptr< jagSG::ToggleByNameVisitor >::shared_ptr ToggleByNameVisitorPtr;


// jagSG
}




