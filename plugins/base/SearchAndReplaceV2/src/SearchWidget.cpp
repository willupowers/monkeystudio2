#include "SearchWidget.h"
#include "SearchThread.h"

#include <MonkeyCore.h>
#include <UIMain.h>
#include <pFileManager.h>
#include <XUPProjectItem.h>
#include <pWorkspace.h>
#include <pAbstractChild.h>
#include <pChild.h>
#include <pEditor.h>

#include <QTextCodec>
#include <QTime>

SearchWidget::SearchWidget( QWidget* parent )
	: QFrame( parent )
{
	setupUi( this );
	
	// threads
	mSearchThread = new SearchThread( this );
	
	// mode actions
	QMenu* menuMode = new QMenu( pbMode );
	QActionGroup* groupMode = new QActionGroup( menuMode );
	
	mModeActions[ SearchAndReplaceV2::ModeSearch ] = menuMode->addAction( tr( "&Search in File" ) );
	mModeActions[ SearchAndReplaceV2::ModeReplace ] = menuMode->addAction( tr( "&Replace in File" ) );
	mModeActions[ SearchAndReplaceV2::ModeSearchDirectory ] = menuMode->addAction( tr( "Search in &Directory" ) );
	mModeActions[ SearchAndReplaceV2::ModeReplaceDirectory ] = menuMode->addAction( tr( "Repla&ce in Directory" ) );
	mModeActions[ SearchAndReplaceV2::ModeSearchProject ] = menuMode->addAction( tr( "Search in &Project" ) );
	mModeActions[ SearchAndReplaceV2::ModeReplaceProject ] = menuMode->addAction( tr( "R&eplace in Project" ) );
	mModeActions[ SearchAndReplaceV2::ModeSearchOpenedFiles ] = menuMode->addAction( tr( "Search in &Opened Files" ) );
	mModeActions[ SearchAndReplaceV2::ModeReplaceOpenedFiles ] = menuMode->addAction( tr( "Replace in Opened &Files" ) );
	
	foreach ( QAction* action, menuMode->actions() )
	{
		action->setCheckable( true );
		groupMode->addAction( action );
	}
	
	pbMode->setMenu( menuMode );
	
	// options actions
	QMenu* menuOptions = new QMenu( pbOptions );
	
	mOptionActions[ SearchAndReplaceV2::OptionCaseSensitive ] = menuOptions->addAction( tr( "&Case Sensitive" ) );
	mOptionActions[ SearchAndReplaceV2::OptionWholeWord ] = menuOptions->addAction( tr( "&Whole Word" ) );
	mOptionActions[ SearchAndReplaceV2::OptionWrap ] = menuOptions->addAction( tr( "Wra&p" ) );
	mOptionActions[ SearchAndReplaceV2::OptionRegularExpression ] = menuOptions->addAction( tr( "&Regular Expression" ) );
	
	foreach ( QAction* action, menuOptions->actions() )
	{
		action->setCheckable( true );
	}
	
	pbOptions->setMenu( menuOptions );
	
	// mac
	pMonkeyStudio::showMacFocusRect( this, false, true );
	pMonkeyStudio::setMacSmallSize( this, true, true );

#ifdef Q_OS_MAC
	const QSize size( 12, 12 );
	
	foreach ( QAbstractButton* button, findChildren<QAbstractButton*>() )
	{
		button->setIconSize( size );
		button->setMaximumHeight( 25 );
	}
	
	vlMain->setSpacing( 0 );
#endif
	
	// codecs
	QStringList codecs;
	foreach ( const QString& codec, QTextCodec::availableCodecs() )
	{
		codecs << codec;
	}
	codecs.sort();
	cbCodec->addItems( codecs );
	
	cbCodec->setCurrentIndex( cbCodec->findText( pMonkeyStudio::defaultCodec() ) );
	
	// connections
	connect( groupMode, SIGNAL( triggered( QAction* ) ), this, SLOT( groupMode_triggered( QAction* ) ) );
	
	setMode( SearchAndReplaceV2::ModeSearch );
}

SearchWidget::~SearchWidget()
{
}

