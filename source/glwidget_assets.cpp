#include <QtGui>
#include <QDebug>
#include "glwidget.h"
#include <vector>

void GLWidget::initializeAssets()
{
    // Prepare a complete shader program...
    initializeShaders();
    initializeCube();
    initializeCone(16, 1.0, 2.0);
    initializeVect(16, 5.0f*vectorLength, vectorRadius, vectorTipLengthRatio, vectorShaftRadiusRatio);
    initializeLights();
}

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

    result = result && cubeShader.addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/shaders/cube.vert" );
    result = result && cubeShader.addShaderFromSourceFile( QOpenGLShader::Fragment, ":/shaders/cube.frag" );

    result = result && standardShader.addShaderFromSourceFile( QOpenGLShader::Vertex,   ":/shaders/standard.vert" );
    result = result && standardShader.addShaderFromSourceFile( QOpenGLShader::Fragment, ":/shaders/standard.frag"  );

    if ( !result ) {
        qWarning() << "Shaders could not be loaded (flat)"    << cubeShader.log();
        qWarning() << "Shaders could not be loaded (diffuse)" << standardShader.log();
    }
    return result;
}

bool GLWidget::initializeCube()
{
    cube.vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    cube.pos_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    cube.mag_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    
    cube.vao = new QOpenGLVertexArrayObject(this);
    cube.vao->create();
    cube.vao->bind();
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

    if (cube.vbo.isCreated()) {
        cube.vbo.destroy();
        cube.pos_vbo.destroy();
        cube.mag_vbo.destroy();
    }
    cube.vbo.create();
    cube.pos_vbo.create();
    cube.mag_vbo.create();
    
    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !cubeShader.bind() )
    {
        qWarning() << "Could not bind shader program to context (flat)"  << cubeShader.log();
        return false;
    }

    // Set usage. Vertices are static, positions and orientations are dynamic
    cube.vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    cube.pos_vbo.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    cube.mag_vbo.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    
    if ( !cube.vbo.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return false;
    }
    cube.vbo.allocate( vertexNormData, 6*6*3*2 * sizeof( float ) );
    cubeShader.setAttributeBuffer( "vertex", GL_FLOAT, 0, 3, 6*sizeof(GLfloat) );
    cubeShader.setAttributeBuffer( "vertexNormal", GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat) );
    cubeShader.enableAttributeArray( "vertex" );
    cubeShader.enableAttributeArray( "vertexNormal" );
    cube.vbo.release();

    if ( !cube.pos_vbo.bind() )
    {
        qWarning() << "Could not bind position buffer to the context";
        return false;
    }
    cube.pos_vbo.allocate( 1 * sizeof(QVector4D) );
    cubeShader.setAttributeBuffer( "translation", GL_FLOAT, 0, 4, 4*sizeof(GLfloat) );
    cubeShader.enableAttributeArray( "translation" );
    gl330Funcs->glVertexAttribDivisor(3, 1); // "translation" vbo
    cube.pos_vbo.release();

    if ( !cube.mag_vbo.bind() )
    {
        qWarning() << "Could not bind magnetization buffer to the context";
        return false;
    }
    cube.mag_vbo.allocate( 1 * sizeof(QVector4D) );
    cubeShader.setAttributeBuffer( "magnetization", GL_FLOAT, 0, 4, 4*sizeof(GLfloat) );
    cubeShader.enableAttributeArray( "magnetization" );
    gl330Funcs->glVertexAttribDivisor(2, 1); // "magnetization" vbo
    cube.mag_vbo.release();

    cube.vao->release();
    return true;
}

