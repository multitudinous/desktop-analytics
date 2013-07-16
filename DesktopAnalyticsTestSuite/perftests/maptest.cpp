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

#define JAG3D_USE_GL3W

//#include "DemoInterface.h"

#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/unordered_map.hpp>

#include <boost/timer.hpp>

using namespace boost::bimaps;
#include <Poco/Dynamic/Var.h>
#include <map>
#include <iostream>
#include <sstream>

struct testStruct {
	int derp;
	std::string derp2;
};


// A dictionary is a bidirectional map from strings to strings

typedef bimap<std::string,std::string> dictionary;
typedef dictionary::value_type translation;

int main()
{
    dictionary d;


	Poco::Dynamic::Var testVar;
	testStruct ts;
	ts.derp2 = "woot";
	testVar = ts;
	//std::cout << reinterpret_cast<testStruct>(testVar).derp2; 
	std::cout << testVar.extract<testStruct>().derp2 << std::endl;
    // Fill up our microdictionary.
    // first members Spanish, second members English.

    d.insert( translation("hola" ,"hello"  ));
    d.insert( translation("adios","goodbye"));
    d.insert( translation("rosa" ,"rose"   ));
    d.insert( translation("mesa" ,"table"  ));

    std::cout << "enter a word" << std::endl;
    std::string word;
    //std::getline(std::cin,word);

    // search the queried word on the from index (Spanish)

    dictionary::left_const_iterator it = d.left.find(word);
	/*
    if( it != d.left.end() )
    {
        // the second part of the element is the equivalent in English

        std::cout << word << " is said "
                  << it->second 
                  << " in English" << std::endl;
    }
    else
    {
        // word not found in Spanish, try our luck in English

        dictionary::right_const_iterator it2 = d.right.find(word);
        if( it2 != d.right.end() )
        {
            std::cout << word << " is said "
                      << it2->second 
                      << " in Spanish" << std::endl;
        }
        else
        {
            std::cout << "No such word in the dictionary" << std::endl;
        }
    }*/

	boost::timer t;
	int numHola;
	int numAdios;
	word = "hola";
	for(auto i = 0; i < 1000000; i++) {
		d.left.find(word);
	}
	std::cout << t.elapsed() <<std::endl;

	std::map<std::string, Poco::Dynamic::Var> testmap;
	testmap["derp"]= 42;
	for(auto i = 0; i < 20; i++) {
		std::stringstream ss;
		ss.clear();
		ss << i;
		testmap[ss.str()]= i*10;
		
	}

	boost::timer t2;
	int numfound = 0;
	for(auto i = 0; i < 10000000; i++) {
		if(testmap["derp"]==42)
			numfound++;
	}
	std::cout << t2.elapsed() << " " << numfound << " in std::map" << std::endl;

	boost::unordered_map<std::string, Poco::Dynamic::Var> boostmap;
	boostmap["derp"] = 42;
	for(auto i = 0; i < 20; i++) {
		std::stringstream ss;
		ss.clear();
		ss << i;
		boostmap[ss.str()]= i*10;
		
	}

	boost::timer t3;
	numfound = 0;
	for(auto i = 0; i < 10000000; i++) {
		if(testmap["derp"]==42)
			numfound++;
	}
	std::cout << t3.elapsed() << " " << numfound << " in boost::unordered_map" << std::endl;

    return 0;
}