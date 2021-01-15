#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>

/// использовать смартпоинтеры?

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
}