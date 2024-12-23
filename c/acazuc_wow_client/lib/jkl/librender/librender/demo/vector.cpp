#include <librender/Vec/Vec1.h>
#include <librender/Vec/Vec2.h>
#include <librender/Vec/Vec3.h>
#include <librender/Vec/Vec4.h>
#include <iostream>

using librender::Vec1;
using librender::Vec2;
using librender::Vec3;
using librender::Vec4;

static void printOk(std::string str)
{
	std::cout << "\e[0m[\e[1;32mOK\e[0m] " << str << "\e[0m" << std::endl;
}

static void printKo(std::string str)
{
	std::cout << "\e[0m[\e[1;31mKO\e[0m] " << str << "\e[0m" << std::endl;
}

static void printResult(std::string str, bool test)
{
	if (test)
		printOk(str);
	else
		printKo(str);
}

static void test_vec1_constructors()
{
	std::cout << __func__ << std::endl;
	{
		Vec1 v(1);
		printResult("vec1(T)", v.x == 1);
	}
	{
		Vec1 v(Vec1(2));
		printResult("vec1(vec1)", v.x == 2);
	}
	std::cout << std::endl;
}

static void test_vec1_class_operators()
{
	std::cout << __func__ << std::endl;
	{
		Vec1 v(1);
		v = -v;
		printResult("-vec1", v.x == -1);
	}
	{
		Vec1 v(1);
		v += 2;
		printResult("vec1 += T", v.x == 3);
	}
	{
		Vec1 v(1);
		v += Vec1(3);
		printResult("vec1 += vec1", v.x == 4);
	}
	{
		Vec1 v(1);
		v -= 3;
		printResult("vec1 -= T", v.x == -2);
	}
	{
		Vec1 v(1);
		v -= Vec1(4);
		printResult("vec1 -= vec1", v.x == -3);
	}
	{
		Vec1 v(1);
		v *= 4;
		printResult("vec1 *= T", v.x == 4);
	}
	{
		Vec1 v(1);
		v *= Vec1(5);
		printResult("vec1 *= vec1", v.x == 5);
	}
	{
		Vec1 v(10);
		v /= 2;
		printResult("vec1 /= T", v.x == 5);
	}
	{
		Vec1 v(20);
		v /= Vec1(5);
		printResult("vec1 /= vec1", v.x == 4);
	}
	{
		Vec1 v(2);
		Vec1 w(2);
		printResult("vec1 == vec1", v == w);
	}
	{
		Vec1 v(2);
		Vec1 w(3);
		printResult("vec1 == vec1", !(v == w));
	}
	{
		Vec1 v(2);
		Vec1 w(3);
		printResult("vec1 != vec1", v != w);
	}
	{
		Vec1 v(2);
		Vec1 w(2);
		printResult("vec1 != vec1", !(v != w));
	}
	std::cout << std::endl;
}

static void test_vec1_operators()
{
	std::cout << __func__ << std::endl;
	{
		Vec1 v(3);
		v = v + Vec1(2);
		printResult("vec1 + vec1", v.x == 5);
	}
	{
		Vec1 v(4);
		v = v + 6.f;
		printResult("vec1 + T", v.x == 10);
	}
	{
		Vec1 v(7);
		v = 4.f + v;
		printResult("T + vec1", v.x == 11);
	}
	{
		Vec1 v(3);
		v = v - Vec1(2);
		printResult("vec1 - vec1", v.x == 1);
	}
	{
		Vec1 v(5);
		v = v - 10.f;
		printResult("vec1 - T", v.x == -5);
	}
	{
		Vec1 v(4);
		v = 10.f - v;
		printResult("T - vec1", v.x == 6);
	}
	{
		Vec1 v(5);
		v = v * Vec1(3);
		printResult("vec1 * vec1", v.x == 15);
	}
	{
		Vec1 v(6);
		v = v * 3.f;
		printResult("vec1 * T", v.x == 18);
	}
	{
		Vec1 v(3);
		v = 8.f * v;
		printResult("T * vec1", v.x == 24);
	}
	{
		Vec1 v(25);
		v = v / Vec1(5);
		printResult("vec1 / vec1", v.x == 5);
	}
	{
		Vec1 v(50);
		v = v / 25.f;
		printResult("vec1 / T", v.x == 2);
	}
	{
		Vec1 v(4);
		v = 80.f / v;
		printResult("T / vec1", v.x == 20);
	}
	std::cout << std::endl;
}

