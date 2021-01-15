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

template< class T, class... Args >
inline TreePtr makePtr( Args&&... args )
{
	return std::make_shared<T>(args...);
}

template <class T>
inline TreeConstPtr makeConstPtr()
{
	return std::make_shared<T const>();
}

class Abstract
{
public:
	virtual int childrenCount() const = 0;

	virtual TreePtr addChild(TreePtr child) = 0;

	virtual void traverse(
		std::function<void(TreeConstPtr)> initial,
		std::function<void(TreeConstPtr)> final) const = 0;

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
		auto initial = [&text](TreeConstPtr tree){
			text += tree->dataToText();
			if(!tree->isLeaf())
			{
				text += "(";
			}
		};
		auto final = [&text](TreeConstPtr tree){
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
	static TreePtr make(Abstract *tree)
	{
		return TreePtr(tree);
	}

	static TreeConstPtr makeConst(Abstract const *tree)
	{
		return TreeConstPtr(tree);
	}

	virtual std::string dataToText() const = 0;
	virtual bool isDataEqual(TreeConstPtr tree) const = 0;
};

class Empty : public Abstract
{
public:
	virtual int childrenCount() const
	{
		return 0;
	}

	virtual TreePtr addChild(TreePtr child)
	{
		return child ? child : Abstract::make(this);
	}

	virtual void traverse(
		std::function<void(TreeConstPtr)> initial,
		std::function<void(TreeConstPtr)> final) const
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

	virtual bool isDataEqual(TreeConstPtr tree) const 
	{
		return std::dynamic_pointer_cast<Empty const>(tree) != nullptr;
	}
};

class Naive : public Abstract
{
public:
	virtual int childrenCount() const
	{
		return children_.size();
	}

	virtual TreePtr addChild(TreePtr child)
	{
		children_.push_back(child);
		return make(this);
	}

	virtual void traverse(std::function<void(TreeConstPtr)> initial,
				  std::function<void(TreeConstPtr)> final) const
	{
		initial(makeConst(this));
		for(auto child = children_.cbegin(); child != children_.cend(); ++child)
		{
			(*child)->traverse(initial, final);
		}
		final(makeConst(this));			
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

	virtual bool isDataEqual(TreeConstPtr tree) const 
	{
		auto stringNode = std::dynamic_pointer_cast<String const>(tree);
		return stringNode && stringNode->data() == data(); 
	}

private:
	std::string data_;
};
}