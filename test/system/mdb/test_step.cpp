// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2015, Andras Kucsma (andras.kucsma@gmail.com)
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

#include <metashell/system_test/any_of.hpp>
#include <metashell/system_test/error.hpp>
#include <metashell/system_test/frame.hpp>
#include <metashell/system_test/metashell_instance.hpp>
#include <metashell/system_test/nocaches.hpp>
#include <metashell/system_test/prompt.hpp>
#include <metashell/system_test/raw_text.hpp>
#include <metashell/system_test/type.hpp>

#include "test_metaprograms.hpp"

#include <gtest/gtest.h>

#include <array>

using namespace metashell::system_test;

using pattern::_;

namespace
{
  json_string step(metashell_instance& mi_)
  {
    return mi_.command("step").front();
  }

  json_string skip_any_further(const std::vector<frame>& frames_,
                               metashell_instance& mi_)
  {
    json_string result = step(mi_);
    while (std::find(frames_.begin(), frames_.end(), result) != frames_.end())
    {
      result = step(mi_);
    }
    return result;
  }

  const std::vector<frame> fib_memoization(std::initializer_list<int> values_)
  {
    std::vector<frame> result;
    for (int value : values_)
    {
      result.emplace_back(type{"fib<" + std::to_string(value) + ">"}, _, _,
                          event_kind::memoization);
    }
    return result;
  }

  const std::vector<frame> fib_additional(int n_)
  {
    std::vector<frame> result = fib_memoization({n_});
    result.emplace_back(type{"fib<" + std::to_string(n_) + ">::value"}, _, _,
                        event_kind::template_instantiation);
    result.emplace_back(type{"fib<" + std::to_string(n_) + ">::value"});
    return result;
  }

} // namespace

TEST(mdb_step, without_evaluation)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;

    mi.command("#msh mdb" + nocache);

    ASSERT_EQ(error("Metaprogram not evaluated yet"), step(mi));
  }
}

TEST(mdb_step, fibonacci)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
              step(mi));
  }
}

TEST(mdb_step, two_fibonacci)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(
        any_of<frame>(
            frame{type{"fib<8>"}, _, _, event_kind::template_instantiation},
            frame{type{"fib<10>"}, _, _, event_kind::memoization}),
        mi.command("step 2").front());
  }
}

TEST(mdb_step, fibonacci_twice)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
              step(mi));

    ASSERT_EQ(
        any_of<frame>(
            frame{type{"fib<8>"}, _, _, event_kind::template_instantiation},
            frame{type{"fib<10>"}, _, _, event_kind::memoization}),
        step(mi));
  }
}

TEST(mdb_step, fibonacci_twice_with_empty_second_line)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
              step(mi));

    ASSERT_EQ(
        any_of<frame>(
            frame{type{"fib<8>"}, _, _, event_kind::template_instantiation},
            frame{type{"fib<10>"}, _, _, event_kind::memoization}),
        step(mi));
  }
}

TEST(mdb_step, fibonacci_twice_with_space_second_line)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
              step(mi));

    // space doesn't repeat
    ASSERT_EQ(prompt("(mdb)"), mi.command(" ").front());
  }
}

TEST(mdb_step, zero_fibonacci_at_start)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    // step 0 doesn't print anything at start
    ASSERT_EQ(prompt("(mdb)"), mi.command("step 0").front());
  }
}

TEST(mdb_step, zero_fibonacci_after_step)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
              step(mi));

    ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
              mi.command("step 0").front());
  }
}

TEST(mdb_step, zero_fibonacci_at_end)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int_<55>")), to_json_string(prompt("(mdb)"))}),
        mi.command("continue"));

    // step 0 doesn't print anything at end
    ASSERT_EQ(prompt("(mdb)"), mi.command("step 0").front());
  }
}

TEST(mdb_step, over_the_whole_metaprogram_one_step)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int_<55>")), to_json_string(prompt("(mdb)"))}),
        mi.command("step 128"));
  }
}

TEST(mdb_step, int_non_template_type)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command("#msh mdb" + nocache + " int");

    ASSERT_EQ(
        frame(type("int"), _, _, event_kind::non_template_type), step(mi));

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int")), to_json_string(prompt("(mdb)"))}),
        mi.command("step"));
  }
}

