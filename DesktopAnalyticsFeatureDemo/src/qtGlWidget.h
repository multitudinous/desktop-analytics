#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>

#include <QGLBuffer>
#include <QGLShaderProgram>

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    GLWidget( const QGLFormat& format, QWidget* parent = 0 );
	GLWidget( QWidget* parent =0);
	public slots:
		void toggleNode();
		
		
protected:
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
	virtual void rayEvent(float x, float y);
    virtual void paintGL();
	
    virtual void keyPressEvent( QKeyEvent* e );
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	bool _leftDrag, _rightDrag, _midDrag;
	float _lastNX, _lastNY;
	int _lastX, _lastY;

};

#endif // GLWIDGET_H
