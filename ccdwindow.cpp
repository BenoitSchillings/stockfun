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

#include <QtGui>

#include "CCDWindow.h"
#include <xmmintrin.h>

//----------------------------------------------------------------------------

#define	ZOOM_T		1.8


int min(int x, int y)
{
	if (x < y) return x;
	return y;
}

//----------------------------------------------------------------------------

QFits::QFits(char *name)
{
	f = new QFile(name);
	
	f->open(QIODevice::WriteOnly);


	header_lines = 0;
}

//----------------------------------------------------------------------------

QFits::~QFits()
{
	f->close();

	delete f;
}

//----------------------------------------------------------------------------

void	QFits::AddKeyword(char *keyword, char *value)
{
	char	line[80];
	int		i;
	int		max;

	for (i = 0; i < 80; i++) line[i] = ' ';

	for (i =0; i < min(8,strlen(keyword)); i++)
		line[i] = keyword[i];

	if (strlen(value) != 0) {
		line[9] = '=';

		for (i = 0; i < min(30, strlen(value)); i++)
			line[15+i] = value[i];
	}

	QDataStream	st(f);

	st.writeRawData((char*)line, 80);
	header_lines++;
}


//----------------------------------------------------------------------------

void	QFits::WriteHeader(int size_x, int size_y)
{
	char	buf[32];


	AddKeyword("SIMPLE"  ,"T");
	AddKeyword("BITPIX"  ,"-32");
	AddKeyword("NAXIS"   ,"2");

	sprintf(buf, "%ld", size_x);
	AddKeyword("NAXIS1"  ,buf);

	sprintf(buf, "%ld", size_y);
	AddKeyword("NAXIS2"  ,buf);

	AddKeyword("SOURCE"   ,"Benoit lucky");

	AddKeyword("END", "");
	while(header_lines != 36)
		AddKeyword("","");
}

//----------------------------------------------------------------------------

void	QFits::WriteData(float *p, int l)
{
	ulong	v;
	ulong	v1;

	QDataStream	st(f);

	while(l--) {
		v = *((long*)p);
		p++;

		v1 = 0;

		v1 <<= 8;
		v1 |= (v&0xff);
		v >>= 8;
		v1 <<= 8;
		v1 |= (v&0xff);
		v >>= 8;
		v1 <<= 8;
		v1 |= (v&0xff);
		v >>= 8;
		v1 <<= 8;
		v1 |= (v&0xff);


		st.writeRawData((char*)&v1, 4);
	}
}

//----------------------------------------------------------------------------

