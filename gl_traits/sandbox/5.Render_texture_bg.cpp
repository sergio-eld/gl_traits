#include "helpers.hpp"

#include "glt_Common.h"
#include "glt_CommonValidate.h"

const char bgVertSrcFile[] = "texture_bg.vs",
bgFragSrcFile[] = "texture_bg.fs";

int main(int argc, const char** argv)
{
    fsys::path exePath{ argv[0] };

    SmartGLFW glfw{ 3, 3 };
    SmartGLFWwindow window{ SCR_WIDTH, SCR_HEIGHT, "BG texture" };

    glfw.MakeContextCurrent(window);
    glfw.LoadOpenGL();

	glt::Program<VAO_texture_bg,
		glt::uniform_collection<std::tuple<texBG_sampler2D>>> prog{};

    {
		glt::VertexShader vs{};
		glt::FragmentShader fs{};

        std::fstream file{ exePath.parent_path().append(bgVertSrcFile), std::fstream::in };
        if (!file)
        {
            std::cerr << "Failed to open " << bgVertSrcFile << std::endl;
            return -1;
        }
        std::string vShaderSource{ std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>() };

        file.close();

        file.open(exePath.parent_path().append(bgFragSrcFile), std::fstream::in);
        if (!file)
        {
            std::cerr << "Failed to open " << bgFragSrcFile << std::endl;
            return -1;
        }

        std::string fShaderSource{ std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>() };

        vs.Compile(vShaderSource);
        fs.Compile(fShaderSource);
		assert(vs && "Failed to compile vertex shader!");
		assert(fs && "Failed to compile fragment shader!");

		prog.Link(vs, fs);
		assert(prog && "Failed to link shader program!");

    }

    // vertex_pos_2d and tex_coord
    static std::vector<glm::vec4> bg_texture{
        { -1.0f, -1.0f,      0.0f, 0.0f },
        { 1.0f, -1.0f,       1.0f, 0.0f },
		{ 1.0f, 1.0f,        1.0f, 1.0f },// 0.0f },
		{ -1.0f, 1.0f,       0.0f, 1.0f }//0.0f }
    };

    VAO_texture_bg vao{};
    vao.Bind();

    glt::Buffer<glm::vec4> buf_bg_tex_coord;
    buf_bg_tex_coord.Bind(glt::BufferTarget::array);
    buf_bg_tex_coord.AllocateMemory(bg_texture.size(), glt::BufUsage::static_draw);

    for (glm::vec4& v : glt::MapGuard(buf_bg_tex_coord(), glt::MapAccessBit::write))
    {
        static std::vector<glm::vec4>::const_iterator iter = bg_texture.cbegin();
        v = *iter++;
    }

	for (const glm::vec4& v : glt::MapGuard(buf_bg_tex_coord(), glt::MapAccessBit::read))
	{
		static std::vector<glm::vec4>::const_iterator iter = bg_texture.cbegin();

		assert(v == *iter && "Failed to load vertex data!");
		++iter;
	}

    vao.AttributePointer(glt::tag_s<0>(), buf_bg_tex_coord()());
    vao.EnablePointers();

    glEnable(GL_DEPTH_TEST);

	glt::Texture2D<glt::TexInternFormat::rgba> tex;

	{
		Image bgIm{ exePath.parent_path().append("resources/textures/waves.jpg") };

		assert(bgIm.Data() && "Failed to load the texture!");

		glActiveTexture(GL_TEXTURE0);
		tex.Bind();

		tex.SetImage(0, bgIm.Width(), bgIm.Height());

		switch (bgIm.NumChannels())
		{
		case 3:
			tex.SubImage(0, bgIm.Width(), bgIm.Height(), glt::TexFormat::rgb,
				glt::TexType::unsigned_byte, bgIm.Data());
			break;
		case 4:
			tex.SubImage(0, bgIm.Width(), bgIm.Height(), glt::TexFormat::rgba,
				glt::TexType::unsigned_byte, bgIm.Data());
			break;
		default:
			break;
		}

		tex.GenerateMipMap();
		//tex.UnBind();
	}

	prog.Use();
	prog.Set(texBG_sampler2D{ 0 });

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.258824f, 0.435294f, 0.258824f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        {
            auto drawGuard = prog.Guard();
            auto& uniforms = drawGuard.Uniforms();

            drawGuard.DrawTriangleFan(vao, 0, bg_texture.size());

        }

		GLenum error = glGetError();
		assert(!error);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}
