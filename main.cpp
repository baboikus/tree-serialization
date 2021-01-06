#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <sstream>

#include "test.h"

/// брэйншторм
/// использовать щаблоны для данных в нодах?
/// асинхронные читалки и писалки для файлов?
/// использовать смартпоинтеры?
/// формат для хранения дерева в памяти неотличим от формата на диске?
/// хранить всё в сыром char массиве?


namespace Tree
{
class Abstract
{
public:
	virtual int childrenCount() const = 0;

	virtual Abstract *addChild(Abstract *child) = 0;

	virtual void traverse(
		std::function<void(Abstract const *)> initial,
		std::function<void(Abstract const *)> final) const = 0;

	bool isLeaf() const
	{
		return childrenCount() == 0;
	}

	bool isEqual(Abstract const *other) const
	{
		return other && (other->toText() == toText());
	}

	std::string toText() const
	{
		std::string text;
		auto initial = [&text](Abstract const *tree){
			text += tree->dataToText();
			if(!tree->isLeaf())
			{
				text += "(";
			}
		};
		auto final = [&text](Abstract const *tree){
			if(!tree->isLeaf())
			{
				text += ")";
			}
		};
		text += "(";
		traverse(initial, final);
		text += ")";
		return text;
	}

protected:
	virtual std::string dataToText() const = 0;
	virtual bool isDataEqual(Abstract const *tree) const = 0;
};

class Empty : public Abstract
{
public:
	virtual int childrenCount() const
	{
		return 0;
	}

	virtual Abstract *addChild(Abstract *child)
	{
		return child ? child : this;
	}

	virtual void traverse(
		std::function<void(Abstract const *)> initial,
		std::function<void(Abstract const *)> final) const
	{
		return;
	}
protected:
	virtual std::string dataToText() const
	{
		return "";
	}

	virtual bool isDataEqual(Abstract const *tree) const 
	{
	//	std::cout << "Empty isDataEqual" << std::endl;
		return dynamic_cast<Empty const *>(tree) != nullptr;
	}
};

class Naive : public Abstract
{
public:
	virtual int childrenCount() const
	{
		return children_.size();
	}

	virtual Abstract *addChild(Abstract *child)
	{
		children_.push_back(child);
		return this;
	}

	virtual void traverse(std::function<void(Abstract const *)> initial,
				  std::function<void(Abstract const *)> final) const
	{
		initial(this);
		for(auto child = children_.cbegin(); child != children_.cend(); ++child)
		{
			(*child)->traverse(initial, final);
		}
		final(this);			
	}

private:
	std::vector<Abstract const *> children_;
};

class Int : public Naive
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

class File
{
public:
	static bool saveToFile(const std::string &fileName, Abstract const *tree)
	{
		return true;
	}
	static bool loadFromFile(const std::string &fileName, Abstract *tree)
	{
		return true;
	}
};

class Tester
{
public:
	static std::string checkSerialization(const char* caseName,
 								   Abstract const *expected,
								   Abstract *init)
	{

		const auto fileName = std::string(caseName) + ".tree";
		if(std::ifstream(fileName.c_str()) && std::remove(fileName.c_str()))
		{
			return std::string("can't remove testing file ") + fileName; 
		}
		std::ofstream stream(fileName.c_str());
		stream << expected->toText();
		if(!File::saveToFile(fileName, expected))
		{
			return std::string("can't save tree to file ") + fileName;
		};
		if(!File::loadFromFile(fileName, init))
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

		ASSERT_EQUALS("Empty() is always equal to Empty()",
			Tree::Empty().isEqual(new Tree::Empty()), true, "");

		ASSERT_EQUALS("(int 42) is NOT equal to ()",
			Tree::Int(42).isEqual(new Tree::Empty()), false, "");

		ASSERT_EQUALS("() is NOT equal to (int 42)",
		 	Tree::Empty().isEqual(new Tree::Int(42)), false, "");

		ASSERT_EQUALS("(int 42) is equal to (int 42)",
			Tree::Int(42).isEqual(new Tree::Int(42)), true, "");

		ASSERT_EQUALS("(int 42) is NOT equal to (int 100)",
			Tree::Int(42).isEqual(new Tree::Int(100)), false, "");

		ASSERT_EQUALS("(int 100) is NOT equal to (int 42)",
			Tree::Int(100).isEqual(new Tree::Int(42)), false, "");

		ASSERT_EQUALS("(int 42) is equal to () + (int 42)",
			Tree::Int(42).isEqual((new Tree::Empty())->addChild(new Tree::Int(42))), true, "");
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
		std::cout << std::endl << "Tree::IO" << std::endl;

		{
			std::ostringstream expected(std::ios_base::binary);

			std::stringstream actual(std::ios_base::binary);

			std::cout << expected.tellp() << " ";
			expected << 'c';
			std::cout << expected.tellp() << " ";
			int i = 32000000;
			expected.write(reinterpret_cast<char*>(&i), sizeof i);
			std::cout << expected.tellp() << " ";
			expected << std::string("asd");
			std::cout << expected.tellp() << " ";
			expected << std::string("йцу");
			std::cout << expected.tellp() << " ";
			expected << 0;
			std::cout << expected.tellp() << " ";
			expected.flush();
			std::cout << expected.tellp() << " ";

			// auto size = expected.tellg();
			// std::cout << size;
			// std::string str(size, '\0');
			// expected.seekg(0);
			// expected.read(&str[0], size);
			// std::cout << "str:" << str << std::endl;

			//expected >> expectedData;

			// Tree::IO::write(stringstream, new Tree::Empty());
			std::string actualData;
			actual >> actualData;

			//std::cout << "a: " << actualData << std::endl;
			std::cout << (1 + 4 + 3 + 6 + 4) << "e: " << expected.tellp() << expected.str() << std::endl;
		}
	}

	{
		std::cout << std::endl << "Tree::File serialization" << std::endl;

		{
			const auto error = 
				Tree::Tester::checkSerialization("empty tree ()",
					new Tree::Empty(),
					new Tree::Empty());
			ASSERT_EQUALS("empty tree", error, std::string(), error);
	    }

	    {
	    	const auto error = 
				Tree::Tester::checkSerialization("(int 42)",
				 	new Tree::Int(42),
				 	new Tree::Empty());
			ASSERT_EQUALS("(int 42)", error, std::string(), error);
	    }

	    {
	    	const auto error = 
				Tree::Tester::checkSerialization("(int 42(int 100))",
					(new Tree::Int(42))->addChild(new Tree::Int(100)),
					new Tree::Empty());
			ASSERT_EQUALS("(int 42(int 100))", error, std::string(), error);
		}
	}

	return 0;
}