SearchAndReplaceV2::Mode SearchWidget::mode() const
{
	return mMode;
}

SearchThread* SearchWidget::searchThread() const
{
	return mSearchThread;
}

void SearchWidget::setMode( SearchAndReplaceV2::Mode mode )
{
	mMode = mode;
	mModeActions[ mMode ]->setChecked( true );
	
	switch ( mMode )
	{
		case SearchAndReplaceV2::ModeNo:
			wSearch->setVisible( false );
			wReplace->setVisible( false );
			wPath->setVisible( false );
			wOptions->setVisible( false );
			break;
		case SearchAndReplaceV2::ModeSearch:
			wSearch->setVisible( true );
			pbPrevious->setVisible( true );
			pbNext->setVisible( true );
			pbSearch->setVisible( false );
			
			wReplace->setVisible( false );
			
			wPath->setVisible( false );
			pbReplace->setVisible( false );
			pbReplaceAll->setVisible( false );
			pbReplaceChecked->setVisible( false );
			
			wOptions->setVisible( true );
			wMask->setVisible( false );
			wCodec->setVisible( false );
			break;
		case SearchAndReplaceV2::ModeReplace:
			wSearch->setVisible( true );
			pbPrevious->setVisible( true );
			pbNext->setVisible( true );
			pbSearch->setVisible( false );
			
			wReplace->setVisible( true );
			
			wPath->setVisible( false );
			pbReplace->setVisible( true );
			pbReplaceAll->setVisible( true );
			pbReplaceChecked->setVisible( false );
			
			wOptions->setVisible( true );
			wMask->setVisible( false );
			wCodec->setVisible( false );
			break;
		case SearchAndReplaceV2::ModeSearchDirectory:
			wSearch->setVisible( true );
			pbPrevious->setVisible( false );
			pbNext->setVisible( false );
			pbSearch->setVisible( true );
			
			wReplace->setVisible( false );
			
			wPath->setVisible( true );
			pbReplace->setVisible( false );
			pbReplaceAll->setVisible( false );
			pbReplaceChecked->setVisible( false );
			
			wOptions->setVisible( true );
			wMask->setVisible( true );
			wCodec->setVisible( true );
			break;
		case SearchAndReplaceV2::ModeReplaceDirectory:
			wSearch->setVisible( true );
			pbPrevious->setVisible( false );
			pbNext->setVisible( false );
			pbSearch->setVisible( true );
			
			wReplace->setVisible( true );
			
			wPath->setVisible( true );
			pbReplace->setVisible( false );
			pbReplaceAll->setVisible( false );
			pbReplaceChecked->setVisible( true );
			
			wOptions->setVisible( true );
			wMask->setVisible( true );
			wCodec->setVisible( true );
			break;
		case SearchAndReplaceV2::ModeSearchProject:
			wSearch->setVisible( true );
			pbPrevious->setVisible( false );
			pbNext->setVisible( false );
			pbSearch->setVisible( true );
			
			wReplace->setVisible( false );
			
			wPath->setVisible( false );
			pbReplace->setVisible( false );
			pbReplaceAll->setVisible( false );
			pbReplaceChecked->setVisible( false );
			
			wOptions->setVisible( true );
			wMask->setVisible( true );
			wCodec->setVisible( true );
			break;
		case SearchAndReplaceV2::ModeReplaceProject:
			wSearch->setVisible( true );
			pbPrevious->setVisible( false );
			pbNext->setVisible( false );
			pbSearch->setVisible( true );
			
			wReplace->setVisible( true );
			
			wPath->setVisible( false );
			pbReplace->setVisible( false );
			pbReplaceAll->setVisible( false );
			pbReplaceChecked->setVisible( true );
			
			wOptions->setVisible( true );
			wMask->setVisible( true );
			wCodec->setVisible( true );
			break;
		case SearchAndReplaceV2::ModeSearchOpenedFiles:
			wSearch->setVisible( true );
			pbPrevious->setVisible( false );
			pbNext->setVisible( false );
			pbSearch->setVisible( true );
			
			wReplace->setVisible( false );
			
			wPath->setVisible( false );
			pbReplace->setVisible( false );
			pbReplaceAll->setVisible( false );
			pbReplaceChecked->setVisible( false );
			
			wOptions->setVisible( true );
			wMask->setVisible( true );
			wCodec->setVisible( false );
			break;
		case SearchAndReplaceV2::ModeReplaceOpenedFiles:
			wSearch->setVisible( true );
			pbPrevious->setVisible( false );
			pbNext->setVisible( false );
			pbSearch->setVisible( true );
			
			wReplace->setVisible( true );
			
			wPath->setVisible( false );
			pbReplace->setVisible( false );
			pbReplaceAll->setVisible( false );
			pbReplaceChecked->setVisible( true );
			
			wOptions->setVisible( true );
			wMask->setVisible( true );
			wCodec->setVisible( false );
			break;
	}
	
	updateLabels();
	updateWidgets();
	initializeProperties();
}

