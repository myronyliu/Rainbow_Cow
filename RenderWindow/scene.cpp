#include "scene.h"
#include "utils.h"

using namespace Scene;
using namespace std;
using namespace glm;
/** Global variables **/
int Object::NEXTID = 0;

/* Method Definitions */
void World::addObject(Object * obj)
{
    _objects.push_back(obj);
    obj->setWorld(this);
}

void World::addObject(Camera * cam)
{
    if (_cam == nullptr)
    {
        _objects.push_back(cam);
        cam->setWorld(this);
        _cam = cam;
    }
    else
    {
        std::cout << "Cam already set!" << std::endl;
    }
}

void World::draw()
{
    for (auto &object : _objects)
    {
        auto shader = _shaderMap.find(object->getID());
        if (shader != _shaderMap.end())
        {
            object->draw(_shaderMap[object->getID()]);
        }
        else
        {
            object->draw();
        }
    }
}

void World::removeObject(Object * obj)
{
    auto sameID = [&](Object * object) { return object->getID() == obj->getID();  };
    auto to_remove = std::remove_if(std::begin(_objects), std::end(_objects), sameID);
    _objects.erase(to_remove);
}

void World::assignShader(Object * obj, Shader * shader)
{
    _shaderMap[obj->getID()] = shader;
}
Shader * World::findShader(Object * obj)
{
    return _shaderMap[obj->getID()];
}


void Object::draw()
{
    if (!_visible) return;

    glPushMatrix();
    glTranslated(_tx, _ty, _tz);
    glRotated(_psi, 0, 0, 1);
    glRotated(_the, 0, 1, 0);
    glRotated(_phi, 0, 0, 1);

    doDraw();

    glPopMatrix();
}

void Object::draw(Shader * shader)
{
    if (!_visible) return;

    glPushMatrix();
    glTranslated(_tx, _ty, _tz);
    glRotated(_psi, 0, 0, 1);
    glRotated(_the, 0, 1, 0);
    glRotated(_phi, 0, 0, 1);

    shader->link();
    doDraw();
    shader->unlink();
    glPopMatrix();
}


void Camera::doDraw()
{
}

void Grid::doDraw()
{
    for (int r = -(_rows / 2); r <= (_rows / 2); r++)
    {
        GlutDraw::drawLine(-(_cols / 2.0f)*_gap, 0, r*_gap,
            (_cols / 2.0f)*_gap, 0, r*_gap);
    }
    for (int c = -(_cols / 2); c <= (_cols / 2); c++)
    {
        GlutDraw::drawLine(c*_gap, 0, -(_rows / 2.0f)*_gap,
            c*_gap, 0, (_rows / 2.0f)*_gap);
    }
}

void Arrow::doDraw() {
    glPushAttrib(GL_CURRENT_BIT);
    glColor3f(_color[0], _color[1], _color[2]);
    GlutDraw::drawLine(_tail[0], _tail[1], _tail[2], _head[0], _head[1], _head[2]);

    //float d = distance(_head, _tail);
    //vec3 n = normalize(_head - _tail);
    //vec3 pivot = _head - (d / 10)*n;
    //glBegin(GL_TRIANGLE_FAN);
    //glVertex3f(_head[0], _head[1], _head[2]);
    //glEnd();
    glPopAttrib();
}

void Sphere::doDraw()
{
    GlutDraw::drawSphere(_r, _n, _m);
}

void ObjGeometry::doDraw()
{
    if (!_geomReady)
    {
        _readGeom();
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, &_vertices[0]);
    glNormalPointer(GL_FLOAT, 0, &_normals[0]);

    glDrawArrays(GL_TRIANGLES, 0, _vertices.size());

    return;
}

