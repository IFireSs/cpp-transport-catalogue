#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace transport::stat {
	void ParseAndPrint(const core::TransportCatalogue& tansport_catalogue, std::string_view request,
		std::ostream& output);
}
