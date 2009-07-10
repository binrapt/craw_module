#pragma once

#include <Python.h>
#include "d2_data.hpp"

namespace python
{
	struct monster_data
	{
		PyObject_HEAD

		unsigned id;
		unsigned mode;

		unsigned flags;

		PyObject * treasure_class;

		ushort level;
		ushort min_hp;
		ushort max_hp;
		ushort experience;

		ushort damage_resistance;
		ushort magic_resistance;

		ushort fire_resistance;
		ushort lightning_resistance;
		ushort cold_resistance;
		ushort poison_resistance;

		void initialise(unit & current_unit, monster_statistics & statistics);
	};

	PyObject * set_automap_handler(PyObject * self, PyObject * arguments);
	PyObject * draw_line(PyObject * self, PyObject * arguments);
	PyObject * draw_text(PyObject * self, PyObject * arguments);

	void perform_automap_callback(unit & current_unit, monster_statistics & statistics, int x, int y);

	PyMODINIT_FUNC initialise_module();

	bool initialise_python();
}
