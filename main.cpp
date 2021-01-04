#include <iostream>
#include <string>

#include "test.h"

namespace Tree
{
class String;

	class Int
	{
	public:
		Int(const int value)
		{

		}

		void addChild(String *child)
		{

		}
	};

	class String
	{
	public:
		String(const char* value)
		{

		}
	};

	class Tester
	{
	public:
		static checkSerialization(const char* caseName, Int *expected)
		{
			expected->saveToFile();
			const auto actual = Tree::fromFile();
			ASSERT_EQUALS(caseName, *actual, *expected, "");
		}
	};
}

int main(int argc, char* argv[])
{
	std::cout << "hello babo! it's cmake!" << std::endl;

	for(int i = 0; i < argc; ++i)
	{
		std::cout << i << ": " << argv[i] << std::endl;
	}

	auto tree = new Tree::Int(8);
	Tree::Tester::checkSerialization("(int 8)", tree);

	tree->addChild(new Tree::String("bar"));
	Tree::Tester::checkSerialization("(int 8(string bar))", tree);
	tree->addChild(new Tree::String("baz"));
	Tree::Tester::checkSerialization("(int 8(string bar, string baz))", tree); 	

	ASSERT_EQUALS("1 is equal to 1", 1, 1, "");
	ASSERT_EQUALS("1 is equal to 2", 1, 2, "1 is not equal to 2");

	return 0;
}