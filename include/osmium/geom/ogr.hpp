#ifndef OSMIUM_GEOM_OGR_HPP
#define OSMIUM_GEOM_OGR_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#define OSMIUM_COMPILE_WITH_CFLAGS_OGR `gdal-config --cflags`
#define OSMIUM_LINK_WITH_LIBS_OGR `gdal-config --libs`

#include <cassert>
#include <memory>
#include <utility>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wpadded"
# include <ogr_geometry.h>
#pragma GCC diagnostic pop

#include <osmium/geom/coordinates.hpp>
#include <osmium/geom/factory.hpp>

namespace osmium {

    namespace geom {

        namespace detail {

            class OGRFactoryImpl {

            public:

                typedef std::unique_ptr<OGRPoint>        point_type;
                typedef std::unique_ptr<OGRLineString>   linestring_type;
                typedef std::unique_ptr<OGRPolygon>      polygon_type;
                typedef std::unique_ptr<OGRMultiPolygon> multipolygon_type;
                typedef std::unique_ptr<OGRLinearRing>   ring_type;

            private:

                linestring_type   m_linestring;
                multipolygon_type m_multipolygon;
                polygon_type      m_polygon;
                ring_type         m_ring;

            public:

                OGRFactoryImpl() = default;

                /* Point */

                point_type make_point(const osmium::geom::Coordinates& xy) const {
                    return point_type(new OGRPoint(xy.x, xy.y));
                }

                /* LineString */

                void linestring_start() {
                    m_linestring = std::unique_ptr<OGRLineString>(new OGRLineString());
                }

                void linestring_add_location(const osmium::geom::Coordinates& xy) {
                    assert(!!m_linestring);
                    m_linestring->addPoint(xy.x, xy.y);
                }

                linestring_type linestring_finish(size_t /* num_points */) {
                    return std::move(m_linestring);
                }

                /* MultiPolygon */

                void multipolygon_start() {
                    m_multipolygon.reset(new OGRMultiPolygon());
                }

                void multipolygon_polygon_start() {
                    m_polygon.reset(new OGRPolygon());
                }

                void multipolygon_polygon_finish() {
                    assert(!!m_multipolygon);
                    assert(!!m_polygon);
                    m_multipolygon->addGeometryDirectly(m_polygon.release());
                }

                void multipolygon_outer_ring_start() {
                    m_ring.reset(new OGRLinearRing());
                }

                void multipolygon_outer_ring_finish() {
                    assert(!!m_polygon);
                    assert(!!m_ring);
                    m_polygon->addRingDirectly(m_ring.release());
                }

                void multipolygon_inner_ring_start() {
                    m_ring.reset(new OGRLinearRing());
                }

                void multipolygon_inner_ring_finish() {
                    assert(!!m_polygon);
                    assert(!!m_ring);
                    m_polygon->addRingDirectly(m_ring.release());
                }

                void multipolygon_add_location(const osmium::geom::Coordinates& xy) {
                    assert(!!m_polygon);
                    assert(!!m_ring);
                    m_ring->addPoint(xy.x, xy.y);
                }

                multipolygon_type multipolygon_finish() {
                    assert(!!m_multipolygon);
                    return std::move(m_multipolygon);
                }

            }; // class OGRFactoryImpl

        } // namespace detail

        template <class TProjection = IdentityProjection>
        using OGRFactory = GeometryFactory<osmium::geom::detail::OGRFactoryImpl, TProjection>;

    } // namespace geom

} // namespace osmium

#endif // OSMIUM_GEOM_OGR_HPP
