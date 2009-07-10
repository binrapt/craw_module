//http://docs.python.org/extending/extending.html
//http://starship.python.net/crew/mwh/toext/node20.html

#include <Python.h>
#include <structmember.h>
#include <ail/string.hpp>
#include <ail/array.hpp>
#include <ail/file.hpp>
#include "python.hpp"
#include "d2_functions.hpp"
#include "arguments.hpp"
#include "utility.hpp"

namespace python
{
	namespace
	{
		int const
			max_x = 800,
			max_y = 600;

		std::string const
			module_name = "craw",
			error_name = "error",
			monster_data_class = "monster_data";

		PyObject * automap_handler = 0;

		PyObject * module_error;

		PyMethodDef module_methods[] =
		{
			{"set_automap_handler", &set_automap_handler, METH_VARARGS, "This allows you to specify an automap unit handler which is called whenever a unit on the automap is being processed."},

			{"draw_line", &draw_line, METH_VARARGS, "Draws a single line."},
			{"draw_text", &draw_text, METH_VARARGS, "Draws text on the screen."},
			{0, 0, 0, 0}
		};

		PyMemberDef monster_data_members[] =
		{
			{"type", T_UINT, offsetof(python_monster_data, type), 0, "Unit type"},
			{"table_index", T_UINT, offsetof(python_monster_data, table_index), 0, "Offset into the monstats.bin table"},
			{"mode", T_UINT, offsetof(python_monster_data, mode), 0, "Mode of the monster"},

			{"monster_flags", T_UINT, offsetof(python_monster_data, monster_flags), 0, "Monster flags"},

			{"flags", T_UINT, offsetof(python_monster_data, flags), 0, "Monster statistics flags"},
			
			{"treasure_class", T_OBJECT, offsetof(python_monster_data, treasure_class), 0, "Treasure class list"},

			{"level", T_USHORT, offsetof(python_monster_data, level), 0, "Level"},
			{"min_hp", T_USHORT, offsetof(python_monster_data, min_hp), 0, "Minimum hit points"},
			{"max_hp", T_USHORT, offsetof(python_monster_data, max_hp), 0, "Maximum hit points"},
			{"experience", T_USHORT, offsetof(python_monster_data, experience), 0, "Experience"},

			{"damage_resistance", T_USHORT, offsetof(python_monster_data, damage_resistance), 0, "Damage resistance"},
			{"magic_resistance", T_USHORT, offsetof(python_monster_data, magic_resistance), 0, "Resistance to magical damage"},

			{"fire_resistance", T_USHORT, offsetof(python_monster_data, fire_resistance), 0, "Fire resistance"},
			{"lightning_resistance", T_USHORT, offsetof(python_monster_data, lightning_resistance), 0, "Lightning resistance"},
			{"cold_resistance", T_USHORT, offsetof(python_monster_data, cold_resistance), 0, "Cold resistance"},
			{"poison_resistance", T_USHORT, offsetof(python_monster_data, poison_resistance), 0, "Poison resistance"},

			{"x", T_INT, offsetof(python_monster_data, x), 0, "Automap x coordinate"},
			{"y", T_INT, offsetof(python_monster_data, y), 0, "Automap y coordinate"},

			{"colour", T_UBYTE, offsetof(python_monster_data, colour), 0, "Colour"},

			{0}
		};

		PyTypeObject monster_data_type =
		{
			PyObject_HEAD_INIT(NULL)

			0,
			0,
			sizeof(python_monster_data),
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			Py_TPFLAGS_DEFAULT,
			"Contains information about a Diablo II monster/NPC.",
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			monster_data_members,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		};
	}

	PyObject * set_handler(PyObject * self, PyObject * arguments, std::string const & name, PyObject * & output)
	{
		PyObject * new_handler;

		if(!PyArg_ParseTuple(arguments, ("O:" + name).c_str(), &new_handler))
			return 0;

		if(!PyCallable_Check(new_handler))
		{
			PyErr_SetString(PyExc_TypeError, "The handler must be a callable object.");
			return 0;
		}

		Py_XINCREF(new_handler);
		Py_XDECREF(output);
		output = output = new_handler;

		Py_INCREF(Py_None);
		return Py_None;
	}
	
	PyObject * set_automap_handler(PyObject * self, PyObject * arguments)
	{
		return set_handler(self, arguments, "set_automap_handler", automap_handler);
	}

	void fix_coordinate(int & coordinate, int maximum)
	{
		if(coordinate < 0)
			coordinate = 0;
		if(coordinate >= maximum)
			coordinate = maximum - 1;
	}

	PyObject * draw_line(PyObject * self, PyObject * arguments)
	{
		int
			start_x,
			start_y,
			end_x,
			end_y,
			colour;

		if(!PyArg_ParseTuple(arguments, "iiiii", &start_x, &start_y, &end_x, &end_y, &colour))
			return 0;

		fix_coordinate(start_x, max_x);
		fix_coordinate(start_y, max_y);

		fix_coordinate(end_x, max_x);
		fix_coordinate(end_y, max_y);

		colour &= 0xff;

		d2_draw_line(start_x, start_y, end_x, end_y, colour, 0xff);

		Py_INCREF(Py_None);
		return Py_None;
	}

