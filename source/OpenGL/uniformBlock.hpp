#pragma once
#include <glew.h>
#include <vector>
#include <string>
#include "../vendor/glm/gtc/type_ptr.hpp"

struct UboElement
{
	unsigned int base;
	unsigned int offset;

	UboElement():base(0), offset(0)
	{
	}

	template <typename T>
	void push(unsigned int totalOffset)
	{
		base = sizeof(T);
		if (base % 8 != 0)
			base += 4;
		offset = totalOffset;
		if (offset % base != 0)
		{
			offset = offset + base - (offset % base);
		}
	}
};

//constructor -> push -> createUbo -> bindBlock -> update

class UniformBlock
{
private:
	unsigned int mID;
	std::vector<UboElement> elements;
public:
	UniformBlock()
	{
		glGenBuffers(1, &mID);
		glBindBuffer(GL_UNIFORM_BUFFER, mID);
	}

	UniformBlock(int i) :mID(0)
	{
	}

	~UniformBlock()
	{
		glDeleteBuffers(1, &mID);
	}
	
	inline void create()
	{
		glGenBuffers(1, &mID);
		glBindBuffer(GL_UNIFORM_BUFFER, mID);
	}

	inline unsigned int getOffset()
	{
		unsigned int total = 0;
		for (auto& el : elements)
		{
			total += el.base;
		}
		return total;
	}

	inline unsigned int getSize()
	{
		auto id = elements.size() - 1;
		return elements[id].base + elements[id].offset;
	}

	template <typename T>
	inline void Push()
	{
		UboElement el;
		el.push<T>(getOffset());
		elements.push_back(el);
	}

	template <typename... Types>
	inline void push()
	{
		(Push<Types>(), ...);
	}

	inline void createUbo(unsigned int flag = GL_STATIC_DRAW)
	{
		glBufferData(GL_UNIFORM_BUFFER, getSize(), NULL, flag);
	}

	template <typename T>
	inline void update(const T& value, unsigned int elementIndex)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, elements[elementIndex].offset, elements[elementIndex].base, &value);
	}

	inline void update(const glm::mat4& value, unsigned int elementIndex)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, elements[elementIndex].offset, sizeof(glm::mat4), glm::value_ptr(value));
	}

	inline void update(const glm::vec3& value, unsigned int elementIndex)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, elements[elementIndex].offset, elements[elementIndex].base, glm::value_ptr(value));
	}

	inline void bindBlock(unsigned int slot, const std::string& name, unsigned int shaderId)
	{
		unsigned int uniform_index = glGetUniformBlockIndex(shaderId, name.c_str());
		glUniformBlockBinding(shaderId, uniform_index, slot);
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, mID);
	}

	inline void bind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, mID);
	}

	inline void unBind()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
};