#ifndef SHADER_SPRITE_PROGRAM_H
# define SHADER_SPRITE_PROGRAM_H

# include "../Program.h"

namespace librender
{

	class ShaderSpriteProgram
	{

	public:
		ProgramLocation *texCoordsLocation;
		ProgramLocation *vertexesLocation;
		ProgramLocation *colorsLocation;
		ProgramLocation *mvpLocation;
		Program *program;
		ShaderSpriteProgram() : texCoordsLocation(nullptr), vertexesLocation(nullptr), colorsLocation(nullptr), mvpLocation(nullptr), program(nullptr) {};

	};

}

#endif
