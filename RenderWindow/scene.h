#ifndef _SCENE_H_
#define _SCENE_H_


#include "stdafx.h"
#include "GlutDraw.h"

namespace Scene
{
enum{
    BINARY_APPROXIMATION_METHOD = 0,
    MIDPOINT_APPROXIMATION_METHOD = 1,
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






template <class T>
class reservable_priority_queue : public std::priority_queue<T>
{
public:
    typedef typename std::priority_queue<T>::size_type size_type;
    reservable_priority_queue(size_type capacity = 0) { reserve(capacity); };
    void reserve(size_type capacity) { this->c.reserve(capacity); }
    size_type capacity() const { return this->c.capacity(); }
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
struct Edge {
    int _u0;
    int _u1;
    glm::vec3 _op; // optimal position
    float _qem; // metric
    int _c0; // timestamp for last collapse
    int _c1;
    Edge() { _u0 = 0; _u1 = 0; _op = glm::vec3(0, 0, 0); _qem = 0; _c0 = 0; _c1 = 0; }
    Edge(const int& u0, const int& u1, const glm::vec3& op, const float& qem, const int & c0, const int& c1) {
        _u0 = u0; _u1 = u1; _op = op; _qem = qem; _c0 = c0; _c1 = c1;
    }
    Edge(const int& u0, const int& u1, const std::pair<glm::vec3, float>& opqem, const int & c0, const int& c1) {
        _u0 = u0; _u1 = u1; _op = opqem.first; _qem = opqem.second; _c0 = c0; _c1 = c1;
    }
    bool operator<(const Edge& rhs) const {
        if (_qem > rhs._qem) return true;
        if (_qem < rhs._qem) return false;
        if (_c0 < rhs._c0) return true;  // these are reversed because we want the larger collapse index to show up first
        if (_c0 > rhs._c0) return false; //
        if (_c1 < rhs._c1) return true;  //
        if (_c1 > rhs._c1) return false; //
        if (_u0 > rhs._u0) return true;  //
        if (_u0 < rhs._u0) return false;
        if (_u1 > rhs._u1) return true;
        if (_u1 < rhs._u1) return false;
        if (_op.x > rhs._op.x) return true;
        if (_op.x < rhs._op.x) return false;
        if (_op.y > rhs._op.y) return true;
        if (_op.y < rhs._op.y) return false;
        if (_op.z > rhs._op.z) return true;
        if (_op.z < rhs._op.z) return false;
        return false;
    }
};
class MeshObject: public Object{
public:
    MeshObject(std::string iFileName) : Object() {
        _t = 1;
        _nCollapses = 0;
        _allowFins = false;
        _drawVertexNormals = true;
        _aggressiveSimplification = true;
        _approximationMethod = QUADRIC_APPROXIMATION_METHOD;
        _iFileName = iFileName;
        _oFileName = iFileName + "pm";
        _drawMode = 0;
        _customColors = false;
    }
    ~MeshObject() {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &_vertexArrayID);
    }
    bool atCorner(const int& v);
    bool atBoundary(const int& v);
    float avgEdgeLength();
    bool isEdge(const int& v0, const int& v1);
    void doDraw();
    int nCollapsablePairs() {
        if (_pairs.size() == 1) return 0;
        int count = 0;
        for (int i = 0; i < nVertices(); i++) {
            count += _partners[i].size();
        }
        return count / 2;
    }
    int nVisibleVertices() {
        std::vector<int> vf = visibleFaces();
        std::vector<int> v(nVertices(), 0);
        for (int i = 0; i < vf.size(); i++) {
            for (int j = 0; j < 3; j++) v[_faces[vf[i]][j]] = 1;
        }
        int count = 0;
        for (int i = 0; i < v.size(); i++) {
            count += v[i];
        }
        return count;
    }
    int nVisibleFaces();
    std::vector<int> visibleFaces();
    int approximationMethod() { return _approximationMethod; }
    void setApproximationMethod(const int& approximationMethod) { _approximationMethod = approximationMethod; }
    void setT(const float& t);
    std::string inFileName() { return _iFileName; }
    std::string outFileName() { return _oFileName; }
    void setInFileName(const std::string& iFileName) { _iFileName = iFileName; _geomReady = false; }
    void setOutFileName(const std::string& oFileName) { _oFileName = oFileName; }
    void setVertexColor(const int& v, const glm::vec4& c) { _vertexColors[v] = c; }

    std::pair<int,int> randomEdge();
    glm::vec3 mergedCoordinates(const int& v0, const int& v1, const int& approximationMethod);
    glm::vec3 mergedCoordinates(const int& v0, const int& v1) { return mergedCoordinates(v0, v1, _approximationMethod); }
    void collapse(const int& v0, const int& v1);
    void collapse(const int& v0, const int& v1, const int& approximationMethod);
    void collapseTo(const float& newComplexity);
    void collapseRandomEdge(const int& approximationMethod = MIDPOINT_APPROXIMATION_METHOD);

