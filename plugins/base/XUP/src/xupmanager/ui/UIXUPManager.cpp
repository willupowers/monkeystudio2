#include "UIXUPManager.h"
#include "ProjectItemModel.h"
#include "FilteredProjectItemModel.h"
#include "XUPItem.h"
#include "XUPManager.h"
#include "AddFilesDialog.h"

#include <QHeaderView>

using namespace XUPManager;

#include <QMessageBox>
void warning( const QString& s )
{ QMessageBox::warning( 0, "Warning...", s ); }

bool question( const QString& s )
{ return QMessageBox::question( 0, "Question...", s, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) == QMessageBox::Yes; }

UIXUPManager::UIXUPManager( QWidget* w )
	: QDockWidget( w ), mModel( new ProjectItemModel( this ) )
{
	// setup widget
	setupUi( this );
	initGui();
	// associate model
	tvProjects->setModel( mModel );
	tvProxiedProjects->setModel( mModel->filteredModel() );
	// hide headers
	tvProjects->header()->hide();
	tvProxiedProjects->header()->hide();
	// set filtered viewq by default
	action( UIXUPManager::Filtered )->setChecked( true );
	// connection
	connect( tvProxiedProjects->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( currentChanged( const QModelIndex&, const QModelIndex& ) ) );
	connect( this, SIGNAL( projectOpen( XUPItem* ) ), this, SLOT( internal_projectOpen( XUPItem* ) ) );
	connect( this, SIGNAL( projectAboutToClose( XUPItem* ) ), this, SLOT( internal_projectAboutToClose( XUPItem* ) ) );
	connect( this, SIGNAL( projectClosed( XUPItem* ) ), this, SLOT( internal_projectClosed( XUPItem* ) ) );
	connect( this, SIGNAL( projectModifiedChanged( XUPItem*, bool ) ), this, SLOT( internal_projectModifiedChanged( XUPItem*, bool ) ) );
	connect( this, SIGNAL( currentProjectChanged( XUPItem* ) ), this, SLOT( internal_currentProjectChanged( XUPItem* ) ) );
	connect( this, SIGNAL( projectDoubleClicked( XUPItem* ) ), this, SLOT( internal_projectDoubleClicked( XUPItem* ) ) );
	connect( this, SIGNAL( fileDoubleClicked( XUPItem*, const QString& ) ), this, SLOT( internal_fileDoubleClicked( XUPItem*, const QString& ) ) );
	// set current proejct to null
	setCurrentProject( 0 );
}

UIXUPManager::~UIXUPManager()
{
	// delete actions
	qDeleteAll( mActions );
	mActions.clear();
	// delete registered items
	qDeleteAll( mRegisteredItems );
	mRegisteredItems.clear();
}

void UIXUPManager::initGui()
{
	// create actions
	setAction( UIXUPManager::New, new QAction( QIcon( ":/actions/new.png" ), tr( "New" ), this ) );
	setAction( UIXUPManager::Open, new QAction( QIcon( ":/actions/open.png" ), tr( "Open" ), this ) );
	setAction( UIXUPManager::SaveCurrent, new QAction( QIcon( ":/actions/save.png" ), tr( "Save Current" ), this ) );
	setAction( UIXUPManager::SaveAll, new QAction( QIcon( ":/actions/saveall.png" ), tr( "Save All" ), this ) );
	setAction( UIXUPManager::CloseCurrent, new QAction( QIcon( ":/actions/close.png" ), tr( "Close Current" ), this ) );
	setAction( UIXUPManager::CloseAll, new QAction( QIcon( ":/actions/closeall.png" ), tr( "Close All" ), this ) );
	setAction( UIXUPManager::Add, new QAction( QIcon( ":/actions/add.png" ), tr( "Add..." ), this ) );
	setAction( UIXUPManager::Remove, new QAction( QIcon( ":/actions/remove.png" ), tr( "Remove..." ), this ) );
	setAction( UIXUPManager::Settings, new QAction( QIcon( ":/actions/settings.png" ), tr( "Settings..." ), this ) );
	setAction( UIXUPManager::Source, new QAction( QIcon( ":/actions/source.png" ), tr( "Source..." ), this ) );
	setAction( UIXUPManager::Filtered, new QAction( QIcon( ":/actions/filtered.png" ), tr( "Filtered" ), this ) );
	// add actions to toolbar
	tbActions->addAction( action( UIXUPManager::New ) );
	tbActions->addAction( action( UIXUPManager::Open ) );
	tbActions->addAction( action( UIXUPManager::SaveCurrent ) );
	tbActions->addAction( action( UIXUPManager::SaveAll ) );
	tbActions->addAction( action( UIXUPManager::CloseCurrent ) );
	tbActions->addAction( action( UIXUPManager::CloseAll ) );
	tbActions->addAction( action( UIXUPManager::Add ) );
	tbActions->addAction( action( UIXUPManager::Remove ) );
	tbActions->addAction( action( UIXUPManager::Settings ) );
	tbActions->addAction( action( UIXUPManager::Source ) );
	tbActions->addAction( action( UIXUPManager::Filtered ) );
}

