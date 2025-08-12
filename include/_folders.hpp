#pragma once

#include <filesystem>

std::filesystem::path get_data_path( std::string const& app, std::string const& company = "SFGrenade" );
