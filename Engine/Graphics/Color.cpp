#include "Color.h"

eae6320::Graphics::Color::Color()
{
	m_clear_color_[0] = 1.0f;
	m_clear_color_[1] = 1.0f;
	m_clear_color_[2] = 1.0f;
	m_clear_color_[3] = 1.0f;
}
eae6320::Graphics::Color::Color(float r, float g, float b, float a )
{
	m_clear_color_[0] =r;
	m_clear_color_[1] = g;
	m_clear_color_[2] = b;
	m_clear_color_[3] = a;
}


eae6320::Graphics::Color::~Color()
{
}