static void test_vec1_min()
{
	std::cout << __func__ << std::endl;
	{
		Vec1 v(10);
		Vec1 w(2);
		Vec1 x(min(v, w));
		printResult("min(vec1, vec1)", x.x == 2);
	}
	{
		Vec1 v(1);
		Vec1 w(2);
		Vec1 x(min(v, w));
		printResult("min(vec1, vec1)", x.x == 1);
	}
	{
		Vec1 v(1);
		Vec1 u(min(v, -5.f));
		printResult("min(vec1, T)", u.x == -5);
	}
	{
		Vec1 v(1);
		Vec1 u(min(v, 5.f));
		printResult("min(vec1, T)", u.x == 1);
	}
	{
		Vec1 v(6);
		Vec1 u(min(20.f, v));
		printResult("min(T, vec1)", u.x == 6);
	}
	{
		Vec1 v(25);
		Vec1 u(min(20.f, v));
		printResult("min(T, vec1)", u.x == 20);
	}
	std::cout << std::endl;
}

static void test_vec1_max()
{
	std::cout << __func__ << std::endl;
	{
		Vec1 v(10);
		Vec1 w(2);
		Vec1 x(max(v, w));
		printResult("max(vec1, vec1)", x.x == 10);
	}
	{
		Vec1 v(10);
		Vec1 w(20);
		Vec1 x(max(v, w));
		printResult("max(vec1, vec1)", x.x == 20);
	}
	{
		Vec1 v(1);
		Vec1 u(max(v, -5.f));
		printResult("max(vec1, T)", u.x == 1);
	}
	{
		Vec1 v(1);
		Vec1 u(max(v, 5.f));
		printResult("max(vec1, T)", u.x == 5);
	}
	{
		Vec1 v(6);
		Vec1 u(max(20.f, v));
		printResult("max(T, vec1)", u.x == 20);
	}
	{
		Vec1 v(60);
		Vec1 u(max(20.f, v));
		printResult("max(T, vec1)", u.x == 60);
	}
	std::cout << std::endl;
}

static void test_vec1_clamp()
{
	std::cout << __func__ << std::endl;
	{
		Vec1 v(6);
		Vec1 u(clamp(v, 0.f, 2.f));
		printResult("clamp(vec1, T, T)", u.x == 2);
	}
	{
		Vec1 v(1);
		Vec1 u(clamp(v, 0.f, 2.f));
		printResult("clamp(vec1, T, T)", u.x == 1);
	}
	{
		Vec1 v(-2);
		Vec1 u(clamp(v, 0.f, 2.f));
		printResult("clamp(vec1, T, T)", u.x == 0);
	}
	{
		Vec1 v(6);
		Vec1 u(clamp(v, Vec1(0), Vec1(2)));
		printResult("clamp(vec1, vec1, vec1)", u.x == 2);
	}
	{
		Vec1 v(1);
		Vec1 u(clamp(v, Vec1(0), Vec1(2)));
		printResult("clamp(vec1, vec1, vec1)", u.x == 1);
	}
	{
		Vec1 v(-2);
		Vec1 u(clamp(v, Vec1(0), Vec1(2)));
		printResult("clamp(vec1, vec1, vec1)", u.x == 0);
	}
	std::cout << std::endl;
}

static void test_vec1_misc()
{
	std::cout << __func__ << std::endl;
	{
		Vec1 u(10);
		Vec1 v(30);
		Vec1 w(mix(u, v, .25f));
		printResult("mix(vec1, vec1, T)", w.x == 15);
	}
	{
		Vec1 u(5);
		Vec1 v(normalize(u));
		printResult("normalize(vec1)", v.x == 1);
	}
	{
		Vec1 u(5);
		Vec1 v(4);
		float w(dot(u, v));
		printResult("dot(vec1, vec1)", w == 20);
	}
	{
		Vec1 u(6);
		float v(length(u));
		printResult("length(vec1)", v == 6);
	}
	std::cout << std::endl;
}

static void test_vec1_math()
{
	std::cout << __func__ << std::endl;
	{
		Vec1 u(11);
		Vec1 v(mod(u, Vec1(3)));
		printResult("mod(vec1, vec1)", v.x == 2);
	}
	{
		Vec1 u(11);
		Vec1 v(mod(u, 2.f));
		printResult("mod(vec1, T)", v.x == 1);
	}
	{
		Vec1 u(11.9);
		Vec1 v(floor(u));
		printResult("floor(vec1)", v.x == 11);
	}
	{
		Vec1 u(11.5);
		Vec1 v(round(u));
		printResult("round(vec1)", v.x == 12);
	}
	{
		Vec1 u(11.45);
		Vec1 v(round(u));
		printResult("round(vec1)", v.x == 11);
	}
	{
		Vec1 u(14.2);
		Vec1 v(ceil(u));
		printResult("ceil(vec1)", v.x == 15);
	}
	{
		Vec1 u(14.6);
		Vec1 v(fract(u));
		printResult("fract(vec1)", abs(v.x - 0.6) < 0.001); //float approx
	}
	std::cout << std::endl;
}

