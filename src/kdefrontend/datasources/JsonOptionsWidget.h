#ifndef JSONOPTIONSWIDGET_H
#define JSONOPTIONSWIDGET_H

#include <backend/datasources/filters/JsonFilter.h>
#include "ui_jsonoptionswidget.h"

class ImportFileWidget;
class JsonFilter;
class QJsonModel;
class QJsonTreeItem;

class JsonOptionsWidget : public QWidget {
	Q_OBJECT

public:
	explicit JsonOptionsWidget(QWidget*, ImportFileWidget*);
	void applyFilterSettings(JsonFilter*) const;
	void clear();
	void loadSettings() const;
	void saveSettings();
	void loadDocument(QString filename);

private slots:
	void indexChanged();

private:
	void setTooltips();
	QVector<int> getIndexRows(const QModelIndex&) const;

	QString m_filename;
	Ui::JsonOptionsWidget ui;
	ImportFileWidget* m_fileWidget;
	QJsonModel* m_model;
};

#endif