// Adopted from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
int ObjGeometry::_readGeom()
{
    std::vector< int > vertexIndices, uvIndices, normalIndices;
    std::vector< glm::vec3 > tempVertices;
    std::vector< glm::vec2 > tempUVs;
    std::vector< glm::vec3 > tempNormals;
    int lineCount = 0;
    int faceCount = 0;
    int vertCount = 0;

    std::ifstream file;
    file.open(_filename, std::ios::in);
    if (!file.is_open())
    {
        std::cout << "Could not open " << _filename << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream linestream(line);
        std::string type;
        if (line.find("v ") == 0)
        {
            glm::vec3 vertex;
            linestream >> type >> vertex.x >> vertex.y >> vertex.z;
            vertex.x = vertex.x;
            vertex.y = vertex.y;
            vertex.z = vertex.z;
            tempVertices.push_back(vertex);
            vertCount++;
        }
        else if (line.find("vn ") == 0)
        {
            glm::vec3 normal;
            linestream >> type >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        }
        else if (line.find("vt ") == 0)
        {
            glm::vec2 uv;
            linestream >> type >> uv.x >> uv.y;
            tempUVs.push_back(uv);
        }
        else if (line.find("f ") == 0)
        {
            int vertexIndex[3], normalIndex[3], uvIndex[3];
            char delim;
            linestream >> type >>
                vertexIndex[0] >> delim >> uvIndex[0] >> delim >> normalIndex[0] >>
                vertexIndex[1] >> delim >> uvIndex[1] >> delim >> normalIndex[1] >>
                vertexIndex[2] >> delim >> uvIndex[2] >> delim >> normalIndex[2];

            for (int i = 0; i < 3; i++)
            {
                vertexIndices.push_back(vertexIndex[i]);
                normalIndices.push_back(normalIndex[i]);
                uvIndices.push_back(uvIndex[i]);
            }
            faceCount++;
        }

        lineCount++;
        //if (lineCount % 1000 == 0)
        //{
        //    std::cout << "Parsing obj line: " << lineCount << "\r";
        //}
    }
    std::cout << "Parsed " << lineCount << " lines Verts: " << vertCount << " Triangles: " << faceCount << std::endl;
    file.close();

    for (int i = 0; i < vertexIndices.size(); i++)
    {
        int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = tempVertices[vertexIndex - 1];
        _vertices.push_back(vertex);
    }
    for (int i = 0; i < normalIndices.size(); i++)
    {
        int normalIndex = normalIndices[i];
        glm::vec3 normal = tempNormals[normalIndex - 1];
        _normals.push_back(normal);
    }
    for (int i = 0; i < uvIndices.size(); i++)
    {
        int uvIndex = uvIndices[i];
        glm::vec2 uv = tempUVs[uvIndex - 1];
        _uvs.push_back(uv);
    }

    _geomReady = true;

    return lineCount;
}

World & Scene::createWorld()
{
    World * new_world = new World();
    return *new_world;
}

void Shader::_initShaders()
{
    if (_vertfile == "" || _fragfile == "")
    {
        std::cout << "No shaders! Initialization failing." << std::endl;
        return;
    }
    else if (_shaderReady)
    {
        std::cout << "Shader has already initialized." << std::endl;
        return;
    }

    char *vs, *fs;

    if (_vertfile == "" && _fragfile == ""){ return; }
    _program = glCreateProgram();

    if (_vertfile != "")
    {
        _vertex = glCreateShader(GL_VERTEX_SHADER);
        vs = textFileRead(_vertfile.c_str());
        const char * vv = vs;
        glShaderSource(_vertex, 1, &vv, NULL);
        free(vs);
        glCompileShader(_vertex);
        if (_checkShaderError(_vertex))
        {
            std::cout << _vertfile << " compiled successfully." << std::endl;
            glAttachShader(_program, _vertex);
        }
    }
    if (_fragfile != "")
    {
        _frag = glCreateShader(GL_FRAGMENT_SHADER);
        fs = textFileRead(_fragfile.c_str());
        const char * ff = fs;
        glShaderSource(_frag, 1, &ff, NULL);
        free(fs);
        glCompileShader(_frag);
        if (_checkShaderError(_frag))
        {
            std::cout << _fragfile << " compiled successfully." << std::endl;
            glAttachShader(_program, _frag);
        }
    }

    glLinkProgram(_program);

    glDetachShader(_program, _vertex);
    glDetachShader(_program, _frag);
    glDeleteShader(_vertex);
    glDeleteShader(_frag);

    _shaderReady = true;
    return;
}

bool Shader::_checkShaderError(GLuint shader)
{
    GLint result = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if (result == GL_TRUE) return true;

    GLint logsize = 0;
    char * log;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
    log = (char *)malloc(logsize + 1);
    glGetShaderInfoLog(shader, logsize, &result, log);

    std::cout << log << std::endl;
    return false;
}

void Shader::link()
{
    glUseProgram(_program);
}

void Shader::unlink()
{
    glUseProgram(0);
}













