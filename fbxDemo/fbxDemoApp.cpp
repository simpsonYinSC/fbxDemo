#include "fbxDemoApp.h"
#include "shader.h"
#include <fbxsdk.h>
// TODO: Write own math helper library
#include "vmath.h"
#include <vector>

fbxDemoApp::fbxDemoApp(void)
{
}


fbxDemoApp::~fbxDemoApp(void)
{
}

GLuint fbxDemoApp::compile_shaders(void)
{
  GLuint vertex_shader;
  GLuint fragment_shader;
  GLuint program;

  vertex_shader = loadShader("vs.glsl", GL_VERTEX_SHADER);
  fragment_shader = loadShader("fs.glsl", GL_FRAGMENT_SHADER);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  
  int result;
  int info_log_length = -1;
  glGetProgramiv(program, GL_LINK_STATUS, &result);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
  if(info_log_length > 0)
  {
    std::vector<char> errorMsg(info_log_length + 1);
    glGetProgramInfoLog(program, info_log_length, NULL, &errorMsg[0]);
    printf("%s\n", &errorMsg[0]);
  }
  return program;
}

void fbxDemoApp::loadFromFBX(const char *filename)
{
  FbxManager* lSdkManager = FbxManager::Create();
  FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager,IOSROOT);
  FbxImporter* lImporter = FbxImporter::Create(lSdkManager,"");
  if(!lImporter->Initialize(filename, -1, lSdkManager->GetIOSettings()))
  {
    printf("Call to FbxImporter::Initialize() failed.\n");
    printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
    exit(-1);
  }
  FbxScene* lScene = FbxScene::Create(lSdkManager,"myScene");
  lImporter->Import(lScene);
  lImporter->Destroy();

  FbxNode* lRootNode = lScene->GetRootNode();
  if(lRootNode)
  {
    traverseFBXNodes(lRootNode);

  }
  lSdkManager->Destroy();
}

void fbxDemoApp::traverseFBXNodes(FbxNode* node)
{
  const char* nodeName = node->GetName();
  printf("Da node name: %s\n", nodeName);
  // Get da transforms
  FbxDouble3 translation = node->LclTranslation.Get();
  FbxDouble3 rotation = node->LclRotation.Get();
  FbxDouble3 scaling = node->LclScaling.Get();

  // Determine # of children the node has
  int numChildren = node->GetChildCount();
  FbxNode* childNode = 0;
  for(int i = 0; i < numChildren; i++)
  {
    childNode = node->GetChild(i);
    FbxMesh* mesh = childNode->GetMesh();
    if(mesh != NULL)
    {
      // Get vertices from the mesh
      int numVertices = mesh->GetControlPointsCount();
      GLfloat* tempVerts = new GLfloat[numVertices*4];
      for(int j = 0; j < numVertices; j++)
      {
        FbxVector4 coord = mesh->GetControlPointAt(j);
        tempVerts[j*4 + 0] = (GLfloat)coord.mData[0];
        tempVerts[j*4 + 1] = (GLfloat)coord.mData[1];
        tempVerts[j*4 + 2] = (GLfloat)coord.mData[2];
        tempVerts[j*4 + 3] = 1.0f;
      }
      this->setGeometry(tempVerts, numVertices);

      // Get indices from the mesh
      int numIndices = mesh->GetPolygonVertexCount();
      int* indices = mesh->GetPolygonVertices();
      this->setIndexBuffer((GLuint*)indices, numIndices);

      FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();
      if(normalElement)
      {
        int numNormals = mesh->GetPolygonCount() * 3;
        GLfloat* tempNormals = new GLfloat[numNormals * 3];
        int vertexCounter = 0;
        for(int polyCounter = 0; polyCounter < mesh->GetPolygonCount(); polyCounter++)
        {
          for(int i = 0; i < 3; i++)
          {
            FbxVector4 normal = normalElement->GetDirectArray().GetAt(vertexCounter);
            tempNormals[vertexCounter*3+0] = (GLfloat)normal[0];
            tempNormals[vertexCounter*3+1] = (GLfloat)normal[1];
            tempNormals[vertexCounter*3+2] = (GLfloat)normal[2];
            vertexCounter++;
          }
        }
        this->setNormals(tempNormals, numNormals);
      }
    }
    this->traverseFBXNodes(childNode);
  } 
}

void fbxDemoApp::setGeometry(GLfloat* vertices, int num_vertices)
{
  this->vertex_data = vertices;
  this->num_vertices = num_vertices;
}

