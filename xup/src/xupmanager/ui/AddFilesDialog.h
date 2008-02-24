#ifndef ADDFILESDIALOG_H
#define ADDFILESDIALOG_H

#include <QFileDialog>
#include <QGroupBox>
#include <QComboBox>

#include "ScopedProjectItemModel.h"
#include "ProjectItemModel.h"
#include "ProjectItem.h"

#include "pTreeComboBox.h"

class AddFilesDialog : public QFileDialog
{
	Q_OBJECT

public:
	AddFilesDialog( ScopedProjectItemModel* scopedmodel, ProjectItem* selecteditem, QWidget* = 0 );

	inline ProjectItem* currentItem() const { return qobject_cast<ProjectItemModel*>( mScoped->sourceModel() )->itemFromIndex( mScoped->mapToSource( tcbProjects->currentIndex() ) ); }
	inline QString currentOperator() const { return cbOperators->currentText(); }

protected:
	ScopedProjectItemModel* mScoped;

	pTreeComboBox* tcbProjects;
	QComboBox* cbOperators;
};

#endif // ADDFILESDIALOG_H
