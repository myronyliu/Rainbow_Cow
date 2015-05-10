#ifndef _SCENE_H_
#define _SCENE_H_


#include "stdafx.h"
#include "GlutDraw.h"

namespace Scene
{
enum{
    MIDPOINT_APPROXIMATION_METHOD = 0,
    QUADRIC_APPROXIMATION_METHOD = 2
};

class Object;
class Shader;
class Camera;

class World
{
public:
    World() : _cam(nullptr) {}

    void addObject(Object *);
    void addObject(Camera *);
    void removeObject(Object *);
    void assignShader(Object *, Shader *);
    Shader * findShader(Object *);

    //void removeObject(Object & obj) {  }

    Camera * getCam() { return _cam; }

    void draw();

    ~World() {};
private:
    std::vector<Object *> _objects;
    std::unordered_map<int, Shader *> _shaderMap;

    Camera * _cam;
};

World & createWorld();

    /* Base class for vert/frag shader. */
class Shader
{
public:
/* Constructors */
    Shader() : _vertfile(), _fragfile(), _shaderReady(false) { };
    Shader(std::string vertfile, std::string fragfile)
        : _vertfile(vertfile), _fragfile(fragfile), _shaderReady(false)
        {
            _initShaders();
        };

    virtual void link();
    virtual void unlink();
    GLuint getProgram() { return _program; };

/* Destructors */
    ~Shader() { glDeleteProgram(_program); }

private:
    std::string _vertfile, _fragfile;
    GLuint _program;
    GLuint _vertex;
    GLuint _frag;
    bool _shaderReady;

    void _initShaders();
    bool _checkShaderError(GLuint);
};

class Object
{
public:
/* Constructors */
    Object() : _tx(0), _ty(0), _tz(0), _phi(0), _the(0), _psi(0), _visible(true)
        {
            _objectID = nextID();
        }
    Object(float tx, float ty, float tz, float phi, float the, float psi) : _tx(tx), _ty(ty), _tz(tz),
        _phi(psi), _the(the), _psi(psi), _visible(true)
        {
            _objectID = nextID();
        }
    void draw();
    void draw(Shader *);
    virtual void doDraw() = 0;

    /* getters */
    float getTx() { return _tx; } const
    float getTy() { return _ty; } const
    float getTz() { return _tz; } const
    float getPhi() { return _phi; } const
    float getThe() { return _the; } const
    float getPsi() { return _psi; } const
    bool getVisible() { return _visible; } const
    World* getWorld() { return _world; } const
    int getID() { return _objectID; } const

    /* setters */
    void setTx(float tx) { _tx = tx; }
    void setTy(float ty) { _ty = ty; }
    void setTz(float tz) { _tz = tz; }
    void setPhi(float phi) { _phi = phi; }
    void setThe(float the) { _the = the; }
    void setPsi(float psi) { _psi = psi; }
    void setVisible(bool visible) { _visible = visible; }
    void setWorld(World * world) { _world = world; }

    /* Single line functions */
    int nextID() { return NEXTID++; }

    ~Object() { _world->removeObject(this); }

protected:
    World * _world;
    int _objectID;
    float _tx, _ty, _tz;
    float _phi, _the, _psi;
    bool _visible;

private:
    static int NEXTID;
};

class Camera : public Object
{
public:
/* Constructors */
    Camera() : Object() { }

    void doDraw();

private:
};

class Grid : public Object
{
public:
/* Constructors */
    Grid() : Object(), _rows(10), _cols(10), _gap(1.0f) { }
    Grid(int rows, int cols, float gap) : Object(),
        _rows(rows), _cols(cols), _gap(gap) { }

    void doDraw();

private:
    int _rows, _cols;
    float _gap;
};

class Arrow : public Object{
public:
    Arrow() : Object(), _tail(glm::vec3(0, 0, 0)), _head(glm::vec3(0, 0, 1)), _color(glm::vec4(1, 1, 1, 1)) {}
    Arrow(const glm::vec3& tail, const glm::vec3& head) : Object(), _tail(tail), _head(head), _color(glm::vec4(1, 1, 1, 1)) {}
    Arrow(const glm::vec3& tail, const glm::vec3& head, const glm::vec4& color) : Object(), _tail(tail), _head(head), _color(color) {}
    void doDraw();
private:
    glm::vec4 _color;
    glm::vec3 _head;
    glm::vec3 _tail;
};

class Sphere : public Object
{
public:
    /* Constructors */
    Sphere() : Object(), _r(5), _n(100), _m(100) { }
    Sphere(float radius, int n, int m) : Object(), _r(radius), _n(n), _m(m) { }

