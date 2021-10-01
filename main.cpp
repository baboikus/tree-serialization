#include "IO.hpp"
#include "Tree.hpp"
#include "test.h"

class Tester
{
public:
	static std::string checkSerialization(const char* caseName,
 								   Tree::TreeConstPtr expected,
								   Tree::TreePtr init)
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

	static void runAllTests()
	{
		{
		using namespace Tree;

		std::cout << std::endl << "Tree::isEqual simple" << std::endl;

		ASSERT_EQUALS("Empty() is always equal to Empty()",
			Tree::Empty().isEqual(Tree::makePtr<Empty>()), true, "");

		ASSERT_EQUALS("(int 42) is NOT equal to ()",
			Tree::Int(42).isEqual(Tree::makePtr<Empty>()), false, "");

		ASSERT_EQUALS("(real 42) is NOT equal to ()",
			Tree::Real(42.9).isEqual(Tree::makePtr<Empty>()), false, "");

		ASSERT_EQUALS("(string asd) is NOT equal to ()",
			Tree::String("asd").isEqual(Tree::makePtr<Empty>()), false, "");

		ASSERT_EQUALS("() is NOT equal to (int 42)",
		 	Tree::Empty().isEqual(Tree::makePtr<Int>(42)), false, "");

		ASSERT_EQUALS("(int 42) is equal to (int 42)",
			Tree::Int(42).isEqual(Tree::makePtr<Int>(42)), true, "");

		ASSERT_EQUALS("(real 7.5) is equal to (real 7.5)",
			Tree::Real(7.5).isEqual(Tree::makePtr<Real>(7.5)), true, "");

		ASSERT_EQUALS("(real 7.5) is NOT equal to (real 7.499)",
			Tree::Real(7.5).isEqual(Tree::makePtr<Real>(7.499)), false, "");

		ASSERT_EQUALS("(int 42) is NOT equal to (int 100)",
			Tree::Int(42).isEqual(Tree::makePtr<Int>(100)), false, "");

		ASSERT_EQUALS("(int 100) is NOT equal to (int 42)",
			Tree::Int(100).isEqual(Tree::makePtr<Int>(42)), false, "");

		{
			auto tree = Tree::makePtr<Empty>() + Tree::makePtr<Int>(42);
			ASSERT_EQUALS("(int 42) is equal to () + (int 42)", Tree::Int(42).isEqual(tree), true, "");
		}

		ASSERT_EQUALS("(string asd) is NOT equal to (string asdf)",
			Tree::String("asd").isEqual(Tree::makePtr<String>("asdf")), false, "");

		ASSERT_EQUALS("(string asd) is NOT equal to (string qwe)",
			Tree::String("asd").isEqual(Tree::makePtr<String>("qwe")), false, "");

		ASSERT_EQUALS("(string asd) is equal to (string asd)",
			Tree::String("asd").isEqual(Tree::makePtr<String>("asd")), true, "");

		ASSERT_EQUALS("(string '') is equal to (string '')",
			Tree::String("").isEqual(Tree::makePtr<String>("")), true, "");
	}

	{
		using namespace Tree;

		std::cout << std::endl << "Tree::isEqual complex" << std::endl;

		auto int42int100 = Tree::makePtr<Int>(42) + Tree::makePtr<Int>(100);
		auto int100int42 = Tree::makePtr<Int>(100) + Tree::makePtr<Int>(42);

		auto int42 = Tree::makePtr<Int>(42);
		auto int100 = Tree::makePtr<Int>(100);

		ASSERT_EQUALS("(int 42(int 100)) is equal to (int 42(int 100))",
			int42int100->isEqual(int42int100),
			true, "");

		ASSERT_EQUALS("(int 100 (int 42)) is NOT equal to (int 42(int 100))",
			int100int42->isEqual(int42int100),
			false, "");

		ASSERT_EQUALS("(int 100 (int 42)) is NOT equal to (int 100)",
			int100int42->isEqual(int100),
			false, "");

		ASSERT_EQUALS("(int 100 (int 42)) is NOT equal to (int 42)",
			int100int42->isEqual(int42),
			false, "");
	}

	{
		using namespace Tree;

		std::cout << std::endl << "Tree::toText" << std::endl;
	
		ASSERT_EQUALS("()", (Tree::makePtr<Empty>())->toText(), "()", "");

		ASSERT_EQUALS("(int 42)", (Tree::makePtr<Int>(42))->toText(), "(int 42)", "");

		ASSERT_EQUALS("(int 42 (int 100, int 333))",
		 (Tree::makePtr<Int>(42) 
		 	+ Tree::makePtr<Int>(100) 
		 	+ Tree::makePtr<Int>(333))->toText(),
		 "(int 42(int 100int 333))", "");

		 ASSERT_EQUALS("(int 42 (int 100(int 0, int 99), int 333))",
		 (Tree::makePtr<Int>(42)
		 	+ (Tree::makePtr<Int>(100)
		 		+ Tree::makePtr<Int>(0)
		 		+ Tree::makePtr<Int>(99))
		 	+  Tree::makePtr<Int>(333))->toText(),
		 "(int 42(int 100(int 0int 99)int 333))", "");
	}

	{
		using namespace Tree;

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
			const auto expectedTree = Tree::makePtr<Empty>();

			std::ostringstream expected(std::ios_base::binary);

			std::ostringstream actual(std::ios_base::binary);
			Tree::OStream(&actual).write(expectedTree);

			ASSERT_EQUALS("write ()", actual.str(), expected.str(), "");

			std::istringstream input(actual.str()); 
			auto actualTree = Tree::IStream(&input).read();
			ASSERT_EQUALS("read ()", actualTree->isEqual(expectedTree), true, "");
		}

		{
			const auto expectedTree = Tree::makePtr<Int>(integer1);

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
			const auto expectedTree = Tree::makePtr<Real>(real1);

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
			const auto expectedTree = Tree::makePtr<String>(string1);

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
			const auto expectedTree = Tree::makePtr<Int>(integer1)
										+ Tree::makePtr<Int>(integer2)
										+ Tree::makePtr<Int>(integer3);

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
					Tree::makePtr<Tree::Empty>(),
					Tree::makePtr<Tree::Empty>());
			ASSERT_EQUALS("empty tree", error, std::string(), error);
	    }

