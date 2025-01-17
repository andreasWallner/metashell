#ifndef METASHELL_ENGINE_CLANG_PREPROCESSOR_SHELL_HPP
#define METASHELL_ENGINE_CLANG_PREPROCESSOR_SHELL_HPP

// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2016, Abel Sinkovics (abel@sinkovics.hu)
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

#include <metashell/iface/preprocessor_shell.hpp>

#include <metashell/engine/clang/binary.hpp>

namespace metashell
{
  namespace engine
  {
    namespace clang
    {
      class preprocessor_shell : public iface::preprocessor_shell
      {
      public:
        explicit preprocessor_shell(binary binary_);

        virtual data::result precompile(const data::cpp_code& exp_) override;

      private:
        binary _binary;
      };
    } // namespace clang
  } // namespace engine
} // namespace metashell

#endif
