#include "draw.h"

#include <sstream>
#include <iomanip>
#include <cassert>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

Drawer::Drawer(const std::string &path): path(path) {
  open();
}

void Drawer::open() {
  struct stat st;
  if (stat(path.c_str(), &st) == -1) {
    mkdir(path.c_str(), 0700);
  }

  size_t file_count = 0;
  DIR* dirp;
  struct dirent* entry;
  dirp = opendir(path.c_str());
  while ((entry = readdir(dirp)) != NULL) {
    if (entry->d_type == DT_REG) {
      file_count++;
    }
  }
  closedir(dirp);

  std::ostringstream filename;
  filename << path << "/";
  filename << std::setw(6) << std::setfill('0') << file_count << ".svg";
  file = std::ofstream(filename.str());
  assert(file);
  file << "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 " << kWidth << ' ' << kHeight << "'>\n";
  file << "<rect x='0' y='0' width='" << kWidth << "' height='" << kHeight << "' style='fill:white' />\n";

  minx = miny = 1e18;
  maxx = maxy = -1e18;
}

void Drawer::update_bounds(Point p) {
  minx = std::min(minx, p.x);
  miny = std::min(miny, p.y);
  maxx = std::max(maxx, p.x);
  maxy = std::max(maxy, p.y);
}

void Drawer::line(Point a, Point b, std::string color, size_t stroke_width) {
  update_bounds(a);
  update_bounds(b);
  lines.emplace_back(a, b, color, stroke_width);
}

void Drawer::point(Point c, std::string color) {
  update_bounds(c);
  circles.emplace_back(c, color);
}

void Drawer::text(Point pos, std::string text) {
  update_bounds(pos);
  texts.emplace_back(pos, text);
}

double Drawer::transform(double x, double l, double r, double size) {
  return kBorder + (x - l) / (r - l) * (size - kBorder * 2);
}

Point Drawer::transform(Point p) {
  return Point(transform(p.x, minx, maxx, kWidth), transform(p.y, miny, maxy, kHeight));
}

void Drawer::close() {
  assert(file);

  for (auto line : lines) {
    Point a = transform(line.a);
    Point b = transform(line.b);
    file << "<line x1='" << a.x << "' y1='" << a.y << "' x2='" << b.x << "' y2='" << b.y << 
      "' stroke='" << line.color << "' stroke-width='" << line.stroke_width << "'/>\n";
  }
  for (auto circle : circles) {
    Point c = transform(circle.c);
    file << "<circle cx='" << c.x << "' cy='" << c.y << "' r='" << kCircleRadius << 
      "' fill='" << circle.color << "'/>\n";
  }
  for (auto text : texts) {
    Point pos = transform(text.pos);
    file << "<text x='" << pos.x << "' y='" << pos.y << "' font-size='10px'>" << text.text << "</text>\n";
  }
  file << "</svg>\n";
  file.close();
}
