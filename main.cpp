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

#include <QApplication>

#include "CCDWindow.h"
#include "Frame.h"

QApplication	*the_app;

extern int xmain(int v, char *p);

//! [0]
int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(imagecomposition);


	the_app = new QApplication(argc, argv);


    CCDWindow composer;
    composer.show();

	QStyle *arthurStyle = new ArthurStyle();
     composer.setStyle(arthurStyle);
	 
     QList<QWidget *> widgets = qFindChildren<QWidget *>(&composer);
     foreach (QWidget *w, widgets)
         w->setStyle(arthurStyle);
	

    return the_app->exec();
}

