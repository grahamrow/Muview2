#include <QtGui>
#include "glwidget.h"
#include <vector>

bool GLWidget::initializeLights()
{
    lightIntensity = QVector4D(1.0,1.0,1.0,1.0);
    lightPosition = QVector4D(10,10,150,1.0);
    lightAmbient = 0.4f;
    return true;
}

bool GLWidget::initializeShaders()
{
    bool result = true;

    result = result && flatShader.addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/shaders/standard.vert" );
    result = result && flatShader.addShaderFromSourceFile( QOpenGLShader::Fragment, ":/shaders/standard.frag" );

#ifdef __APPLE__
    result = result && diffuseShader.addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/shaders/standard.vert" );
    result = result && diffuseShader.addShaderFromSourceFile( QOpenGLShader::Fragment, ":/shaders/diffuseMacOSX.frag"  );
#else
    result = result && diffuseShader.addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/shaders/standard.vert" );
    result = result && diffuseShader.addShaderFromSourceFile( QOpenGLShader::Fragment, ":/shaders/diffuse.frag"  );
#endif

    if ( !result ) {
        qWarning() << "Shaders could not be loaded" << flatShader.log();
        qWarning() << "Shaders could not be loaded" << diffuseShader.log();
    }
    return result;
}

bool GLWidget::initializeCube()
{

    cube.vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

    glGenVertexArrays(1, &cube.vao);
    glBindVertexArray(cube.vao);
    cube.count = 6*3*2;

    // Generate the coordinates, normals
        GLfloat vertexNormData[] = {
            //  X     Y     Z      Normal
            // bottom
            -1.0f,-1.0f,-1.0f,     0.0f, -1.0f, 0.0f,
             1.0f,-1.0f,-1.0f,     0.0f, -1.0f, 0.0f,
            -1.0f,-1.0f, 1.0f,     0.0f, -1.0f, 0.0f,
             1.0f,-1.0f,-1.0f,     0.0f, -1.0f, 0.0f,
             1.0f,-1.0f, 1.0f,     0.0f, -1.0f, 0.0f,
            -1.0f,-1.0f, 1.0f,     0.0f, -1.0f, 0.0f,
            // top
            -1.0f, 1.0f,-1.0f,     0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 1.0f,     0.0f, 1.0f, 0.0f,
             1.0f, 1.0f,-1.0f,     0.0f, 1.0f, 0.0f,
             1.0f, 1.0f,-1.0f,     0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 1.0f,     0.0f, 1.0f, 0.0f,
             1.0f, 1.0f, 1.0f,     0.0f, 1.0f, 0.0f,
            // front
            -1.0f,-1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
             1.0f,-1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
             1.0f,-1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
             1.0f, 1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
            // back
            -1.0f,-1.0f,-1.0f,     0.0f, 0.0f, -1.0f,
            -1.0f, 1.0f,-1.0f,     0.0f, 0.0f, -1.0f,
             1.0f,-1.0f,-1.0f,     0.0f, 0.0f, -1.0f,
             1.0f,-1.0f,-1.0f,     0.0f, 0.0f, -1.0f,
            -1.0f, 1.0f,-1.0f,     0.0f, 0.0f, -1.0f,
             1.0f, 1.0f,-1.0f,     0.0f, 0.0f, -1.0f,
            // left
            -1.0f,-1.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f,-1.0f,     -1.0f, 0.0f, 0.0f,
            -1.0f,-1.0f,-1.0f,     -1.0f, 0.0f, 0.0f,
            -1.0f,-1.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
            -1.0f, 1.0f,-1.0f,     -1.0f, 0.0f, 0.0f,
            // right
            1.0f,-1.0f, 1.0f,     1.0f, 0.0f, 0.0f,
            1.0f,-1.0f,-1.0f,     1.0f, 0.0f, 0.0f,
            1.0f, 1.0f,-1.0f,     1.0f, 0.0f, 0.0f,
            1.0f,-1.0f, 1.0f,     1.0f, 0.0f, 0.0f,
            1.0f, 1.0f,-1.0f,     1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f,     1.0f, 0.0f, 0.0f
        };

    if (!cube.vbo.isCreated()) {
        cube.vbo.create();
    } else {
        cube.vbo.destroy();
        cube.vbo.create();
    }

    cube.vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !cube.vbo.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return false;
    }

    cube.vbo.allocate( vertexNormData, 6*6*3*2 * sizeof( float ) );

    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !flatShader.bind() )
    {
        qWarning() << "Could not bind shader program to context"  << flatShader.log();
        return false;
    }

    flatShader.setAttributeBuffer( "vertex", GL_FLOAT, 0, 3, 6*sizeof(GLfloat) );
    flatShader.enableAttributeArray( "vertex" );
    flatShader.setAttributeBuffer( "vertexNormal", GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat) );
    flatShader.enableAttributeArray( "vertexNormal" );

    glBindVertexArray(0);
    return true;
}