vector<float> MeshObject::readGeom(){
    printf("------------------------- READING .OFF FILE -------------------------\n");
    string file = _filename;
    string line;
    ifstream modelfile(_filename);
    if (!modelfile.is_open()) exit;
    getline(modelfile, line);
    if (line != "OFF") exit;
    getline(modelfile, line);
    int sp0, sp1, sp2, sp3; // location of spaces
    sp0 = line.find(' ');
    sp1 = line.find(' ', sp0 + 1);
    string nVs = line.substr(0, sp0);
    string nFs = line.substr(sp0 + 1, sp1);
    int nV = atoi(nVs.c_str());
    int nF = atoi(nFs.c_str());
    int printStepV = ceil((float)nV / 100.0);
    int printStepF = ceil((float)nF / 100.0);
    _quadrics.resize(nV);
    _vertices.reserve(nV);
    _vertexNormals.reserve(nV);
    _vertexColors.reserve(nV);
    _faces.reserve(nF);
    _indices.reserve(3 * nF);
    _faceNormals.reserve(nF);
    string s0, s1, s2;
    float xMin = 0;
    float xMax = 0;
    float yMin = 0;
    float yMax = 0;
    float zMin = 0;
    float zMax = 0;
    for (int i = 0; i < nV; i++){
        if (i%printStepV == 0 || i == nV - 1) printf("We're on face %i/%i\r", i + 1, nV);
        getline(modelfile, line);
        sp0 = line.find(' ');
        sp1 = line.find(' ', sp0 + 1);
        sp2 = line.find(' ', sp1 + 1);
        s0 = line.substr(0, sp0);
        s1 = line.substr(sp0 + 1, sp1);
        s2 = line.substr(sp1 + 1, sp2);
        float x = atof(s0.c_str());
        float y = atof(s1.c_str());
        float z = atof(s2.c_str());
        if (i == 0) { xMin = x; xMax = x; yMin = y; yMax = y; zMin = z; zMax = z; }
        else {
            if (x < xMin) xMin = x;
            if (y < yMin) yMin = y;
            if (z < zMin) zMin = z;
            if (x > xMax) xMax = x;
            if (y > yMax) yMax = y;
            if (z > zMax) zMax = z;
        }
        //_colors.push_back(vec4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1));
        _vertices.push_back(vec3(x, y, z));
        _vertexColors.push_back(vec4(1, 1, 1, 1));
        _vertexNormals.push_back(vec3(0, 0, 1)); // default normals to z-hat until computed
        _adjacency.emplace(i, set<int>());
    }
    std::cout << std::endl;
    float dAvg = 0; // rough estimate for average edge length. Not actually correct, but it suffices for picking appropriate _t
    for (int i = 0; i < nF; i++){
        if (i%printStepF == 0 || i == nF - 1) printf("We're on face %i/%i\r", i + 1, nF);
        getline(modelfile, line);
        sp0 = line.find(' ');
        sp1 = line.find(' ', sp0 + 1);
        sp2 = line.find(' ', sp1 + 1);
        sp3 = line.find(' ', sp2 + 1);
        s0 = line.substr(sp0 + 1, sp1);
        s1 = line.substr(sp1 + 1, sp2);
        s2 = line.substr(sp2 + 1, sp3);
        int v0 = atoi(s0.c_str());
        int v1 = atoi(s1.c_str());
        int v2 = atoi(s2.c_str());
        Face f = { v0, v1, v2 };
        _faces.push_back(f);
        _indices.push_back(v0); _indices.push_back(v1); _indices.push_back(v2);
        _faceNormals.push_back(vec3(0, 0, 1));
        _adjacency[v0].insert(_faces.size() - 1);
        _adjacency[v1].insert(_faces.size() - 1);
        _adjacency[v2].insert(_faces.size() - 1);
        float d = glm::distance(_vertices[v0], _vertices[v1]);
        d += glm::distance(_vertices[v1], _vertices[v2]);
        d += glm::distance(_vertices[v2], _vertices[v0]);
        d /= 3 * nF;
        dAvg += d;
    }
    printf("\n");
    printf("PROCESSING: Vertex/Face Normals\n");
    reComputeVertexNormals();
    printf("            Vertex Quadrics\n");
    reComputeQuadrics();
    printf("            Quadric Error Metrics...\n");
    setT(5.0 * dAvg);
    printf("------------------------------------------------------------\n");
    _geomReady = true;
    return vector<float>({ xMin, xMax, yMin, yMax, zMin, zMax });
}