void fbxDemoApp::setIndexBuffer(GLuint* indices, int num_indices)
{
  this->vertex_indices = indices;
  this->num_indices = num_indices;
  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices, vertex_indices, GL_STATIC_DRAW);
}

void fbxDemoApp::setNormals(GLfloat* normals, int num_normals)
{
  this->vertex_normals = normals;
  this->num_normals = num_normals;
}

void fbxDemoApp::loadVBO()
{
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * num_vertices * 4, this->vertex_data, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * num_normals * 3, this->vertex_normals, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
}

void fbxDemoApp::startup(void)
{
  w = 800;
  h = 600;
  aspect = (float)w/(float)h;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vertex_vbo);
  glGenBuffers(1, &index_vbo);
  program = compile_shaders();

  this->loadFromFBX("file.fbx");
  this->loadVBO();

  //glEnable(GL_CULL_FACE);
  //glFrontFace(GL_CCW);
  
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  mv_location = glGetUniformLocation(program, "mv_matrix");
  proj_location = glGetUniformLocation(program, "proj_matrix");

  proj_matrix = vmath::perspective(50.0f, aspect, 0.1f, 1000.0f);
  rotationEnabled = false;
  translationEnabled = false;
  rot_x = rot_y = 0.0f;
  tran_x = tran_y = 0.0f;
  zoom = -1.0f;
  wireframe_mode = false;
}

void fbxDemoApp::shutdown(void)
{
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(program);
}
 
void fbxDemoApp::render(double currentTime)
{
  const GLfloat bgcolor[] = { 0.4f, 0.4f, 0.4f, 1.0f };
  static const GLfloat one = 1.0f;
  glViewport(0, 0, this->w, this->h);
  glClearBufferfv(GL_COLOR, 0, bgcolor);
  glClearBufferfv(GL_DEPTH, 0, &one);
  glUseProgram(program);
  glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);
  vmath::mat4 mv_matrix = vmath::translate(tran_x, tran_y, zoom);
  mv_matrix *= vmath::rotate(rot_x, 0.0f, 0.0f);
  mv_matrix *= vmath::rotate(0.0f, rot_y, 0.0f);
  
  glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
  if(wireframe_mode)
  {
    glDrawElements(GL_LINES, num_indices, GL_UNSIGNED_INT, (GLvoid*)0);
  }
  else
  {
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, (GLvoid*)0);
  }
}

void fbxDemoApp::onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if(key == GLFW_KEY_W && action == GLFW_RELEASE)
  {
    wireframe_mode = !wireframe_mode;
  }
}

void fbxDemoApp::onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
  if(button == GLFW_MOUSE_BUTTON_1)
  {
    if(action == GLFW_PRESS)
    {
      printf("Left mouse button clicked!\n");
      rotationEnabled = true;
      mouse_base_x = mouse_x;
      mouse_base_y = mouse_y;
    }
    else if(action == GLFW_RELEASE)
    {
      printf("Left mouse button released!\n");
      rotationEnabled = false;
    }
  }
  if(button == GLFW_MOUSE_BUTTON_MIDDLE)
  {
    if(action == GLFW_PRESS)
    {
      printf("Middle mouse button clicked!\n");
      translationEnabled = true;
      mouse_base_x = mouse_x;
      mouse_base_y = mouse_y;
    }
    else if (action == GLFW_RELEASE)
    {
      printf("Middle mouse button released!\n");
      translationEnabled = false;
    }
  }
}

void fbxDemoApp::onMouseMove(GLFWwindow* window, double x, double y)
{
  mouse_x = x;
  mouse_y = y;
  if(rotationEnabled)
  {
    rot_y += (float)(mouse_x - mouse_base_x)/1.0f;
    rot_x += (float)(mouse_y - mouse_base_y)/1.0f;
    mouse_base_x = mouse_x;
    mouse_base_y = mouse_y;
  }
  if(translationEnabled)
  {
    tran_x += -zoom * (float)(mouse_x - mouse_base_x)/(float)w;
    tran_y -= -zoom * (float)(mouse_y - mouse_base_y)/(float)h;
    mouse_base_x = mouse_x;
    mouse_base_y = mouse_y;
  }
}

void fbxDemoApp::onMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
{
  zoom += (float)yoffset * 0.1f;
}

void fbxDemoApp::onResize(GLFWwindow* window, int w, int h)
{
  this->w = w;
  this->h = h;
  aspect = (float)w/(float)h;
}

int main(int argc, const char** argv)
{
  fbxDemoApp* app = new fbxDemoApp();
  app->run(app);
  delete app;
  return 0;
}