	    {
	    	const auto error = 
				Tester::checkSerialization("(int 42)",
				 	Tree::makePtr<Tree::Int>(42),
				 	Tree::makePtr<Tree::Empty>());
			ASSERT_EQUALS("(int 42)", error, std::string(), error);
	    }

	    {
	    	using namespace Tree;

	    	const auto error = 
				Tester::checkSerialization("(int 42(int 100, real 99.99))",
					Tree::makePtr<Int>(42)
						+ Tree::makePtr<Int>(100)
						+ Tree::makePtr<Real>(99.99),
					Tree::makePtr<Empty>());
			ASSERT_EQUALS("(int 42(int 100, real 99.99))", error, std::string(), error);
		}
	}

	{
		using namespace Tree;

		const auto exampleTree = 
			Tree::makePtr<Int>(8)
				+ (Tree::makePtr<String>("bar")
					+ (Tree::makePtr<Real>(2.015)
						+ Tree::makePtr<Int>(9))
					+ Tree::makePtr<Int>(2015)
					+ Tree::makePtr<String>("2015"))
				+ (Tree::makePtr<String>("baz")
					+ Tree::makePtr<String>("foo")
					+ (Tree::makePtr<Real>(6.28318)
						+ Tree::makePtr<String>("hello")));
		ASSERT_EQUALS("exampleTree is equal to itself", exampleTree->isEqual(exampleTree), true, "");

		const auto error = 
			Tester::checkSerialization("exampleTree",
				exampleTree,
				Tree::makePtr<Empty>());
		ASSERT_EQUALS("exampleTree serialization", error, std::string(), error);
	}

	}

	static void runBenchmark(const int depth)
	{
		using namespace Tree;

		auto bigFlopaTree = makePtr<String>("string string string!");
		for(int i = 0; i < depth; ++i)
		{
			bigFlopaTree = bigFlopaTree + makePtr<String>("flopa flopa!");
		}

		checkSerialization("big flopa benchmark", bigFlopaTree, bigFlopaTree);
	}

	static void runBasicTreeBuildingBenchmark(const int dataSize)
	{
		using namespace Tree;

		auto bigFlopaTree = makePtr<String>(std::to_string(std::rand() % 1000000));
		for(int i = 0; i < dataSize; ++i)
		{
			bigFlopaTree + makePtr<String>(std::to_string(std::rand() % 1000000));
		}
	}
};

inline void printHelp()
{
	std::cout << "Usage: tree -i [INPUT_FILE] -o [OUTPUT_FILE]" << std::endl;
	std::cout << "    or tree --run-tests" << std::endl;
}

inline int notEnoughtArgsError()
{
	std::cout << "not enought args" << std::endl;
	printHelp();
	return 1;
}

inline int tooManyArgsError()
{
	std::cout << "too many args" << std::endl;
	printHelp();
	return 2;
}

int main(int argc, char* argv[])
{
	std::string inputFileName;
	std::string outputFileName;

	for(int i = 0; i < argc; ++i)
	{
		auto arg = std::string(argv[i]);
		if(arg == "--run-tests")
		{
			Tester::runAllTests();
			return 0;
		}
		else if(arg == "--run-benchmarks")
		{
			Tester::runBasicTreeBuildingBenchmark(std::stoi(argv[i + 1]));
			return 0;
		}
		else if(arg == "-i")
		{
			if(!inputFileName.empty())
			{
				return tooManyArgsError();
			}

			++i;
			if(i < argc)
			{
				inputFileName = argv[i];
			}
			else
			{
				return notEnoughtArgsError();
			}
		}
		else if(arg == "-o")
		{
			if(!outputFileName.empty())
			{
				return tooManyArgsError();
			}

			++i;
			if(i < argc)
			{
				outputFileName = argv[i];
			}
			else
			{
				return notEnoughtArgsError();
			}
		}
	}
	
	if(inputFileName.empty() || outputFileName.empty())
	{
		return notEnoughtArgsError();
	}

	auto tree = Tree::File::loadFromFile(inputFileName);
	if(!tree)
	{
		std::cout << "load file error" << std::endl;
		printHelp();
		return 3;
	}
	tree->print();
	Tree::File::saveToFile(outputFileName, tree);

	return 0;
}
