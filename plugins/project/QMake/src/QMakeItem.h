/****************************************************************************
**
** 		Created using Monkey Studio v1.7.0
** Author    : Nox P@sNox aka Azevedo Filipe <pasnox@gmail.com>
** Project   : QMakeItem
** FileName  : QMakeItem.h
** Date      : 2007-09-04T15:07:31
** License   : GPL
** Comment   : Your comment here
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QMAKEITEM_H
#define QMAKEITEM_H

#include "ProjectItem.h"

class QMakeItem : public ProjectItem
{

public:
	QMakeItem( ProjectsModel::NodeType = ProjectsModel::ProjectType, ProjectItem* = 0 );

	virtual QString pluginName() const { return "QMake"; }
	
	virtual void setType( ProjectsModel::NodeType );
	virtual void setValue( const QString& );
	virtual void setFilePath( const QString& );
	virtual QString getIndent() const;
	virtual QString getEol() const;
	virtual bool isFirst() const;
	virtual bool isLast() const;
	virtual QString scope() const;
	virtual QString checkScope( const QString& ) const;
	virtual bool isEqualScope( const QString&, const QString& ) const;
	
	virtual void appendRow( ProjectItem* );
	virtual void insertRow( int, ProjectItem* );
	virtual bool swapRow( int, int );
	virtual bool moveRowUp( int );
	virtual bool moveRowDown( int );
	virtual bool moveUp();
	virtual bool moveDown();
	virtual void remove();
	
	virtual void close();
	virtual void save( bool = true );
	virtual void saveAll( bool = true );
	virtual void debug();
	
	virtual ProjectItem* getItemScope( const QString&, bool ) const;
	virtual QStringList getListValues( const QString&, const QString& = "=", const QString& = QString::null ) const;
	virtual QString getStringValues( const QString&, const QString& = "=", const QString& = QString::null ) const;
	virtual void setListValues( const QStringList&, const QString&, const QString& = "=", const QString& = QString::null );
	virtual void setStringValues( const QString&, const QString&, const QString& = "=", const QString& = QString::null );
	virtual void addListValues( const QStringList&, const QString&, const QString& = "=", const QString& = QString::null );
	virtual void addStringValues( const QString&, const QString&, const QString& = "=", const QString& = QString::null );
	
protected:
	virtual void redoLayout( ProjectItem* );
	virtual void writeProject();
	virtual void writeItem( ProjectItem* );

};

#endif // QMAKEITEM_H
