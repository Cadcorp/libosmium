#include "catch.hpp"

#include <osmium/geom/factory.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/projection.hpp>

TEST_CASE("Identity Projection") {
    osmium::geom::IdentityProjection projection;
    REQUIRE(4326 == projection.epsg());
    REQUIRE("+proj=longlat +datum=WGS84 +no_defs" == projection.proj_string());
}

TEST_CASE("Projection 4326") {
    osmium::geom::Projection projection{4326};
    REQUIRE(4326 == projection.epsg());
#ifdef OSMIUM_USE_PROJ_4
    REQUIRE("+init=epsg:4326" == projection.proj_string());
#endif
#ifdef OSMIUM_USE_PROJ_6
    REQUIRE("EPSG:4326" == projection.proj_string());
#endif

    const osmium::Location loc{1.0, 2.0};
    const osmium::geom::Coordinates c{1.0, 2.0};
    REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
    REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
}

TEST_CASE("Projection 4326 from init string") {
#ifdef OSMIUM_USE_PROJ_4
    osmium::geom::Projection projection{"+init=epsg:4326"};
#endif
#ifdef OSMIUM_USE_PROJ_6
    osmium::geom::Projection projection{"EPSG:4326"};
#endif
    REQUIRE(-1 == projection.epsg());
#ifdef OSMIUM_USE_PROJ_4
    REQUIRE("+init=epsg:4326" == projection.proj_string());
#endif
#ifdef OSMIUM_USE_PROJ_6
    REQUIRE("EPSG:4326" == projection.proj_string());
#endif

    const osmium::Location loc{1.0, 2.0};
    const osmium::geom::Coordinates c{1.0, 2.0};
    REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
    REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
}

TEST_CASE("Creating projection from unknown init string") {
    REQUIRE_THROWS_AS(osmium::geom::Projection{"abc"}, const osmium::projection_error&);
}

TEST_CASE("Creating projection from unknown EPSG code") {
    REQUIRE_THROWS_AS(osmium::geom::Projection{9999999}, const osmium::projection_error&);
}

TEST_CASE("Projection 3857") {
    osmium::geom::Projection projection{3857};
    REQUIRE(3857 == projection.epsg());
#ifdef OSMIUM_USE_PROJ_4
    REQUIRE("+init=epsg:3857" == projection.proj_string());
#endif
#ifdef OSMIUM_USE_PROJ_6
    REQUIRE("EPSG:3857" == projection.proj_string());
#endif

    SECTION("Zero coordinates") {
        const osmium::Location loc{0.0, 0.0};
        const osmium::geom::Coordinates c{0.0, 0.0};
        REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
        REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
    }

    SECTION("Max longitude") {
        const osmium::Location loc{180.0, 0.0};
        const osmium::geom::Coordinates c{20037508.34, 0.0};
        REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
        REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
    }

    SECTION("Min longitude") {
        const osmium::Location loc{-180.0, 0.0};
        const osmium::geom::Coordinates c{-20037508.34, 0.0};
        REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
        REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
    }

    SECTION("Max latitude") {
        const osmium::Location loc{0.0, 85.0511288};
        const osmium::geom::Coordinates c{0.0, 20037508.34};
        REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
        REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
    }
}

TEST_CASE("MercatorProjection: Zero coordinates") {
    osmium::geom::MercatorProjection projection;
    const osmium::Location loc{0.0, 0.0};
    const osmium::geom::Coordinates c{0.0, 0.0};
    REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
    REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
}

TEST_CASE("MercatorProjection: Max longitude") {
    osmium::geom::MercatorProjection projection;
    const osmium::Location loc{180.0, 0.0};
    const osmium::geom::Coordinates c{20037508.34, 0.0};
    REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
    REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
}

TEST_CASE("MercatorProjection: Min longitude") {
    osmium::geom::MercatorProjection projection;
    const osmium::Location loc{-180.0, 0.0};
    const osmium::geom::Coordinates c{-20037508.34, 0.0};
    REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
    REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
}

TEST_CASE("MercatorProjection: Max latitude") {
    osmium::geom::MercatorProjection projection;
    const osmium::Location loc{0.0, 85.0511288};
    const osmium::geom::Coordinates c{0.0, 20037508.34};
    REQUIRE(projection(loc).x == Approx(c.x).epsilon(0.00001));
    REQUIRE(projection(loc).y == Approx(c.y).epsilon(0.00001));
}