TEST(mdb_step, over_the_whole_metaprogram_multiple_steps)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
              step(mi));
    ASSERT_EQ(frame(type("fib<8>"), _, _, event_kind::template_instantiation),
              skip_any_further(fib_additional(10), mi));
    ASSERT_EQ(frame(type("fib<6>"), _, _, event_kind::template_instantiation),
              skip_any_further(fib_additional(8), mi));
    ASSERT_EQ(frame(type("fib<4>"), _, _, event_kind::template_instantiation),
              skip_any_further(fib_additional(6), mi));
    ASSERT_EQ(frame(type("fib<2>"), _, _, event_kind::template_instantiation),
              skip_any_further(fib_additional(4), mi));
    ASSERT_EQ(frame(type("fib<0>"), _, _, event_kind::memoization),
              skip_any_further(fib_additional(2), mi));
    ASSERT_EQ(frame(type{"fib<1>"}, _, _, event_kind::memoization), step(mi));

    const json_string after_1 = step(mi);
    if (after_1 !=
        frame{type{"fib<3>"}, _, _, event_kind::template_instantiation})
    {
      ASSERT_EQ(frame(type{"fib<2>"}, _, _, event_kind::memoization), after_1);
      ASSERT_EQ(frame(type{"fib<3>"}, _, _, event_kind::template_instantiation),
                step(mi));
    }

    ASSERT_EQ(frame(type("fib<1>"), _, _, event_kind::memoization),
              skip_any_further(fib_additional(3), mi));
    ASSERT_EQ(frame(type("fib<2>"), _, _, event_kind::memoization), step(mi));

    const json_string after_2 = step(mi);
    if (after_2 !=
        frame{type{"fib<5>"}, _, _, event_kind::template_instantiation})
    {
      ASSERT_EQ(frame(type{"fib<3>"}, _, _, event_kind::memoization), after_2);
      ASSERT_EQ(frame(type{"fib<4>"}, _, _, event_kind::memoization), step(mi));
      ASSERT_EQ(frame(type{"fib<5>"}, _, _, event_kind::template_instantiation),
                step(mi));
    }

    ASSERT_EQ(frame(type("fib<3>"), _, _, event_kind::memoization),
              skip_any_further(fib_additional(5), mi));
    ASSERT_EQ(frame(type("fib<4>"), _, _, event_kind::memoization), step(mi));

    const json_string after_4 = step(mi);
    if (after_4 !=
        frame{type{"fib<7>"}, _, _, event_kind::template_instantiation})
    {
      ASSERT_EQ(frame(type{"fib<5>"}, _, _, event_kind::memoization), after_4);
      ASSERT_EQ(frame(type{"fib<6>"}, _, _, event_kind::memoization), step(mi));
      ASSERT_EQ(frame(type{"fib<7>"}, _, _, event_kind::template_instantiation),
                step(mi));
    }

    ASSERT_EQ(frame(type("fib<5>"), _, _, event_kind::memoization),
              skip_any_further(fib_additional(7), mi));
    ASSERT_EQ(frame(type("fib<6>"), _, _, event_kind::memoization), step(mi));

    const json_string after_6 = step(mi);
    if (after_6 !=
        frame{type{"fib<9>"}, _, _, event_kind::template_instantiation})
    {
      ASSERT_EQ(frame(type{"fib<7>"}, _, _, event_kind::memoization), after_6);
      ASSERT_EQ(frame(type{"fib<8>"}, _, _, event_kind::memoization), step(mi));
      ASSERT_EQ(frame(type{"fib<9>"}, _, _, event_kind::template_instantiation),
                step(mi));
    }

    ASSERT_EQ(frame(type("fib<7>"), _, _, event_kind::memoization),
              skip_any_further(fib_additional(9), mi));
    ASSERT_EQ(frame(type("fib<8>"), _, _, event_kind::memoization), step(mi));

    const json_string after_8 = step(mi);
    if (after_8 !=
        frame{type{"int_<55>"}, _, _, event_kind::template_instantiation})
    {
      ASSERT_EQ(frame(type{"fib<9>"}, _, _, event_kind::memoization), after_8);
      ASSERT_EQ(
          frame(type{"fib<10>"}, _, _, event_kind::memoization), step(mi));
      ASSERT_EQ(
          frame(type{"int_<55>"}, _, _, event_kind::template_instantiation),
          step(mi));
    }

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int_<55>")), to_json_string(prompt("(mdb)"))}),
        mi.command("step"));
  }
}