void SearchWidget::keyPressEvent( QKeyEvent* event )
{
	if ( event->modifiers() == Qt::NoModifier )
	{
		switch ( event->key() )
		{
			case Qt::Key_Escape:
			{
				MonkeyCore::workspace()->focusEditor();
				hide();
				
				break;
			}
			case Qt::Key_Enter:
			case Qt::Key_Return:
			{
				switch ( mMode )
				{
					case SearchAndReplaceV2::ModeNo:
						break;
					case SearchAndReplaceV2::ModeSearch:
						pbNext->click();
						break;
					case SearchAndReplaceV2::ModeSearchDirectory:
					case SearchAndReplaceV2::ModeSearchProject:
					case SearchAndReplaceV2::ModeSearchOpenedFiles:
						pbSearch->click();
						break;
					case SearchAndReplaceV2::ModeReplace:
						pbReplace->click();
						break;
					case SearchAndReplaceV2::ModeReplaceDirectory:
					case SearchAndReplaceV2::ModeReplaceProject:
					case SearchAndReplaceV2::ModeReplaceOpenedFiles:
						pbReplaceChecked->click();
						break;
				}
				
				break;
			}
		}
	}
	
	QWidget::keyPressEvent( event );
}

void SearchWidget::updateLabels()
{
	int width = 0;
	
	if ( lSearch->isVisible() )
	{
		width = qMax( width, lSearch->minimumSizeHint().width() );
	}
	
	if ( lReplace->isVisible() )
	{
		width = qMax( width, lReplace->minimumSizeHint().width() );
	}
	
	if ( lPath->isVisible() )
	{
		width = qMax( width, lPath->minimumSizeHint().width() );
	}
	
	lSearch->setMinimumWidth( width );
	lReplace->setMinimumWidth( width );
	lPath->setMinimumWidth( width );
}

void SearchWidget::updateWidgets()
{
	int width = 0;
	
	if ( wSearchRight->isVisible() )
	{
		width = qMax( width, wSearchRight->minimumSizeHint().width() );
	}
	
	if ( wReplaceRight->isVisible() )
	{
		width = qMax( width, wReplaceRight->minimumSizeHint().width() );
	}
	
	if ( wPathRight->isVisible() )
	{
		width = qMax( width, wPathRight->minimumSizeHint().width() );
	}
	
	wSearchRight->setMinimumWidth( width );
	wReplaceRight->setMinimumWidth( width );
	wPathRight->setMinimumWidth( width );
}

