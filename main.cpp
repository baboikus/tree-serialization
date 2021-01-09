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

	virtual void writeDataToStream(std::ostream *stream) const = 0;

	virtual char type() const = 0;
	virtual std::pair<const char*, int> bytes() const = 0;
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

	virtual void writeDataToStream(std::ostream *stream) const
	{
		return;
	}

	virtual char type() const 
	{
		return 'e';
	}
	virtual std::pair<const char*, int> bytes() const
	{
		return {nullptr, 0};
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

	virtual void writeDataToStream(std::ostream *stream) const
	{
		(*stream) << 'i';
		stream->write(reinterpret_cast<const char*>(&data_), sizeof data_);
	}

	virtual char type() const 
	{
		return 'i';
	}

	virtual std::pair<const char*, int> bytes() const
	{
		return {reinterpret_cast<const char*>(&data_), sizeof data_};
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

class IO
{
public:
	static std::ostream* write(std::ostream *stream, Abstract const * tree)
	{		
		auto initial = [&stream](Abstract const *tree){
			const auto type = tree->type();
			const auto [data, dataSize] = tree->bytes();
			if(dataSize <= 0)
			{
				return ;
			}
			const auto childrenCount = tree->childrenCount();
			stream->write(reinterpret_cast<const char*>(&type), sizeof type);
			stream->write(reinterpret_cast<const char*>(&childrenCount), sizeof childrenCount);
			stream->write(reinterpret_cast<const char*>(&dataSize), sizeof dataSize);
			stream->write(reinterpret_cast<const char*>(data), dataSize);
		};
		tree->traverse(initial, [](Abstract const *){});
		stream->flush();

		return stream;
	}

	static Abstract* read(std::istream *stream, Abstract *tree = nullptr)
	{
		char type = 'e';
		int dataSize = 0;
		int childrenCount = 0;

		stream->read(reinterpret_cast<char*>(&type), sizeof type);
		stream->read(reinterpret_cast<char*>(&childrenCount), sizeof childrenCount);
		stream->read(reinterpret_cast<char*>(&dataSize), sizeof dataSize);
		if(dataSize <= 0 || childrenCount < 0)
		{
			return new Empty();
		}
		Abstract *newSubTree = nullptr;
		switch(type)
		{
			case 'i':
				int data;
				stream->read(reinterpret_cast<char*>(&data), sizeof data);
				std::cout << "data: " << data << " " << sizeof data << std::endl;
				newSubTree = new Int(data);
		}
		if(tree == nullptr)
		{
			tree = newSubTree;
		}
		else
		{
			tree->addChild(newSubTree);
		}
		for(int i = 0; i < childrenCount; ++i)
		{
			read(stream, tree);
		}		

		return tree;
	}
};

class File
{
public:
	static bool saveToFile(const std::string &fileName, Abstract const *tree)
	{
		std::ofstream stream(fileName.c_str());
		if(!stream)
		{
			return false;
		}
		IO::write(&stream, tree);
		return true;
	}
	static Abstract* loadFromFile(const std::string &fileName)
	{
		std::ifstream stream(fileName.c_str());
		if(!stream)
		{
			return new Empty();
		}
		return IO::read(&stream);
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
		std::cout << std::endl << "Tree::IO write and read" << std::endl;

		const int zero = 0;
		const int one = 1;
		const int two = 2;
		const int integer1 = 42;
		const int integer2 = -100;
		const int integer3 = 999999;

		{
			const auto expectedTree = new Tree::Empty();

			std::ostringstream expected(std::ios_base::binary);

			std::ostringstream actual(std::ios_base::binary);
			Tree::IO::write(&actual, expectedTree);

			ASSERT_EQUALS("write ()", actual.str(), expected.str(), "");

			std::istringstream input(actual.str()); 
			auto actualTree = Tree::IO::read(&input);
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
			Tree::IO::write(&actual, expectedTree);

			ASSERT_EQUALS("write (int 42)", actual.str(), expected.str(), "");

			std::istringstream input(actual.str()); 
			auto actualTree = Tree::IO::read(&input);
			std::cout << "actualTree: " << actualTree->toText() << std::endl;
			std::cout << "expectedTree: " << expectedTree->toText() << std::endl;
			ASSERT_EQUALS("read (int 42)", actualTree->isEqual(expectedTree), true, "");	
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
			Tree::IO::write(&actual, expectedTree);

			ASSERT_EQUALS("write (int 42 (int -100, int 999999))", actual.str(), expected.str(), "");	

			std::istringstream input(actual.str()); 
			auto actualTree = Tree::IO::read(&input);
			ASSERT_EQUALS("read (int 42 (int -100, int 999999))", actualTree->isEqual(expectedTree), true, "");
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