TEST(mdb_step, over_environment_multiple_steps)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;

    mi.command(fibonacci_mp +
               "int_<fib<5>::value> x;"
               "int_<fib<6>::value> y;");
    mi.command("#msh mdb" + nocache + " -");

    const frame memoization5{type{"int_<5>"}, _, _, event_kind::memoization};
    const frame memoization8{type{"int_<8>"}, _, _, event_kind::memoization};

    ASSERT_EQ(frame(type("fib<5>"), _, _, event_kind::template_instantiation),
              step(mi));
    ASSERT_EQ(frame(type("fib<3>"), _, _, event_kind::template_instantiation),
              skip_any_further(fib_additional(5), mi));
    ASSERT_EQ(frame(type("fib<1>"), _, _, event_kind::memoization),
              skip_any_further(fib_additional(3), mi));
    ASSERT_EQ(frame(type("fib<2>"), _, _, event_kind::template_instantiation),
              step(mi));
    ASSERT_EQ(frame(type("fib<0>"), _, _, event_kind::memoization),
              skip_any_further(fib_additional(2), mi));
    ASSERT_EQ(frame(type("fib<4>"), _, _, event_kind::template_instantiation),
              skip_any_further(fib_memoization({1, 2, 3}), mi));
    ASSERT_EQ(frame(type("fib<2>"), _, _, event_kind::memoization),
              skip_any_further(fib_additional(4), mi));
    ASSERT_EQ(frame(type("fib<3>"), _, _, event_kind::memoization), step(mi));

    const json_string after_3 = step(mi);
    if (after_3 !=
        frame{type{"int_<5>"}, _, _, event_kind::template_instantiation})
    {
      ASSERT_EQ(frame(type{"fib<4>"}, _, _, event_kind::memoization), after_3);
      ASSERT_EQ(frame(type{"fib<5>"}, _, _, event_kind::memoization), step(mi));
      ASSERT_EQ(
          frame(type{"int_<5>"}, _, _, event_kind::template_instantiation),
          step(mi));
    }

    ASSERT_EQ(memoization5, step(mi));
    ASSERT_EQ(frame(type("fib<6>"), _, _, event_kind::template_instantiation),
              skip_any_further({memoization5}, mi));
    ASSERT_EQ(frame(type("fib<4>"), _, _, event_kind::memoization),
              skip_any_further(fib_additional(6), mi));
    ASSERT_EQ(frame(type("fib<5>"), _, _, event_kind::memoization), step(mi));

    const json_string after_5 = step(mi);

    if (after_5 !=
        frame{type{"int_<8>"}, _, _, event_kind::template_instantiation})
    {
      ASSERT_EQ(frame(type{"fib<6>"}, _, _, event_kind::memoization), after_5);
      ASSERT_EQ(
          frame(type{"int_<8>"}, _, _, event_kind::template_instantiation),
          step(mi));
    }

    ASSERT_EQ(memoization8, step(mi));

    ASSERT_EQ(
        raw_text("Metaprogram finished"), skip_any_further({memoization8}, mi));
  }
}

TEST(mdb_step, over_the_whole_metaprogram_multiple_steps_in_full_mode)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;

    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " -full int_<fib<4>::value>");

    ASSERT_EQ(frame(type("fib<4>")), step(mi));
    ASSERT_EQ(frame(type("fib<2>")), skip_any_further(fib_additional(4), mi));
    ASSERT_EQ(frame(type("fib<0>")), skip_any_further(fib_additional(2), mi));
    ASSERT_EQ(frame(type("fib<1>")), step(mi));
    ASSERT_EQ(frame(type("fib<3>")), step(mi));
    ASSERT_EQ(frame(type("fib<1>")), skip_any_further(fib_additional(3), mi));
    ASSERT_EQ(frame(type("fib<2>")), step(mi));

    const json_string after_2 = step(mi);
    if (after_2 != frame{type{"int_<3>"}})
    {
      ASSERT_EQ(frame{type{"fib<0>"}}, after_2);
      ASSERT_EQ(frame{type{"fib<1>"}}, step(mi));
      ASSERT_EQ(frame{type{"int_<3>"}}, step(mi));
    }

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int_<3>")), to_json_string(prompt("(mdb)"))}),
        mi.command("step"));
  }
}

TEST(mdb_step, minus_1_at_start)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(raw_text("Metaprogram reached the beginning"),
              mi.command("step -1").front());
  }
}

TEST(mdb_step, minus_1_after_step)
{
  metashell_instance mi;
  mi.command(fibonacci_mp);
  mi.command("#msh mdb int_<fib<10>::value>");
  mi.command("step 1");

  ASSERT_EQ(raw_text("Metaprogram reached the beginning"),
            mi.command("step -1").front());
}

