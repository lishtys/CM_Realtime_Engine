#pragma once

namespace eae6320
{
	namespace Graphics
	{

		class Color
		{
		public:
			Color();
			Color(float r, float g, float b, float a);
			~Color();
			float m_clear_color_[4];
		};

	}
}


