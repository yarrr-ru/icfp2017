#pragma once
#include <vector>
#include <string>
#include <fstream>

struct Point {
  double x, y;

  Point(): x(0), y(0) {}

  Point(double x, double y): x(x), y(y) {}
};

struct Line {
  Point a, b;
  std::string color;
  size_t stroke_width;

  Line(Point a, Point b, std::string color, size_t stroke_width):
      a(a), b(b), color(std::move(color)), stroke_width(stroke_width) {}
};

struct Circle {
  Point c;
  std::string color;

  Circle(Point c, std::string color): c(c), color(std::move(color)) {}
};

struct Text {
  Point pos;
  std::string text;

  Text(Point pos, std::string text): pos(pos), text(std::move(text)) {}
};

class Drawer {
public:
  Drawer() {}
  Drawer(const std::string &path);

  void line(Point a, Point b, std::string color, size_t stroke_width);
  void point(Point c, std::string color);
  void text(Point pos, std::string text);
  void close();

private:
  static constexpr size_t kWidth = 2000;
  static constexpr const size_t kHeight = 1000;
  static constexpr const size_t kBorder = 20;
  static constexpr const size_t kCircleRadius = 3;

  const std::string path;
  std::ofstream file;
  double minx, maxx, miny, maxy;

  std::vector<Line> lines;
  std::vector<Circle> circles;
  std::vector<Text> texts;

  void open();
  void update_bounds(Point p);
  double transform(double x, double l, double r, double size);
  Point transform(Point p);
};
