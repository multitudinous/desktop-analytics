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

#include <boost/algorithm/string/case_conv.hpp>

#include <deque>


namespace jagSG {


	

/** \class IntersectVisitor IntersectVisitor.h <jagSG/IntersectVisitor.h>
\brief TBD
\details TBD
*/
class  HighlightNodeByNamesVisitor : public Visitor
{
public:
	HighlightNodeByNamesVisitor();
	//HighlightNodesByNameVisitor( osg::Node* node, const std::vector< std::string >& nodeNames,
    //                            bool addGlow = true, bool ignoreCase = false,
    //                            osg::Vec3 glowColor = osg::Vec3( 1.0, 0.0, 0.0 ) );
    HighlightNodeByNamesVisitor( jagSG::NodePtr node, const std::vector<std::string>& nodeNames, bool addGlow = true, bool ignoreCase = false, gmtl::Vec4d glowColor = gmtl::Vec4d(1,0,0,1 ));
    HighlightNodeByNamesVisitor( const HighlightNodeByNamesVisitor& rhs );
    virtual ~HighlightNodeByNamesVisitor();


   

    /** \brief TBD
    \details TBD */
    void reset();


    /** \brief TBD
    \details TBD */
    virtual void visit( jagSG::Node& node ) {
		numVisits++;
		//std::cout << "here" << std::endl;
		if(!(numVisits%1000)) 
			std::cout << numVisits << std::endl;
		bool hlthisnode = false;
		bool matchedName = false;

		//std::cout << "nodeName:: " << node.getUserDataName() << std::endl;
		CommandMapStackHelper cmdh( *this, node.getCommandMap() );
		
		MatrixStackHelper msh( *this, node.getTransform() );
		
		std::string nodeName = node.getUserDataName();
		if(_ignoreCase) {
			boost::algorithm::to_lower(nodeName);
		}
		std::string ms;
		BOOST_FOREACH(std::string name, _nodeNames) {
			if(_ignoreCase) {
			boost::algorithm::to_lower(name);
		}
			size_t found = nodeName.find(name);
			//std::cout << name << " " << nodeName << std::endl;
			if(found!= std::string::npos && name.size()!= 0) {
				matchedName=true;
				ms = name;
				//std::cout << "matched '" << name << "' to '" << nodeName <<"'" << std::endl;
				break;
			}
		}
		//matchedName = true;
		if(matchedName || belowHighlight) {
			if(matchedName) {
				hlthisnode = true;
				belowHighlight = true;
			}
			 jagDraw::UniformBlockPtr highlights( jagDraw::UniformBlockPtr(
            new jagDraw::UniformBlock( "Highlight" ) ) );

			 jagDraw::UniformSetPtr usp( jagDraw::UniformSetPtr(
        new jagDraw::UniformSet() ) );
			// std::cout << "ADDED GLOW" << std::endl;
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
	bool _ignoreCase;
	bool _addGlow;
	gmtl::Vec4d _glowColor;
	int numVisits;
  
   std::vector< std::string> _nodeNames;
};

typedef jagBase::ptr< jagSG::HighlightNodeByNamesVisitor >::shared_ptr HighlightNodeByNamesVisitorPtr;


// jagSG
}




