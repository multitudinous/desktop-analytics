/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * Date modified: $Date: 2012-10-18 22:27:28 -0500 (Thu, 18 Oct 2012) $
 * Version:       $Rev: 17236 $
 * Author:        $Author: mccdo $
 * Id:            $Id: NaturalSortQTreeWidgetItem.h 17236 2012-10-19 03:27:28Z mccdo $
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/
#pragma once
//#include <ves/VEConfig.h>

#include <QtGui/QTreeWidgetItem>

namespace ves
{
namespace conductor
{
class  NaturalSortQTreeWidgetItem : public QTreeWidgetItem
{
public:
    explicit NaturalSortQTreeWidgetItem( QTreeWidget* parent,
                                     const QStringList & strings,
                                     int type = Type );
    bool operator<(const QTreeWidgetItem &other) const;
};

}} // namespace