bool	TryToLoad(float *p, QString name)
{
	QFile	f(name);
	uchar	junk;


	if (f.exists()) {
		f.open(QIODevice::ReadOnly);

		QDataStream	st(&f);

		for (int i = 0; i < 0xb40; i++)			//skip header for now
			st.readRawData((char*)&junk, 1);

		int		l;

		l = XS*YS;


		while(l--) {
			ulong	v, v1;

			st.readRawData((char*)&v, 4);
			v1 = 0;

			v1 <<= 8;
			v1 |= (v&0xff);
			v >>= 8;
			v1 <<= 8;
			v1 |= (v&0xff);
			v >>= 8;
			v1 <<= 8;
			v1 |= (v&0xff);
			v >>= 8;
			v1 <<= 8;
			v1 |= (v&0xff);

			*((long*)p) = v1;
			p++;
		}
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------


QBits::QBits(QWidget *parent)
     : QWidget(parent)
{
	int		x,y;
 

	image = new QImage(XS, YS, QImage::Format_ARGB32);
	fdata = new float[XS*(YS+4)]; //padded for edge effects. 2 lines +-
	fdata += XS*2;

	dark = new float[XS*YS];
	flat = new float[XS*YS];
	sum_1 = new float[XS*YS];
	sum_2 = new float[XS*YS];

	global_cx = 100;
	global_cy = 100;

	dx = 0;
	dy = 0;

	stopped = FALSE;

	for (y = 0; y < YS; y++)
		for (x = 0; x < XS; x++) {
			int	d;

			d = y*XS + x;

			fdata[d] = 1000;
			dark[d] = 0.0;
			flat[d] = 1.0;
			sum_1[d] = 0;
			sum_2[d] = 0;
		}




	display = DISPLAY_LIFE;

	star_x = -100;
	star_y = -100;
	has_dark = FALSE;
	has_flat = FALSE;

	has_dark = TryToLoad(dark, "dark.fit");
	has_flat = TryToLoad(flat, "flat.fit");

	SetState(IDLE);
}
	
//----------------------------------------------------------------------------

void QBits::SetState(int a_state)
{
	state = a_state;
}

//----------------------------------------------------------------------------

void QBits::zero_sum()
{
	int	x,y;


	for (y =0; y < AC_Y; y++)
		for (x = 0; x < AC_X; x++) {
			sum_1[y*AC_X+x] = 0.0;
		}
}

//----------------------------------------------------------------------------

void QBits::zero_flat()
{
	int	x,y;


	for (y =0; y < YS; y++)
		for (x = 0; x < XS; x++) {
			flat[y*XS+x] = 1.0;
		}
}

 
//----------------------------------------------------------------------------

void QBits::zero_dark()
{
	int	x,y;


	for (y =0; y < YS; y++)
		for (x = 0; x < XS; x++) {
			dark[y*XS+x] = 0.0;
		}
}

 
//----------------------------------------------------------------------------

int	time_d;
int time_d1;

//----------------------------------------------------------------------------

void	QBits::do_flat()
{
	int	x,y;

	QString	s; s.sprintf("flat left=%ld\n", flat_frame_left);msg(s);


	for (y =0; y < YS; y++) {
		for (x = 0; x < XS; x++) {
			flat[y*XS+x] += fdata[y*XS+x];
		}
	}

	flat_frame_left--;

	if (flat_frame_left == 0) {
		float avg = 0;

		for (y = 0; y < YS; y++)
			for (x = 0; x < XS; x++)
				avg += flat[y*XS+x];

		avg /= (XS*YS);

		for (y =0; y < YS; y++) {
			for (x = 0; x < XS; x++) {
				flat[y*XS+x] /= avg;
				flat[y*XS+x] = 1.0/flat[y*XS+x];
			}
		}
		
		QFits *f = new QFits("flat.fit");

		f->WriteHeader(XS, YS);
		f->WriteData(flat, XS*YS);

		delete f;

		state = IDLE;
		has_flat = TRUE;
		
	}
}

//----------------------------------------------------------------------------
//#define	SSE2


void	QBits::do_dark()
{
	int		x,y;
	QTime	t;

	QString	s; s.sprintf("dark left=%ld\n", dark_frame_left);msg(s);

	t.start();

#ifdef SSE2
	for (y =0; y < YS; y++) {
		 float	 *pdark;
		 float	 *pdata;

		pdark = dark + (y * XS);
		pdata = fdata + (y * XS);

		for (x = 0; x < XS; x+=4) {

			register __m128 v1 = _mm_load_ps(pdark);
			register __m128 v2 = _mm_load_ps(pdata);
			register __m128 v3 = _mm_add_ps(v1, v2);
			_mm_store_ps(pdark, v3);
			
			pdark += 4;
			pdata += 4;
		}
	}

#else
	for (y =0; y < YS; y++) {
		for (x = 0; x < XS; x++) {
			dark[y*XS+x] += fdata[y*XS+x];
		}
	}
#endif

	time_d = t.elapsed();

	dark_frame_left--;

	if (dark_frame_left == 0) {
		for (y =0; y < YS; y++) {
			for (x = 0; x < XS; x++) {
				dark[y*XS+x] /= (dark_total_frames*1.0);
				//dark[y*XS+x] = 2.0;
			}
		}

		QFits *f = new QFits("dark.fit");

		f->WriteHeader(XS, YS);
		f->WriteData(dark, XS*YS);

		delete f;


		state = IDLE;
		has_dark = TRUE;
	}
}

//----------------------------------------------------------------------------

#define	d(x_,y_) fdata[(y_) * XS + x_]


void QBits::real_centroid(int max_x, int max_y, float *px, float *py)
{
	int	DBOX = 5;	//10 by 10 box
	int	x, y;

	float	sum_x;
	float	sum_y;
	float	sum;


	sum_x = 0;
	sum_y = 0;
	sum = 0;

	for (y = (max_y - DBOX); y <= (max_y + DBOX); y++) {
		for (x = (max_x - DBOX); x <= (max_x + DBOX); x++) {
			sum_x += (x*d(x,y));
			sum_y += (y*d(x,y));
			sum += d(x,y);
		}
	}
	sum_x /= sum;
	sum_y /= sum;

	*px = sum_x;
	*py = sum_y;
}

//----------------------------------------------------------------------------
	
	float	QBits::local(int x, int y)
{
	float	sum;
	int		dx,dy;
	int		dbox = 3;

	sum = 0;

	for (dy = -dbox; dy <= dbox; dy++)
		for (dx = -dbox; dx <= dbox; dx++)
			sum += d(x+dx,y+dy);
/*
	sum = (5.0 * d(x,y)) + 2*d(x+1,y) + 2*d(x-1,y) + 2*d(x,y+1) + 2*d(x,y-1);
	sum += d(x+1,y+1) + d(x-1,y+1) + d(x-1, y-1) + d(x+1, y-1);
	sum += (0.5) * (d(x+2, y) +  d(x-2, y) +  d(x, y+2) +  d(x, y-2));
*/

	sum *= (1.0/(dbox*dbox));
	return sum;
}

//----------------------------------------------------------------------------

float	QBits::find_centroid(int hint_x, int hint_y, float *cx, float *cy)
{
	int			x,y;
	int			MAX_SEARCH = 25;
	int			max_x;
	int			max_y;
	float		max_v;



	if (hint_x < 0 || hint_y < 0) {
		*cx = 0;
		*cy = 0;
		return -1;
	}

	MAX_SEARCH++;
	if (hint_x < MAX_SEARCH)
		hint_x = MAX_SEARCH;
	if (hint_y < MAX_SEARCH)
		hint_y = MAX_SEARCH;


	if (hint_x > (XS-MAX_SEARCH))
		hint_x = YS-MAX_SEARCH;

	if (hint_y > (YS-MAX_SEARCH))
		hint_y = YS-MAX_SEARCH;
	MAX_SEARCH--;

//search for local maximum within box

	max_v = 0;

	for (y = (hint_y - MAX_SEARCH); y <= (hint_y + MAX_SEARCH); y++) {
		for (x = (hint_x - MAX_SEARCH); x <= (hint_x + MAX_SEARCH); x++) {
			if (local(x,y) > max_v) {
				max_x = x;
				max_y = y;
				max_v = local(x,y);
			}
		}
	}

	

	*cx = max_x;
	*cy = max_y;

	float	px,py;

	real_centroid(max_x, max_y, &px, &py);

	global_cx = (px+0.5);
	global_cy = (py+0.5);

	*cx = px;
	*cy = py;

	return max_v;
}


//----------------------------------------------------------------------------

void	QBits::msg(QString s)
{
	win->statusBar()->showMessage(s);
}


//----------------------------------------------------------------------------

void	QBits::Stop()
{
	msg("stopped");
	stopped = TRUE;
}

//----------------------------------------------------------------------------

void	QBits::save_cur_frame()
{
	QFits	*f;
	QTime	t;
	char	buf[80];

	t = QTime::currentTime();
	sprintf(buf, "zqfile%ld%ld.fit", t.minute(), t.second());
	
	f = new QFits(buf);

	f->WriteHeader(XS, YS);
	f->WriteData(sum_1, XS*YS);

	delete f;
}

//----------------------------------------------------------------------------

void	QBits::save_temp_frame()
{
	QFits	*f;
	QTime	t;
	char	buf[80];

	t = QTime::currentTime();
	sprintf(buf, "temp.fit");
	
	f = new QFits(buf);

	f->WriteHeader(XS, YS);
	f->WriteData(sum_1, XS*YS);

	delete f;
}

//----------------------------------------------------------------------------


/*
sum = sum + x
1 -> sum = sum + x
2 -> sum = (sum*0.5) + (x*0.5)
3 -> sum = (sum*0.6666) + (x*0.33333)

gen -> sum = (sum(1.0-(1.0/frame)) + (data/frame);
*/

void	QBits::do_capture()
{
	int		x,y;
	int		v;
	int		v1;

	float	k1, k2;
	float	cx, cy;
	int		dx, dy;
	int		x1, y1;
	char	filter;
	float	quantum;
	float	max_v = 1e9;

	filter = FALSE;

	quantum = win->quantum;

	if (quantum > 10) filter = TRUE;

	if (star_x > 0) {
		float	max_v = find_centroid(hint_x_pos, hint_y_pos, &cx, &cy);

		QString	s; s.sprintf("Capture=%ld, cx=%f, cy=%f, vmax=%f\n", frame_number, cx, cy, max_v);msg(s);

		if (max_v < win->vlevel)
			return;

		hint_x_pos = cx;
		hint_y_pos = cy;

		dx = star_x - (cx+0.5);
		dy = star_y - (cy+0.5);
	}
	else {
		dx = 0;
		dy = 0;
		QString	s; s.sprintf("Capture_blind=%ld", frame_number);msg(s);
	}


	frame_number++;



	k2 = 1.0/frame_number;
	k1 = 1.0 - k2;

	


	for (y =0; y < YS; y++) {
		v = y * XS;
		y1 = y - dy;

		if (y1 < 0) goto skip1;

		if (y1 >= YS) goto skip1;

		x1 = -dx;
		v1 = y1 * XS + x1;

		for (x = 0; x < XS; x++) {

			if (x1 < 0) goto skip;
			if (x1 >= XS) goto skip1;

			if (filter) {
				if ((fdata[v1]>quantum) &&
					(fdata[v1]>=fdata[v1+1]) &&
					(fdata[v1]>=fdata[v1-1]) &&
					(fdata[v1]>=fdata[v1+XS]) &&
					(fdata[v1]>=fdata[v1-XS]) &&
					(fdata[v1]>=fdata[v1+1+XS]) &&
					(fdata[v1]>=fdata[v1-1+XS]) &&
					(fdata[v1]>=fdata[v1+1-XS]) &&
					(fdata[v1]>=fdata[v1-1-XS]) 
					
					
					)
				
				{
					sum_1[v] = sum_1[v] + 6 ;
				}

			}
			else {
				//if (fdata[v1]> 100)
				sum_1[v] = sum_1[v] * k1 + fdata[v1] * k2;
			}
skip:;
			v++;
			v1++;  
			x1++;
		}
skip1:;
	}


	if (frame_number % 5000 == 0) { 
		save_temp_frame();
	}

	if (frame_number == frame_total) {
		save_cur_frame();
		state = IDLE;
	}
}

//----------------------------------------------------------------------------

void QBits::HandleBits()
{
	
	if (stopped) {
		if (state == CAPTURE)
			save_cur_frame();
		stopped = FALSE;
		state = IDLE;
	}

	switch (state) {
		case IDLE:		break;
		case FLAT:
			do_flat();
			break;
		case DARK:		
			do_dark();
			break;
		case CAPTURE:
			do_capture();
			break;
	}
}

//----------------------------------------------------------------------------


void QBits::Set(float *data)
{
	int		x,y;
	int		v;


	if (has_dark && has_flat) {
		for (y =0; y < YS; y++) {
			v = y * XS;
			for (x = 0; x < XS; x+=2) {
				fdata[v] = (data[v] - dark[v]) * flat[v];
				v++;
				fdata[v] = (data[v] - dark[v]) * flat[v];
				v++;
			}
		}
	}
	else
	if (has_dark) {
		for (y =0; y < YS; y++) {
			v = y * XS;
			for (x = 0; x < XS; x+=2) {
				fdata[v] = data[v] - dark[v];
				v++;
				fdata[v] = data[v] - dark[v];
				v++;
			}
		}
	}
	else
		memcpy(fdata, data, sizeof(float)*XS*YS);


	UpdateBits(lmin, lmax);

	HandleBits();

}
	 
//----------------------------------------------------------------------------

void QBits::setView(int v)
{
	int	x;

	display = v;
}


//----------------------------------------------------------------------------

void QBits::mousePressEvent(QMouseEvent *e)
{
	float	x,y;

	cur_pos = QCursor::pos();

	if (e->buttons() == Qt::LeftButton) {
		x = e->x();
		y = e->y();

		x -= dx;
		y -= dy;


		x/=win->vzoom;
		y/=win->vzoom;


		star_x = x;
		star_y = y;


		this->update();
	}
}

//----------------------------------------------------------------------------

void QBits::mouseMoveEvent(QMouseEvent *e)
{
	float	x,y;
	QPoint	point;

	if (e->buttons() == Qt::RightButton) {
		point = QCursor::pos();
		int cdx = point.x() - cur_pos.x();
		int cdy = point.y() - cur_pos.y();
		dx += cdx;
		dy += cdy;
		
		if (dx > 0) dx = 0;
		if (dy > 0) dy = 0;

		update();
		cur_pos = QCursor::pos();
	}

	if (e->buttons() == Qt::LeftButton) {
		x = e->x();
		y = e->y();

		x -= dx;
		y -= dy;

		x/=win->vzoom;
		y/=win->vzoom;

		star_x = x;
		star_y = y;


		this->update();
	}
}

//----------------------------------------------------------------------------


void QBits::UpdateBits(float min, float max)
{
	if (display == DISPLAY_LIFE)
		UpdateBits(fdata, min, max);
	if (display == DISPLAY_DARK)
		UpdateBits(dark, min, max);
	if (display == DISPLAY_FLAT)
		UpdateBits(flat, min, max);
	if (display == DISPLAY_SUM1)
		UpdateBits(sum_1, min, max);
	if (display == DISPLAY_SUM2);
		//UpdateBits(sum_2, min, max);


	if (display != DISPLAY_SUM2)
		this->update();
}

//----------------------------------------------------------------------------

void QBits::UpdateBits(float *data, float min, float max)
{
	int			x;
	int			y;
	ulong		pixel;
	uchar		byte;
	uchar		xor;
	float		v;
	ulong		*p;
	float		scale;

	lmin = min;
	lmax = max;

	if (max < min) {
		float tmp = min;
		min = max;
		max = tmp;
		xor = 0xff;
	}
	else
		xor = 0;

	scale = max - min;
	scale /= 255.0;

	scale = 1.0/scale;

	for (y = 0; y < YS; y++) {
		p = (ulong*)image->bits();
		p += (XS * y);

		for (x = 0; x < XS; x++) {
			v = *data++;
			v -= min;
			if (v < 0) {
				v = 0;
			}
			else {
				v = v * scale;
				if (v > 255.0) v = 255.0;
			}
			
			byte = v;
			byte ^= xor;

			pixel = (0xff<<24)|(byte<<16)|(byte<<8)|byte;
			
			*p++= pixel;
		}
	}
}

	
//----------------------------------------------------------------------------


void QBits::paintEvent(QPaintEvent *)
 {
     QPainter painter(this);
	 QPoint	  where(0, 0);
	 float	  zoom = 1;

	 zoom = win->vzoom;

	 painter.translate(dx,dy);

	 painter.drawImage(QRect(0, 0, image->width()*zoom, image->height()*zoom), *image, QRect(0, 0, image->width(), image->height()));


     for (int h = 0; h <= zoom*image->width(); h += zoom*40) {
		 painter.setPen(QPen(QColor(100, 100, 100, 150), 1));
		 painter.drawLine(QLineF(h,0, h, zoom*image->height()));
     }

     for (int v = 0; v <= zoom*image->height(); v += zoom*40) {
		 painter.setPen(QPen(QColor(100, 100, 100, 150), 1));
		 painter.drawLine(QLineF(0,v, zoom*image->width(), v));
     }


	 painter.setPen(QPen(QColor(255, 0, 0, 255), 1));
	
	 QBrush	b(Qt::SolidPattern);
	 b.setColor(QColor(200,0,0,78));

	 painter.setBrush(b);
	
	 painter.drawEllipse(QRectF(zoom*(star_x-8), zoom*(star_y-8), zoom*16, zoom*16));

	 b.setColor(QColor(0,0,200,78));

	 painter.setPen(QPen(QColor(0, 0, 255, 255), 1));
	 painter.setBrush(b);

	 painter.drawEllipse(QRectF(zoom*(global_cx-8), zoom*(global_cy-8), zoom*16, zoom*16));

	 QString	d;

	 d.sprintf("%f", time_d/1000.0);
	 painter.drawText(QPoint(50,50), d); 

	 d.sprintf("%f", time_d1/1000.0);
	 painter.drawText(QPoint(50,150), d); 

}

//----------------------------------------------------------------------------

 void CCDWindow::createMenus()
 {
	 QAction	*action;
	 QMenu		*fileMenu;
	 QMenu		*settings;


	 fileMenu = menuBar()->addMenu(tr("&File"));

	 action = new QAction(tr("&make dark"), this);
     action->setStatusTip(tr("Create dark frame"));
     connect(action, SIGNAL(triggered()), this, SLOT(MakeDark()));
	 fileMenu->addAction(action);

	 action = new QAction(tr("&make flat"), this);
     action->setStatusTip(tr("Create flat frame"));
     connect(action, SIGNAL(triggered()), this, SLOT(MakeFlat()));
	 fileMenu->addAction(action);

	 action = new QAction(tr("&Record sum"), this);
     action->setStatusTip(tr("Start a sequence"));
     connect(action, SIGNAL(triggered()), this, SLOT(MakeCapture()));
	 fileMenu->addAction(action);


	 settings = menuBar()->addMenu(tr("&Setup"));

	 action = new QAction(tr("&SetGain"), this);
     connect(action, SIGNAL(triggered()), this, SLOT(MakeFlat()));
	 settings->addAction(action);

	 action = new QAction(tr("&SetExposure"), this);
     connect(action, SIGNAL(triggered()), this, SLOT(MakeFlat()));
	 settings->addAction(action);

 }


 //----------------------------------------------------------------------------

void CCDWindow::MakeCapture()
 {
	bool ok;
 
	statusBar()->showMessage(tr("Capture"));
 
     int i = QInputDialog::getInteger(this, tr("Frame Count"),
                                      tr("Frames:"), 222500, 0, 222500, 1, &ok);
	 if (ok) {
		bits->state = CAPTURE;
		if (i > 200000) i = 200000;
		if (i < 1) i = 1;

		bits->zero_sum();

		bits->frame_number = 0;
		bits->frame_total = i;
		bits->hint_x_pos = bits->star_x;
		bits->hint_y_pos = bits->star_y;
     }
}
 
//----------------------------------------------------------------------------

void CCDWindow::MakeFlat()
 {
	bool ok;
 
	statusBar()->showMessage(tr("make_flat"));
 
     int i = QInputDialog::getInteger(this, tr("Flat Count"),
                                      tr("Frames:"), 32000, 0, 32000, 1, &ok);
	 if (ok) {
		bits->state = FLAT;
		if (i > 32000) i = 32000;
		if (i < 1) i = 1;
		bits->has_flat = FALSE;

		bits->flat_total_frames = i;
		bits->flat_frame_left = i;
		bits->zero_flat();
     }
}

//----------------------------------------------------------------------------

void CCDWindow::MakeDark()
 {
	bool ok;
 
	statusBar()->showMessage(tr("make_dark"));
 
     int i = QInputDialog::getInteger(this, tr("Dark Count"),
                                      tr("Frames:"), 10000, 0, 10000, 1, &ok);
	 if (ok) {
		bits->state = DARK;
		if (i > 10000) i = 10000;
		if (i < 1) i = 1;
		bits->has_dark = FALSE;

		bits->dark_total_frames = i;
		bits->dark_frame_left = i;
		bits->zero_dark();
     }
}

//----------------------------------------------------------------------------


void CCDWindow::SetGain(int v)
{
	QString	q;
	float	gain;

	gain = v;
	q.sprintf("Gain %3.3f", gain);

	gainGroup->setTitle(q);
	camera->SetGain(v);
}

//----------------------------------------------------------------------------

void CCDWindow::SetiGain(int v)
{
	QString	q;
	float	igain;

	igain = v;
	quantum = v/4.0;
	if (c1394) {
		q.sprintf("Quantum %3.3f", quantum);
	}
	else
		q.sprintf("IGain %3.3f", igain);

	igainGroup->setTitle(q);
	camera->SetAmplifier(igain);
}

//----------------------------------------------------------------------------

void CCDWindow::SetMin(int v)
{
	QString	q;

	vmin = v;
	q.sprintf("Min %3.0f", vmin);

	minGroup->setTitle(q);

	bits->UpdateBits(vmin,vmax);
}

//----------------------------------------------------------------------------

void CCDWindow::SetMax(int v)
{
	QString	q;

	vmax = v;
	q.sprintf("Max %3.0f", vmax);

	maxGroup->setTitle(q);
	bits->UpdateBits(vmin,vmax);
}
//----------------------------------------------------------------------------

void CCDWindow::SetLevel(int v)
{
	QString	q;

	vlevel = v;
	q.sprintf("Level %3.0f", vlevel);

	levelGroup->setTitle(q);
	//bits->UpdateBits(vmin,vmax);
}

//----------------------------------------------------------------------------

void CCDWindow::SetZoom(int v)
{
	QString	q;

	vzoom = v/10.0;

	if (vzoom < 1.0) vzoom = 1.0;

	q.sprintf("Zoom %3.2f", vzoom);

	zoomGroup->setTitle(q);
	bits->update();
}

//----------------------------------------------------------------------------

void CCDWindow::SetExposure(int v)
{
	QString	q;
	float	exp;

	exp = v / 100.0;

	exp = exp * exp;

	exp /= 600.0;

	if (c1394) {
		//exp /= 30.0;
	}

	q.sprintf("Exposure %3.3f", exp);

	exposureGroup->setTitle(q);
	camera->SetExposure(exp);
}
 
//----------------------------------------------------------------------------

void CCDWindow::got_image()
{
	QTime	t;

	t.start();

	bits->Set((float*)&(camera->cur_frame.data));

	time_d1 = t.elapsed();

}


//----------------------------------------------------------------------------


bool CCDWindow::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::User) {
		got_image();
		return true;
	}

	return false;

}

