/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009, 2010 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

This file is part of the QGROUNDCONTROL project

    QGROUNDCONTROL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QGROUNDCONTROL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Definition of the class WaypointGroupNode.
 *
 *   @author Lionel Heng <hengli@student.ethz.ch>
 *
 */

#include "WaypointGroupNode.h"

#include <osg/LineWidth>
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>

#include "Imagery.h"

WaypointGroupNode::WaypointGroupNode()
{

}

void
WaypointGroupNode::init(void)
{

}

void
WaypointGroupNode::update(MAV_FRAME frame, UASInterface *uas)
{
    if (uas)
    {
        double robotX = 0.0;
        double robotY = 0.0;
        double robotZ = 0.0;
        if (frame == MAV_FRAME_GLOBAL)
        {
            double latitude = uas->getLatitude();
            double longitude = uas->getLongitude();
            double altitude = uas->getAltitude();

            QString utmZone;
            Imagery::LLtoUTM(latitude, longitude, robotX, robotY, utmZone);
            robotZ = -altitude;
        }
        else if (frame == MAV_FRAME_LOCAL)
        {
            robotX = uas->getLocalX();
            robotY = uas->getLocalY();
            robotZ = uas->getLocalZ();
        }

        if (getNumChildren() > 0)
        {
            removeChild(0, getNumChildren());
        }

        const QVector<Waypoint *>& list = uas->getWaypointManager().getWaypointList();

        for (int i = 0; i < list.size(); i++)
        {
            Waypoint* wp = list.at(i);

            double wpX, wpY, wpZ;
            getPosition(wp, wpX, wpY, wpZ);

            osg::ref_ptr<osg::ShapeDrawable> sd = new osg::ShapeDrawable;
            osg::ref_ptr<osg::Cylinder> cylinder =
                    new osg::Cylinder(osg::Vec3d(0.0, 0.0, -wpZ / 2.0),
                                      wp->getOrbit(),
                                      fabs(wpZ));

            sd->setShape(cylinder);
            sd->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

            if (wp->getCurrent())
            {
                sd->setColor(osg::Vec4(1.0f, 0.3f, 0.3f, 0.5f));
            }
            else
            {
                sd->setColor(osg::Vec4(0.0f, 1.0f, 0.0f, 0.5f));
            }

            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable(sd);

            char wpLabel[10];
            sprintf(wpLabel, "wp%d", i);
            geode->setName(wpLabel);

            if (i < list.size() - 1)
            {
                double nextWpX, nextWpY, nextWpZ;
                getPosition(list.at(i + 1), nextWpX, nextWpY, nextWpZ);

                osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
                osg::ref_ptr<osg::Vec3dArray> vertices = new osg::Vec3dArray;
                vertices->push_back(osg::Vec3d(0.0, 0.0, -wpZ));
                vertices->push_back(osg::Vec3d(nextWpY - wpY,
                                               nextWpX - wpX,
                                               -nextWpZ));
                geometry->setVertexArray(vertices);

                osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
                colors->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 0.5f));
                geometry->setColorArray(colors);
                geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

                geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));

                osg::ref_ptr<osg::LineWidth> linewidth(new osg::LineWidth());
                linewidth->setWidth(2.0f);
                geometry->getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);
                geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

                geode->addDrawable(geometry);
            }

            osg::ref_ptr<osg::PositionAttitudeTransform> pat =
                    new osg::PositionAttitudeTransform;

            pat->setPosition(osg::Vec3d(wpY - robotY,
                                        wpX - robotX,
                                        robotZ));

            addChild(pat);
            pat->addChild(geode);
        }
    }
}

void
WaypointGroupNode::getPosition(Waypoint* wp, double &x, double &y, double &z)
{
    if (wp->getFrame() == MAV_FRAME_GLOBAL)
    {
        double latitude = wp->getY();
        double longitude = wp->getX();
        double altitude = wp->getZ();

        QString utmZone;
        Imagery::LLtoUTM(latitude, longitude, x, y, utmZone);
        z = -altitude;
    }
    else if (wp->getFrame() == MAV_FRAME_LOCAL)
    {
        x = wp->getX();
        y = wp->getY();
        z = wp->getZ();
    }
}
