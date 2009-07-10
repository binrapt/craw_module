#pragma once

#include <Python.h>
#include "d2_data.hpp"

namespace python
{
	struct python_monster_data
	{
		PyObject_HEAD

		unsigned type;
		unsigned table_index;
		unsigned mode;

		unsigned monster_flags;

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

		int x;
		int y;

		uchar colour;

		void initialise(unit & current_unit);
	};

	PyObject * set_automap_handler(PyObject * self, PyObject * arguments);

	PyObject * draw_line(PyObject * self, PyObject * arguments);
	PyObject * draw_text(PyObject * self, PyObject * arguments);

	void perform_automap_callback(unit & current_unit, int x, int y, uchar colour);

	PyMODINIT_FUNC initialise_module();

	bool initialise_python();
}
