#include "../include/GLEW/glew.h"
#include "../include/GLFW/glfw3.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>


struct ShaderProgramSource
{
	std::string	VertexSource;
	std::string FragmentSource;
};


inline static void GetNextShader(std::ifstream & stream, std::string & line)
{
	while (std::getline(stream, line))
		if (line.find("#shader") != std::string::npos)
			return;
}

static bool GetVFShaderSources(ShaderProgramSource & vf_shadersources, std::string const & filepath)
{
	enum ShaderType {
		VERTEX,
		FRAGMENT
	}					shader_type;
	std::size_t			shaders_found = 0;
	std::ifstream		stream(filepath); //open file
	std::string			line;
	std::stringstream	ss[2];

	if (stream.is_open() == true)
	{
		GetNextShader(stream, line);
		while (stream.good() == true && shaders_found < 3)
		{
			if (line.find("#shader") != std::string::npos)			//on finding shader hash
			{
				if (line.find("vertex") != std::string::npos)			//set vertex shader mode
					shader_type = ShaderType::VERTEX;
				else if (line.find("fragment") != std::string::npos)	//set fragment shader mode
					shader_type = ShaderType::FRAGMENT;
				else
					break ;												//bad shader define
				++shaders_found;
			}
			else
				ss[shader_type] << line << '\n';
			std::getline(stream, line);
		}
		if (stream.bad() == true)
			std::cerr << "GetVFShaderSources failed on Unknown getline problem." << std::endl;
		else if (ss[ShaderType::VERTEX].rdbuf()->in_avail() == 0 || ss[ShaderType::FRAGMENT].rdbuf()->in_avail() == 0)
			std::cerr << "GetVFShaderSources failed on Source file missing at least one of Vertex or Fragment shaders." << std::endl;
		else
		{
			ss[shader_type] << line << '\n';
			vf_shadersources.VertexSource = ss[ShaderType::VERTEX].str();
			vf_shadersources.FragmentSource = ss[ShaderType::FRAGMENT].str();
			return true;
		}
	}
	else
		std::cerr << "GetVFShaderSources failed on file open failure; check filepath." << std::endl;
	return false;
}

static uint	CompileShader(uint type, std::string const & source)
{
	uint			id_shader = glCreateShader(type);	//returns ID for shader.
	char const *	raw_source = source.c_str();
	glShaderSource(id_shader, 1, &raw_source, nullptr);
	glCompileShader(id_shader);

	//Error handling
	int	result;
	glGetShaderiv(id_shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id_shader, GL_INFO_LOG_LENGTH, &length);
		char * message = static_cast<char *>(alloca(length * sizeof(char))); //stack-allocate space for error message
		glGetShaderInfoLog(id_shader, length, &length, message);
		std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex shader: " : "fragment shader: ") << '\n' << message << std::endl;
		return 0;
	}

	return id_shader;
}

static uint CreateShader(std::string const & vertex_shader, std::string const & fragment_shader)
{
	uint	id_program = glCreateProgram();	//returns ID for program
	uint	id_vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_shader);
	uint	id_fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);
	
	assert(id_vertex_shader);
	assert(id_fragment_shader);

	glAttachShader(id_program, id_vertex_shader);		//link shaders to program
	glAttachShader(id_program, id_fragment_shader);
	glLinkProgram(id_program);							//compile program with shaders
	glValidateProgram(id_program);						//validate
	// glDetachShader(id_program, id_vertex_shader);	//debug // should be done after debugging is finished
	// glDetachShader(id_program, id_fragment_shader);

	glDeleteShader(id_vertex_shader);					//delete intermediate stage shaders
	glDeleteShader(id_fragment_shader);

	return id_program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
		return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	/* Initialize GLEW for Modern OpenGL Extensions */
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return -1;
	}
	else
		std::cout << glGetString(GL_VERSION) << std::endl;

	/** Define Vertex Buffer **/
	
	float positions[6] {
		-0.5f, -0.5f,
		 0.0f,  0.5f,
		 0.5f, -0.5f
	};
	uint	id_vertex_buffer;							//Vertex Buffer ID
	glGenBuffers(1, &id_vertex_buffer);					//generate buffer
	glBindBuffer(GL_ARRAY_BUFFER, id_vertex_buffer);	//select buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);	//set pointer to vertex index 0, length 2 elements, type float, already normalized, size 8 bytes, offset 0 from vertex start
	glEnableVertexAttribArray(0);											//enable vertex

	// std::string vertex_shader = R"delim(
	// #version 330 core
	// layout(location = 0) in vec4 position;
	// void main(void)
	// {
	// 	gl_Position = position;
	// }
	// )delim";

	// std::string fragment_shader = R"delim(
	// #version 330 core
	// layout(location = 0) out vec4 color;
	// void main(void)
	// {			//    R    G    B    A
	// 	color = vec4(1.0, 0.0, 0.0, 1.0);
	// }
	// )delim";

	ShaderProgramSource	source;
	
	if (GetVFShaderSources(source, "res/shaders/basic.shader") == false)
	{
		glfwTerminate();
		return -1;
	}

	uint shader = CreateShader(source.VertexSource, source.FragmentSource);
	glUseProgram(shader);								//select shader

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

		// Legacy triangle
		// glBegin(GL_TRIANGLES);
		// glVertex2f(-0.5f, -0.5f);
		// glVertex2f(0.0f, 0.5f);
		// glVertex2f(0.5f, -0.5f);
		// glEnd();

		glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

	//glDeleteProgram(shader);							//delete shader program

    glfwTerminate();
    return 0;
}