bool GLWidget::initializeCone(int slices, float radius, float height)
{

    if (!cone.vbo.isCreated()) {
        cone.vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        cone.pos_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        cone.mag_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        cone.vbo.create();
        cone.pos_vbo.create();
        cone.mag_vbo.create();

        cone.vao = new QOpenGLVertexArrayObject(this);
        cone.vao->create();
    } else {
        cone.vbo.destroy();
        cone.pos_vbo.destroy();
        cone.mag_vbo.destroy();
        cone.vbo.create();
        cone.pos_vbo.create();
        cone.mag_vbo.create();
        cone.vao->destroy();
        cone.vao->create();
    }

    cone.vao->bind();
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
        vertices.push_back( normScale*height*cos(2.0*PI*(i+1.0)/slices));
        vertices.push_back(-normScale*height*sin(2.0*PI*(i+1.0)/slices));
        vertices.push_back( radius*normScale );

        vertices.push_back( radius*cos(2.0*PI*i/slices));
        vertices.push_back(-radius*sin(2.0*PI*i/slices));
        vertices.push_back(0.0f);
        // Normal
        vertices.push_back( normScale*height*cos(2.0*PI*(i)/slices));
        vertices.push_back(-normScale*height*sin(2.0*PI*(i)/slices));
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

    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !standardShader.bind() )
    {
        qWarning() << "Could not bind shader program to context (cone)" << standardShader.log();
        return false;
    }

    // Set usage. Vertices are static, positions and orientations are dynamic
    cone.vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    cone.pos_vbo.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    cone.mag_vbo.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    
    if ( !cone.vbo.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return false;
    }
    cone.vbo.allocate( &vertices.front(), vertices.size() * sizeof(vertices[0]) );
    standardShader.setAttributeBuffer( "vertex", GL_FLOAT, 0, 3, 6*sizeof(GLfloat) );
    standardShader.enableAttributeArray( "vertex" );
    standardShader.setAttributeBuffer( "vertexNormal", GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat) );
    standardShader.enableAttributeArray( "vertexNormal" );
    cone.vbo.release();

    if ( !cone.pos_vbo.bind() )
    {
        qWarning() << "Could not bind position buffer to the context";
        return false;
    }
    cone.pos_vbo.allocate( 1 * sizeof(QVector4D) );
    standardShader.setAttributeBuffer( "translation", GL_FLOAT, 0, 4, 4*sizeof(GLfloat) );
    standardShader.enableAttributeArray( "translation" );
    gl330Funcs->glVertexAttribDivisor(3, 1); // "translation" vbo
    cone.pos_vbo.release();

    if ( !cone.mag_vbo.bind() )
    {
        qWarning() << "Could not bind magnetization buffer to the context";
        return false;
    }
    cone.mag_vbo.allocate( 1 * sizeof(QVector4D) );
    standardShader.setAttributeBuffer( "magnetization", GL_FLOAT, 0, 4, 4*sizeof(GLfloat) );
    standardShader.enableAttributeArray( "magnetization" );
    gl330Funcs->glVertexAttribDivisor(2, 1); // "magnetization" vbo
    cone.mag_vbo.release();

    cone.vao->release();
    standardShader.release();
    return true;
}