TEST(mdb_step, minus_1_after_step_in_full_mode)
{
  metashell_instance mi;
  mi.command(fibonacci_mp);
  mi.command("#msh mdb -full int_<fib<10>::value>");
  mi.command("step 1");

  ASSERT_EQ(raw_text("Metaprogram reached the beginning"),
            mi.command("step -1").front());
}

TEST(mdb_step, minus_1_after_step_2)
{
  metashell_instance mi;
  mi.command(fibonacci_mp);
  mi.command("#msh mdb int_<fib<10>::value>");
  mi.command("step 2");

  ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
            mi.command("step -1").front());
}

TEST(mdb_step, minus_1_after_step_2_in_full_mode)
{
  metashell_instance mi;
  mi.command(fibonacci_mp);
  mi.command("#msh mdb -full int_<fib<10>::value>");
  mi.command("step 2");

  ASSERT_EQ(frame(type("fib<10>")), mi.command("step -1").front());
}

TEST(mdb_step, over_fib_from_root)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int_<55>")), to_json_string(prompt("(mdb)"))}),
        mi.command("step over"));
  }
}

TEST(mdb_step, over_fib_from_after_step)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<10>::value>");

    ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
              mi.command("step").front());

    ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::memoization),
              mi.command("step over").front());
  }
}

TEST(mdb_step, over_minus_1_fib_from_after_step)
{
  metashell_instance mi;
  mi.command(fibonacci_mp);
  mi.command("#msh mdb int_<fib<10>::value>");

  ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
            mi.command("step").front());

  ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::memoization),
            mi.command("step over").front());

  ASSERT_EQ(frame(type("fib<10>"), _, _, event_kind::template_instantiation),
            mi.command("step over -1").front());
}

TEST(mdb_step, over_minus_1_multi_fib_from_after_step)
{
  metashell_instance mi;
  mi.command(multi_fibonacci_mp);
  mi.command("#msh mdb int_<multi_fib<10>::value>");

  const json_string step_4 = mi.command("step 4").front();

  if (step_4 ==
      frame{type{"multi_fib<6>"}, _, _, event_kind::template_instantiation})
  {
    ASSERT_EQ(frame(type("multi_fib<6>"), _, _, event_kind::memoization),
              mi.command("step over").front());

    ASSERT_EQ(frame(type("multi_fib<6>::value"), _, _,
                    event_kind::template_instantiation),
              mi.command("step over").front());

    ASSERT_EQ(frame(type("multi_fib<6>"), _, _, event_kind::memoization),
              mi.command("step over -1").front());
  }
  else
  {
    ASSERT_EQ(
        frame(type{"multi_fib<4>"}, _, _, event_kind::template_instantiation),
        step_4);

    ASSERT_EQ(frame(type("multi_fib<4>"), _, _, event_kind::memoization),
              mi.command("step over").front());

    ASSERT_EQ(frame(type("multi_fib<3>"), _, _, event_kind::memoization),
              mi.command("step over").front());

    ASSERT_EQ(frame(type("multi_fib<4>"), _, _, event_kind::memoization),
              mi.command("step over -1").front());
  }
}

TEST(mdb_step, out_fib_from_root)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<5>::value>");

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int_<5>")), to_json_string(prompt("(mdb)"))}),
        mi.command("step out"));
  }
}

TEST(mdb_step, out_fib_after_one_step)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<5>::value>");
    mi.command("step");

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int_<5>")), to_json_string(prompt("(mdb)"))}),
        mi.command("step out"));
  }
}

TEST(mdb_step, out_fib_after_two_steps)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<5>::value>");
    mi.command("step 2");

    const json_string step_out = mi.command("step out").front();

    if (step_out != raw_text{"Metaprogram finished"})
    {
      ASSERT_EQ(frame(type{"fib<5>"}, _, _, event_kind::memoization), step_out);
    }
  }
}

TEST(mdb_step, out_fib_after_three_steps)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<5>::value>");
    mi.command("step 3");

    const json_string step_out = mi.command("step out").front();

    if (step_out != raw_text{"Metaprogram finished"})
    {
      ASSERT_EQ(frame(type{"fib<3>"}, _, _, event_kind::memoization), step_out);
    }
  }
}

TEST(mdb_step, out_fib_twice_after_five_steps)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<5>::value>");
    mi.command("step 5");

    const json_string step_out_2 = mi.command("step out 2").front();

    if (step_out_2 != raw_text{"Metaprogram finished"})
    {
      ASSERT_EQ(
          frame(type{"fib<3>"}, _, _, event_kind::memoization), step_out_2);
    }
  }
}