Edge MeshObject::randomEdge() {
    if (_adjacency.size() == 0) return Edge({ -1, -1 });
    if (_adjacency.size() < 3) {
        printf("WARNING: No more triangles left to collapse,\n");
        return Edge({ _adjacency.begin()->first, _adjacency.begin()->first });
    }
    int v0, v1;
    while (true) {
        int m = fmin(_adjacency.size() - 1, (float)_adjacency.size()*rand() / RAND_MAX);
        map<int, set<int>>::iterator mIt = _adjacency.begin();
        for (int i = 0; i < m; i++) mIt++;
        v0 = mIt->first;
        set<int> fSet0 = mIt->second;
        if (fSet0.size() == 0) {
            printf("Vertex %i at (%f, %f, %f) has no adjacent faces. Trying again.\n", v0, _vertices[v0][0], _vertices[v0][1], _vertices[v0][2]);
            continue;
        }
        int f = fmin(fSet0.size() - 1, (float)fSet0.size()*rand() / RAND_MAX);
        set<int>::iterator fIt = fSet0.begin();
        int r = 1 + fmin(1, 2.0*rand() / RAND_MAX);
        for (int i = 0; i < f; i++) fIt++;
        for (int i = 0; i < 3; i++) {
            if (_faces[*fIt][i] == v0) {
                v1 = _faces[*fIt][(i + r) % 3];
                break;
            }
        }
        return Edge(fmin(v0,v1), fmax(v0,v1));
    }
}