//----------------------------------------------------------------------------

void	CCDWindow::make_slider(QGroupBox *parent, QGroupBox **groupBox, QSlider **slider, char *title, const char *slot)
{
	*groupBox = new QGroupBox(parent);
	{
	(*groupBox)->setAttribute(Qt::WA_ContentsPropagated);
    (*groupBox)->setTitle("LevelCut = 0");
    *slider = new QSlider(Qt::Horizontal, *groupBox);
    (*slider)->setRange(-200, 12000);
    (*slider)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QVBoxLayout *tmpGroupLayout = new QVBoxLayout(*groupBox);
    tmpGroupLayout->addWidget(*slider);
	
	connect(*slider, SIGNAL(valueChanged(int)), this, slot);
	}


}

//----------------------------------------------------------------------------


CCDWindow::CCDWindow()
{
	QWidget	*top_view;

	

    this->installEventFilter(this);

	top_view = new QWidget();

	createMenus();


    bits = new QBits();

	QWidget *tmp_view = new QWidget();

	QHBoxLayout *h0 = new QHBoxLayout;

	h0->addWidget(bits);
	tmp_view->setLayout(h0);


	
	QHBoxLayout	*hl;

	hl = new QHBoxLayout;

    QGroupBox* mainGroup = new QGroupBox(0);
    mainGroup->setTitle("Controls");
	mainGroup->setMaximumSize(210,580);
	mainGroup->setMinimumSize(210,580);



	make_slider(mainGroup, &exposureGroup, &exposureSlider, "Exposure", SLOT(SetExposure(int))); exposureSlider->setRange(0, 5000);

	if (c1394) {
		make_slider(mainGroup, &gainGroup, &gainSlider, "Gain", SLOT(SetGain(int))); gainSlider->setRange(-5, 29);
		make_slider(mainGroup, &igainGroup, &igainSlider, "Quantum", SLOT(SetiGain(int))); igainSlider->setRange(0, 2500);
	}
	else {
		make_slider(mainGroup, &gainGroup, &gainSlider, "Gain", SLOT(SetGain(int))); gainSlider->setRange(0, 63);
		make_slider(mainGroup, &igainGroup, &igainSlider, "IGain", SLOT(SetiGain(int))); igainSlider->setRange(0, 230);
	}

	make_slider(mainGroup, &minGroup, &minSlider, "Min", SLOT(SetMin(int)));minSlider->setRange(-1000,8500);
	make_slider(mainGroup, &maxGroup, &maxSlider, "Max", SLOT(SetMax(int)));maxSlider->setRange(0,32000);
	make_slider(mainGroup, &zoomGroup, &zoomSlider, "Zoom", SLOT(SetZoom(int))); zoomSlider->setRange(10,60.0);
	make_slider(mainGroup, &levelGroup, &levelSlider, "LevelCut", SLOT(SetLevel(int))); levelSlider->setRange(0,27000);


	QVBoxLayout *mainGroupLayout = new QVBoxLayout(mainGroup);
    mainGroupLayout->addWidget(exposureGroup);
    mainGroupLayout->addWidget(gainGroup);
    mainGroupLayout->addWidget(igainGroup);

	//mainGroupLayout->addWidget(new QLabel("Display"));
	mainGroupLayout->addWidget(minGroup);
    mainGroupLayout->addWidget(maxGroup);
    mainGroupLayout->addWidget(zoomGroup);
	//mainGroupLayout->addWidget(new QLabel("Select"));
    mainGroupLayout->addWidget(levelGroup);


	QPushButton	*start_button = new QPushButton("Start", mainGroup);
	QPushButton	*stop_button = new QPushButton("Stop", mainGroup);

	mainGroupLayout->addWidget(start_button);
	mainGroupLayout->addWidget(stop_button);

	connect(start_button, SIGNAL(clicked()), this, SLOT(MakeCapture()));
	connect(stop_button, SIGNAL(clicked()), bits, SLOT(Stop()));

 	
	hl->addWidget(mainGroup);

	QWidget	*document;

	document = new QWidget();
	QVBoxLayout *VLayout = new QVBoxLayout();
	
	QTabBar * bar = new QTabBar();
	//bar->setShape(QTabBar::TriangularNorth);

	bar->addTab("real_time");
	bar->addTab("dark");
	bar->addTab("flat");
	bar->addTab("sum_A");
	bar->addTab("sum_b");

	bar->setMaximumWidth(600);
	connect(bar, SIGNAL(currentChanged(int)), bits, SLOT(setView(int)));

	VLayout->addWidget(bar);
	VLayout->addWidget(tmp_view);

	document->setLayout(VLayout);


	hl->addWidget(document);

	top_view->setLayout(hl);

	setCentralWidget(top_view);

    setWindowTitle(tr("tc253 Lucky"));

	camera = new TC253(this);


	exposureSlider->setValue(800);
	gainSlider->setValue(15);
	igainSlider->setValue(1);
	igainSlider->setValue(0);
	minSlider->setValue(3000);
	maxSlider->setValue(8000);
	levelSlider->setValue(0);
	zoomSlider->setValue(10.0);

	bits->win = this;
}

//----------------------------------------------------------------------------
