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











string MeshObject::currentMeshString() {
    string str;
    // write vertices to string
    for (map<int, set<int>>::iterator adj = _adjacency.begin(); adj != _adjacency.end(); adj++) {
        int vIndex = adj->first;
        set<int> fIndexSet = adj->second;
        str += to_string(vIndex) + ' ' + to_string(_vertices[vIndex][0]) + ' ' + to_string(_vertices[vIndex][1]) + ' ' + to_string(_vertices[vIndex][2]) + '\n';
    }
    // write faces to string
    vector<int> visFaceIndices = visibleFaces();
    for (int i = 0; i < visFaceIndices.size(); i++){
        int f = visFaceIndices[i];
        vector<int> v = _faces[f];
        str += to_string(f) + ' ' + to_string(v[0]) + ' ' + to_string(v[1]) + ' ' + to_string(v[2]) + '\n';
    }
    return str;
}
void MeshObject::writeCollapse(
    const int& v0,
    const glm::vec3& xyz0,
    const set<int>& fSet0,
    const int& v1,
    const glm::vec3& xyz1,
    const set<int>& fSet1,
    const int& v,
    const glm::vec3& xyz,
    const set<int>& fSet,
    const set<int>& fSetR) 
{
    _collapseString += to_string(v0) + ' ' + to_string(xyz0[0]) + ' ' + to_string(xyz0[1]) + ' ' + to_string(xyz0[2]);
    for (set<int>::iterator f = fSet0.begin(); f != fSet0.end(); f++) _collapseString += ' ' + to_string(*f);
    _collapseString += '\n';
    _collapseString += to_string(v1) + ' ' + to_string(xyz1[0]) + ' ' + to_string(xyz1[1]) + ' ' + to_string(xyz1[2]);
    for (set<int>::iterator f = fSet1.begin(); f != fSet1.end(); f++) _collapseString += ' ' + to_string(*f);
    _collapseString += '\n';
    _collapseString += to_string(v) + ' ' + to_string(xyz[0]) + ' ' + to_string(xyz[1]) + ' ' + to_string(xyz[2]);
    for (set<int>::iterator f = fSet.begin(); f != fSet.end(); f++) _collapseString += ' ' + to_string(*f);
    _collapseString += '\n';
    for (set<int>::iterator f = fSetR.begin(); f != fSetR.end(); f++) {
        _collapseString += to_string(*f) + ' ' + to_string(_faces[*f][0]) + ' ' + to_string(_faces[*f][1]) + ' ' + to_string(_faces[*f][2]);
        if (f != fSetR.end()) _collapseString += ' ';
    }
    _collapseString += '\n';
}
void MeshObject::makeProgressiveMeshFile() {
    std::ofstream oFile;
    oFile.open(_oFileName);
    oFile << "OFFPM\n";
    oFile << std::to_string(_vertices.size()) + " " + std::to_string(_faces.size()) + "\n";
    oFile << std::to_string(_adjacency.size()) + " " + std::to_string(nVisibleFaces()) + " " + std::to_string(_vertices.size() - _adjacency.size()) + "\n";
    oFile << currentMeshString();
    oFile << _collapseString;
    oFile.close();
}