bool intersect_union(const set<int>& fSet0, const set<int>& fSet1, set<int>& fIntersect, set<int>& fUnion){
    fIntersect.clear();
    fUnion = fSet1;
    for (set<int>::iterator f0 = fSet0.begin(); f0 != fSet0.end(); f0++){
        bool uniqueFlag = true;
        for (set<int>::iterator f1 = fSet1.begin(); f1 != fSet1.end(); f1++){
            if (*f0 == *f1) {
                fIntersect.insert(*f1);
                uniqueFlag = false;
                break;
            }
        }
        if (uniqueFlag == true) fUnion.insert(*f0);
    }
    if (fIntersect.size() == 0) return false;
    else return true;
}
void MeshObject::collapse(const int& v0, const int& v1) { collapse(v0, v1, _approximationMethod); }
void MeshObject::collapse(const int& v0, const int& v1, const int& approximationMethod) { // the former vertex is kept. the latter is discarded from adjacency
    if (_adjacency.size() < 3) return;
    _vertices[v0] = mergedCoordinates(v0, v1, approximationMethod);
    set<int> fSet0 = _adjacency[v0];
    set<int> fSet1 = _adjacency[v1];
    set<int> fUnion, fIntersect; // shared faces along edge (typically two unless mesh isn't "closed")
    intersect_union(fSet0, fSet1, fIntersect, fUnion);

    vector<int> vFinVec; // the third vertices (!=v0 && !=v1) of the shared faces
    for (set<int>::iterator f = fIntersect.begin(); f != fIntersect.end(); f++) { // For each of the shared faces
        _indices[3 * (*f) + 0] = 0; // Make the shared face degenerate in the index buffer so it doesn't get drawn
        _indices[3 * (*f) + 1] = 0;
        _indices[3 * (*f) + 2] = 0;
        for (int corner = 0; corner < 3; corner++) { // For each vertex that is connected to the shared face _faces[*f][v] ...
            set<int>::iterator it = _adjacency[_faces[*f][corner]].find(*f); // Remove the shared face *f from that vertex's list of adjacent faces
            if (it != _adjacency[_faces[*f][corner]].end()) _adjacency[_faces[*f][corner]].erase(it);
            if (_adjacency[_faces[*f][corner]].size() == 0) _adjacency.erase(_faces[*f][corner]);
            if (_faces[*f][corner] != v0 &&_faces[*f][corner] != v1) vFinVec.push_back(_faces[*f][corner]);
        }
    }
    // change all associations of faces adjacent to v1 from "v1 to v0"
    fSet1 = _adjacency[v1];
    for (set<int>::iterator f = fSet1.begin(); f != fSet1.end(); f++) {
        int newCorner = 0;
        for (int corner = 0; corner < 3; corner++) {
            if (_faces[*f][corner] == v0) { // if face already has v0 as a corner then it becomes degenerate, and we remove it
                for (int i = 0; i < 3; i++){
                    _indices[3 * (*f) + 0] = 0;
                    _indices[3 * (*f) + 1] = 0;
                    _indices[3 * (*f) + 2] = 0;
                    set<int>::iterator it = _adjacency[_faces[*f][i]].find(*f);
                    _adjacency[_faces[*f][i]].erase(it);
                    if (_adjacency[_faces[*f][i]].size() == 0) _adjacency.erase(_faces[*f][i]);
                    break;
                }
            }
            if (_faces[*f][corner] == v1) newCorner = corner;
            if (corner == 2) { // otherwise if we get to the end of the loop, replacing v1 with v0 in the face is no problem, so we proceed as such
                _faces[*f][newCorner] = v0;
                _indices[3 * (*f) + newCorner] = v0;
                _adjacency[v0].insert(*f); // DON'T FORGET TO ADD V1's NEIGHBORS TO V0's ADJACENCY
            }
        }
    }
    _adjacency.erase(v1); // Remove v1 from the _adjacency list
    // Update normals for FACES adjacent to v0
    fSet0 = _adjacency[v0];
    set<int> vSet0;
    for (set<int>::iterator f = fSet0.begin(); f != fSet0.end(); f++) {
        vec3 p[3] = { _vertices[_faces[*f][0]], _vertices[_faces[*f][1]], _vertices[_faces[*f][2]] };
        for (int c = 0; c < 3; c++) vSet0.insert(_faces[*f][c]);
        _faceNormals[*f] = normalize(cross(p[1] - p[0], p[2] - p[0]));
    }
    // Update normals for VERTICES adjacent to above faces
    for (set<int>::iterator v = vSet0.begin(); v != vSet0.end(); v++) {
        vec3 normal(0, 0, 0);
        for (set<int>::iterator fs = _adjacency[*v].begin(); fs != _adjacency[*v].end(); fs++) {
            normal += _faceNormals[*fs];
        }
        _vertexNormals[*v] = normal / (float)_adjacency[*v].size();
        // RAINBOW COWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
        float thetaX = acos(dot(_vertexNormals[*v], vec3(1, 0, 0)));
        float thetaY = acos(dot(_vertexNormals[*v], vec3(0, 1, 0)));
        float thetaZ = acos(dot(_vertexNormals[*v], vec3(0, 0, 1)));
        _vertexColors[*v] = vec4(0.75*cos(thetaX / 1.5), 0.75*cos(thetaY / 1.5), 0.75*cos(thetaZ / 1.5), 1);
    }
    // Remove from _metrics all the vertex pairs that contain v1
    for (map<int, set<int>>::iterator adj = _adjacency.begin(); adj != _adjacency.end(); adj++) {
        Edge key(fmin(v1, adj->first), fmax(v1, adj->first));
        map<Edge, float>::iterator m = _pairMetric.find(key);
        if (m == _pairMetric.end()) continue;
        map<float,set<Edge>>::iterator it = _metricPairs.find(m->second);
        if (it->second.size()>1) it->second.erase(key);
        else _metricPairs.erase(it);
        _pairMetric.erase(m);
    }
    // And update Quadrics (BLARGH... this order of operations stuff is driving me mad)
    updateLocalQuadricsAndMetrics(v0);
    // FINALLY! remove the fins if any exist ---------------------------------------------
    for (int i = 0; i < vFinVec.size(); i++) {
        fSet0 = _adjacency[v0];
        int vFin = vFinVec[i];
        int uFin = -1; // the third vertex index of the fin
        int fFin = -1; // the face index of the fin
        bool finFound = false;
        for (set<int>::iterator f = fSet0.begin(); f != fSet0.end(); f++) {
            Face corners = _faces[*f];
            for (int j = 0; j < 3; j++) {
                if (corners[j] != vFin) continue;
                int u = 0;
                if (corners[(j + 1) % 3] == v0) u = corners[(j + 2) % 3];
                else u = corners[(j + 1) % 3];
                if (u != uFin) {
                    uFin = u;
                    fFin = *f;
                }
                else {
                    finFound = true;
                    collapse(uFin, vFin, approximationMethod); // to remove fin call collapseEdge(_,_) recursively
                    collapse(v0, uFin, approximationMethod); // NOTE: the order of arguments in both lines
                }
                break;
            }
        }
        if (finFound == true) continue;
    }
}

