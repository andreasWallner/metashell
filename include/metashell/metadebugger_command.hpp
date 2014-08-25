#ifndef METADEBUGGER_COMMAND_HPP
#define METADEBUGGER_COMMAND_HPP

// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2014, Andras Kucsma (andras.kucsma@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <string>

namespace metashell {

class metadebugger_shell;

class metadebugger_command {
public:
  typedef void (metadebugger_shell::*function)(const std::string& args);

  metadebugger_command(const std::string& key, function func);

  const std::string& get_key() const;
  function get_func() const;
private:
  std::string key;
  function func;
};

bool operator<(
    const metadebugger_command& lhs,
    const metadebugger_command& rhs);

bool operator<(
    const metadebugger_command& lhs,
    const std::string& rhs);

}

#endif