void UIXUPManager::registerItem( XUPItem* it )
{
	const QString s = it->metaObject()->className();
	if ( mRegisteredItems.keys().contains( s ) )
		delete mRegisteredItems[s];
	mRegisteredItems[s] = it;
}

void UIXUPManager::unRegisterItem( XUPItem* it )
{
	const QString s = it->metaObject()->className();
	if ( mRegisteredItems.keys().contains( s ) )
		delete mRegisteredItems.take( s );
}

QStringList UIXUPManager::projectsFilters() const
{
	QStringList l, e;
	foreach ( XUPItem* rpi, mRegisteredItems )
	{
		foreach ( QString label, rpi->suffixes().keys() )
		{
			e << rpi->suffixes().value( label );
			l << QString( "%1 (%2)" ).arg( label ).arg( rpi->suffixes().value( label ).join( " " ) );
		}
	}
	qSort( l );
	if ( l.count() > 1 )
		l.prepend( QString( tr( "All Project Files (%1)" ) ).arg( e.join( " " ) ) );
	return l;
}

void UIXUPManager::initializeProject( XUPItem* pi )
{
	// add project into model
	mModel->appendRow( pi );
	// tell proejct is open
	emit projectOpen( pi );
	// set current project
	setCurrentProject( pi );
	// connection
	connect( pi, SIGNAL( modifiedChanged( XUPItem*, bool ) ), this, SIGNAL( projectModifiedChanged( XUPItem*, bool ) ) );
	connect( pi, SIGNAL( aboutToClose( XUPItem* ) ), this, SIGNAL( projectAboutToClose( XUPItem* ) ) );
	connect( pi, SIGNAL( closed( XUPItem* ) ), this, SIGNAL( projectClosed( XUPItem* ) ) );
}

XUPItem* UIXUPManager::currentProject() const
{
	if ( FilteredProjectItem* fit = mModel->filteredModel()->itemFromIndex( tvProxiedProjects->currentIndex() ) )
		return fit->project();
	return 0;
}

void UIXUPManager::setCurrentProject( const XUPItem* pi )
{
	// update gui
	action( UIXUPManager::SaveCurrent )->setEnabled( pi && pi->modified() );
	action( UIXUPManager::SaveAll )->setEnabled( mModel->topLevelProjects().count() );
	action( UIXUPManager::CloseCurrent )->setEnabled( pi );
	action( UIXUPManager::CloseAll )->setEnabled( pi );
	action( UIXUPManager::Add )->setEnabled( pi );
	action( UIXUPManager::Remove )->setEnabled( currentValue() );
	action( UIXUPManager::Settings )->setEnabled( pi );
	action( UIXUPManager::Source )->setEnabled( pi );
	// change index
	if ( currentProject() != pi )
		tvProxiedProjects->setCurrentIndex( mModel->filteredModel()->mapFromSource( pi ? pi->index() : QModelIndex() ) );
}

XUPItem* UIXUPManager::currentValue() const
{
	if ( FilteredProjectItem* fit = mModel->filteredModel()->itemFromIndex( tvProxiedProjects->currentIndex() ) )
		return fit->item()->isType( "value" ) ? fit->item() : 0;
	return 0;
}

