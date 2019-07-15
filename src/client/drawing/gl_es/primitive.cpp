#include "primitive.hpp"
#include "../gl_shared/program.hpp"

namespace Drawing
{

namespace Detail
{

namespace GLES
{

static const std::size_t ATTR_LENGTHS[ATTR_COUNT] =
{
	Vertex::length(),
	Color::length()
};

Primitive::Primitive(const GLShared::Program& program) : program(program)
{
	glGenBuffers(ATTR_COUNT, vbo.data());
	usedVbo.fill(false);
	vboSize.fill(0);
}

Primitive::~Primitive()
{
	glDeleteBuffers(ATTR_COUNT, vbo.data());
}

void Primitive::SetDrawMode(const PrimitiveDrawMode& pdm)
{
	mode = GLDrawModeFromPDM(pdm);
}

void Primitive::SetVertices(const Vertices& vertices)
{
	vboSize[ATTR_VERTICES] = vertices.size();
	const std::size_t numBytes = vboSize[ATTR_VERTICES] * VERTEX_SIZE;
	glBindBuffer(GL_ARRAY_BUFFER, vbo[ATTR_VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, numBytes, vertices.data(), GL_STATIC_DRAW);
	usedVbo[ATTR_VERTICES] = true;
}

void Primitive::SetColors(const Colors& colors)
{
	vboSize[ATTR_COLORS] = colors.size();
	const std::size_t numBytes = vboSize[ATTR_COLORS] * COLOR_SIZE;
	glBindBuffer(GL_ARRAY_BUFFER, vbo[ATTR_COLORS]);
	glBufferData(GL_ARRAY_BUFFER, numBytes, colors.data(), GL_STATIC_DRAW);
	usedVbo[ATTR_COLORS] = true;
}

void Primitive::Draw()
{
	program.Use();
	TryEnableVBO(ATTR_VERTICES);
	TryEnableVBO(ATTR_COLORS);
	glDrawArrays(mode, 0, vboSize[ATTR_VERTICES]);
}

void Primitive::TryEnableVBO(AttrLocation attrLoc)
{
	if(!usedVbo[attrLoc])
		return;
	glBindBuffer(GL_ARRAY_BUFFER, vbo[attrLoc]);
	glVertexAttribPointer(attrLoc, ATTR_LENGTHS[attrLoc], GL_FLOAT,
	                      GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(attrLoc);
}

} // GLES

} // Detail

} // Drawing