void MeshObject::collapseRandomEdge(const int& approximationMethod) {
    Edge re = randomEdge();
    collapse(re.first, re.second, approximationMethod);
}

void MeshObject::setT(const float& t) {
    printf("Setting distance threshold to %f\n", t);
    printf("  Updating quadric error metrics between sufficiently close vertices\n");
    _t = t;
    for (int j = 0; j < _vertices.size(); j++) {
        for (int i = 0; i < j; i++) {
            float newMet = metric(i, j);
            Edge key(Edge(i, j)); // i<j is guaranteed. no need to sort
            map<Edge, float>::iterator m = _pairMetric.find(key);

            if (m != _pairMetric.end()) {
                float oldMet = m->second;
                if (oldMet == newMet) continue;
                map<float, set<Edge>>::iterator it = _metricPairs.find(oldMet); // this is for efficiency
                if (newMet < 0 || glm::distance(_vertices[i], _vertices[j]) > _t) {
                    if (it->second.size() > 1) it->second.erase(key);
                    else _metricPairs.erase(it);
                    _pairMetric.erase(m);
                }
                else {
                    if (it->second.size() > 1) it->second.erase(key);
                    else _metricPairs.erase(it);
                    _metricPairs[newMet].insert(key);
                    m->second = newMet;
                }
            }
            else {
                if (newMet < 0 || glm::distance(_vertices[i], _vertices[j]) > _t) continue;
                _metricPairs[newMet].insert(key);
                _pairMetric.emplace(key, newMet);
            }
        }
    }
    printf("  Found %i collapseable vertex pairs\n", _pairMetric.size());
}
bool MeshObject::isEdge(const int& v0, const int& v1) {
    set<int> adjFaces = _adjacency[v0];
    if (adjFaces.size() == 0) return false;
    for (set<int>::iterator f = adjFaces.begin(); f != adjFaces.end(); f++) {
        for (int c = 0; c < 3; c++) {
            if (_faces[*f][c] == v0) return true;
        }
    }
    return false;
}
void MeshObject::reComputeFaceNormals() {
    for (int i = 0; i < _faces.size(); i++) {
        vec3 p0 = _vertices[_faces[i][0]];
        vec3 p1 = _vertices[_faces[i][1]];
        vec3 p2 = _vertices[_faces[i][2]];
        vec3 e01 = p1 - p0; // edge 0->1 of face
        vec3 e02 = p2 - p0; //      0->2
        _faceNormals[i] = normalize(cross(e01, e02));
    }
    _faceNormalsReady = true;
}
void MeshObject::reComputeVertexNormals() {
    if (_faceNormalsReady == false) reComputeFaceNormals();
    for (map<int, set<int>>::const_iterator i = _adjacency.begin(); i != _adjacency.end(); i++){
        vec3 normal(0, 0, 0);
        set<int> adjFaces = i->second; // adjacent faces
        for (set<int>::iterator j = adjFaces.begin(); j != adjFaces.end(); j++){
            normal += _faceNormals[*j];
        }
        _vertexNormals[i->first] = normal / (float)adjFaces.size();
        // RAINBOW COWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
        float thetaX = acos(dot(_vertexNormals[i->first], vec3(1, 0, 0)));
        float thetaY = acos(dot(_vertexNormals[i->first], vec3(0, 1, 0)));
        float thetaZ = acos(dot(_vertexNormals[i->first], vec3(0, 0, 1)));
        _vertexColors[i->first] = vec4(0.75*cos(thetaX / 1.5), 0.75*cos(thetaY / 1.5), 0.75*cos(thetaZ / 1.5), 1);
    }
}
void printMat4(const mat4& M) {
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            cout << M[i][j];
        }
        cout << endl;
    }
    cout << endl;
}
vec3 MeshObject::mergedCoordinates(const int& v0, const int& v1, const int& approximationMethod) {
    if (approximationMethod == MIDPOINT_APPROXIMATION_METHOD) return (_vertices[v0] + _vertices[v1]) / 2.0f;
    if (approximationMethod == QUADRIC_APPROXIMATION_METHOD) {
        mat4 dQ = _quadrics[v0] + _quadrics[v1];
        dQ[0][3] = 0;
        dQ[1][3] = 0;
        dQ[2][3] = 0;
        dQ[3][3] = 1;
        vec4 optPos = inverse(dQ)*vec4(0, 0, 0, 1);
        return vec3(optPos[0], optPos[1], optPos[2]) / optPos[3];
    }
}
void MeshObject::reComputeQuadrics() {
    for (map<int, set<int>>::iterator m = _adjacency.begin(); m != _adjacency.end(); m++) {
        _quadrics[m->first] = quadric(m->first);
    }
    _quadricsReady = true;
}
void MeshObject::reComputeMetrics() {
    for (map<Edge, float>::iterator m = _pairMetric.begin(); m != _pairMetric.end(); m++) {
        Edge key = m->first;
        float oldMet = m->second;
        float newMet = metric(key.first, key.second);
        map<float, set<Edge>>::iterator it = _metricPairs.find(oldMet);
        if (newMet > 0 || glm::distance(_vertices[key.first], _vertices[key.second]) > _t) {
            _pairMetric[key] = newMet;
            if (it != _metricPairs.end()) _metricPairs[oldMet].erase(key);
            _metricPairs[newMet].insert(key);
        }
        else {
            _pairMetric.erase(key);
            if (it != _metricPairs.end()) _metricPairs[oldMet].erase(key);
        }
    }
}
mat4 MeshObject::quadric(const int& v) {
    if (_faceNormalsReady == false) reComputeFaceNormals();
    mat4 Q(0.0f);
    for (set<int>::iterator f = _adjacency[v].begin(); f != _adjacency[v].end(); f++){
        vec3 n = _faceNormals[*f];
        float d = -dot(_vertices[_faces[*f][0]], n);
        vec4 plane = vec4(n[0], n[1], n[2], d);
        Q += outerProduct(plane, plane);
    }
    return Q;
}