bool GLWidget::initializeVect(int slices, float height, float radius, float fractionTip, float fractionInner)
{
    if (!vect.vbo.isCreated()) {
        vect.vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        vect.pos_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        vect.mag_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        vect.vbo.create();
        vect.pos_vbo.create();
        vect.mag_vbo.create();

        vect.vao = new QOpenGLVertexArrayObject(this);
        vect.vao->create();
    } else {
        vect.vbo.destroy();
        vect.pos_vbo.destroy();
        vect.mag_vbo.destroy();
        vect.vbo.create();
        vect.pos_vbo.create();
        vect.mag_vbo.create();
        vect.vao->destroy();
        vect.vao->create();
    }

    vect.vao->bind();
    float normScale = 1.0/sqrt(height*height + radius*radius);
    float headOffset, tailOffset;
    if (vectorOrigin == "Tail") {
        tailOffset = 0.0f;
        headOffset = height;
    } else { // Center origin
        tailOffset = -height/2.0f;
        headOffset =  height/2.0f;
    }
    float centerOffset = tailOffset + height*(1.0f-fractionTip);

    float tipHeight = height*fractionTip;

    std::vector<GLfloat> vertices;
    // Top (Pointy part)
    for (int i = 0; i<slices; i++) {
        // Vertex
        vertices.push_back( 0.0f);
        vertices.push_back( 0.0f);
        vertices.push_back( headOffset);
        // Normal
        vertices.push_back( normScale*tipHeight*cos(2.0*PI*(i+0.5)/slices));
        vertices.push_back(-normScale*tipHeight*sin(2.0*PI*(i+0.5)/slices));
        vertices.push_back( radius*normScale );
        // Vertex
        vertices.push_back( radius*cos(2.0*PI*(i+1)/slices));
        vertices.push_back(-radius*sin(2.0*PI*(i+1)/slices));
        vertices.push_back( centerOffset);
        // Normal
        vertices.push_back( normScale*tipHeight*cos(2.0*PI*(i+1)/slices));
        vertices.push_back(-normScale*tipHeight*sin(2.0*PI*(i+1)/slices));
        vertices.push_back( radius*normScale );
        // Vertex
        vertices.push_back( radius*cos(2.0*PI*i/slices));
        vertices.push_back(-radius*sin(2.0*PI*i/slices));
        vertices.push_back( centerOffset);
        // Normal
        vertices.push_back( normScale*tipHeight*cos(2.0*PI*(i)/slices));
        vertices.push_back(-normScale*tipHeight*sin(2.0*PI*(i)/slices));
        vertices.push_back( radius*normScale );
    }
    // Bottom (Pointy part)
    for (int i = 0; i<slices; i++) {
        // Vertex
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(centerOffset);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        // Vertex
        vertices.push_back( radius*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(centerOffset);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        // Vertex
        vertices.push_back( radius*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*sin(2.0*PI*(i)/slices));
        vertices.push_back(centerOffset);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
    }
    // Bottom (tail)
    for (int i = 0; i<slices; i++) {
        // Vertex
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(tailOffset);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        // Vertex
        vertices.push_back( radius*fractionInner*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*fractionInner*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(tailOffset);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        // Vertex
        vertices.push_back( radius*fractionInner*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*fractionInner*sin(2.0*PI*(i)/slices));
        vertices.push_back(tailOffset);
        // Normal
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
    }
    // Sides (tail)
    for (int i = 0; i<slices; i++) {
        // Vertex
        vertices.push_back( radius*fractionInner*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*fractionInner*sin(2.0*PI*(i)/slices));
        vertices.push_back(tailOffset);
        // Normal
        vertices.push_back( cos(2.0*PI*(i)/slices));
        vertices.push_back(-sin(2.0*PI*(i)/slices));
        vertices.push_back(0.0f);
        // Vertex
        vertices.push_back( radius*fractionInner*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*fractionInner*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(tailOffset);
        // Normal
        vertices.push_back( cos(2.0*PI*(i-1.0)/slices));
        vertices.push_back(-sin(2.0*PI*(i-1.0)/slices));
        vertices.push_back(0.0f);
        // Vertex
        vertices.push_back( radius*fractionInner*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*fractionInner*sin(2.0*PI*(i)/slices));
        vertices.push_back(centerOffset);
        // Normal
        vertices.push_back( cos(2.0*PI*(i)/slices));
        vertices.push_back(-sin(2.0*PI*(i)/slices));
        vertices.push_back(0.0f);

        // =======================================
        // Vertex
        vertices.push_back( radius*fractionInner*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*fractionInner*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(tailOffset);
        // Normal
        vertices.push_back( cos(2.0*PI*(i-1.0)/slices));
        vertices.push_back(-sin(2.0*PI*(i-1.0)/slices));
        vertices.push_back(0.0f);
        // Vertex
        vertices.push_back( radius*fractionInner*cos(2.0*PI*(i-1)/slices));
        vertices.push_back(-radius*fractionInner*sin(2.0*PI*(i-1)/slices));
        vertices.push_back(centerOffset);
        // Normal
        vertices.push_back( cos(2.0*PI*(i-1.0)/slices));
        vertices.push_back(-sin(2.0*PI*(i-1.0)/slices));
        vertices.push_back(0.0f);
        // Vertex
        vertices.push_back( radius*fractionInner*cos(2.0*PI*(i)/slices));
        vertices.push_back(-radius*fractionInner*sin(2.0*PI*(i)/slices));
        vertices.push_back(centerOffset);
        // Normal
        vertices.push_back( cos(2.0*PI*(i)/slices));
        vertices.push_back(-sin(2.0*PI*(i)/slices));
        vertices.push_back(0.0f);
    }

    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !standardShader.bind() )
    {
        qWarning() << "Could not bind shader program to context (vect)" << standardShader.log();
        return false;
    }

    // Set usage. Vertices are static, positions and orientations are dynamic
    vect.vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    vect.pos_vbo.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    vect.mag_vbo.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    
    if ( !vect.vbo.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return false;
    }
    vect.vbo.allocate( &vertices.front(), vertices.size() * sizeof(vertices[0]) );
    vect.count = vertices.size();
    standardShader.setAttributeBuffer( "vertex", GL_FLOAT, 0, 3, 6*sizeof(GLfloat) );
    standardShader.enableAttributeArray( "vertex" );
    standardShader.setAttributeBuffer( "vertexNormal", GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat) );
    standardShader.enableAttributeArray( "vertexNormal" );
    vect.vbo.release();

    if ( !vect.pos_vbo.bind() )
    {
        qWarning() << "Could not bind position buffer to the context";
        return false;
    }
    vect.pos_vbo.allocate( 1 * sizeof(QVector4D) );
    standardShader.setAttributeBuffer( "translation", GL_FLOAT, 0, 4, 4*sizeof(GLfloat) );
    standardShader.enableAttributeArray( "translation" );
    gl330Funcs->glVertexAttribDivisor(3, 1); // "translation" vbo
    vect.pos_vbo.release();

    if ( !vect.mag_vbo.bind() )
    {
        qWarning() << "Could not bind magnetization buffer to the context";
        return false;
    }
    vect.mag_vbo.allocate( 1 * sizeof(QVector4D) );
    standardShader.setAttributeBuffer( "magnetization", GL_FLOAT, 0, 4, 4*sizeof(GLfloat) );
    standardShader.enableAttributeArray( "magnetization" );
    gl330Funcs->glVertexAttribDivisor(2, 1); // "magnetization" vbo
    vect.mag_vbo.release();

    vect.vao->release();
    standardShader.release();
    return true;
    // vect.vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    // if ( !vect.vbo.bind() )
    // {
    //     qWarning() << "Could not bind vertex buffer to the context";
    //     return false;
    // }

    // vect.vbo.allocate( &vertices.front(), vertices.size() * sizeof(vertices[0]) );
    // vect.count = vertices.size();

    // // Bind the shader program so that we can associate variables from
    // // our application to the shaders
    // if ( !standardShader.bind() )
    // {
    //     qWarning() << "Could not bind shader program to context (vector)" << standardShader.log();
    //     return false;
    // }

    // standardShader.setAttributeBuffer( "vertex", GL_FLOAT, 0, 3, 6*sizeof(GLfloat) );
    // standardShader.enableAttributeArray( "vertex" );
    // standardShader.setAttributeBuffer( "vertexNormal", GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat) );
    // standardShader.enableAttributeArray( "vertexNormal" );

    // vect.vao->release();
    // standardShader.release();
    // return true;
}