bool GLWidget::initializeCone(int slices, float radius, float height)
{
//    if (!diffuseShader.isLinked()) {
//        bool result = true;

//        if ( !result )
//            qWarning() << "Shaders could not be loaded" << diffuseShader.log();
//    }

    if (!cone.vbo.isCreated()) {
        cone.vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        cone.vbo.create();
    } else {
        cone.vbo.destroy();
        cone.vbo.create();
    }

    glGenVertexArrays(1, &cone.vao);
    glBindVertexArray(cone.vao);
    cone.count = 2*slices*3;

    float normScale = 1.0/sqrt(height*height + radius*radius);

    std::vector<GLfloat> vertices;
    // Top (Pointy part)
    for (int i = 0; i<slices; i++) {
        // Vertex
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(height);
        // Normal
        vertices.push_back( normScale*height*cos(2.0*PI*(i+0.5)/slices));
        vertices.push_back(-normScale*height*sin(2.0*PI*(i+0.5)/slices));
        vertices.push_back( radius*normScale );

        vertices.push_back( radius*cos(2.0*PI*(i+1)/slices));
        vertices.push_back(-radius*sin(2.0*PI*(i+1)/slices));
        vertices.push_back(0.0f);
        // Normal
        vertices.push_back( normScale*height*cos(2.0*PI*(i+0.5)/slices));
        vertices.push_back(-normScale*height*sin(2.0*PI*(i+0.5)/slices));
        vertices.push_back( radius*normScale );

        vertices.push_back( radius*cos(2.0*PI*i/slices));
        vertices.push_back(-radius*sin(2.0*PI*i/slices));
        vertices.push_back(0.0f);
        // Normal
        vertices.push_back( normScale*height*cos(2.0*PI*(i+0.5)/slices));
        vertices.push_back(-normScale*height*sin(2.0*PI*(i+0.5)/slices));
        vertices.push_back( radius*normScale );
    }

    // Bottom
    for (int i = 0; i<slices; i++) {
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);

        vertices.push_back( radius*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(0.0f);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);

        vertices.push_back( radius*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*sin(2.0*PI*(i)/slices));
        vertices.push_back(0.0f);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
    }

    cone.vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !cone.vbo.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return false;
    }

    cone.vbo.allocate( &vertices.front(), vertices.size() * sizeof(vertices[0]) );

    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !diffuseShader.bind() )
    {
        qWarning() << "Could not bind shader program to context";
        return false;
    }

    diffuseShader.setAttributeBuffer( "vertex", GL_FLOAT, 0, 3, 6*sizeof(GLfloat) );
    diffuseShader.enableAttributeArray( "vertex" );
    diffuseShader.setAttributeBuffer( "vertexNormal", GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat) );
    diffuseShader.enableAttributeArray( "vertexNormal" );

    glBindVertexArray(0);
    return true;
}

