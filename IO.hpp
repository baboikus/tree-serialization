#pragma once

#include <fstream>
#include <sstream>
#include <cstring>

#include "Tree.hpp"

namespace Tree
{
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
			if(dynamicData_)
			{
				delete[] dynamicData_;
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

	OStream& write(TreeConstPtr tree)
	{
		auto initial = [this](Abstract const * tree)
		{
			if(!tree)
			{
				return;
			}
			const auto [data, dataSize] = tree->bytes();
			Segment s{.type_ = tree->type(),
                      .childrenCount_ = tree->childrenCount(),
					  .dataSize_ = dataSize,
					  .constData_ = data,
					  .dynamicData_ = nullptr};
			processSegment(s);
		};
		tree->traverse(initial, [](Abstract const * ){});

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

	TreePtr read()
	{
		Segment s;
		processSegment(s);
		if(!s.dynamicData_)
		{
			return Tree::makePtr<Empty>();
		}

		TreePtr tree;
		switch(s.type_)
		{
		 	case Type::INVALID:
		 	{
		 		return Tree::makePtr<Empty>();
		 	}
		 	case Type::INT:
		 	{
		 		tree = Tree::makePtr<Int>(convertTo<int>(s.dynamicData_));
		 		break;
		 	}
		 	case Type::REAL:
		 	{
		 		tree = Tree::makePtr<Real>(convertTo<double>(s.dynamicData_));
		 		break;
		 	}
		 	case Type::STRING:
		 	{
		 		tree = Tree::makePtr<String>(s.dynamicData_);
		 		break;
		 	}
		}
		for(int i = 0; i < s.childrenCount_; ++i)
		{
			tree + read();
		}	
		return tree;
	}

protected:
	virtual void processType(Type &t)
	{
		char signature = 'e';
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
	static bool saveToFile(const std::string &fileName, TreeConstPtr tree)
	{
		std::ofstream stream(fileName.c_str());
		if(!stream)
		{
			return false;
		}
		OStream(&stream).write(tree);
		return true;
	}
	static TreePtr loadFromFile(const std::string &fileName)
	{
		std::ifstream stream(fileName.c_str());
		if(!stream)
		{
			return Tree::makePtr<Empty>();
		}
		return IStream(&stream).read();
	}
};

}