XUPItem* UIXUPManager::currentItem() const
{
	if ( FilteredProjectItem* fit = mModel->filteredModel()->itemFromIndex( tvProxiedProjects->currentIndex() ) )
		return fit->item();
	return 0;
}

bool UIXUPManager::openProject( const QString& s )
{
	// get project item
	XUPItem* pi = 0;
	foreach ( XUPItem* rpi, mRegisteredItems )
	{
		foreach ( QStringList suffixes, rpi->suffixes().values() )
		{
			foreach ( QString suffix, suffixes )
			{
				if ( QDir::match( suffix, s ) )
				{
					pi = rpi->clone( false );
					break;
				}
			}
		}
	}

	// if no suitable item cancel
	if ( !pi )
		return false;

	// load project
	if ( pi->loadProject( s ) )
	{
		initializeProject( pi );
		return true;
	}

	// can load project
	delete pi;
	return false;
}

bool UIXUPManager::saveProject( XUPItem* pi, const QString& s )
{ return pi ? pi->saveProject( s ) : false; }

QAction* UIXUPManager::action( UIXUPManager::Actions at ) const
{ return mActions.value( at ); }

void UIXUPManager::setAction( UIXUPManager::Actions at, QAction* a )
{
	// get old action if available
	QAction* oa = action( at );
	// check actions are differents
	if ( oa == a )
		return;
	// disconnect old action, connect new one
	switch ( at )
	{
		case UIXUPManager::New:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionNewTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionNewTriggered() ) );
			break;
		case UIXUPManager::Open:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionOpenTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionOpenTriggered() ) );
			break;
		case UIXUPManager::SaveCurrent:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionSaveCurrentTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionSaveCurrentTriggered() ) );
			break;
		case UIXUPManager::SaveAll:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionSaveAllTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionSaveAllTriggered() ) );
			break;
		case UIXUPManager::CloseCurrent:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionCloseCurrentTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionCloseCurrentTriggered() ) );
			break;
		case UIXUPManager::CloseAll:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionCloseAllTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionCloseAllTriggered() ) );
			break;
		case UIXUPManager::Add:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionAddTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionAddTriggered() ) );
			break;
		case UIXUPManager::Remove:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionRemoveTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionRemoveTriggered() ) );
			break;
		case UIXUPManager::Settings:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionSettingsTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionSettingsTriggered() ) );
			break;
		case UIXUPManager::Source:
			if ( oa )
				disconnect( oa, SIGNAL( triggered() ), this, SLOT( actionSourceTriggered() ) );
			if ( a )
				connect( a, SIGNAL( triggered() ), this, SLOT( actionSourceTriggered() ) );
			break;
		case UIXUPManager::Filtered:
			if ( oa )
				disconnect( oa, SIGNAL( toggled( bool ) ), this, SLOT( actionFilteredToggled( bool ) ) );
			if ( a )
			{
				a->setCheckable( true );
				a->setChecked( swViews->currentWidget() == tvProxiedProjects );
				connect( a, SIGNAL( toggled( bool ) ), this, SLOT( actionFilteredToggled( bool ) ) );
			}
			break;
		default:
			break;
	}
	// delete old action
	delete mActions.take( at );
	// set new action
	if ( a )
		mActions[at] = a;
}

void UIXUPManager::actionNewTriggered()
{ warning( tr( "Not Yet Implemented" ) ); }

void UIXUPManager::actionOpenTriggered()
{
	const QString s = QFileDialog::getOpenFileName( window(), tr( "Choose a project to open..." ), QString(), projectsFilters().join( ";;" ) );
	if ( !s.isNull() && !openProject( s ) )
		warning( tr( "An error occur while opening project : %1" ).arg( s ) );
}

void UIXUPManager::actionSaveCurrentTriggered()
{
	if ( XUPItem* pi = currentProject() )
		if ( !saveProject( pi, QString() ) )
			warning( tr( "An error occur while saving project" ) );
}