	PyObject * draw_text(PyObject * self, PyObject * arguments)
	{
		char * text;

		int
			x,
			y;

		PyObject * bool_object;

		if(!PyArg_ParseTuple(arguments, "sii|O", &text, &x, &y, &bool_object))
			return 0;

		fix_coordinate(x, max_x);
		fix_coordinate(y, max_y);

		bool centered = (bool_object == Py_True);

		::draw_text(text, x, y, 0, centered);

		Py_INCREF(Py_None);
		return Py_None;
	}

	void python_monster_data::initialise(unit & current_unit)
	{
		uchar difficulty = d2_get_difficulty();

		monster_statistics & statistics = get_monster_statistics(current_unit.table_index);
		monster_data & data = *reinterpret_cast<monster_data *>(current_unit.unit_data);

		type = current_unit.type;
		table_index = current_unit.table_index;
		mode = current_unit.mode;

		if(type == 1)
		{
			monster_flags = data.flags;

			flags = statistics.flags;

			std::size_t treasure_size = ail::countof(statistics.treasure_classes[difficulty].treasure_class);
			treasure_class = PyList_New(treasure_size);
			if(treasure_class == 0)
			{
				error("Failed to create Python list");
				return;
			}

			for(std::size_t i = 0; i < treasure_size; i++)
			{
				PyObject * integer = PyInt_FromLong(statistics.treasure_classes[difficulty].treasure_class[i]);
				if(PyList_SetItem(treasure_class, i, integer) < 0)
				{
					error("Failed to initialise a treasure class item");
					return;
				}
			}

			level = statistics.level[difficulty];
			min_hp = statistics.min_hp[difficulty];
			max_hp = statistics.max_hp[difficulty];
			experience = statistics.experience[difficulty];

			damage_resistance = statistics.damage_resistance[difficulty];
			magic_resistance = statistics.magic_resistance[difficulty];

			fire_resistance = statistics.fire_resistance[difficulty];
			lightning_resistance = statistics.lightning_resistance[difficulty];
			cold_resistance = statistics.cold_resistance[difficulty];
			poison_resistance = statistics.poison_resistance[difficulty];
		}
	}

	void perform_automap_callback(unit & current_unit, int x, int y, uchar colour)
	{
		if(!automap_handler)
			return;

		python_monster_data * monster_data_pointer = PyObject_New(python_monster_data, &monster_data_type);
		python_monster_data & current_monster_data = *monster_data_pointer;
		current_monster_data.initialise(current_unit);
		current_monster_data.x = x;
		current_monster_data.y = y;
		current_monster_data.colour = colour;

		PyObject * return_value = PyObject_CallFunction(automap_handler, "O", monster_data_pointer);
		if(!return_value)
			return;

		Py_DECREF(return_value);
	}

	PyMODINIT_FUNC initialise_module()
	{
		monster_data_type.tp_new = PyType_GenericNew;
		monster_data_type.tp_name = (module_name + "." + monster_data_class).c_str();
		if(PyType_Ready(&monster_data_type) < 0)
		{
			error("Failed to initialise the Python monster data type");
			return;
		}

		PyObject * module = Py_InitModule(module_name.c_str(), module_methods);
		if(module == 0)
			return;

		Py_INCREF(&monster_data_type);
		PyModule_AddObject(module, "monster_data", reinterpret_cast<PyObject *>(&monster_data_type));

		module_error = PyErr_NewException(const_cast<char *>((module_name + "." + error_name).c_str()), 0, 0);
		Py_INCREF(module_error);
		PyModule_AddObject(module, error_name.c_str(), module_error);
	}

	bool get_base_name(std::string const & input, std::string & output)
	{
		std::size_t offset = input.rfind('\\');
		if(offset == std::string::npos)
			return false;
		output = input.substr(0, offset);
		return true;
	}

	bool initialise_python()
	{
		if(python_script.empty())
			return true;

		std::string content;
		if(!ail::read_file(python_script, content))
		{
			error("Failed to load Python script \"" + python_script + "\"");
			return false;
		}

		if(prompt_mode)
			initialise_console();

		content = ail::replace_string(content, "\r", "");

		Py_Initialize();
		initialise_module();
		
		std::string script_directory;
		if(get_base_name(python_script, script_directory))
			PyRun_SimpleString(("import sys\nsys.path.append('" + script_directory + "')\n").c_str());

		if(PyRun_SimpleString(content.c_str()) != 0)
			return false;

		//PyRun_SimpleString(("execfile('" + ail::replace_string(python_script, "\\", "\\\\") + "')").c_str());

		return true;
	}
}
