#include "qCtagsSenseSQL.h"

#include <QStringList>
#include <QSqlError>
#include <QDebug>

const QString qCtagsSenseSQL::mDBFilePath = "/home/pasnox/Development/Qt4/mks/classexplorer/qCtagsSense.sqlite";

qCtagsSenseSQL::qCtagsSenseSQL( QObject* parent )
	: QObject( parent )
{
	mDBConnectionName = "qCtagsSenseSQL";
}

qCtagsSenseSQL::~qCtagsSenseSQL()
{
}

bool qCtagsSenseSQL::initializeDatabase()
{
	QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", mDBConnectionName );
	
	//db.setDatabaseName( qCtagsSenseSQL::mDBFilePath );
	db.setDatabaseName( ":memory:" );
	//db.setConnectOptions( "QSQLITE_BUSY_TIMEOUT=500" ); // in milliseconds
	
	if ( !db.open() )
	{
		qWarning() << db.lastError().text();
		return false;
	}
	
	if ( !db.tables().contains( "files", Qt::CaseInsensitive ) )
	{
		if ( !initializeTables() )
		{
			db.close();
			QSqlDatabase::removeDatabase( mDBConnectionName );
			qWarning() << "Can't initialize tables";
			return false;
		}
	}
	
	return true;
}

bool qCtagsSenseSQL::initializeTables() const
{
	const QString sql_files = QString(
		"CREATE TABLE 'main'.'files' ("
		"'id' INTEGER PRIMARY KEY AUTOINCREMENT,"
		"'fileName' TEXT NOT NULL,"
		"'language' TEXT NOT NULL"
		");"
	);
	
	const QString sql_entries = QString(
		"CREATE TABLE 'main'.'entries' ("
		"'id' INTEGER PRIMARY KEY AUTOINCREMENT,"
		"'file_id' INTEGER NOT NULL,"
		"'line_number_entry' INTEGER NOT NULL,"
		"'line_number' INTEGER NOT NULL,"
		"'is_file_scope' INTEGER NOT NULL,"
		"'is_file_entry' INTEGER NOT NULL,"
		"'truncate_line' INTEGER NOT NULL,"
		"'name' TEXT NOT NULL,"
		"'kind' INTEGER NOT NULL,"
		"'access' TEXT,"
		"'file_scope' TEXT,"
		"'implementation' TEXT,"
		"'inheritance' TEXT,"
		"'scope_value' TEXT,"
		"'scope_key' TEXT,"
		"'signature' TEXT,"
		"'type' TEXT,"
		"'type_name' TEXT"
		");"
	);
	
	const QString sql_files_trigger = QString(
		"CREATE TRIGGER 'files_after_delete' AFTER DELETE ON 'files' FOR EACH ROW\n"
		"BEGIN\n"
		"DELETE FROM 'entries' WHERE 'entries'.'file_id' = 'old'.'id';\n"
		"END;"
	);
	
	const QString sql_files_filename_asc = QString(
		"CREATE INDEX 'files_filename_asc' on 'files' (fileName ASC)"
	);
	
	const QString sql_files_language_asc = QString(
		"CREATE INDEX 'files_language_asc' on 'files' (language ASC)"
	);
	
	const QString sql_entries_name_asc = QString(
		"CREATE INDEX 'entries_name_asc' on 'entries' (name ASC)"
	);
	
	const QString sql_entries_scope_key_asc = QString(
		"CREATE INDEX 'entries_scope_key_asc' on 'entries' (scope_key ASC)"
	);
	
	QSqlQuery q = query();
	
	if ( q.exec( sql_files ) )
	{
		if ( q.exec( sql_entries ) )
		{
			if ( q.exec( sql_files_trigger ) )
			{
				/*
				if ( q.exec( sql_files_filename_asc ) )
				{
					if ( q.exec( sql_files_language_asc ) )
					{
						if ( q.exec( sql_entries_name_asc ) )
						{
							if ( q.exec( sql_entries_scope_key_asc ) )
							{
								return true;
							}
							else
							{
								qWarning() << "Can't create scope_key index" << q.lastError().text();
							}
						}
						else
						{
							qWarning() << "Can't create name index" << q.lastError().text();
						}
					}
					else
					{
						qWarning() << "Can't create language index" << q.lastError().text();
					}
				}
				else
				{
					qWarning() << "Can't create filename index" << q.lastError().text();
				}
				*/
				
				return true;
			}
			else
			{
				qWarning() << "Can't create files trigger" << q.lastError().text();
			}
		}
		else
		{
			qWarning() << "Can't create entries table" << q.lastError().text();
		}
	}
	else
	{
		qWarning() << "Can't create files table" << q.lastError().text();
	}
	
	return false;
}

QSqlDatabase qCtagsSenseSQL::database() const
{
	return QSqlDatabase::database( mDBConnectionName, true );
}

QSqlQuery qCtagsSenseSQL::query() const
{
	QSqlQuery q( database() );
	q.setForwardOnly( true );
	
	return q;
}