void UIXUPManager::actionSaveAllTriggered()
{
	foreach ( XUPItem* pi, mModel->topLevelProjects() )
	{
		if ( !saveProject( pi, QString() ) )
			warning( tr( "An error occur while saving project: %1" ).arg( pi->defaultValue() ) );
		foreach ( XUPItem* cpi, pi->children( true, false ) )
			if ( cpi->isProject() )
				if ( !saveProject( cpi, QString() ) )
					warning( tr( "An error occur while saving project: %1" ).arg( cpi->defaultValue() ) );
	}
}

void UIXUPManager::actionCloseCurrentTriggered()
{
	if ( XUPItem* pi = currentProject() )
		pi->closeProject();
}

void UIXUPManager::actionCloseAllTriggered()
{
	foreach ( XUPItem* pi, mModel->topLevelProjects() )
		pi->closeProject();
}

void UIXUPManager::actionAddTriggered()
{
	if ( XUPItem* pi = currentItem() )
	{
		AddFilesDialog d( mModel->scopedModel(), pi, window() );
		if ( d.exec() && !d.selectedFiles().isEmpty() )
			d.currentItem()->addFiles( d.selectedFiles(), d.currentItem(), d.currentOperator() );
	}
}

void UIXUPManager::actionRemoveTriggered()
{
	if ( XUPItem* it = currentValue() )
	{
		if ( question( tr( "Are you sur you want to remove this value ?" ) ) )
		{
			// if file based
			if ( it->fileVariables().contains( it->parent()->defaultValue() ) )
			{
				QString fp = it->filePath();
				if ( QFile::exists( fp ) && question( tr( "Do you want to delete the associate file ?" ) ) )
					if ( !QFile::remove( fp ) )
						warning( tr( "Can't delete file: %1" ).arg( fp ) );
			}
			// get parent
			XUPItem* parent = it->parent();
			// remove item
			it->remove();
			// delete parent is empty
			if ( !parent->hasChildren() )
				parent->remove();
		}
	}
}

void UIXUPManager::actionSettingsTriggered()
{
	if ( XUPItem* pi = currentProject() )
	{
		// get plugin name that can manage this project
		const QString name = pi->value( "plugin" );
		if ( name.isEmpty() )
		{
			warning( tr( "Your project is not yet editable, please select a correct project plugin manager" ) );
		}
	}
}

void UIXUPManager::actionSourceTriggered()
{
	if ( XUPItem* pi = currentProject() )
		teLog->setPlainText( pi->domDocument().toString() );
}

void UIXUPManager::actionFilteredToggled( bool toggled )
{ swViews->setCurrentWidget( toggled ? tvProxiedProjects : tvProjects ); }

void UIXUPManager::currentChanged( const QModelIndex& c, const QModelIndex& o )
{
	// old
	FilteredProjectItem* ofit = mModel->filteredModel()->itemFromIndex( o );
	XUPItem* op = ofit ? ofit->project() : 0;
	// current
	FilteredProjectItem* fit = mModel->filteredModel()->itemFromIndex( c );
	XUPItem* p = fit ? fit->project() : 0;
	// update some actions
	action( UIXUPManager::Remove )->setEnabled( currentValue() );
	// if new project != old update gui
	if ( op != p )
	{
		setCurrentProject( p );
		emit currentProjectChanged( p );
	}
}

void UIXUPManager::on_tvProxiedProjects_collapsed( const QModelIndex& i )
{
	if ( FilteredProjectItem* fit = mModel->filteredModel()->itemFromIndex( i ) )
		fit->item()->setValue( "expanded", "false" );
}

void UIXUPManager::on_tvProxiedProjects_expanded( const QModelIndex& i )
{
	if ( FilteredProjectItem* fit = mModel->filteredModel()->itemFromIndex( i ) )
		fit->item()->setValue( "expanded", "true" );
}

void UIXUPManager::on_tvProxiedProjects_doubleClicked( const QModelIndex& i )
{
	if ( FilteredProjectItem* fit = mModel->filteredModel()->itemFromIndex( i ) )
	{
		XUPItem* it = fit->item();
		if ( it->isProject() )
			emit projectDoubleClicked( it );
		else if ( it->isType( "value" ) && it->fileVariables().contains( it->parent()->defaultValue() ) )
			emit fileDoubleClicked( it, it->filePath() );
	}
}

