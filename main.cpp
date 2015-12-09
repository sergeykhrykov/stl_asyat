
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <map>
#include <unordered_set>
#include <conio.h>

struct Point {

  double x, y, z;

  bool operator<(const Point &other) const {
    if (x != other.x)
      return (x < other.x);

    if (y != other.y)
      return (y < other.y);

    return (z < other.z);
  }
};

typedef std::pair<Point, int> point_and_index;

bool compare(const point_and_index &x, const point_and_index &y) {
  return x.second < y.second;
};

struct Triangle {
  int N[3];
};

struct Edge {
  int A, B;

  bool operator==(const Edge &other) const {
    return (((A == other.A) && (B == other.B)) ||
            ((A == other.B) && (B == other.A)));
  }
};

struct Bin {
  float Normal_vector[3];
  float Vertex[3][3];
  unsigned short Attribute_byte_count;
};

template <> struct std::hash<Edge> {
  std::size_t operator()(const Edge &e) const {
    return std::hash<int>()(e.A) ^ std::hash<int>()(e.B);
  }
};

using namespace std;

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "Russian");

  if (1 == argc) {
    cout << "Usage: " << argv[0] << " <file.stl>" << endl;
    exit(EXIT_FAILURE);
  }

  ifstream fin;
  fin.open(argv[1], ios::in | ios::binary);

  if (!fin.is_open()) {
    cout << "Error: cannot open file " << argv[1] << endl;
    exit(EXIT_FAILURE);
  };

  char str[80];
  fin >> str;

  map<Point, int>
      points; // Store point indices in a map to eliminate duplicate points
  unordered_set<Edge> edges;
  vector<Triangle> triangles;

  Point new_point;
  Triangle new_triangle;

  int max_pt_index = 0;
  int curr_pt_index = 0;

  if (!strcmp(str, "solid")) //Файл текстовый?
  {
    string line;

    cout << "Файл имеет текстовый формат\n";

    getline(fin, line); // omit name of model

    string term;
    // fin >> term;

    // Reading loop
    while (term != "endsolid") {

      getline(fin, line); // omit normal
      /*fin >> term;
      fin >> term;
      fin >> new_point.x >> new_point.y >> new_point.z;*/

      getline(fin, line); // omit outer loop

      for (int i = 0; i < 3; i++) {
        fin >> term >> new_point.x >> new_point.y >>
            new_point.z;    // read vertex coords
        getline(fin, line); // go to the next line

        std::pair<const Point, const int> pt = {new_point, max_pt_index};
        auto res = points.insert(pt); // try to insert new point into map
        {
          if (res.second) {
            // if it was inserted, increment point index
            curr_pt_index = max_pt_index;
            max_pt_index++;
          } else {
            curr_pt_index = res.first->second; // if it wasn't, get index of the
            // point that had been added before
          }
        }
        new_triangle.N[i] = curr_pt_index;
      }

      triangles.push_back(new_triangle);

      // Insert edges to count unique edges
      Edge edge1{new_triangle.N[0], new_triangle.N[1]};
      Edge edge2{new_triangle.N[1], new_triangle.N[2]};
      Edge edge3{new_triangle.N[0], new_triangle.N[2]};
      edges.insert(edge1);
      edges.insert(edge2);
      edges.insert(edge3);

      getline(fin, line); // omit endloop
      getline(fin, line); // omit endfacet
      fin >> term;        // get next term to check whether endsolid is reached
    }
  } else // Файл бинарный
  {
    // cout << "bin.stl\n";
    Bin s;
    char Header[80];
    unsigned int Number_of_triangles;

    cout << "Файл имеет бинарный формат.\n";

    fin.seekg(0);

    fin.read((char *)Header, sizeof(Header));
    fin.read((char *)&Number_of_triangles, sizeof(Number_of_triangles));
    cout << "Название модели: " << Header << "\n"
         << "Треугольников: " << Number_of_triangles << " : \n";

    // for (unsigned int i = 0; i < 10; i++) //30треугольников
    for (unsigned int i = 0; i < Number_of_triangles;
         i++) //считывает все треугольники
    {
      fin.read((char *)s.Normal_vector, sizeof(s.Normal_vector));

      for (int i = 0; i < 3; i++) // Считываем вершины треугольника
      {
        fin.read((char *)s.Vertex[i], sizeof(float) * 3);

        new_point.x = s.Vertex[i][0];
        new_point.y = s.Vertex[i][1];
        new_point.z = s.Vertex[i][2];

        std::pair<const Point, const int> pt = {new_point, max_pt_index};
        auto res = points.insert(pt); // try to insert new point into map
        {
          if (res.second) {
            // if it was inserted, increment point index
            curr_pt_index = max_pt_index;
            max_pt_index++;
          } else {
            curr_pt_index = res.first->second; // if it wasn't, get index of the
            // point that had been added before
          }
        }
        new_triangle.N[i] = curr_pt_index;
      }

      triangles.push_back(new_triangle);

      // Insert edges to count unique edges
      Edge edge1{new_triangle.N[0], new_triangle.N[1]};
      Edge edge2{new_triangle.N[1], new_triangle.N[2]};
      Edge edge3{new_triangle.N[0], new_triangle.N[2]};
      edges.insert(edge1);
      edges.insert(edge2);
      edges.insert(edge3);

      fin.read((char *)&s.Attribute_byte_count, sizeof s.Attribute_byte_count);
    }
  }

  // Sort points by index because OFF doesn't allow to specify point index
  // explicitly
  vector<point_and_index> points_sorted(points.begin(), points.end());
  sort(points_sorted.begin(), points_sorted.end(), compare);

  const int num_points = points.size();
  const int num_triangles = triangles.size();
  const int num_edges = edges.size();

  if (num_points - num_edges + num_triangles == 2) {
    cout << "Ошибок геометрии не выявлено." << endl;
  } else {
    cout << "Обнаружены ошибки геометрии." << endl;
    _getch();
    return 1;
  }

  // Define Volume
  double V = 0.0, Vi = 0.0;
  int i = 0;

  vector<Point> pts;
  for (auto pt : points)
    pts.push_back(pt.first);

  for (i = 0; i < triangles.size(); ++i) {
    double x1, y1, z1, x2, y2, z2, x3, y3, z3;
    Triangle t = triangles[i];

    x1 = pts[t.N[0]].x;
    y1 = pts[t.N[0]].y;
    z1 = pts[t.N[0]].z;

    x2 = pts[t.N[1]].x;
    y2 = pts[t.N[1]].y;
    z2 = pts[t.N[1]].z;

    x3 = pts[t.N[2]].x;
    y3 = pts[t.N[2]].y;
    z3 = pts[t.N[2]].z;

    Vi = 1.0 / 6.0 * (-x3 * y2 * z1 + x2 * y3 * z1 + x3 * y1 * z2 -
                      x1 * y3 * z2 - x2 * y1 * z3 + x1 * y2 * z3);

    cout << "V[" << i << "] = " << Vi << "\n";

    V += Vi;
  }

  cout << "Объем: " << V << endl;

  _getch();
  return EXIT_SUCCESS;
}