TEST(mdb_step, out_fib_four_times_after_five_steps)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<5>::value>");
    mi.command("step 5");

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int_<5>")), to_json_string(prompt("(mdb)"))}),
        mi.command("step out 4"));
  }
}

TEST(mdb_step, out_fib_four_after_five_steps)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<5>::value>");
    mi.command("step 5");

    ASSERT_EQ(
        (std::vector<json_string>{
            to_json_string(raw_text("Metaprogram finished")),
            to_json_string(type("int_<5>")), to_json_string(prompt("(mdb)"))}),
        mi.command("step out 4"));
  }
}

TEST(mdb_step, out_minus_1_at_root_of_fib)
{
  metashell_instance mi;
  mi.command(fibonacci_mp);
  mi.command("#msh mdb int_<fib<5>::value>");

  ASSERT_EQ(raw_text("Metaprogram reached the beginning"),
            mi.command("step out -1").front());
}

TEST(mdb_step, out_minus_1_after_step_4_in_fib)
{
  metashell_instance mi;
  mi.command(fibonacci_mp);
  mi.command("#msh mdb int_<fib<5>::value>");
  mi.command("step 4");

  ASSERT_EQ(any_of<frame>(
                frame{type{"fib<3>"}, _, _, event_kind::template_instantiation},
                frame{type{"fib<5>::value"}, _, _,
                      event_kind::template_instantiation}),
            mi.command("step out -1").front());
}

TEST(mdb_step, over_template_spec_no_deduced_event)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(template_specialization_mp);
    mi.command("#msh mdb" + nocache + " int_<foo<3, 1>::value>");

    ASSERT_EQ(
        frame(type("foo<3, 1>"), _, _, event_kind::template_instantiation),
        step(mi));

    ASSERT_EQ(frame(type("foo<N, 1>"), _, _,
                    event_kind::deduced_template_argument_substitution),
              step(mi));

    ASSERT_EQ(
        frame(type("foo<3, 1>"), _, _, event_kind::memoization), step(mi));

    ASSERT_EQ(any_of<frame>(frame{type{"int_<45>"}, _, _,
                                  event_kind::template_instantiation},
                            frame{type{"foo<3, 1>::value"}, _, _,
                                  event_kind::template_instantiation}),
              step(mi));

    const std::vector<json_string> last = mi.command("step");

    ASSERT_EQ(prompt{"(mdb)"}, last.back());

    if (last.size() == 2)
    {
      ASSERT_EQ(
          frame(type{"int_<45>"}, _, _, event_kind::template_instantiation),
          last[0]);
    }
    else
    {
      ASSERT_EQ(3, last.size());
      ASSERT_EQ(raw_text{"Metaprogram finished"}, last[0]);
      ASSERT_EQ(type{"int_<45>"}, last[1]);
    }
  }
}

TEST(mdb_step, garbage_argument)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<2>::value>");

    ASSERT_EQ(
        error("Error: Invalid integer: asd\n"), mi.command("step asd").front());
  }
}

TEST(mdb_step, cant_step_forward_when_metaprogram_errored)
{
  for (const std::string& nocache : nocaches())
  {
    metashell_instance mi;
    mi.command(missing_value_fibonacci_mp);
    mi.command("#msh mdb" + nocache + " int_<fib<5>::value>");

    const std::vector<json_string> r_continue = mi.command("continue");

    ASSERT_EQ(raw_text("Metaprogram finished"), r_continue[0]);
    ASSERT_EQ(error(_), r_continue[1]);

    const std::vector<json_string> r_step = mi.command("step");

    ASSERT_EQ(raw_text("Metaprogram finished"), r_step[0]);
    ASSERT_EQ(error(_), r_step[1]);
  }
}

TEST(mdb_step, step_backwards_fails_without_caching)
{
  const error caching_backwards_disabled(
      "Error: Caching is disabled in the debugger and stepping backwards "
      "requires caching.\n");

  metashell_instance mi;
  mi.command("#msh mdb -nocache int");
  mi.command("step");
  ASSERT_EQ(caching_backwards_disabled, mi.command("step -1").front());
  ASSERT_EQ(caching_backwards_disabled, mi.command("step over -1").front());
  ASSERT_EQ(caching_backwards_disabled, mi.command("step out -1").front());
}