    void makeProgressiveMeshFile();

    void allowFins() { _allowFins = true; }
    void disallowFins() { _allowFins = false; }
    float faceArea(const int& f);
    glm::vec3 faceNormal(const int& f);

    void reComputeQuadrics();

    glm::mat4 quadric(const int& v);
    std::pair<glm::vec3,float> metric(const int& v0, const int& v1);

    void updateQuadricsAndMetrics(const int& v0, const int& v1, const std::set<int>& vShared); // updates _pairs ASSUMING THAT THE _PAIRS.TOP() was collapsed.
    void quadricSimplify();

    void reComputeVertexNormals();
    void reComputeFaceNormals();
    void readGeom();
    void readGeomOFF(); // read full data
    void readGeomOFFPM(); // read progressive mesh

    float xMin() { return _xMin; }
    float xMax() { return _xMax; }
    float yMin() { return _yMin; }
    float yMax() { return _yMax; }
    float zMin() { return _zMin; }
    float zMax() { return _zMax; }
    int nVertices() { return _dummy.size(); }
    int nVerticesCollapsed() { return _dummyCollapsed.size(); }
    int nFaces() { return _faces.size(); }

    float complexity() { return _complexity; }
    std::string format() { return _format; }
    int drawMode() { return _drawMode; }
    void toggleDrawMode() { _drawMode = (_drawMode + 1) % 2; }
    bool customColors() { return _customColors; }
    void toggleCustomColors() { _customColors = !_customColors; }
    std::map<int, std::set<int>> adjacency() { return _adjacency; }
    std::set<int> adjacency(const int& v) { return _adjacency[v]; }
    void makeAdjacencyFromIndices();

    std::pair<std::vector<glm::vec3>,std::vector<glm::vec4>> vRedundant() {
        std::vector<glm::vec3> out;
        std::vector<glm::vec4> out2;
        for (int i = 0; i < _triangleIndices.size(); i++) {
            out.push_back(_vertexPositions[_triangleIndices[i]]);
            out2.push_back(_vertexColors[_triangleIndices[i]]);
        }
        return std::pair<std::vector<glm::vec3>, std::vector<glm::vec4>>(out, out2);
    }

    std::vector<int> faces(const int& f) { return _faces[f]; }
protected:
    std::string _format;
    int _nVcollapsed;
    int _nFcollapsed;
    //int _nV;
    //int _nF;
    float _xMin;
    float _xMax;
    float _yMin;
    float _yMax;
    float _zMin;
    float _zMax;

    bool _geomReady;
    bool _allowFins;
    bool _metricsReady;
    bool _quadricsReady;
    bool _faceNormalsReady; // hm maybe I should also make a vector<bool> _faceNormalReady
    bool _drawVertexNormals;
    bool _vertexNormalsReady;
    bool _aggressiveSimplification;
    bool _customColors;
    int _drawMode; // 0 for wire, 1 for filled triangles

    int _nCollapses;
    int _approximationMethod;

    std::string _iFileName;
    std::string _oFileName;

    std::map<int, std::set<int>> _adjacency;
    GLuint _vertexArrayID;

    std::vector<glm::vec3> _vertexPositions; // these are for feeding into the vertex, normal, index buffers
    std::vector<glm::vec3> _vertexNormals; // we duplicate it for drawing the normals
    std::vector<glm::vec4> _vertexColors;
    std::vector<Face> _faces;
    std::vector<glm::vec3> _faceNormals;
    std::vector<float> _faceAreas;
    std::vector<int> _triangleIndices;
    std::vector<int> _lineIndices;

    float _t; // the distance threshold for quadric simplification
    std::vector<glm::mat4> _quadrics;
    reservable_priority_queue<Edge> _pairs;
    std::vector<int> _lastUpdate;
    std::vector<std::set<int>> _partners;

    ////////////////////////////////////////
    ///// STUFF FOR PROGRESSIVE MESHES /////
    ////////////////////////////////////////
    std::vector<int> _v0;
    std::vector<int> _v1;
    std::vector<glm::vec3> _n0;
    std::vector<glm::vec3> _n1;
    std::vector<glm::vec3> _n;
    std::vector<glm::vec3> _xyz0; // coordinates of v0 before collapse
    std::vector<glm::vec3> _xyz1; // coordinates of v1 before collapse
    std::vector<glm::vec3> _xyz;  // coordinates of merge(v0,v1) after collapse (REPLACES xyz0)
    std::vector<std::vector<int>> _fVec1;
    std::vector<std::vector<int>> _fVec;
    std::vector<std::vector<int>> _fVecR; // shared faces to remove
    std::vector<std::vector<std::vector<int>>> _fVecRijk;

    std::vector<bool> _dummyCollapsed;
    std::vector<bool> _dummy;

    float _complexity; // the current number of vertices
};



}


#endif