void UIXUPManager::internal_projectOpen( XUPItem* pi )
{
	if ( pi )
	{
		teLog->append( tr( "Project Open: %1" ).arg( pi->defaultValue() ) );
		teLog->append( tr( "Restoring Session..." ) );
		// restore project
		foreach ( XUPItem* it, QList<XUPItem*>() << pi << pi->children( true, false ) )
		{
			// expand needed items
			if ( bool b = QVariant( it->value( "expanded", "false" ) ).toBool() )
				tvProxiedProjects->setExpanded( mModel->filteredModel()->mapFromSource( it->index() ), b );
			// if value and file based variable
			if ( it->value( "type" ) == "value" && it->fileVariables().contains( it->parent()->value( "name" ) ) )
			{
				/*
				// file configuration
				qWarning( "file name: %s", qPrintable( it->filePath() ) );
				qWarning( "file open: %s", qPrintable( it->value( "open", "false" ) ) );
				qWarning( "file cursor: %s", qPrintable( it->value( "cursor", "none" ) ) );
				qWarning( "file selection: %s", qPrintable( it->value( "selection", "none" ) ) );
				qWarning( "file folds: %s", qPrintable( it->value( "folds", "none" ) ) );
				qWarning( "file bookmarks: %s", qPrintable( it->value( "bookmarks", "none" ) ) );
				qWarning( "file breakpoint: %s", qPrintable( it->value( "breakpoints", "none" ) ) );
				*/
			}
		}
		teLog->append( tr( "Restoring Session Finished..." ) );
	}
}

void UIXUPManager::internal_projectAboutToClose( XUPItem* pi )
{
	if ( pi )
	{
		teLog->append( tr( "Project About To Close: %1" ).arg( pi->defaultValue() ) );
		// save project session
		teLog->append( tr( "Saving Session..." ) );
		foreach ( XUPItem* it, QList<XUPItem*>() << pi << pi->children( true, false ) )
		{
			// check files and set correct attribute on files item ( open, cursor position, ... )
			if ( it->value( "type" ) == "value" && pi->fileVariables().contains( it->parent()->value( "name" ) ) )
			{
				// to do
			}
		}
		teLog->append( tr( "Saving Session Finished..." ) );
		// save project if needed
		if ( pi->modified() && question( tr( "Project modified '%1', save it ?" ).arg( pi->defaultValue() ) ) )
			if ( !saveProject( pi, QString() ) )
				warning( tr( "An error occur while saving project" ) );
	}
}

void UIXUPManager::internal_projectClosed( XUPItem* pi )
{
	if ( pi )
	{
		teLog->append( tr( "Project Closed: %1" ).arg( pi->defaultValue() ) );
		// remove project from model
		pi->remove();
		// set current project to none if there is no longer opened project
		if ( !mModel->topLevelProjects().count() )
			setCurrentProject( 0 );
		else if ( !currentProject() )
			setCurrentProject( mModel->topLevelProjects().value( 0 ) );
	}
}

void UIXUPManager::internal_projectModifiedChanged( XUPItem* pi, bool b )
{
	if ( pi )
	{
		if ( pi->project() == currentProject() )
			action( UIXUPManager::SaveCurrent )->setEnabled( b );
		action( UIXUPManager::SaveAll )->setEnabled( mModel->topLevelProjects().count() );
		teLog->append( tr( "Project Modified Changed: %1, %2" ).arg( pi->defaultValue() ).arg( b ? tr( "True" ) : tr( "False" ) ) );
	}
}

void UIXUPManager::internal_currentProjectChanged( XUPItem* it )
{
	if ( it )
		teLog->append( tr( "Current Project Changed: %1" ).arg( it->defaultValue() ) );
}

void UIXUPManager::internal_projectDoubleClicked( XUPItem* it )
{
	if ( it )
		teLog->append( tr( "Project Double Clicked: %1" ).arg( it->defaultValue() ) );
}

void UIXUPManager::internal_fileDoubleClicked( XUPItem* it, const QString& s )
{
	if ( it )
		teLog->append( tr( "File Double Clicked: %1, %2" ).arg( it->defaultValue() ).arg( s ) );
}