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
 * Date modified: $Date: 2012-10-18 16:08:48 -0500 (Thu, 18 Oct 2012) $
 * Version:       $Rev: 17234 $
 * Author:        $Author: rptaylor $
 * Id:            $Id: NaturalSortQTreeWidgetItem.cxx 17234 2012-10-18 21:08:48Z rptaylor $
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/
#include "NaturalSortQTreeWidgetItem.h"
#include "alphanum.h"

#include <QtCore/QString>

namespace ves
{
namespace conductor
{
NaturalSortQTreeWidgetItem::NaturalSortQTreeWidgetItem(QTreeWidget* parent, const QStringList& strings, int type) :
    QTreeWidgetItem(parent, strings, type)
{
}
////////////////////////////////////////////////////////////////////////////////
bool NaturalSortQTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    int column = treeWidget() ? treeWidget()->sortColumn() : 0;

    // The usual string-sort version looks like this:
    //return text(column) < other.text(column);

    // We replace this with a natural-sort version:
    std::string left = text(column).toStdString();
    std::string right = other.text(column).toStdString();
    return ( doj::alphanum_comp< std::string >( left, right ) < 0 );
}
////////////////////////////////////////////////////////////////////////////////
}} // namespace