vector<float> MeshObject::readGeom(){
    printf("------------------------- READING .OFF FILE -------------------------\n");
    string file = _iFileName;
    string line;
    ifstream modelfile(_iFileName);
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
    _vertexNormalTailHeads.reserve(2*nV);
    _vertexNormalTailHeadColors.reserve(2 * nV);
    _faces.reserve(nF);
    _triangleIndices.reserve(3 * nF);
    _lineIndices.reserve(2 * nF);
    _faceNormals.reserve(nF);
    _faceAreas.reserve(nF);
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
        _lineIndices.push_back(2 * i + 0);
        _lineIndices.push_back(2 * i + 1);
        _vertexNormalTailHeads.push_back(vec3(0, 0, 1));
        _vertexNormalTailHeads.push_back(vec3(0, 0, 1));
        _vertexNormalTailHeadColors.push_back(vec4(0, 0, 0, 0));
        _vertexNormalTailHeadColors.push_back(vec4(0, 0, 0, 0));
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
        _triangleIndices.push_back(v0); _triangleIndices.push_back(v1); _triangleIndices.push_back(v2);
        _faceNormals.push_back(vec3(0, 0, 1));
        _faceAreas.push_back(0);
        _adjacency[v0].insert(_faces.size() - 1);
        _adjacency[v1].insert(_faces.size() - 1);
        _adjacency[v2].insert(_faces.size() - 1);
        float d = glm::distance(_vertices[v0], _vertices[v1]);
        d += glm::distance(_vertices[v1], _vertices[v2]);
        d += glm::distance(_vertices[v2], _vertices[v0]);
        d /= 3 * nF;
        dAvg += d;
    }
    _scale = vec3(xMax - xMin, yMax - yMin, zMax - zMin);
    printf("\n");
    printf("PROCESSING: Vertex/Face Normals\n");
    reComputeVertexNormals();
    printf("            Vertex Quadrics\n");
    reComputeQuadrics();
    printf("            Quadric Error Metrics...\n");
    setT(2 * dAvg);
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
    if (_adjacency.size() < 4) {
        for (auto m = _adjacency.begin(); m != _adjacency.end(); m++) {
            _lineIndices[2 * m->first + 0] = 0;
            _lineIndices[2 * m->first + 1] = 0;
        }
        return;
    }
    vec3 xyz0 = _vertices[v0];
    vec3 xyz1 = _vertices[v1];
    set<int> fs0 = _adjacency[v0];
    set<int> fs1 = _adjacency[v1];
    _lineIndices[2 * v1 + 0] = 0;
    _lineIndices[2 * v1 + 1] = 0;
    vec3 xyz = mergedCoordinates(v0, v1, approximationMethod);
    _vertices[v0] = xyz;
    set<int> fSet0 = fs0;
    set<int> fSet1 = fs1;
    set<int> fUnion, fIntersect; // shared faces along edge (typically two unless mesh isn't "closed")
    intersect_union(fSet0, fSet1, fIntersect, fUnion);
    vector<int> fVecR;
    vector<vector<int>> fVecR_corners;

    vector<int> vFinVec; // the third vertices (!=v0 && !=v1) of the shared faces
    for (set<int>::iterator f = fIntersect.begin(); f != fIntersect.end(); f++) { // For each of the shared faces
        _triangleIndices[3 * (*f) + 0] = 0; // Make the shared face degenerate in the index buffer so it doesn't get drawn
        _triangleIndices[3 * (*f) + 1] = 0;
        _triangleIndices[3 * (*f) + 2] = 0;
        for (int corner = 0; corner < 3; corner++) { // For each vertex that is connected to the shared face _faces[*f][v] ...
            set<int>::iterator it = _adjacency[_faces[*f][corner]].find(*f); // Remove the shared face *f from that vertex's list of adjacent faces
            if (it != _adjacency[_faces[*f][corner]].end()) _adjacency[_faces[*f][corner]].erase(it);
            if (_adjacency[_faces[*f][corner]].size() == 0) _adjacency.erase(_faces[*f][corner]);
            if (_faces[*f][corner] != v0 &&_faces[*f][corner] != v1) vFinVec.push_back(_faces[*f][corner]);
        }
    }
    // change all associations of faces adjacent to v1 from "v1 to v0"
    fSet1 = _adjacency[v1]; // this is important. we don't want to change _faces[fIntersect], since we need it later for writing to ProgMesh file
    for (set<int>::iterator f = fSet1.begin(); f != fSet1.end(); f++) {
        int newCorner = 0;
        for (int corner = 0; corner < 3; corner++) {
            /*if (_faces[*f][corner] == v0) { // if face already has v0 as a corner then it becomes degenerate, and we remove it
                printf("wait... this check should be redundant, since we already removed the shared faces\n");
                _lineIndices[2 * _faces[*f][corner] + 0] = 0;
                _lineIndices[2 * _faces[*f][corner] + 1] = 0;
                for (int i = 0; i < 3; i++){
                    _triangleIndices[3 * (*f) + 0] = 0;
                    _triangleIndices[3 * (*f) + 1] = 0;
                    _triangleIndices[3 * (*f) + 2] = 0;
                    set<int>::iterator it = _adjacency[_faces[*f][i]].find(*f);
                    _adjacency[_faces[*f][i]].erase(it);
                    if (_adjacency[_faces[*f][i]].size() == 0) _adjacency.erase(_faces[*f][i]);
                    break;
                }
            }*/
            if (_faces[*f][corner] == v1) newCorner = corner;
            if (corner == 2) { // otherwise if we get to the end of the loop, replacing v1 with v0 in the face is no problem, so we proceed as such
                _faces[*f][newCorner] = v0;
                _triangleIndices[3 * (*f) + newCorner] = v0;
                _adjacency[v0].insert(*f); // DON'T FORGET TO ADD V1's NEIGHBORS TO V0's ADJACENCY
            }
        }
    }
    _adjacency.erase(v1); // Remove v1 from the _adjacency list
    if (_saveCollapses == true) writeCollapse(v0, xyz0, fs0, v1, xyz1, fs1, v0, xyz, _adjacency[v0], fIntersect);
    // Update normals for FACES adjacent to v0
    fSet0 = _adjacency[v0];
    set<int> vSet0;
    for (set<int>::iterator f = fSet0.begin(); f != fSet0.end(); f++) {
        vec3 p[3] = { _vertices[_faces[*f][0]], _vertices[_faces[*f][1]], _vertices[_faces[*f][2]] };
        for (int c = 0; c < 3; c++) vSet0.insert(_faces[*f][c]);
        vec3 n = cross(p[1] - p[0], p[2] - p[0]);
        _faceAreas[*f] = n.length();
        _faceNormals[*f] = normalize(n);
    }
    // Update normals for VERTICES adjacent to above faces (including v0 itself)
    for (set<int>::iterator v = vSet0.begin(); v != vSet0.end(); v++) {
        vec3 n(0, 0, 0);
        float nScale = 0;
        for (set<int>::iterator fs = _adjacency[*v].begin(); fs != _adjacency[*v].end(); fs++) {
            n += _faceNormals[*fs];
            nScale += _faceAreas[*fs];
        }
        n = normalize(n / (float)_adjacency[*v].size());
        nScale = sqrt(nScale / (float)_adjacency[*v].size()) / 8;
        _vertexNormals[*v] = n;
        _vertexNormalTailHeads[2 * (*v) + 0] = _vertices[*v];
        _vertexNormalTailHeads[2 * (*v) + 1] = _vertices[*v] + nScale*n;
        _vertexNormalTailHeadColors[2 * (*v) + 0] = _vertexColors[*v];
        _vertexNormalTailHeadColors[2 * (*v) + 1] = _vertexColors[*v];
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
    if (_allowFins == false) {
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
    // RAINBOW COWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
    for (set<int>::iterator v = vSet0.begin(); v != vSet0.end(); v++) {
        float thetaX = acos(dot(_vertexNormals[*v], vec3(1, 0, 0)));
        float thetaY = acos(dot(_vertexNormals[*v], vec3(0, 1, 0)));
        float thetaZ = acos(dot(_vertexNormals[*v], vec3(0, 0, 1)));
        _vertexColors[*v] = vec4(0.75*cos(thetaX / 2.0), 0.75*cos(thetaY / 2.0), 0.75*cos(thetaZ / 2.0), 1);
        if (_vertexColors[*v] == vec4(0, 0, 0, 0)) printf("%f %f %f\n", _vertexNormals[*v][0], _vertexNormals[*v][1], _vertexNormals[*v][2]);
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
    for (map<int, set<int>>::iterator jt = _adjacency.begin(); jt != _adjacency.end(); jt++) {
        int j = jt->first;
        for (map<int, set<int>>::iterator it = _adjacency.begin(); it != jt; it++) {
            int i = it->first;
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
float MeshObject::faceArea(const int& f) {
    vec3 p0 = _vertices[_faces[f][0]];
    vec3 p1 = _vertices[_faces[f][1]];
    vec3 p2 = _vertices[_faces[f][2]];
    vec3 e01 = p1 - p0; // edge 0->1 of face
    vec3 e02 = p2 - p0; //      0->2
    return cross(e01, e02).length();
}
vec3 MeshObject::faceNormal(const int& f) {
    vec3 p0 = _vertices[_faces[f][0]];
    vec3 p1 = _vertices[_faces[f][1]];
    vec3 p2 = _vertices[_faces[f][2]];
    vec3 e01 = p1 - p0; // edge 0->1 of face
    vec3 e02 = p2 - p0; //      0->2
    return normalize(cross(e01, e02));
}
void MeshObject::reComputeFaceNormals() {
    for (int f = 0; f < _faces.size(); f++) {
        vec3 p0 = _vertices[_faces[f][0]];
        vec3 p1 = _vertices[_faces[f][1]];
        vec3 p2 = _vertices[_faces[f][2]];
        vec3 e01 = p1 - p0; // edge 0->1 of face
        vec3 e02 = p2 - p0; //      0->2
        vec3 n = cross(e01, e02);
        _faceAreas[f] = n.length();
        _faceNormals[f] = normalize(n);
    }
    _faceNormalsReady = true;
}
void MeshObject::reComputeVertexNormals() {
    if (_faceNormalsReady == false) reComputeFaceNormals();
    for (map<int, set<int>>::const_iterator i = _adjacency.begin(); i != _adjacency.end(); i++){
        vec3 n(0, 0, 0);
        float nScale = 0;
        set<int> adjFaces = i->second; // adjacent faces
        for (set<int>::iterator j = adjFaces.begin(); j != adjFaces.end(); j++){
            n += _faceNormals[*j];
            nScale += _faceAreas[*j];
        }
        n = normalize(n / (float)adjFaces.size());
        nScale = sqrt(nScale / (float)adjFaces.size()) / 8;
        _vertexNormals[i->first] = n;
        _vertexNormalTailHeads[2 * (i->first) + 0] = _vertices[i->first];
        _vertexNormalTailHeads[2 * (i->first) + 1] = _vertices[i->first] + nScale*n;
        // RAINBOW COWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
        float thetaX = acos(dot(_vertexNormals[i->first], vec3(1, 0, 0)));
        float thetaY = acos(dot(_vertexNormals[i->first], vec3(0, 1, 0)));
        float thetaZ = acos(dot(_vertexNormals[i->first], vec3(0, 0, 1)));
        _vertexColors[i->first] = vec4(0.75*cos(thetaX / 2.0), 0.75*cos(thetaY / 2.0), 0.75*cos(thetaZ / 2.0), 1);
        _vertexNormalTailHeadColors[2 * (i->first) + 0] = _vertexColors[i->first];
        _vertexNormalTailHeadColors[2 * (i->first) + 1] = _vertexColors[i->first];
        if (_vertexColors[i->first] == vec4(0, 0, 0, 0)) printf("%f %f %f\n", _vertexNormals[i->first][0], _vertexNormals[i->first][1], _vertexNormals[i->first][2]);
    }
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

int MeshObject::nVisibleFaces() {
    int count = 0;
    for (int i = 0; i < _faces.size(); i++) {
        if (_triangleIndices[3 * i + 0] == _triangleIndices[3 * i + 1]) continue;
        if (_triangleIndices[3 * i + 1] == _triangleIndices[3 * i + 2]) continue;
        if (_triangleIndices[3 * i + 2] == _triangleIndices[3 * i + 0]) continue;
        count++;
    }
    return count;
}
vector<int> MeshObject::visibleFaces() {
    vector<int> visFaces;
    visFaces.reserve(_faces.size());
    for (int i = 0; i < _faces.size(); i++) {
        if (_triangleIndices[3 * i + 0] == _triangleIndices[3 * i + 1]) continue;
        if (_triangleIndices[3 * i + 1] == _triangleIndices[3 * i + 2]) continue;
        if (_triangleIndices[3 * i + 2] == _triangleIndices[3 * i + 0]) continue;
        visFaces.push_back(i);
    }
    return visFaces;
}

float MeshObject::avgEdgeLength() { // approximate cause i don't feel like dealing with the double counting at the borders
    vector<int> f = visibleFaces();
    int n = f.size();
    float dAvg = 0;
    for (int i = 0; i < n; i++) {
        vector<int> c = _faces[f[i]]; // face corner vertex indices
        float d = 0;
        for (int j = 0; j < 3; j++) d += glm::distance(_vertices[c[j]], _vertices[c[(j + 1) % 3]]);
        dAvg += d / (3 * n);
    }
    return dAvg;
}

void MeshObject::quadricSimplify() {
    if (_pairMetric.size() == 0) {
        if (_aggressiveSimplification == false) {
            printf("WARNING: No points satisfy distance threshold. Consider increasing it OR turn on \"agressive simplification\"\n");
            return;
        }
        setT(2 * avgEdgeLength());
        quadricSimplify();
    }
    float minCost = _metricPairs.begin()->first;
    set<Edge> minCostEdgeSet = _metricPairs.begin()->second;
    int rn = fmin(minCostEdgeSet.size() - 1, (float)minCostEdgeSet.size()*rand() / RAND_MAX);
    set<Edge>::iterator it = minCostEdgeSet.begin();
    for (int i = 0; i < rn; i++) it++;
    Edge minCostEdge = *it;
    if (minCost == INFINITY) return;
    else collapse(minCostEdge.first, minCostEdge.second, QUADRIC_APPROXIMATION_METHOD);
}

void MeshObject::updateIndexBuffer() { // NOT SAFE WITH REGARDS TO EVERYTHING ELSE
    vector<int> visFaceIndices = visibleFaces();
    _triangleIndices.clear();
    for (int i = 0; i < visFaceIndices.size(); i++){
        int fIndex = visFaceIndices[i];
        for (int j = 0; j < 3; j++){
            _triangleIndices.push_back(_faces[fIndex][j]);
        }
    }
}

void MeshObject::removeRedundancies() {
    MeshObject old = *this;
    _faces.clear();
    _triangleIndices.clear();
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
        _triangleIndices.push_back(c0);
        _triangleIndices.push_back(c1);
        _triangleIndices.push_back(c2);
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_triangleIndices.size(), &_triangleIndices[0], GL_STATIC_DRAW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, _triangleIndices.size(), GL_UNSIGNED_INT, 0);
    
    if (_drawVertexNormals == false) return;

    /////////////////////////////
    ///// ALSO DRAW NORMALS /////
    /////////////////////////////

    glVertexPointer(3, GL_FLOAT, 0, &_vertexNormalTailHeads[0]);
    glColorPointer(4, GL_FLOAT, 0, &_vertexNormalTailHeadColors[0]);
    
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*_lineIndices.size(), &_lineIndices[0], GL_STATIC_DRAW);

    glDrawElements(GL_LINES, _lineIndices.size(), GL_UNSIGNED_INT, 0);

    return;
}









vector<float> parseLine(const string& line) {
    vector<int> sp;
    sp.push_back(-1);
    int spNext = line.find(' ', sp.back() + 1);
    sp.push_back(spNext);
    while (spNext != string::npos) {
        spNext = line.find(' ', sp.back() + 1);
        sp.push_back(spNext);
    }
    if (sp.size() == 1) return vector<float>(0);
    vector<float> nums;
    for (int i = 0; i < sp.size() - 1; i++){
        string s = line.substr(sp[i]+1, sp[i+1]);
        nums.push_back(atof(s.c_str()));
    }
    return nums;
}
vector<float> ProgressiveMeshObject::readGeom() {
    printf("------------------------- READING .OFFPM FILE -------------------------\n");
    string file = _iFileName;
    string line;
    ifstream modelfile(_iFileName);
    if (!modelfile.is_open()) exit;
    getline(modelfile, line);
    if (line != "OFFPM") exit;
    getline(modelfile, line);
    vector<float> pl = parseLine(line);
    int nV_full = pl[0];
    int nF_full = pl[1];
    getline(modelfile, line);
    pl = parseLine(line);
    int nV = pl[0];
    int nF = pl[1];
    int nC = pl[2];
    int printStepV = ceil((float)nV / 100);
    int printStepF = ceil((float)nF / 100);
    int printStepC = ceil((float)nC / 100);
    _position = nC;
    _vertices.resize(nV_full);
    _vertexNormals.resize(nV_full);
    _vertexColors.resize(nV_full);
    //_vertexNormalTailHeads.resize(2 * nV_full);
    //_vertexNormalTailHeadColors.resize(2 * nV_full);
    //_lineIndices.resize(2*nV_full);
    _faces.resize(nF_full);
    _triangleIndices.resize(3 * nF_full);
    _faceNormals.resize(nF_full);
    _faceAreas.resize(nF_full);
    float xMin = 0;
    float xMax = 0;
    float yMin = 0;
    float yMax = 0;
    float zMin = 0;
    float zMax = 0;
    for (int i = 0; i < nV; i++){
        if (i%printStepV == 0 || i == nV - 1) printf("We're on face %i/%i\r", i + 1, nV);
        getline(modelfile, line);
        pl = parseLine(line);
        int v = pl[0];
        float x = pl[1];
        float y = pl[2];
        float z = pl[3];
        if (i == 0) { xMin = x; xMax = x; yMin = y; yMax = y; zMin = z; zMax = z; }
        else {
            if (x < xMin) xMin = x;
            if (y < yMin) yMin = y;
            if (z < zMin) zMin = z;
            if (x > xMax) xMax = x;
            if (y > yMax) yMax = y;
            if (z > zMax) zMax = z;
        }
        _vertices[i] = vec3(x, y, z);
        _vertexColors[i] = vec4(1, 1, 1, 1);
        _adjacency.emplace(i, set<int>());
    }
    std::cout << std::endl;
    for (int i = 0; i < nF; i++){
        if (i%printStepF == 0 || i == nF - 1) printf("We're on face %i/%i\r", i + 1, nF);
        getline(modelfile, line);
        pl = parseLine(line);
        int f = pl[0];
        int v0 = pl[1];
        int v1 = pl[2];
        int v2 = pl[3];
        _faces[i] = { v0, v1, v2 };
        _triangleIndices[3 * f + 0] = v0;
        _triangleIndices[3 * f + 1] = v1;
        _triangleIndices[3 * f + 2] = v2;
        _adjacency[v0].insert(f);
        _adjacency[v1].insert(f);
        _adjacency[v2].insert(f);
    }
    _v0.reserve(nC);
    _v1.reserve(nC);
    _xyz0.reserve(nC);
    _xyz1.reserve(nC);
    _xyz.reserve(nC);
    _fVec0.reserve(nC);
    _fVec1.reserve(nC);
    _fVecR.reserve(nC);
    _fVecRx.reserve(nC);
    _fVecRy.reserve(nC);
    _fVecRz.reserve(nC);
    vector<int> f;
    for (int i = 0; i < nC; i++){
        if (i%printStepC == 0 || i == nC - 1) printf("We're on collapse %i/%i\r", i + 1, nC);
        getline(modelfile, line); /////
        pl = parseLine(line);
        _v0.push_back(pl[0]);
        _xyz0.push_back(vec3(pl[1], pl[2], pl[3]));
        f.clear();
        for (int j = 4; j < pl.size(); j++) f.push_back(pl[j]);
        _fVec0.push_back(f);
        getline(modelfile, line); /////
        pl = parseLine(line);
        _v1.push_back(pl[0]);
        _xyz1.push_back(vec3(pl[1], pl[2], pl[3]));
        f.clear();
        for (int j = 4; j < pl.size(); j++) f.push_back(pl[j]);
        _fVec1.push_back(f);
        getline(modelfile, line); /////
        pl = parseLine(line);
        _v.push_back(pl[0]);
        _xyz.push_back(vec3(pl[1], pl[2], pl[3]));
        f.clear();
        for (int j = 4; j < pl.size(); j++) f.push_back(pl[j]);
        _fVec.push_back(f);
        getline(modelfile, line); /////
        pl = parseLine(line);
        f.clear();
        vector<int> f_x, f_y, f_z;
        for (int j = 0; j < pl.size(); j+=4){
            f.push_back(pl[j + 0]);
            f_x.push_back(pl[j + 0]);
            f_y.push_back(pl[j + 0]);
            f_z.push_back(pl[j + 0]);
        }
        _fVecR.push_back(f);
        _fVecRx.push_back(f_x);
        _fVecRy.push_back(f_y);
        _fVecRz.push_back(f_z);
    }
    _scale = vec3(xMax - xMin, yMax - yMin, zMax - zMin);
    _geomReady = true;
    return vector<float>({ xMin, xMax, yMin, yMax, zMin, zMax });
}


void ProgressiveMeshObject::collapseTo(const float& newPos) {
    int index = _position; // this corresponds to the current mesh "adjacency" (BEFORE carrying out collapse[index])
    int newIndex = newPos;
    float alpha = newPos - newIndex;
    if (_position == newPos) return;
    else if (_position < newPos) { // COLLAPSE
        for (int i = index; i < newIndex; i++) { // for each "full" collapse to get to newPos
            _vertices[_v[i]] = _xyz[i]; // update the coordinates of v=v0
            for (int j = 0; j < _fVec[i].size(); j++) { // for each face in the updated adjacency for v=v0
                int f = _fVec[i][j];
                for (int k = 0; k < 3; k++) {
                    if (_faces[f][k] == _v1[i]) _faces[f][k] = _v0[i];
                    if (_triangleIndices[3 * f + k] == _v1[i]) _triangleIndices[3 * f + k] = _v[i]; // change all corners from v1 to v=v0
                }
            }
            for (int j = 0; j < _fVecR[i].size(); j++) { // for each face that is shared between v0,v1
                int f = _fVecR[i][j];
                for (int k = 0; k < 3; k++) _triangleIndices[3 * f + k] = 0; // obliterate it from existence
            }
        }
        _vertices[_v0[newIndex]] = (1 - alpha)*_xyz0[newIndex] + alpha*_xyz[newIndex];
        _vertices[_v1[newIndex]] = (1 - alpha)*_xyz1[newIndex] + alpha*_xyz[newIndex];
    }
    else if (_position > newPos) { // SPLIT
        for (int i = index - 1; i > newIndex - 1; i++) {
            if (i == newIndex) {
                _vertices[_v0[i]] = (1 - alpha)*_xyz0[i] + alpha*_xyz[i];
                _vertices[_v1[i]] = (1 - alpha)*_xyz1[i] + alpha*_xyz[i];
            }
            else {
                _vertices[_v0[i]] = _xyz0[i];
                _vertices[_v1[i]] = _xyz1[i];
            }
            for (int j = 0; j < _fVecR[i].size(); j++) {
                int f = _fVecR[i][j];
                _faces[f][0] = _fVecRx[i][j];
                _faces[f][1] = _fVecRy[i][j];
                _faces[f][2] = _fVecRz[i][j];
                for (int k = 0; k < 3; k++)_triangleIndices[3 * f + k] = _faces[f][k];
            }
            for (int j = 0; j < _fVec1[i].size(); j++) {
                int f = _fVec1[i][j];
                if (_fVecRx[i][j] == _v0[i]) _faces[f][0] = _v1[i];
                else if (_fVecRy[i][j] == _v0[i]) _faces[f][1] = _v1[i];
                else _faces[f][2] = _v1[i];
                for (int k = 0; k < 3; k++)_triangleIndices[3 * f + k] = _faces[f][k];
            }
        }
    }
}