#include "pOpenedFileExplorer.h"
#include "pWorkspace.h"

#include <MonkeyCore.h>

#include <pIconManager.h>
#include <pDockWidgetTitleBar.h>

pOpenedFileExplorer::pOpenedFileExplorer( pWorkspace* workspace )
	: pDockWidget( workspace )
{
	Q_ASSERT( workspace );
	mWorkspace = workspace;
	mModel = new pOpenedFileModel( workspace );
	setupUi( this );
	setFocusProxy( tvFiles );
	tvFiles->setModel( mModel );
	setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
	
	// sort menu
	mSortMenu = new QMenu( this );
	QActionGroup* group = new QActionGroup( mSortMenu );
	
	group->addAction( tr( "Opening order" ) );
	group->addAction( tr( "File name" ) );
	group->addAction( tr( "URL" ) );
	group->addAction( tr( "Suffixes" ) );
	group->addAction( tr( "Custom" ) );
	mSortMenu->addActions( group->actions() );
	
	for ( int i = pOpenedFileModel::OpeningOrder; i < pOpenedFileModel::Custom +1; i++ )
	{
		QAction* action = group->actions().at( i );
		action->setData( (pOpenedFileModel::SortMode)i );
		action->setCheckable( true );
		
		if ( i == pOpenedFileModel::OpeningOrder )
		{
			action->setChecked( true );
		}
	}
	
	QAction* aSortMenu = new QAction( tr( "Sorting" ), this );
	aSortMenu->setMenu( mSortMenu );
	aSortMenu->setIcon( pIconManager::icon( "sort.png" ) );
	aSortMenu->setToolTip( aSortMenu->text() );
	
	QToolButton* tb = qobject_cast<QToolButton*>( titleBar()->addAction( aSortMenu, 0 ) );
	tb->setPopupMode( QToolButton::InstantPopup );
	titleBar()->addSeparator( 1 );
	
	tvFiles->viewport()->setAcceptDrops( true );
	
	connect( group, SIGNAL( triggered ( QAction* ) ), this, SLOT( sortTriggered ( QAction* ) ) );
	connect( workspace, SIGNAL( documentChanged( pAbstractChild* ) ), this, SLOT( documentChanged( pAbstractChild* ) ) );
	connect( workspace, SIGNAL( currentDocumentChanged( pAbstractChild* ) ), this, SLOT( currentDocumentChanged( pAbstractChild* ) ) );
	connect( mModel, SIGNAL( documentMoved( pAbstractChild* ) ), this, SLOT( currentDocumentChanged( pAbstractChild* ) ) );
	connect( mModel, SIGNAL( sortModeChanged( pOpenedFileModel::SortMode ) ), this, SLOT( sortModeChanged( pOpenedFileModel::SortMode ) ) );
	connect( tvFiles->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( selectionModel_selectionChanged( const QItemSelection&, const QItemSelection& ) ) );
}

void pOpenedFileExplorer::sortTriggered ( QAction* action )
{
	pOpenedFileModel::SortMode mode = (pOpenedFileModel::SortMode)action->data().toInt();
	mModel->setSortMode( mode );
}

void pOpenedFileExplorer::documentChanged( pAbstractChild* document )
{
	Q_UNUSED( document );
}

void pOpenedFileExplorer::currentDocumentChanged( pAbstractChild* document )
{
	const QModelIndex index = mModel->index( document );
	tvFiles->setCurrentIndex( index );
}

void pOpenedFileExplorer::sortModeChanged( pOpenedFileModel::SortMode mode )
{
	foreach ( QAction* action, mSortMenu->actions() )
	{
		if ( action->data().toInt() == mode )
		{
			if ( !action->isChecked() )
			{
				action->setChecked( true );
			}
			
			return;
		}
	}
}

void pOpenedFileExplorer::selectionModel_selectionChanged( const QItemSelection& selected, const QItemSelection& deselected )
{
	Q_UNUSED( deselected );
	const QModelIndex index = selected.indexes().value( 0 );
	pAbstractChild* document = mModel->document( index );
	tvFiles->setCurrentIndex( index );
	mWorkspace->setCurrentDocument( document );
	setFocus(); // setting active mdi window still the focus
}

void pOpenedFileExplorer::on_tvFiles_customContextMenuRequested( const QPoint& pos )
{
	QMenu menu;
	menu.addAction( MonkeyCore::menuBar()->action( "mFile/mClose/aCurrent" ) );
	menu.addAction( MonkeyCore::menuBar()->action( "mFile/mSave/aCurrent" ) );
	menu.addSeparator();
	menu.addAction( mSortMenu->menuAction() );
	menu.exec( tvFiles->mapToGlobal( pos ) );
}
