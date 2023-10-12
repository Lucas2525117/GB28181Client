#include "PlayBackWidget.h"
#include <QPainter>
#include "XFFmpeg.h"

static void VideoYuvCB(const YuvFrame* yuv, void* userData)
{
	assert(userData);
    PlayBackWidget* wgt = (PlayBackWidget*)userData;
	wgt->Play(yuv);
}

PlayBackWidget::PlayBackWidget(QWidget *parent)
	: QOpenGLWidget(parent)
{
    textureUniformY = 0;
    textureUniformU = 0;
    textureUniformV = 0;
    id_y = 0;
    id_u = 0;
    id_v = 0;
    m_pBufYuv420p = nullptr;
    m_pVSHader = nullptr;
    m_pFSHader = nullptr;
    m_pShaderProgram = nullptr;
    m_pTextureY = nullptr;
    m_pTextureU = nullptr;
    m_pTextureV = nullptr;
    m_nVideoH = 0;
    m_nVideoW = 0;
}

PlayBackWidget::~PlayBackWidget()
{
    if (m_pBufY)
    {
        delete m_pBufY;
        m_pBufY = nullptr;
    }

    if (m_pBufU)
    {
        delete m_pBufU;
        m_pBufU = nullptr;
    }

    if (m_pBufV)
    {
        delete m_pBufV;
        m_pBufV = nullptr;
    }
}

void PlayBackWidget::Init()
{
	m_ffmpeg = new(std::nothrow) XFFmpeg(VideoYuvCB, this);
}

void PlayBackWidget::Play(const YuvFrame* yuv)
{
    m_nVideoW = yuv->w;
    m_nVideoH = yuv->h;

    // 申请内存存一帧yuv图像数据，其大小为分辨率的1.5倍
    int nLen = m_nVideoW * m_nVideoH * 3 / 2;
    //if (NULL == m_pBufYuv420p)
    if(nullptr == m_pBufY && nullptr == m_pBufU && nullptr == m_pBufV)
    {
        //m_pBufYuv420p = new unsigned char[nLen];
        m_pBufY = new unsigned char[nLen];
        m_pBufU = new unsigned char[nLen];
        m_pBufV = new unsigned char[nLen];
    }

    //m_pBufYuv420p = yuv->y;
    m_pBufY = yuv->y;
    m_pBufU = yuv->u;
    m_pBufV = yuv->v;

    // 刷新界面,触发paintGL接口
    update();
}

void PlayBackWidget::AddData(int avtype, void* data, int len)
{
	if (m_ffmpeg && data && len >= 0)
		m_ffmpeg->Input(avtype, data, len);
}

void PlayBackWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    // opengl渲染管线依赖着色器来处理传入的数据
    // 着色器：就是使用openGL着色语言(OpenGL Shading Language, GLSL)编写的一个小函数,
    //       GLSL是构成所有OpenGL着色器的语言,具体的GLSL语言的语法需要读者查找相关资料
    // 初始化顶点着色器、对象
    m_pVSHader = new QOpenGLShader(QOpenGLShader::Vertex, this);

    // 顶点着色器源码
    const char* vsrc = "attribute vec4 vertexIn; \
    attribute vec2 textureIn; \
    varying vec2 textureOut;  \
    void main(void)           \
    {                         \
        gl_Position = vertexIn; \
        textureOut = textureIn; \
    }";

    // 编译顶点着色器程序
    bool bCompile = m_pVSHader->compileSourceCode(vsrc);
    if (!bCompile)
    {
    }

    // 初始化片段着色器功能gpu中yuv转换成rgb
    m_pFSHader = new QOpenGLShader(QOpenGLShader::Fragment, this);

    // 片段着色器源码
    const char* fsrc = "varying vec2 textureOut; \
    uniform sampler2D tex_y; \
    uniform sampler2D tex_u; \
    uniform sampler2D tex_v; \
    void main(void) \
    { \
        vec3 yuv; \
        vec3 rgb; \
        yuv.x = texture2D(tex_y, textureOut).r; \
        yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
        yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
        rgb = mat3( 1,       1,         1, \
                    0,       -0.39465,  2.03211, \
                    1.13983, -0.58060,  0) * yuv; \
        gl_FragColor = vec4(rgb, 1); \
    }";

    // 将glsl源码送入编译器编译着色器程序
    bCompile = m_pFSHader->compileSourceCode(fsrc);
    if (!bCompile)
    {
    }

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1
    // 创建着色器程序容器
    m_pShaderProgram = new QOpenGLShaderProgram;
    // 将片段着色器添加到程序容器
    m_pShaderProgram->addShader(m_pFSHader);
    // 将顶点着色器添加到程序容器
    m_pShaderProgram->addShader(m_pVSHader);
    // 绑定属性vertexIn到指定位置ATTRIB_VERTEX,该属性在顶点着色源码其中有声明
    m_pShaderProgram->bindAttributeLocation("vertexIn", ATTRIB_VERTEX);
    // 绑定属性textureIn到指定位置ATTRIB_TEXTURE,该属性在顶点着色源码其中有声明
    m_pShaderProgram->bindAttributeLocation("textureIn", ATTRIB_TEXTURE);
    // 链接所有所有添入到的着色器程序
    m_pShaderProgram->link();
    // 激活所有链接
    m_pShaderProgram->bind();

    // 读取着色器中的数据变量tex_y, tex_u, tex_v的位置,这些变量的声明可以在
    // 片段着色器源码中可以看到
    textureUniformY = m_pShaderProgram->uniformLocation("tex_y");
    textureUniformU = m_pShaderProgram->uniformLocation("tex_u");
    textureUniformV = m_pShaderProgram->uniformLocation("tex_v");

    // 顶点矩阵
    static const GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         -1.0f, 1.0f,
         1.0f, 1.0f,
    };

    // 纹理矩阵
    static const GLfloat textureVertices[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };

    // 设置属性ATTRIB_VERTEX的顶点矩阵值以及格式
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    // 设置属性ATTRIB_TEXTURE的纹理矩阵值以及格式
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    // 启用ATTRIB_VERTEX属性的数据,默认是关闭的
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    // 启用ATTRIB_TEXTURE属性的数据,默认是关闭的
    glEnableVertexAttribArray(ATTRIB_TEXTURE);

    // 分别创建y,u,v纹理对象
    m_pTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureY->create();
    m_pTextureU->create();
    m_pTextureV->create();

    // 获取返回y分量的纹理索引值
    id_y = m_pTextureY->textureId();
    // 获取返回u分量的纹理索引值
    id_u = m_pTextureU->textureId();
    // 获取返回v分量的纹理索引值
    id_v = m_pTextureV->textureId();
    glClearColor(0.3, 0.3, 0.3, 0.0); // 设置背景色
//    qDebug("addr=%x id_y = %d id_u=%d id_v=%d\n", this, id_y, id_u, id_v);

    // 启动定时器
    /*QTimer* ti = new QTimer(this);
    connect(ti, SIGNAL(timeout()), this, SLOT(PlayOneFrame()));
    ti->start(40);*/
}

void PlayBackWidget::resizeGL(int w, int h)
{
    if (h == 0) // 防止被零除
    {
        h = 1;  // 将高设为1
    }

    // 设置视口
    glViewport(0, 0, w, h);
}

void PlayBackWidget::paintGL()
{
    // 加载y数据纹理
    // 激活纹理单元GL_TEXTURE0
    glActiveTexture(GL_TEXTURE0);

    // 使用来自y数据生成纹理
    glBindTexture(GL_TEXTURE_2D, id_y);

    // 使用内存中m_pBufYuv420p数据创建真正的y数据纹理
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW, m_nVideoH, 0, GL_RED, GL_UNSIGNED_BYTE, m_pBufY/*m_pBufYuv420p*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 加载u数据纹理
    glActiveTexture(GL_TEXTURE1); // 激活纹理单元GL_TEXTURE1
    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW / 2, m_nVideoH / 2, 0, GL_RED,
        GL_UNSIGNED_BYTE, m_pBufU/*(char*)m_pBufYuv420p + m_nVideoW * m_nVideoH*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 加载v数据纹理
    glActiveTexture(GL_TEXTURE2); // 激活纹理单元GL_TEXTURE2
    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW / 2, m_nVideoH / 2, 0, GL_RED,
        GL_UNSIGNED_BYTE, m_pBufV/*(char*)m_pBufYuv420p + m_nVideoW * m_nVideoH * 5 / 4*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 指定y纹理要使用新值 只能用0,1,2等表示纹理单元的索引，这是opengl不人性化的地方
    // 0对应纹理单元GL_TEXTURE0 1对应纹理单元GL_TEXTURE1 2对应纹理的单元
    glUniform1i(textureUniformY, 0);
    // 指定u纹理要使用新值
    glUniform1i(textureUniformU, 1);
    // 指定v纹理要使用新值
    glUniform1i(textureUniformV, 2);
    // 使用顶点数组方式绘制图形
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    return;
}