void SearchWidget::initializeProperties()
{
	QTime tracker;
	tracker.start();
	
	mProperties.searchText = cbSearch->currentText();
	mProperties.replaceText = cbReplace->currentText();
	mProperties.searchPath = cbPath->currentText();
	mProperties.mode = mMode;
	mProperties.mask.clear();
	mProperties.codec = cbCodec->currentText();
	mProperties.options = SearchAndReplaceV2::Options( SearchAndReplaceV2::ModeNo );
	mProperties.openedFiles.clear();
	mProperties.project = MonkeyCore::fileManager()->currentProject();
	mProperties.sourcesFiles.clear();
	
	// update masks
	foreach ( const QString& part, cbMask->currentText().split( " ", QString::SkipEmptyParts ) )
	{
		mProperties.mask << part;
	}
	
	// update options
	foreach ( const SearchAndReplaceV2::Option& option, mOptionActions.keys() )
	{
		QAction* action = mOptionActions[ option ];
		
		if ( action->isChecked() )
		{
			mProperties.options |= option;
		}
	}
	
	// update opened files
	foreach ( pAbstractChild* document, MonkeyCore::workspace()->documents() )
	{
		mProperties.openedFiles[ document->filePath() ] = document->fileBuffer();
	}
	
	// update project
	mProperties.project = mProperties.project ? mProperties.project->topLevelProject() : 0;
	
	// update sources files
	mProperties.sourcesFiles = mProperties.project ? mProperties.project->topLevelProjectSourceFiles() : QStringList();
	
	qWarning() << "Initialize properties in:" << tracker.elapsed() /1000.0;
}

void SearchWidget::showMessage( const QString& status )
{
	if ( !status.isNull() )
	{
		MonkeyCore::mainWindow()->statusBar()->showMessage( tr( "Search: %1" ).arg( status ), 30000 );
	}
	else
	{
		MonkeyCore::mainWindow()->statusBar()->clearMessage();
	}
}

void SearchWidget::setState( SearchWidget::InputField field, SearchWidget::State state )
{
	QWidget* widget = 0;
	QColor color = QColor( Qt::white );
	
	switch ( field )
	{
		case SearchWidget::Search:
			widget = cbSearch->lineEdit();
			break;
		case SearchWidget::Replace:
			widget = cbReplace->lineEdit();
			break;
	}
	
	switch ( state )
	{
		case SearchWidget::Normal:
			color = QColor( Qt::white );
			break;
		case SearchWidget::Good:
			color = QColor( Qt::green );
			break;
		case SearchWidget::Bad:
			color = QColor( Qt::red );
			break;
	}
	
	QPalette pal = widget->palette();
	pal.setColor( widget->backgroundRole(), color );
	widget->setPalette( pal );
}

bool SearchWidget::searchFile( bool forward )
{
	pAbstractChild* document = MonkeyCore::workspace()->currentDocument();
	pChild* child = document ? static_cast<pChild*>( document ) : 0;
	pEditor* editor = child ? child->editor() : 0;
	
	if ( !editor )
	{
		setState( SearchWidget::Search, SearchWidget::Normal );
		showMessage( tr( "No active editor" ) );
		return false;
	}

	// get cursor position
	int x, y;
	editor->getCursorPosition( &y, &x );

	if ( !forward )
	{
		int temp;
		editor->getSelection( &y, &x, &temp, &temp );
	}

	// search
	const bool found = editor->findFirst( mProperties.searchText, mProperties.options & SearchAndReplaceV2::OptionRegularExpression, mProperties.options & SearchAndReplaceV2::OptionCaseSensitive, mProperties.options & SearchAndReplaceV2::OptionWholeWord, mProperties.options & SearchAndReplaceV2::OptionWrap, forward, y, x );

	// change background acording to found or not
	setState( SearchWidget::Search, found ? SearchWidget::Good : SearchWidget::Bad );
	
	// show message if needed
	showMessage( found ? QString::null : tr( "Not Found" ) );

	// return found state
	return found;
}

void SearchWidget::groupMode_triggered( QAction* action )
{
	setMode( mModeActions.key( action ) );
}

void SearchWidget::on_pbPrevious_clicked()
{
	initializeProperties();
	searchFile( false );
}

void SearchWidget::on_pbNext_clicked()
{
	initializeProperties();
	searchFile( true );
}

void SearchWidget::on_pbSearch_clicked()
{
	initializeProperties();
	mSearchThread->search( mProperties );
}

void SearchWidget::on_pbReplace_clicked()
{
	initializeProperties();
}

void SearchWidget::on_pbReplaceAll_clicked()
{
	initializeProperties();
}

void SearchWidget::on_pbReplaceChecked_clicked()
{
	initializeProperties();
}

void SearchWidget::on_pbBrowse_clicked()
{
}