include(FlightGearComponent)

set(SOURCES
    od_gauge.cxx
    )

set(HEADERS
    od_gauge.hxx
    )


flightgear_component(Cockpit "${SOURCES}" "${HEADERS}")
