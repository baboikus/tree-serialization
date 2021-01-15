// #include <iostream>
// #include <string>
// #include <vector>
// #include <functional>
// #include <fstream>
// #include <sstream>
// #include <cstring>

#include "IO.hpp"
#include "Tree.hpp"
#include "test.h"

class Tester
{
public:
	static std::string checkSerialization(const char* caseName,
 								   Tree::Abstract const *expected,
								   Tree::Abstract *init)
	{
		using namespace Tree;

		const auto fileName = std::string(caseName) + ".tree";
		if(std::ifstream(fileName.c_str()) && std::remove(fileName.c_str()))
		{
			return std::string("can't remove testing file ") + fileName; 
		}

		if(!File::saveToFile(fileName, expected))
		{
			return std::string("can't save tree to file ") + fileName;
		};

		init = File::loadFromFile(fileName);
		if(!init)
		{
			return std::string("can't load tree from file ") + fileName;
		}

		std::string error;
		if(!init->isEqual(expected))
		{
			error += "loaded tree is not equal to initial. ";
		}
		if(!expected->isEqual(init))
		{
			error += "initial tree is not equal to loaded. ";
		}
		return error;
	}
};

int main(int argc, char* argv[])
{
	std::cout << "hello babo! it's cmake!" << std::endl;

	for(int i = 0; i < argc; ++i)
	{
		std::cout << i << ": " << argv[i] << std::endl;
	}

	{
		std::cout << std::endl << "Tree::isEqual simple" << std::endl;

		ASSERT_EQUALS("Empty() is always equal to Empty()",
			Tree::Empty().isEqual(new Tree::Empty()), true, "");

		ASSERT_EQUALS("(int 42) is NOT equal to ()",
			Tree::Int(42).isEqual(new Tree::Empty()), false, "");

		ASSERT_EQUALS("(real 42) is NOT equal to ()",
			Tree::Real(42.9).isEqual(new Tree::Empty()), false, "");

		ASSERT_EQUALS("(string asd) is NOT equal to ()",
			Tree::String("asd").isEqual(new Tree::Empty()), false, "");

		ASSERT_EQUALS("() is NOT equal to (int 42)",
		 	Tree::Empty().isEqual(new Tree::Int(42)), false, "");

		ASSERT_EQUALS("(int 42) is equal to (int 42)",
			Tree::Int(42).isEqual(new Tree::Int(42)), true, "");

		ASSERT_EQUALS("(real 7.5) is equal to (real 7.5)",
			Tree::Real(7.5).isEqual(new Tree::Real(7.5)), true, "");

		ASSERT_EQUALS("(real 7.5) is NOT equal to (real 7.499)",
			Tree::Real(7.5).isEqual(new Tree::Real(7.499)), false, "");

		ASSERT_EQUALS("(int 42) is NOT equal to (int 100)",
			Tree::Int(42).isEqual(new Tree::Int(100)), false, "");

		ASSERT_EQUALS("(int 100) is NOT equal to (int 42)",
			Tree::Int(100).isEqual(new Tree::Int(42)), false, "");

		ASSERT_EQUALS("(int 42) is equal to () + (int 42)",
			Tree::Int(42).isEqual((new Tree::Empty())->addChild(new Tree::Int(42))), true, "");

		ASSERT_EQUALS("(string asd) is NOT equal to (string asdf)",
			Tree::String("asd").isEqual(new Tree::String("asdf")), false, "");

		ASSERT_EQUALS("(string asd) is NOT equal to (string qwe)",
			Tree::String("asd").isEqual(new Tree::String("qwe")), false, "");

		ASSERT_EQUALS("(string asd) is equal to (string asd)",
			Tree::String("asd").isEqual(new Tree::String("asd")), true, "");

		ASSERT_EQUALS("(string '') is equal to (string '')",
			Tree::String("").isEqual(new Tree::String("")), true, "");
	}

	{
		std::cout << std::endl << "Tree::isEqual complex" << std::endl;

		auto int42int100 = (new Tree::Int(42))->addChild(new Tree::Int(100));
		auto int100int42 = (new Tree::Int(100))->addChild(new Tree::Int(42));

		Tree::Int int42(42);
		Tree::Int int100(100);

		ASSERT_EQUALS("(int 42(int 100)) is equal to (int 42(int 100))",
			int42int100->isEqual(int42int100),
			true, "");

		ASSERT_EQUALS("(int 100 (int 42)) is NOT equal to (int 42(int 100))",
			int100int42->isEqual(int42int100),
			false, "");

		ASSERT_EQUALS("(int 100 (int 42)) is NOT equal to (int 100)",
			int100int42->isEqual(&int100),
			false, "");

		ASSERT_EQUALS("(int 100 (int 42)) is NOT equal to (int 42)",
			int100int42->isEqual(&int42),
			false, "");
	}

	{
		std::cout << std::endl << "Tree::toText" << std::endl;
	
		ASSERT_EQUALS("()", (new Tree::Empty())->toText(), "()", "");

		ASSERT_EQUALS("(int 42)", (new Tree::Int(42))->toText(), "(int 42)", "");

		ASSERT_EQUALS("(int 42 (int 100, int 333))",
		 (new Tree::Int(42))
		 	->addChild(new Tree::Int(100))
		 	->addChild(new Tree::Int(333))->toText(),
		 "(int 42(int 100int 333))", "");

		 ASSERT_EQUALS("(int 42 (int 100(int 0, int 99), int 333))",
		 (new Tree::Int(42))
		 	->addChild((new Tree::Int(100))
		 		->addChild(new Tree::Int(0))
		 		->addChild(new Tree::Int(99)))
		 	->addChild(new Tree::Int(333))->toText(),
		 "(int 42(int 100(int 0int 99)int 333))", "");
	}

	{
		std::cout << std::endl << "Tree::IO write and read" << std::endl;

		const int zero = 0;
		const int one = 1;
		const int two = 2;
		const int integer1 = 42;
		const int integer2 = -100;
		const int integer3 = 999999;
		const double real1 = 7.5;
		const std::string string1("asd");

		{
			const auto expectedTree = new Tree::Empty();

			std::ostringstream expected(std::ios_base::binary);

			std::ostringstream actual(std::ios_base::binary);
			Tree::OStream(&actual).write(expectedTree);

			ASSERT_EQUALS("write ()", actual.str(), expected.str(), "");

			std::istringstream input(actual.str()); 
			auto actualTree = Tree::IStream(&input).read();
			ASSERT_EQUALS("read ()", actualTree->isEqual(expectedTree), true, "");
		}

		{
			const auto expectedTree = new Tree::Int(integer1);

			std::ostringstream expected(std::ios_base::binary);
			expected << 'i';
			expected.write(reinterpret_cast<const char*>(&zero), sizeof zero);
			const int size = sizeof integer1;
			expected.write(reinterpret_cast<const char*>(&size), sizeof size);
			expected.write(reinterpret_cast<const char*>(&integer1), sizeof integer1);

			std::ostringstream actual(std::ios_base::binary);
			Tree::OStream(&actual).write(expectedTree);

			ASSERT_EQUALS("write (int 42)", actual.str(), expected.str(), "");

			std::istringstream input(actual.str()); 
			auto actualTree = Tree::IStream(&input).read();
			ASSERT_EQUALS("read (int 42)", actualTree->isEqual(expectedTree), true, "");	
		}

		{
			const auto expectedTree = new Tree::Real(real1);

			std::ostringstream expected(std::ios_base::binary);
			expected << 'r';
			expected.write(reinterpret_cast<const char*>(&zero), sizeof zero);
			const int size = sizeof real1;
			expected.write(reinterpret_cast<const char*>(&size), sizeof size);
			expected.write(reinterpret_cast<const char*>(&real1), sizeof real1);

			std::ostringstream actual(std::ios_base::binary);
			Tree::OStream(&actual).write(expectedTree);

			ASSERT_EQUALS("write (real 7.5)", actual.str(), expected.str(), "");

			std::istringstream input(actual.str()); 
			auto actualTree = Tree::IStream(&input).read();
			ASSERT_EQUALS("read (real 7.5)", actualTree->isEqual(expectedTree), true, "");	
		}

		{
			const auto expectedTree = new Tree::String(string1);

			std::ostringstream expected(std::ios_base::binary);
			expected << 's';
			expected.write(reinterpret_cast<const char*>(&zero), sizeof zero);
			const int size = string1.size();
			expected.write(reinterpret_cast<const char*>(&size), sizeof size);
			expected.write(&string1[0], string1.size());

			std::ostringstream actual(std::ios_base::binary);
			Tree::OStream(&actual).write(expectedTree);

			ASSERT_EQUALS("write (string asd)", actual.str(), expected.str(), "");

			std::istringstream input(actual.str()); 
			auto actualTree = Tree::IStream(&input).read();
			ASSERT_EQUALS("read (string asd)", actualTree->isEqual(expectedTree), true, "");	
		}

		{
			const auto expectedTree = (new Tree::Int(integer1))
										->addChild(new Tree::Int(integer2))
										->addChild(new Tree::Int(integer3));

			const int size = sizeof integer1;

			std::ostringstream expected(std::ios_base::binary);
			expected << 'i';
			expected.write(reinterpret_cast<const char*>(&two), sizeof two);
			expected.write(reinterpret_cast<const char*>(&size), sizeof size);
			expected.write(reinterpret_cast<const char*>(&integer1), sizeof integer1);

			expected << 'i';
			expected.write(reinterpret_cast<const char*>(&zero), sizeof zero);
			expected.write(reinterpret_cast<const char*>(&size), sizeof size);
			expected.write(reinterpret_cast<const char*>(&integer2), sizeof integer2);

			expected << 'i';
			expected.write(reinterpret_cast<const char*>(&zero), sizeof zero);
			expected.write(reinterpret_cast<const char*>(&size), sizeof size);
			expected.write(reinterpret_cast<const char*>(&integer3), sizeof integer3);

			std::ostringstream actual(std::ios_base::binary);
			Tree::OStream(&actual).write(expectedTree);

			ASSERT_EQUALS("write (int 42 (int -100, int 999999))", actual.str(), expected.str(), "");	

			std::istringstream input(actual.str());
			{ 
				auto actualTree = Tree::IStream(&input).read();
				ASSERT_EQUALS("read (int 42 (int -100, int 999999))", actualTree->isEqual(expectedTree), true, "");
			}
		}
	}

	{
		std::cout << std::endl << "Tree::File serialization" << std::endl;

		{
			const auto error = 
				Tester::checkSerialization("empty tree ()",
					new Tree::Empty(),
					new Tree::Empty());
			ASSERT_EQUALS("empty tree", error, std::string(), error);
	    }

	    {
	    	const auto error = 
				Tester::checkSerialization("(int 42)",
				 	new Tree::Int(42),
				 	new Tree::Empty());
			ASSERT_EQUALS("(int 42)", error, std::string(), error);
	    }

	    {
	    	const auto error = 
				Tester::checkSerialization("(int 42(int 100, real 99.99))",
					(new Tree::Int(42))
						->addChild(new Tree::Int(100))
						->addChild(new Tree::Real(99.99)),
					new Tree::Empty());
			ASSERT_EQUALS("(int 42(int 100, real 99.99))", error, std::string(), error);
		}
	}

	{
		using namespace Tree;

		const auto exampleTree = 
			(new Int(8))
				->addChild((new String("bar"))
					->addChild((new Real(2.015))
						->addChild(new Int(9)))
					->addChild(new Int(2015))
					->addChild(new String("2015")))
				->addChild((new String("baz"))
					->addChild(new String("foo"))
					->addChild((new Real(6.28318))
						->addChild(new String("hello"))));
		ASSERT_EQUALS("exampleTree is equal to itself", exampleTree->isEqual(exampleTree), true, "");

		const auto error = 
			Tester::checkSerialization("exampleTree",
				exampleTree,
				new Tree::Empty());
		ASSERT_EQUALS("exampleTree serialization", error, std::string(), error);
	}

	return 0;
}