static void test_vec1()
{
	std::cout << __func__ << std::endl;
	test_vec1_constructors();
	test_vec1_class_operators();
	test_vec1_operators();
	test_vec1_min();
	test_vec1_max();
	test_vec1_clamp();
	test_vec1_misc();
	test_vec1_math();
	std::cout << std::endl;
}

static void test_vec2_constructors()
{
	std::cout << __func__ << std::endl;
	{
		Vec2 v(1);
		printResult("vec2(T)", v.x == 1 && v.y == 1);
	}
	{
		Vec2 v(1, 2);
		printResult("vec2(T, T)", v.x == 1 && v.y == 2);
	}
	std::cout << std::endl;
}

static void test_vec2_class_operators()
{
	std::cout << __func__ << std::endl;
	{
		Vec2 v(1, 2);
		v = -v;
		printResult("-vec2", v.x == -1 && v.y == -2);
	}
	{
		Vec2 v(1, 2);
		v += 2;
		printResult("vec2 += T", v.x == 3 && v.y == 4);
	}
	{
		Vec2 v(1, 2);
		v += Vec2(4, 5);
		printResult("vec2 += vec2", v.x == 5 && v.y == 7);
	}
	{
		Vec2 v(1, 2);
		v -= 3;
		printResult("vec2 -= T", v.x == -2 && v.y == -1);
	}
	{
		Vec2 v(1, 3);
		v -= Vec2(4, 5);
		printResult("vec2 -= vec2", v.x == -3 && v.y == -2);
	}
	{
		Vec2 v(1, 2);
		v *= 4;
		printResult("vec2 *= T", v.x == 4 && v.y == 8);
	}
	{
		Vec2 v(1, 4);
		v *= Vec2(5);
		printResult("vec2 *= vec2", v.x == 5 && v.y == 20);
	}
	{
		Vec2 v(10, 20);
		v /= 2;
		printResult("vec2 /= T", v.x == 5 && v.y == 10);
	}
	{
		Vec2 v(20, 40);
		v /= Vec2(5, 20);
		printResult("vec2 /= vec2", v.x == 4 && v.y == 2);
	}
	{
		Vec2 v(10, 14);
		Vec2 w(10, 14);
		printResult("vec2 == vec2", v == w);
	}
	{
		Vec2 v(10, 14);
		Vec2 w(10, 15);
		printResult("vec2 == vec2", !(v == w));
	}
	{
		Vec2 v(2, 3);
		Vec2 w(3, 6);
		printResult("vec2 != vec2", v != w);
	}
	{
		Vec2 v(2, 3);
		Vec2 w(2, 3);
		printResult("vec2 != vec2", !(v != w));
	}
	std::cout << std::endl;
}

static void test_vec2_operators()
{
	std::cout << __func__ << std::endl;
	{
		Vec2 v(3, 5);
		v = v + Vec2(2, 3);
		printResult("vec2 + vec2", v.x == 5 && v.y == 8);
	}
	{
		Vec2 v(4, 5);
		v = v + 6.f;
		printResult("vec2 + T", v.x == 10 && v.y == 11);
	}
	{
		Vec2 v(7, 3);
		v = 4.f + v;
		printResult("T + vec2", v.x == 11 && v.y == 7);
	}
	{
		Vec2 v(3, 5);
		v = v - Vec2(2, 1);
		printResult("vec2 - vec2", v.x == 1 && v.y == 4);
	}
	{
		Vec2 v(5, 8);
		v = v - 10.f;
		printResult("vec2 - T", v.x == -5 && v.y == -2);
	}
	{
		Vec2 v(4, 5);
		v = 10.f - v;
		printResult("T - vec2", v.x == 6 && v.y == 5);
	}
	{
		Vec2 v(5, 8);
		v = v * Vec2(3, 2);
		printResult("vec2 * vec2", v.x == 15 && v.y == 16);
	}
	{
		Vec2 v(6, 8);
		v = v * 3.f;
		printResult("vec2 * T", v.x == 18 && v.y == 24);
	}
	{
		Vec2 v(3, 7);
		v = 8.f * v;
		printResult("T * vec2", v.x == 24 && v.y == 56);
	}
	{
		Vec2 v(25, 30);
		v = v / Vec2(5, 3);
		printResult("vec2 / vec2", v.x == 5 && v.y == 10);
	}
	{
		Vec2 v(50, 75);
		v = v / 25.f;
		printResult("vec2 / T", v.x == 2 && v.y == 3);
	}
	{
		Vec2 v(4, 40);
		v = 80.f / v;
		printResult("T / vec2", v.x == 20 && v.y == 2);
	}
	std::cout << std::endl;
}

static void test_vec2()
{
	std::cout << __func__ << std::endl;
	test_vec2_constructors();
	test_vec2_class_operators();
	test_vec2_operators();
	std::cout << std::endl;
}

int main()
{
	test_vec1();
	test_vec2();
}