float MeshObject::metric(const int& v0, const int& v1) {
    if (_quadricsReady == false) reComputeQuadrics();
    mat4 Q = _quadrics[v0] + _quadrics[v1];
    mat4 dQ = Q;
    dQ[0][3] = 0;
    dQ[1][3] = 0;
    dQ[2][3] = 0;
    dQ[3][3] = 1;
    if (fabs(determinant(dQ)) < 0.000001) return INFINITY;
    vec4 Q_vPrime = Q*inverse(dQ)*vec4(0, 0, 0, 1);
    return dot(Q_vPrime, Q_vPrime);
}
void MeshObject::updateLocalQuadricsAndMetrics(const int& v) {
    set<int> adjFaces = _adjacency[v];
    set<int> vSet({ v }); // v's neighbors (including itself)
    for (set<int>::iterator f = adjFaces.begin(); f != adjFaces.end(); f++) {
        for (int c = 0; c < 3; c++) {
            int vCurrent = _faces[*f][c];
            if (vCurrent == v) continue;
            vSet.insert(vCurrent);
        }
    }
    for (set<int>::iterator vIt = vSet.begin(); vIt != vSet.end(); vIt++) {
        _quadrics[*vIt] = quadric(*vIt);
    }
    // Now that we have updated all local quadrics, we move onto updating all the metrics
    for (set<int>::iterator vIt = vSet.begin(); vIt != vSet.end(); vIt++) {
        for (map<int, set<int>>::iterator vf = _adjacency.begin(); vf != _adjacency.end(); vf++) {
            if (*vIt == vf->first) continue;
            float newMet = metric(*vIt, vf->first);
            Edge key(fmin(*vIt, vf->first), fmax(*vIt, vf->first));
            map<Edge, float>::iterator m = _pairMetric.find(key);

            if (m != _pairMetric.end()) {
                float oldMet = m->second;
                if (oldMet == newMet) continue;
                map<float, set<Edge>>::iterator it = _metricPairs.find(oldMet); // this is for efficiency
                if (newMet < 0 || glm::distance(_vertices[*vIt], _vertices[vf->first]) > _t) {
                    if (it->second.size() > 1) it->second.erase(key);
                    else _metricPairs.erase(it); // the old pair in metricPair has now been deleted
                    _pairMetric.erase(m);
                }
                else {
                    if (it->second.size() > 1) it->second.erase(key);
                    else _metricPairs.erase(it); // the old pair in metricPair has now been deleted
                    _metricPairs[newMet].insert(key);
                    m->second = newMet;
                }
            }
            else {
                if (newMet < 0 || glm::distance(_vertices[*vIt], _vertices[vf->first]) > _t) continue;
                _metricPairs[newMet].insert(key);
                _pairMetric.emplace(key, newMet);
            }
        }
    }
}

