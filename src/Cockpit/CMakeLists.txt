include(FlightGearComponent)

set(SOURCES
    od_gauge.cxx
    render_area_2d.cxx
    )

set(HEADERS
    od_gauge.hxx
    render_area_2d.hxx
    )


flightgear_component(Cockpit "${SOURCES}" "${HEADERS}")
