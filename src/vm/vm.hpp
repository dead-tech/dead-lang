#ifndef VM_HPP
#define VM_HPP

#include "parser.hpp"

namespace vm
{
	class Vm
	{
	public:
		void run(const char* file_path);
	private:
		Stack stack;
	};
};

#endif //VM_HPP