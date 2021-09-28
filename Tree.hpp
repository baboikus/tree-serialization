#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace Tree
{
enum class Type
{
	INVALID = 0,
	INT = 1,
	REAL = 2,
	STRING = 3
};

class Abstract;
using TreePtr = std::shared_ptr<Abstract>;
using TreeConstPtr = std::shared_ptr<Abstract const>;

template< class T>
inline TreePtr makePtr()
{
	return std::make_shared<T>();
}

template< class T, class... Args >
inline TreePtr makePtr( Args&&... args )
{
	return std::make_shared<T>(std::forward<T>(args...))
}

template <class T>
inline TreeConstPtr makeConstPtr()
{
	return std::make_shared<T const>();
} 

class Abstract
{
	friend TreePtr operator + (TreePtr parent, TreePtr child);

public:
	virtual bool isEmpty() = 0;
	virtual int childrenCount() const = 0;

	virtual void traverse(
		std::function<void(Abstract const *)> initial,
		std::function<void(Abstract const *)> final) const = 0;

	bool isLeaf() const
	{
		return childrenCount() == 0;
	}

	bool isEqual(TreeConstPtr other) const
	{
		return other && (other->toText() == toText());
	}

	std::string toText() const
	{
		std::string text;
		auto initial = [&text](Abstract const * tree){
			text += tree->dataToText();
			if(!tree->isLeaf())
			{
				text += "(";
			}
		};
		auto final = [&text](Abstract const * tree){
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

	void print()
	{
		int indent = 0;
		auto initial = [&indent](Abstract const * tree){
			if(indent > 0)
			{
				for(int i = 0; i < indent - 1; ++i)
				{
					std::cout << "	";
				}
				std::cout << "+";
			}
			std::cout << tree->dataToText() << std::endl;
			++indent;
		};
		auto final = [&indent](Abstract const * tree){
			--indent;
		};
		traverse(initial, final);
	}

	virtual Type type() const = 0;
	virtual std::pair<const char*, int> bytes() const = 0;
protected:
	virtual void addChild(TreePtr child) = 0;

	virtual std::string dataToText() const = 0;
	virtual bool isDataEqual(TreeConstPtr tree) const = 0;
};

TreePtr operator + (TreePtr parent, TreePtr child)
{
	if(!child || child->isEmpty())
	{
		return parent;
	}
	if(!parent || parent->isEmpty())
	{
		return child;
	}
	parent->addChild(child);
	return parent;
}

class Empty : public Abstract
{
public:
	virtual bool isEmpty()
	{
		return true;
	}

	virtual int childrenCount() const
	{
		return 0;
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
	virtual void addChild(TreePtr child)
	{
	}

	virtual std::string dataToText() const
	{
		return "";
	}

	virtual bool isDataEqual(TreeConstPtr tree) const 
	{
		return std::dynamic_pointer_cast<Empty const>(tree) != nullptr;
	}
};

class Naive : public Abstract
{
public:
	virtual bool isEmpty()
	{
		return false;
	}

	virtual int childrenCount() const
	{
		return children_.size();
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

protected:	
	virtual void addChild(TreePtr child)
	{
		children_.push_back(child);
	}

private:
	std::vector<TreeConstPtr> children_;
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

	virtual bool isDataEqual(TreeConstPtr tree) const 
	{
		auto intNode = std::dynamic_pointer_cast<Int const>(tree);
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

	virtual bool isDataEqual(TreeConstPtr tree) const 
	{
		auto realNode = std::dynamic_pointer_cast<Real const>(tree);
		return realNode && realNode->data() == data(); 
	}

private:
	double data_;
};


class String : public Naive
{
public:
	String(const std::string &value) :
	 data_(value)
	{
		std::cout << "lvalue string" << std::endl;
	}

	String(std::string &&value) :
	 data_(value)
	{
		std::cout << "rvalue string" << std::endl;
	}

	String(const char *value) :
		data_(value)
	{
		std::cout << "const char*" << std::endl;
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

	virtual bool isDataEqual(TreeConstPtr tree) const 
	{
		auto stringNode = std::dynamic_pointer_cast<String const>(tree);
		return stringNode && stringNode->data() == data(); 
	}

private:
	std::string data_;
};
}
