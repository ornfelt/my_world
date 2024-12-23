#include <librender/Context/Context.h>
#include <librender/Sprite/SpriteBatched.h>
#include <librender/Sprite/SpriteBatch.h>
#include <librender/Text/TextBatched.h>
#include <librender/Text/TextBatch.h>
#include <librender/Font/FontModel.h>
#include <librender/Sprite/Sprite.h>
#include <librender/Window/Window.h>
#include <librender/Text/Text.h>
#include <librender/GL.h>
#include <iostream>

using librender::Window;
using librender::Context;
using librender::Buffer;
using librender::Sprite;
using librender::VERTEX_ATTRIB_FLOAT;
using librender::PRIMITIVE_TRIANGLE;
using librender::CLEAR_COLOR_BUFFER_BIT;
using librender::CLEAR_DEPTH_BUFFER_BIT;
using librender::Mat4;
using librender::BLEND_SRC_ALPHA;
using librender::BLEND_INV_SRC_ALPHA;
using librender::BLEND_ONE;
using librender::BLEND_SRC_COLOR;
using librender::BLEND_SUB;
using librender::Vec3;
using librender::VertexArray;
using librender::VERTEX_ATTRIB_UNSIGNED_SHORT;
using librender::VertexAttribType;
using librender::VertexAttrib;
using librender::FontModel;
using librender::Font;
using librender::Text;
using librender::Color;
using librender::SpriteBatch;
using librender::SpriteBatched;
using librender::TextBatch;
using librender::TextBatched;

int64_t nanotime()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000000000 + ts.tv_nsec);
}

void glErrors(std::string text)
{
	GLenum code;
	while ((code = glGetError()))
	{
		std::cerr << text << ": " << code << std::endl;
	}
}

