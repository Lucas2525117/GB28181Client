#include <QtWidgets/qwidget.h>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <qtimer.h>
#include <mutex>
#include "public.h"
#include "VideoDecoder.h"

#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

class PlayBackWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    PlayBackWidget(QWidget *parent = nullptr);
	virtual ~PlayBackWidget();

	void Init();
	void Play(const YuvFrame* yuv);
	void AddData(int avtype, void* data, int len);

protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;

private:
    CVideoDecoder* m_videoDecoder = nullptr;

    /**
     * ������һ��2DͼƬ��������������������ϸ�ڣ���ͼ����������Ը��ֱ��κ�
     * ������ͬ��״�������ڡ�����ֱ����������ʾ��Ƶ֡
     */
    GLuint textureUniformY; // y��������λ��
    GLuint textureUniformU; // u��������λ��
    GLuint textureUniformV; // v��������λ��
    GLuint id_y; // y�������ID
    GLuint id_u; // u�������ID
    GLuint id_v; // v�������ID
    QOpenGLTexture* m_pTextureY;  // y�������
    QOpenGLTexture* m_pTextureU;  // u�������
    QOpenGLTexture* m_pTextureV;  // v�������
    /* ��ɫ��������GPU���л��� */
    QOpenGLShader* m_pVSHader;  // ������ɫ���������
    QOpenGLShader* m_pFSHader;  // Ƭ����ɫ������
    QOpenGLShaderProgram* m_pShaderProgram; // ��ɫ����������
    int m_nVideoW; // ��Ƶ�ֱ��ʿ�
    int m_nVideoH; // ��Ƶ�ֱ��ʸ�
    unsigned char* m_pBufYuv420p;
    unsigned char* m_pBufY = nullptr;
    unsigned char* m_pBufU = nullptr;
    unsigned char* m_pBufV = nullptr;
};

