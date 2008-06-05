#ifndef REGEXPEDITOR_H
#define REGEXPEDITOR_H

#include <pluginsmanager.h>

class UIRegExpEditor;

class RegExpEditor : public BasePlugin
{
	Q_OBJECT
	Q_INTERFACES( BasePlugin )
	
public:
	RegExpEditor();
	~RegExpEditor();
	virtual bool setEnabled( bool );
	virtual QPixmap pixmap() const;

protected:
	QPointer<UIRegExpEditor> mEditor;

protected slots:
	void action_triggered();
};

#endif // REGEXPEDITOR_H