void MeshObject::quadricSimplify() {
    if (_pairMetric.size() == 0) {
        printf("WARNING: No points satisfy distance threshold. Consider increasing it.\n");
        return;
    }
    float minCost = _metricPairs.begin()->first;
    set<Edge> minCostEdgeSet = _metricPairs.begin()->second;
    int rn = fmin(minCostEdgeSet.size() - 1, (float)minCostEdgeSet.size()*rand() / RAND_MAX);
    set<Edge>::iterator it = minCostEdgeSet.begin();
    for (int i = 0; i < rn; i++) it++;
    Edge minCostEdge = *it;
    //printf("Collapsing pair %i %i\n", it->first, it->second);
    if (minCost == INFINITY) collapseRandomEdge(MIDPOINT_APPROXIMATION_METHOD);
    else collapse(minCostEdge.first, minCostEdge.second, QUADRIC_APPROXIMATION_METHOD);
}

void MeshObject::updateIndexBuffer() {
    _indices.clear();
    set<int> faceSet;
    for (map<int, set<int>>::const_iterator i = _adjacency.begin(); i != _adjacency.end(); i++){
        set<int> adjFaces = i->second; // adjacent faces
        for (set<int>::iterator j = adjFaces.begin(); j != adjFaces.end(); j++){
            faceSet.insert(*j); // add UNIQUE faces to the faceSet for later use (hence a "set")
        }
    }
    for (set<int>::iterator i = faceSet.begin(); i != faceSet.end(); i++){
        for (int j = 0; j < 3; j++){
            _indices.push_back(_faces[*i][j]);
        }
    }
}

void MeshObject::removeRedundancies() {
    MeshObject old = *this;
    _faces.clear();
    _indices.clear();
    _pairMetric.clear();
    _quadrics.clear();
    _vertices.clear();
    _faceNormals.clear();
    _vertexColors.clear();
    _vertexNormals.clear();
    map<int, int> vOld_vNew;
    set<int> faceSet;
    int vCounter = 0;
    for (map<int, set<int>>::const_iterator vfs = _adjacency.begin(); vfs != _adjacency.end(); vfs++){
        int v = vfs->first;
        set<int> fs = vfs->second; // adjacent faces
        vOld_vNew.emplace(v, vCounter);
        vCounter++;
        _vertices.push_back(old._vertices[v]);
        _quadrics.push_back(old._quadrics[v]);
        _vertexColors.push_back(old._vertexColors[v]);
        _vertexNormals.push_back(old._vertexNormals[v]);
        for (set<int>::iterator f = fs.begin(); f != fs.end(); f++) faceSet.insert(*f);
    }
    for (set<int>::iterator f = faceSet.begin(); f != faceSet.end(); f++){
        int c0 = vOld_vNew[old._faces[*f][0]];
        int c1 = vOld_vNew[old._faces[*f][1]];
        int c2 = vOld_vNew[old._faces[*f][2]];
        _indices.push_back(c0);
        _indices.push_back(c1);
        _indices.push_back(c2);
        _faces.push_back({ c0, c1, c2 });
        _faceNormals.push_back(old._faceNormals[*f]);
    }
    for (map<Edge, float>::iterator m = old._pairMetric.begin(); m != old._pairMetric.end(); m++) {
        int v0 = vOld_vNew[m->first.first];
        int v1 = vOld_vNew[m->first.second];
        _pairMetric.emplace(Edge(v0, v1), m->second);
    }
}

void MeshObject::doDraw()
{
    if (!_geomReady) readGeom();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, &_vertices[0]);
    glNormalPointer(GL_FLOAT, 0, &_vertexNormals[0]);
    glColorPointer(4, GL_FLOAT, 0, &_vertexColors[0]);

    GLuint IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_indices.size(), &_indices[0], GL_STATIC_DRAW);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);

    
    /* glEnableClientState(GL_VERTEX_ARRAY);
    vector<glm::vec3> asdf;
    asdf.reserve(2 * _vertices.size());
    for (int i = 0; i < _vertices.size(); i++) {
        asdf.push_back(_vertices[i]);
        asdf.push_back(_vertices[i] + _faceNormals[i] / 5.0f);
    }

    glVertexPointer(3, GL_FLOAT, 0, &asdf[0]);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*asdf.size(), &asdf[0], GL_STATIC_DRAW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_LINES, 0, asdf.size()); */

    return;
}