int run(Window *window)
{
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	Context *context = window->getContext();
	std::cout << "sizeof(enum VertexAttribType): " << sizeof(enum VertexAttribType) << std::endl;
	std::cout << "sizeof(VertexAttrib): " << sizeof(VertexAttrib) << std::endl;
	std::cout << "sizeof(VertexArray): " << sizeof(VertexArray) << std::endl;
	std::cout << "sizeof(string): " << sizeof(std::string) << std::endl;
	/*VertexArray vertexArray1;
	VertexArray vertexArray2;
	Buffer vertexBuffer1;
	Buffer colorBuffer1;
	Buffer vertexBuffer2;
	Buffer colorBuffer2;
	Buffer indexBuffer2;
	context->createVertexBuffer(&vertexBuffer1);
	context->createVertexBuffer(&colorBuffer1);
	context->createVertexBuffer(&vertexBuffer2);
	context->createVertexBuffer(&colorBuffer2);
	context->createIndiceBuffer(&indexBuffer2);
	context->createVertexArray(&vertexArray1);
	context->createVertexArray(&vertexArray2);
	int vertexAttrib = context->getAttribLocation("vertex");
	int colorAttrib = context->getAttribLocation("color");

	context->bindVertexArray(&vertexArray1);
	context->setAttrib(vertexAttrib, VERTEX_ATTRIB_FLOAT, 2, 0, 0);
	context->setAttrib(colorAttrib, VERTEX_ATTRIB_FLOAT, 4, 0, 0);
	context->setAttribBuffer(vertexAttrib, &vertexBuffer1);
	context->setAttribBuffer(colorAttrib, &colorBuffer1);
	float vertexes1[] = {0, 0, 100, 0, 100, 100, 0, 0, 100, 100, 0, 100};
	float colors1[] = {1, 0, 0, .5, 0, 1, 0, .5, 0, 0, 1, .5, 1, 1, 0, .5, 0, 1, 1, .5, 1, 1, 1, .5};
	context->updateBuffer(&vertexBuffer1, vertexes1, sizeof(vertexes1));
	context->updateBuffer(&colorBuffer1, colors1, sizeof(colors1));

	context->bindVertexArray(&vertexArray2);
	context->setAttrib(vertexAttrib, VERTEX_ATTRIB_FLOAT, 2, 0, 0);
	context->setAttrib(colorAttrib, VERTEX_ATTRIB_FLOAT, 3, 0, 0);
	context->setAttribBuffer(vertexAttrib, &vertexBuffer2);
	context->setAttribBuffer(colorAttrib, &colorBuffer2);
	context->setIndexAttribType(VERTEX_ATTRIB_UNSIGNED_SHORT);
	context->setIndexAttribBuffer(&indexBuffer2);
	float vertexes2[] = {200, 200, 300, 200, 300, 300, 200, 300};
	float colors2[] = {1, .5, .5, .5, 1, .5, .5, .5, 1};
	uint16_t indices2[] = {0, 1, 2};
	context->updateBuffer(&vertexBuffer2, vertexes2, sizeof(vertexes2));
	context->updateBuffer(&colorBuffer2, colors2, sizeof(colors2));
	context->updateBuffer(&indexBuffer2, indices2, sizeof(indices2));*/

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	FontModel *fontModel = new FontModel("/home/jukino/stellar-legacy/client/data/fonts/opensans/regular.ttf");
	Font *font = fontModel->derive(20);
	Text text(context);
	text.setFont(font);
	std::string txt("Salut tout le monde, c'est un super message pour tester l'efficacité du rendu avec la nouvelle API. Sinon, ça va ?\n");
	txt += txt;
	txt += txt;
	txt += txt;
	txt += txt;
	txt += txt;
	text.setText(txt);
	text.setPos(0, 100);
	text.setColor(Color::RED);
	Sprite sprite(context);
	sprite.setSize(100, 100);
	SpriteBatch spriteBatch(context);
	for (uint32_t i = 0; i < 200; ++i)
	{
		SpriteBatched *spr = new SpriteBatched();
		spr->setSize(10, 10);
		int32_t x = (i % 10) * 20;
		if ((i / 10) % 2 == 1)
			x += 10;
		int32_t y = 100 + i / 10 * 10;
		spr->setPos(x, y);
		spr->setColor(Color(0, x / 190., (y - 100) / 200.));
		spriteBatch.addChild(spr);
	}
	TextBatch textBatch(context);
	textBatch.setFont(font);
	textBatch.setPos(600, 400);
	for (uint32_t i = 0; i < 8; ++i)
	{
		TextBatched *tx = new TextBatched();
		std::string t("text number " + std::to_string(i + 1));
		tx->setText(t);
		tx->setColor(Color::GREEN);
		tx->setShadow(1, 1, 1, Color::BLACK);
		textBatch.addChild(tx);
	}
	int projectionLocation = context->getUniformLocation("matrix_projection");
	int modelviewLocation = context->getUniformLocation("matrix_modelview");
	int64_t frame = nanotime();
	int64_t fps = 0;
	spriteBatch.setScale(2);
	while (!window->closeRequested())
	{
		int64_t cur = nanotime();
		if (cur - frame > 1000000000)
		{
			std::cout << "fps: " << fps << std::endl;
			fps = 0;
			frame += 1000000000;
		}
		context->setBlendFunc(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
		context->setViewport(0, 0, window->getWidth(), window->getHeight());
		context->setUniform(projectionLocation, Mat4::ortho(0, window->getWidth(), window->getHeight(), 0, -2, 2));
		context->setUniform(modelviewLocation, Mat4(1));
		context->clear(0x222222FF, CLEAR_DEPTH_BUFFER_BIT | CLEAR_COLOR_BUFFER_BIT);
		/*context->bindVertexArray(&vertexArray1);
		context->draw(PRIMITIVE_TRIANGLE, 0, 6);
		context->bindVertexArray(&vertexArray2);
		context->drawIndexed(PRIMITIVE_TRIANGLE, 3);*/
		sprite.setX((cos(nanotime() / 2000000000. * M_PI) * (window->getWidth() - 100) / 2) + (window->getWidth() - 100) / 2);
		sprite.draw();
		text.draw();
		spriteBatch.draw();
		for (size_t i = 0; i < textBatch.getChilds().size(); ++i)
		{
			float x = std::cos(M_PI * 2 * i / (textBatch.getChilds().size() - 1) + nanotime() / 3000000000. * M_PI) * 200;
			float y = std::sin(M_PI * 2 * i / (textBatch.getChilds().size() - 1) + nanotime() / 3000000000. * M_PI) * 200;
			textBatch.getChilds()[i]->setPos(x, y);
		}
		textBatch.setScale(std::abs(std::cos(nanotime() / 5000000000. * M_PI)) * .5 + .5);
		textBatch.draw();
		window->update();
		glErrors("9");
		window->pollEvents();
		fps++;
	}
	while (textBatch.getChilds().size())
		delete (textBatch.getChilds()[0]);
	while (spriteBatch.getChilds().size())
		delete (spriteBatch.getChilds()[0]);
	delete (font);
	delete (fontModel);
	return 1;
}

int main()
{
	glfwInit();
	Window *window = new Window("Test", 1280, 720);
	window->show();
	window->setVSync(false);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Glad failed" << std::endl;
		return 0;
	}
	int ret = run(window);
	delete (window);
	glfwTerminate();
	return ret;
}