    void doDraw();

protected:
    int _n, _m; // number of theta and phi subdivisions respectively
    float _r;
};

class ObjGeometry : public Object
{
public:
    ObjGeometry(std::string filename) : Object() { _filename = filename; };
    void doDraw();

    ~ObjGeometry()
    {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &_vertexArrayID);
    }

private:
    bool _geomReady;
    int _readGeom();

    std::string _filename;
    std::vector<glm::vec3> _vertices;
    std::vector<glm::vec3> _normals;
    std::vector<glm::vec2> _uvs;

    GLuint _vertexArrayID;
};







struct VecComp {
    bool operator() (glm::vec3 lhs, glm::vec3 rhs) const
    {
        if (lhs[0] < rhs[0]) return true;
        else if (lhs[1] < rhs[1]) return true;
        else if (lhs[2] < rhs[2]) return true;
        else return false;
    }
};

using Vertex = int;
using Face = std::vector < int > ;
using Edge = std::pair < int, int > ;

struct EdgeComp{
    bool operator() (Edge lhs, Edge rhs) const {
        if (lhs.first < rhs.first) return true;
        else if (lhs.first > rhs.first) return false;
        else {
            if (lhs.second < rhs.second) return true;
            else return false;
        }
    }
};
class MeshObject: public Object{
public:
    MeshObject(std::string filename) : Object(),_approximationMethod(QUADRIC_APPROXIMATION_METHOD),_t(10) { _filename = filename; };
    ~MeshObject() {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &_vertexArrayID);
    }
    bool isEdge(const int& v0, const int& v1);
    void doDraw();
    int nVertices() { return _vertices.size(); }
    int nFaces() { return _faces.size(); }
    int approximationMethod() { return _approximationMethod; }
    void setApproximationMethod(const int& approximationMethod) { _approximationMethod = approximationMethod; }
    void setT(const float& t);
    void setFilename(const std::string& filename) { _filename = filename; _geomReady = false; }
    Edge randomEdge();
    glm::vec3 mergedCoordinates(const int& v0, const int& v1, const int& approximationMethod);
    glm::vec3 mergedCoordinates(const int& v0, const int& v1) { return mergedCoordinates(v0, v1, _approximationMethod); }
    void collapse(const int& v0, const int& v1);
    void collapse(const int& v0, const int& v1, const int& approximationMethod);
    void collapseRandomEdge(const int& approximationMethod = MIDPOINT_APPROXIMATION_METHOD);

    void reComputeQuadrics();
    void reComputeMetrics();
    void reComputeQuadricsAndMetrics();

    glm::mat4 quadric(const int& v);
    float metric(const int& v0, const int& v1);

    void updateLocalQuadricsAndMetrics(const int& v);
    void quadricSimplify();

    void reComputeVertexNormals();
    void reComputeFaceNormals();
    void removeRedundancies();
    void updateIndexBuffer();
    std::vector<float> readGeom();

protected:
    bool _geomReady;
    bool _faceNormalsReady; // hm maybe I should also make a vector<bool> _faceNormalReady
    bool _vertexNormalsReady;
    bool _quadricsReady;
    bool _metricsReady;
    int _approximationMethod;

    std::string _filename;
    std::map<int, std::set<int>> _adjacency;
    GLuint _vertexArrayID;

    std::vector<glm::vec3> _vertices; // these are for feeding into the vertex, normal, index buffers
    std::vector<glm::vec3> _vertexNormals;
    std::vector<glm::vec4> _vertexColors;
    std::vector<Face> _faces;
    std::vector<glm::vec3> _faceNormals;
    std::vector<int> _indices;

    float _t; // the distance threshold for quadric simplification
    std::vector<glm::mat4> _quadrics;
    std::map<Edge, float, EdgeComp> _pairMetric; // Quadric error metric for pairs of vertices that are within _t distance of one another
    std::map<float, std::set<Edge>> _metricPairs;
};


};


#endif