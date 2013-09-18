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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

namespace jagSG {


	

/** \class IntersectVisitor IntersectVisitor.h <jagSG/IntersectVisitor.h>
\brief TBD
\details TBD
*/
class  RemoveNodeNameVisitor : public Visitor
{
public:
	RemoveNodeNameVisitor();
    RemoveNodeNameVisitor( jagSG::NodePtr node );
    RemoveNodeNameVisitor( const RemoveNodeNameVisitor& rhs );
    virtual ~RemoveNodeNameVisitor();


   

    /** \brief TBD
    \details TBD */
    void reset();


    /** \brief TBD
    \details TBD */
    virtual void visit( jagSG::Node& node ) {
		std::string name = node.getUserDataName();

    if( name == "body" )
    {
        name.erase();
    }

    boost::algorithm::erase_regex( name, boost::regex( "^body\\ \\#[0-9]+" ) );

    //if( boost::algorithm::starts_with( name, "body #" ) )
    //{
    //    name.erase();
    //}

    if( !name.empty() )
    {
        boost::algorithm::erase_regex( name, boost::regex( "^a_" ) );

        //Remove all of the #1, #2 suffixes
        boost::algorithm::erase_regex( name, boost::regex( "\\ \\#[0-9]+$" ) );
    }

	node.setUserDataName( name );
		checkMaskAndTraverse(node);
		
	}


	
    

    


    

	

  

protected:
    
	void checkMaskAndTraverse(jagSG::Node& node) {
		node.traverse(*this);
	}
	
    unsigned int _currentID;
    bool belowHighlight;

   std::string _name;
};

typedef jagBase::ptr< jagSG::RemoveNodeNameVisitor >::shared_ptr RemoveNodeNameVisitorPtr;


// jagSG
}




