#include <iostream>
#include <string>
#include <vector>

#include "test.h"

/// брэйншторм
/// использовать щаблоны для данных в нодах?
/// асинхронные читалки и писалки для файлов?
/// использовать смартпоинтеры?
/// формат для хранения дерева в памяти неотличим от формата на диске?
/// хранить всё в сыром char массиве?


namespace Tree
{
class String;

	class Abstract
	{
	public:
		virtual bool isEqual(Abstract *other) const
		{
			if(!other)
			{
				return false;
			}

			if(children_.size() != other->children_.size())
			{
				return false;
			}

			if(!isDataEqual(other))
			{
				return false;
			}

			for(auto myChild = children_.cbegin(), otherChild = other->children_.cbegin();
				myChild != children_.cend() || otherChild != other->children_.cend();
				++myChild, ++otherChild)
			{
				if(!((*myChild)->isDataEqual(*otherChild)))
				{
					return false;
				}
			}
			return true;
		}

		Abstract *addChild(Abstract const *child)
		{
			children_.push_back(child);
			return this;
		}

		virtual std::string toText() const
		{
			std::string text = "(";
			text += dataToText();
			if(children_.size() > 0)
			{
				text += "(";
			}
			for(auto child = children_.cbegin(); child != children_.cend(); ++child)
			{
				if(child != children_.cbegin())
				{
					text += ", ";
				}
				text += (*child)->toText();
			}
			if(children_.size() > 0)
			{
				text += ")";
			}
			text += ")";
			return text;
		}

	protected:
		virtual std::string dataToText() const = 0;
		virtual bool isDataEqual(Abstract const *tree) const = 0;
	private:
		std::vector<Abstract const *> children_;
	};

	class Root : public Abstract
	{
	public:

	protected:
		virtual std::string dataToText() const
		{
			return "";
		}

		virtual bool isDataEqual(Abstract const *tree) const 
		{
		//	std::cout << "Root isDataEqual" << std::endl;
			return dynamic_cast<Root const *>(tree) != nullptr;
		}
	};

	class Int : public Abstract
	{
	public:
		Int(const int value) :
		 data_(value)
		{

		}

		int data() const
		{
			return data_;
		}

	protected:
		virtual std::string dataToText() const
		{
			return std::string("int ") + std::to_string(data());
		}

		virtual bool isDataEqual(Abstract const *tree) const 
		{
			auto intNode = dynamic_cast<Int const *>(tree);
		//	std::cout << "Int isDataEqual " << (intNode == nullptr) << std::endl;
			return intNode && intNode->data() == data(); 
		}

	private:
		int data_;
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
		static bool checkSerialization(const char* caseName, Int *expected)
		{
		//	expected->saveToFile();
		//	const auto actual = Tree::fromFile();
		//	ASSERT_EQUALS(caseName, *actual, *expected, "");
			return false;
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


	{
		std::cout << std::endl << "Tree::isEqual simple" << std::endl;

		ASSERT_EQUALS("root() is always equal to root()",
			Tree::Root().isEqual(new Tree::Root()), true, "");

		ASSERT_EQUALS("(int 42) is NOT equal to ()",
			Tree::Int(42).isEqual(new Tree::Root()), false, "");

		ASSERT_EQUALS("() is NOT equal to (int 42)",
		 	Tree::Root().isEqual(new Tree::Int(42)), false, "");

		ASSERT_EQUALS("(int 42) is equal to (int 42)",
			Tree::Int(42).isEqual(new Tree::Int(42)), true, "");

		ASSERT_EQUALS("(int 42) is NOT equal to (int 100)",
			Tree::Int(42).isEqual(new Tree::Int(100)), false, "");

		ASSERT_EQUALS("(int 100) is NOT equal to (int 42)",
			Tree::Int(100).isEqual(new Tree::Int(42)), false, "");
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
	
		ASSERT_EQUALS("()", (new Tree::Root())->toText(), "()", "");

		ASSERT_EQUALS("(int 42)", (new Tree::Int(42))->toText(), "(int 42)", "");

		ASSERT_EQUALS("(int 42 (int 100, int 333))",
		 (new Tree::Int(42))
		 	->addChild(new Tree::Int(100))
		 	->addChild(new Tree::Int(333))->toText(),
		 "(int 42((int 100), (int 333)))", "");
	}

	// auto tree = new Tree::Int(8);
	// Tree::Tester::checkSerialization("(int 8)", tree);

	// tree->addChild(new Tree::String("bar"));
	// Tree::Tester::checkSerialization("(int 8(string bar))", tree);
	// tree->addChild(new Tree::String("baz"));
	// Tree::Tester::checkSerialization("(int 8(string bar, string baz))", tree); 	

	// ASSERT_EQUALS("1 is equal to 1", 1, 1, "");
	// ASSERT_EQUALS("1 is equal to 2", 1, 2, "1 is not equal to 2");

	return 0;
}