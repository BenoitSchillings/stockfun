#ifndef Frame_H
#define Frame_H

#define	C1394

#ifdef	C1394
#define	_PIXEL		1024
#define	_MAXLINES	768
#endif

#ifndef C1394
#define _PIXEL		720					// no of pixels min 600, max 8100, TI:690
#define _MAXLINES	480					// max. no. of lines
#endif

extern	bool	c1394;

class	Frame {
public:
	float 	data[_MAXLINES][_PIXEL];

public:
};



class	TC253 : public QObject {
private:
	int		v_gain;
	int		v_amplifier;
	int		new_gain;
	int		new_amp;
	float	new_exp;

	Frame	*cur;
	char	busy;
	QObject *vparent;
public:
	Frame	cur_frame;


			TC253(QObject *parent);
	void	SetGain(int gain);
	void	SetAmplifier(int amplifier);
	void	SetExposure(float t);
	void	xSetGain(int gain);
	void	xSetAmplifier(int amplifier);
	void	xSetExposure(float t);

	void	GetFrame();
	void	Consumer();

signals:
	void	NewBits();
};



#endif