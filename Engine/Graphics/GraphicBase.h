 #pragma once
 #include "Graphics.h"

namespace eae6320 {
	namespace Graphics {
		class Color;
	}
}

 namespace eae6320
 {
 	namespace GraphicsBase
 	{
		void  ClearTarget(Graphics::Color c);
 		cResult CleanUp();
 		cResult Initialize(const Graphics::sInitializationParameters& i_initializationParameters);
		void Swap();
 	}
 }
