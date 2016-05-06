/****************************************************************************
**
** Copyright (C) 2006-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CCDWindow_H
#define CCDWindow_H

#include <QPainter>
#include <QWidget>
#include <QSlider>
#include <QMainWindow>
#include <QFile>
#include "arthurwidgets.h"
#include "Frame.h"


QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QToolButton;
class QMenu;

QT_END_NAMESPACE


#define	XS	_PIXEL
#define	YS	_MAXLINES


#define AC_X	XS
#define	AC_Y	YS


#define	IDLE	1
#define	FLAT	2
#define	DARK	3
#define	CAPTURE	4


#define	DISPLAY_LIFE	0
#define	DISPLAY_DARK	1
#define	DISPLAY_FLAT	2
#define	DISPLAY_SUM1	3
#define	DISPLAY_SUM2	4

class CCDWindow;


class QBits : public QWidget
 {
     Q_OBJECT

 public:
 
			QBits(QWidget *parent = 0);
	void	UpdateBits(float *data, float min, float max);
	void	UpdateBits(float min, float max);
	void	Set(float *data);
	void	SetState(int a_state);
	void	HandleBits();

	void	do_dark();
	void	do_flat();
	void	do_capture();

	void	zero_sum();
	void	zero_dark();
	void	zero_flat();

	void	real_centroid(int max_x, int max_y, float *px, float *py);

	float	find_centroid(int hint_x, int hint_y, float *x, float *y);

	 QSize	sizeHint() const { return QSize(1000, 500); }
	void	msg(QString s);
	float	local(int x, int y);
	void	save_cur_frame();
	void	save_temp_frame();


 public slots:
	 void	setView(int v);
	 void	Stop();


 protected:
     void paintEvent(QPaintEvent *event);
	 void mousePressEvent(QMouseEvent *event);
	 void mouseMoveEvent(QMouseEvent *e);

 private:
	QImage		*image;
	float		*fdata;
	float		*dark;
	float		*flat;
	
	float		*sum_1;
	float		*sum_2;

	float		lmin;
	float		lmax;

	int			display;
	bool		stopped;

	int			dx,dy;

 public:;
	CCDWindow	*win;
	QPoint		cur_pos;
	bool		has_dark;
	bool		has_flat;
	int			state;
	int			flat_frame_left;
	int			flat_total_frames;
	int			dark_frame_left;
	int			dark_total_frames;
	int			frame_number;		//current frame number in capture mode
	int			frame_total;		//target frame count in capture mode

	float		star_x;
	float		star_y;

	float		global_cx;
	float		global_cy;


	float		hint_x_pos;
	float		hint_y_pos;
 };


class QFits
{

private:
	QFile			*f;
	QDataStream		*st;
	int				header_lines;

public:
			QFits(char *name);
			~QFits();
	void	AddKeyword(char *keyword, char *value);
	void	WriteHeader(int size_x, int size_y);
	void	WriteData(float *p, int l);

};


class CCDWindow : public QMainWindow
{
    Q_OBJECT

friend	QBits;

public:
    CCDWindow();

protected:
	bool eventFilter(QObject *obj, QEvent *e);

	private slots:
     void	MakeDark();
     void	MakeFlat();
     void	MakeCapture();
	 void	SetExposure(int);
	 void	SetGain(int);
	 void	SetiGain(int);
	 void	SetMin(int);
	 void	SetMax(int);
	 void	SetLevel(int);
	 void	SetZoom(int v);


private:
	void	createMenus();
	void	got_image();
	void	make_slider(QGroupBox *parent, QGroupBox **groupBox, QSlider **slider, char *title, const char *slot);


	QBits		*bits;
    QLabel		*equalLabel;
	QGroupBox	*exposureGroup;
	QGroupBox	*gainGroup;
	QGroupBox	*igainGroup;
	QGroupBox	*minGroup;
	QGroupBox	*maxGroup;
	QGroupBox	*levelGroup;
	QGroupBox	*zoomGroup;

	QSlider		*exposureSlider;
	QSlider		*gainSlider;
	QSlider		*igainSlider;
	QSlider		*minSlider;
	QSlider		*maxSlider;
	QSlider		*levelSlider;
	QSlider		*zoomSlider;

	TC253		*camera;

	float		vmin;
	float		vmax;
	float		vlevel;
	float		vzoom;
	float		quantum;

};

#endif
