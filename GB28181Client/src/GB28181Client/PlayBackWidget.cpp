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

    // �����ڴ��һ֡yuvͼ�����ݣ����СΪ�ֱ��ʵ�1.5��
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

    // ˢ�½���,����paintGL�ӿ�
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

    // opengl��Ⱦ����������ɫ���������������
    // ��ɫ��������ʹ��openGL��ɫ����(OpenGL Shading Language, GLSL)��д��һ��С����,
    //       GLSL�ǹ�������OpenGL��ɫ��������,�����GLSL���Ե��﷨��Ҫ���߲����������
    // ��ʼ��������ɫ��������
    m_pVSHader = new QOpenGLShader(QOpenGLShader::Vertex, this);

    // ������ɫ��Դ��
    const char* vsrc = "attribute vec4 vertexIn; \
    attribute vec2 textureIn; \
    varying vec2 textureOut;  \
    void main(void)           \
    {                         \
        gl_Position = vertexIn; \
        textureOut = textureIn; \
    }";

    // ���붥����ɫ������
    bool bCompile = m_pVSHader->compileSourceCode(vsrc);
    if (!bCompile)
    {
    }

    // ��ʼ��Ƭ����ɫ������gpu��yuvת����rgb
    m_pFSHader = new QOpenGLShader(QOpenGLShader::Fragment, this);

    // Ƭ����ɫ��Դ��
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

    // ��glslԴ�����������������ɫ������
    bCompile = m_pFSHader->compileSourceCode(fsrc);
    if (!bCompile)
    {
    }

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1
    // ������ɫ����������
    m_pShaderProgram = new QOpenGLShaderProgram;
    // ��Ƭ����ɫ����ӵ���������
    m_pShaderProgram->addShader(m_pFSHader);
    // ��������ɫ����ӵ���������
    m_pShaderProgram->addShader(m_pVSHader);
    // ������vertexIn��ָ��λ��ATTRIB_VERTEX,�������ڶ�����ɫԴ������������
    m_pShaderProgram->bindAttributeLocation("vertexIn", ATTRIB_VERTEX);
    // ������textureIn��ָ��λ��ATTRIB_TEXTURE,�������ڶ�����ɫԴ������������
    m_pShaderProgram->bindAttributeLocation("textureIn", ATTRIB_TEXTURE);
    // ���������������뵽����ɫ������
    m_pShaderProgram->link();
    // ������������
    m_pShaderProgram->bind();

    // ��ȡ��ɫ���е����ݱ���tex_y, tex_u, tex_v��λ��,��Щ����������������
    // Ƭ����ɫ��Դ���п��Կ���
    textureUniformY = m_pShaderProgram->uniformLocation("tex_y");
    textureUniformU = m_pShaderProgram->uniformLocation("tex_u");
    textureUniformV = m_pShaderProgram->uniformLocation("tex_v");

    // �������
    static const GLfloat vertexVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         -1.0f, 1.0f,
         1.0f, 1.0f,
    };

    // �������
    static const GLfloat textureVertices[] = {
        0.0f,  1.0f,
        1.0f,  1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };

    // ��������ATTRIB_VERTEX�Ķ������ֵ�Լ���ʽ
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, vertexVertices);
    // ��������ATTRIB_TEXTURE���������ֵ�Լ���ʽ
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, textureVertices);
    // ����ATTRIB_VERTEX���Ե�����,Ĭ���ǹرյ�
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    // ����ATTRIB_TEXTURE���Ե�����,Ĭ���ǹرյ�
    glEnableVertexAttribArray(ATTRIB_TEXTURE);

    // �ֱ𴴽�y,u,v�������
    m_pTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureY->create();
    m_pTextureU->create();
    m_pTextureV->create();

    // ��ȡ����y��������������ֵ
    id_y = m_pTextureY->textureId();
    // ��ȡ����u��������������ֵ
    id_u = m_pTextureU->textureId();
    // ��ȡ����v��������������ֵ
    id_v = m_pTextureV->textureId();
    glClearColor(0.3, 0.3, 0.3, 0.0); // ���ñ���ɫ
//    qDebug("addr=%x id_y = %d id_u=%d id_v=%d\n", this, id_y, id_u, id_v);

    // ������ʱ��
    /*QTimer* ti = new QTimer(this);
    connect(ti, SIGNAL(timeout()), this, SLOT(PlayOneFrame()));
    ti->start(40);*/
}

void PlayBackWidget::resizeGL(int w, int h)
{
    if (h == 0) // ��ֹ�����
    {
        h = 1;  // ������Ϊ1
    }

    // �����ӿ�
    glViewport(0, 0, w, h);
}

void PlayBackWidget::paintGL()
{
    // ����y��������
    // ��������ԪGL_TEXTURE0
    glActiveTexture(GL_TEXTURE0);

    // ʹ������y������������
    glBindTexture(GL_TEXTURE_2D, id_y);

    // ʹ���ڴ���m_pBufYuv420p���ݴ���������y��������
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW, m_nVideoH, 0, GL_RED, GL_UNSIGNED_BYTE, m_pBufY/*m_pBufYuv420p*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // ����u��������
    glActiveTexture(GL_TEXTURE1); // ��������ԪGL_TEXTURE1
    glBindTexture(GL_TEXTURE_2D, id_u);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW / 2, m_nVideoH / 2, 0, GL_RED,
        GL_UNSIGNED_BYTE, m_pBufU/*(char*)m_pBufYuv420p + m_nVideoW * m_nVideoH*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // ����v��������
    glActiveTexture(GL_TEXTURE2); // ��������ԪGL_TEXTURE2
    glBindTexture(GL_TEXTURE_2D, id_v);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nVideoW / 2, m_nVideoH / 2, 0, GL_RED,
        GL_UNSIGNED_BYTE, m_pBufV/*(char*)m_pBufYuv420p + m_nVideoW * m_nVideoH * 5 / 4*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // ָ��y����Ҫʹ����ֵ ֻ����0,1,2�ȱ�ʾ����Ԫ������������opengl�����Ի��ĵط�
    // 0��Ӧ����ԪGL_TEXTURE0 1��Ӧ����ԪGL_TEXTURE1 2��Ӧ����ĵ�Ԫ
    glUniform1i(textureUniformY, 0);
    // ָ��u����Ҫʹ����ֵ
    glUniform1i(textureUniformU, 1);
    // ָ��v����Ҫʹ����ֵ
    glUniform1i(textureUniformV, 2);
    // ʹ�ö������鷽ʽ����ͼ��
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    return;
}
