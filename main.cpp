#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <sstream>
#include <cstring>

#include "test.h"

/// брэйншторм
/// использовать щаблоны для данных в нодах?
/// асинхронные читалки и писалки для файлов?
/// использовать смартпоинтеры?
/// формат для хранения дерева в памяти неотличим от формата на диске?
/// хранить всё в сыром char массиве?


namespace Tree
{
enum class Type
{
	INVALID = 0,
	INT = 1,
	REAL = 2,
	STRING = 3
};

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

	virtual Type type() const = 0;
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

	virtual Type type() const 
	{
		return Type::INVALID;
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

	virtual Type type() const 
	{
		return Type::INT;
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
		return intNode && intNode->data() == data(); 
	}

private:
	int data_;
};

class Real : public Naive
{
public:
	Real(const double value) :
	 data_(value)
	{

	}

	double data() const
	{
		return data_;
	}

	virtual Type type() const 
	{
		return Type::REAL;
	}

	virtual std::pair<const char*, int> bytes() const
	{
		return {reinterpret_cast<const char*>(&data_), sizeof data_};
	}

protected:
	virtual std::string dataToText() const
	{
		return std::string("real ") + std::to_string(data());
	}

	virtual bool isDataEqual(Abstract const *tree) const 
	{
		auto realNode = dynamic_cast<Real const *>(tree);
		return realNode && realNode->data() == data(); 
	}

private:
	double data_;
};


class String : public Naive
{
public:
	String(const std::string value) :
	 data_(value)
	{

	}

	const std::string& data() const
	{
		return data_;
	}

	virtual Type type() const 
	{
		return Type::STRING;
	}

	virtual std::pair<const char*, int> bytes() const
	{
		return {data_.c_str(), data_.size()};
	}

protected:
	virtual std::string dataToText() const
	{
		return std::string("string ") + data();
	}

	virtual bool isDataEqual(Abstract const *tree) const 
	{
		auto stringNode = dynamic_cast<String const *>(tree);
		return stringNode && stringNode->data() == data(); 
	}

private:
	std::string data_;
};

template <class Stream>
class IO
{
protected:
	IO(Stream *stream) : 
		stream_(stream)
	{
	}

	struct Segment
	{
		Type type_ = Type::INVALID;
		int childrenCount_ = 0;
		int dataSize_ = 0;
		const char* constData_ = nullptr;
		char* dynamicData_ = nullptr;

		~Segment()
		{
			if(!dynamicData_)
			{
				delete dynamicData_;
				dynamicData_ = nullptr;
			}
		} 
	};

	bool processSegment(Segment &s)
	{
		processType(s.type_);
		processInt(s.childrenCount_);
		processInt(s.dataSize_);
		if(s.dataSize_ <= 0)
		{
			return false;
		}
		processData(s.type_, s.dataSize_, s.constData_, s.dynamicData_);
		return true;
	}

	template <typename T>
	void writeData(const T *data, const int size)
	{
		stream_->write(reinterpret_cast<const char*>(data), size);
	}

	template <typename T>
	void readData(T *data, const int size)
	{
		stream_->read(reinterpret_cast<char*>(data), size);
	}

	template <typename T>
	T convertTo(const char *data)
	{		
		T converted;
		std::memcpy(&converted, data, sizeof converted);
		return converted;
	}

	virtual void processType(Type &t) = 0;
	virtual void processInt(int &n) = 0;
	virtual void processData(const Type type,
							 const int dataSize,
							 const char *constData, 
							 char *&dynamicData) = 0;

	static const char signatureForType(const Type t)
	{
		switch(t)
		{
			case Type::INVALID: return 'e';
			case Type::INT: return 'i';
			case Type::REAL: return 'r';
			case Type::STRING: return 's';
		}
		return 'e';
	}

	static const Type typeForSignature(const char s)
	{
		switch(s)
		{
			case 'e': return Type::INVALID;
			case 'i': return Type::INT;
			case 'r': return Type::REAL;
			case 's': return Type::STRING;
		}
		return Type::INVALID;
	} 

private:
	Stream *stream_;
};

class OStream : public IO<std::ostream>
{
public:
	OStream(std::ostream *stream) :
		IO(stream) 
	{
	}

	OStream& write(Abstract const *tree)
	{
		auto initial = [this](Abstract const *tree)
		{
			if(!tree)
			{
				return;
			}
			const auto [data, dataSize] = tree->bytes();
			Segment s{tree->type(), tree->childrenCount(), dataSize, data, nullptr};
			processSegment(s);
		};
		tree->traverse(initial, [](Abstract const *){});

		return *this;	
	}

protected:	
	virtual void processType(Type &t)
	{
		const auto signature = IO::signatureForType(t);
		writeData<char>(&signature, sizeof signature);
	}

	virtual void processInt(int &n)
	{
		writeData<int>(&n, sizeof n);
	}

	virtual void processData(const Type type,
							 const int dataSize,
							 const char *constData,
							 char *&dynamicData)
	{
		writeData<char>(constData, dataSize);
	}
};

class IStream : public IO<std::istream>
{
public:
	IStream(std::istream *stream) :
		IO(stream)
	{

	}

	Abstract* read()
	{
		Segment s;
		processSegment(s);
		if(!s.dynamicData_)
		{
			return new Empty();
		}

		Abstract *tree;
		switch(s.type_)
		{
		 	case Type::INVALID:
		 	{
		 		return new Empty();
		 	}
		 	case Type::INT:
		 	{
		 		tree = new Int(convertTo<int>(s.dynamicData_));
		 		break;
		 	}
		 	case Type::REAL:
		 	{
		 		tree = new Real(convertTo<double>(s.dynamicData_));
		 		break;
		 	}
		 	case Type::STRING:
		 	{
		 		tree = new String(s.dynamicData_);
		 		break;
		 	}
		}
		for(int i = 0; i < s.childrenCount_; ++i)
		{
			tree->addChild(read());
		}	
		return tree;
	}

protected:
	virtual void processType(Type &t)
	{
		char signature;
		readData<char>(&signature, sizeof signature);
		t = typeForSignature(signature);
	}

	virtual void processInt(int &n)
	{
		readData<int>(&n, sizeof n);
	}

	virtual void processData(const Type type,
							 const int dataSize,
							 const char *constData, 
							 char *&dynamicData)
	{
		if(dataSize <= 0)
		{
			return;
		}
		dynamicData = new char[dataSize + 1];
		dynamicData[dataSize] = '\0';
		readData<char>(dynamicData, dataSize);
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
		OStream(&stream).write(tree);
		return true;
	}
	static Abstract* loadFromFile(const std::string &fileName)
	{
		std::ifstream stream(fileName.c_str());
		if(!stream)
		{
			return new Empty();
		}
		return IStream(&stream).read();
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
				Tree::Tester::checkSerialization("(int 42(int 100, real 99.99))",
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
			Tree::Tester::checkSerialization("exampleTree",
				exampleTree,
				new Tree::Empty());
		ASSERT_EQUALS("exampleTree serialization", error, std::string(), error);
	}

	return 0;
}