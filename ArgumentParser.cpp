/**
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ArgumentParser.h"

#include <algorithm>
#include <cstdlib>

ArgumentParser::ArgumentParser(int m_argc, char* m_argv[]) :
  argv(&m_argv[1], &m_argv[1] + (m_argc-1))
{ }

bool ArgumentParser::is(const std::string& s) const {
  return std::find(argv.begin(), argv.end(), s) != argv.end();
}

std::string ArgumentParser::getStr(const std::string& s, const std::string& defaultValue) const {
  std::vector<std::string>::const_iterator it = std::find(argv.begin(), argv.end(), s);
  return it != argv.end() and (it+1) != argv.end() ?
    *(it+1) :
    defaultValue;
}

int ArgumentParser::getInt(const std::string& s, int defaultValue) const {
  std::vector<std::string>::const_iterator it = std::find(argv.begin(), argv.end(), s);
  return it != argv.end() and (it+1) != argv.end() ?
    atoi((*(it+1)).c_str()) :
    defaultValue;
}

float ArgumentParser::getFloat(const std::string& s, float defaultValue) const {
  std::vector<std::string>::const_iterator it = std::find(argv.begin(), argv.end(), s);
  return it != argv.end() and (it+1) != argv.end()?
    atof((*(it+1)).c_str()) :
    defaultValue;
}