bool GLWidget::initializeVect(int slices, float radius, float height, float ratioTipToTail, float ratioInnerToOuter)
{
//    if (!diffuseShader.isLinked()) {
//        bool result = true;
//#ifdef __APPLE__
//        result = result && diffuseShader.addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/shaders/standard.vert" );
//        result = result && diffuseShader.addShaderFromSourceFile( QOpenGLShader::Fragment, ":/shaders/diffuseMacOSX.frag"  );
//#else
//        result = result && diffuseShader.addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/shaders/standard.vert" );
//        result = result && diffuseShader.addShaderFromSourceFile( QOpenGLShader::Fragment, ":/shaders/diffuse.frag"  );
//#endif
//        if ( !result )
//            qWarning() << "Shaders could not be loaded" << diffuseShader.log();
//    }

    if (!vect.vbo.isCreated()) {
        vect.vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        vect.vbo.create();
    } else {
        vect.vbo.destroy();
        vect.vbo.create();
    }

    glGenVertexArrays(1, &vect.vao);
    glBindVertexArray(vect.vao);

    float normScale = 1.0/sqrt(height*height + radius*radius);
    float offset = ratioTipToTail*height;

    std::vector<GLfloat> vertices;
    // Top (Pointy part)
    for (int i = 0; i<slices; i++) {
        // Vertex
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(height + offset);
        // Normal
        vertices.push_back( normScale*height*cos(2.0*PI*(i+0.5)/slices));
        vertices.push_back(-normScale*height*sin(2.0*PI*(i+0.5)/slices));
        vertices.push_back( radius*normScale );

        vertices.push_back( radius*cos(2.0*PI*(i+1)/slices));
        vertices.push_back(-radius*sin(2.0*PI*(i+1)/slices));
        vertices.push_back(offset);
        // Normal
        vertices.push_back( normScale*height*cos(2.0*PI*(i+0.5)/slices));
        vertices.push_back(-normScale*height*sin(2.0*PI*(i+0.5)/slices));
        vertices.push_back( radius*normScale );

        vertices.push_back( radius*cos(2.0*PI*i/slices));
        vertices.push_back(-radius*sin(2.0*PI*i/slices));
        vertices.push_back(offset);
        // Normal
        vertices.push_back( normScale*height*cos(2.0*PI*(i+0.5)/slices));
        vertices.push_back(-normScale*height*sin(2.0*PI*(i+0.5)/slices));
        vertices.push_back( radius*normScale );
    }
    // Bottom (Pointy part)
    for (int i = 0; i<slices; i++) {
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(offset);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);

        vertices.push_back( radius*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(offset);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);

        vertices.push_back( radius*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*sin(2.0*PI*(i)/slices));
        vertices.push_back(offset);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
    }
    // Bottom (tail)
    for (int i = 0; i<slices; i++) {
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);

        vertices.push_back( radius*ratioInnerToOuter*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*ratioInnerToOuter*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(0.0);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);

        vertices.push_back( radius*ratioInnerToOuter*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*ratioInnerToOuter*sin(2.0*PI*(i)/slices));
        vertices.push_back(0.0);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
    }
    // Sides (tail)
    for (int i = 0; i<slices; i++) {
        vertices.push_back( radius*ratioInnerToOuter*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*ratioInnerToOuter*sin(2.0*PI*(i)/slices));
        vertices.push_back(0.0);
        // Normal
        vertices.push_back( cos(2.0*PI*(i-0.5)/slices));
        vertices.push_back(-sin(2.0*PI*(i-0.5)/slices));
        vertices.push_back(0.0f);

        vertices.push_back( radius*ratioInnerToOuter*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*ratioInnerToOuter*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(0.0);
        // Normal
        vertices.push_back( cos(2.0*PI*(i-0.5)/slices));
        vertices.push_back(-sin(2.0*PI*(i-0.5)/slices));
        vertices.push_back(0.0f);

        vertices.push_back( radius*ratioInnerToOuter*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*ratioInnerToOuter*sin(2.0*PI*(i)/slices));
        vertices.push_back(offset);
        // Normal
        vertices.push_back( cos(2.0*PI*(i-0.5)/slices));
        vertices.push_back(-sin(2.0*PI*(i-0.5)/slices));
        vertices.push_back(0.0f);

        // =======================================
        vertices.push_back( radius*ratioInnerToOuter*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*ratioInnerToOuter*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(0.0);
        // Normal
        vertices.push_back( cos(2.0*PI*(i-0.5)/slices));
        vertices.push_back(-sin(2.0*PI*(i-0.5)/slices));
        vertices.push_back(0.0f);

        vertices.push_back( radius*ratioInnerToOuter*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*ratioInnerToOuter*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(offset);
        // Normal
        vertices.push_back( cos(2.0*PI*(i-0.5)/slices));
        vertices.push_back(-sin(2.0*PI*(i-0.5)/slices));
        vertices.push_back(0.0f);

        vertices.push_back( radius*ratioInnerToOuter*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*ratioInnerToOuter*sin(2.0*PI*(i)/slices));
        vertices.push_back(offset);
        // Normal
        vertices.push_back( cos(2.0*PI*(i-0.5)/slices));
        vertices.push_back(-sin(2.0*PI*(i-0.5)/slices));
        vertices.push_back(0.0f);
    }

    vect.vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !vect.vbo.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return false;
    }

    vect.vbo.allocate( &vertices.front(), vertices.size() * sizeof(vertices[0]) );
    vect.count = vertices.size();

    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !diffuseShader.bind() )
    {
        qWarning() << "Could not bind shader program to context" << diffuseShader.log();
        return false;
    }

    diffuseShader.setAttributeBuffer( "vertex", GL_FLOAT, 0, 3, 6*sizeof(GLfloat) );
    diffuseShader.enableAttributeArray( "vertex" );
    diffuseShader.setAttributeBuffer( "vertexNormal", GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat) );
    diffuseShader.enableAttributeArray( "vertexNormal" );

    glBindVertexArray(0);
    return true;
}

