#ifndef TDMFILEVIEWER_H
#define TDMFILEVIEWER_H
//#include <TdmFileViewer.h>

#include <QMainWindow>
#include <QTabWidget>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>

#include <memory>

#include <QTDM.h>
#include <tdmfiletablemodel.h>
namespace myUI{

	class TdmFileViewerUI{
	public:
		TdmFileViewerUI();
		void setupUI(QMainWindow* TdmFileViewer);
		QAction *actionSave;
		QWidget *centralWidget;
		QVBoxLayout *verticalLayout;
		QTabWidget *tabWidget;
		QToolBar *mainToolBar;
		QMenuBar *menuBar;
		QMenu *menuFile;
		QDockWidget *dockWidget;
		QWidget *dockWidgetContents;
		QVBoxLayout *verticalLayout_2;
		QTreeView *treeView;

		QTableView* addTab(const QString& tableName);
		QTableView* tableIndex(size_t index);
	};
}

class TdmFileViewer : public QMainWindow
{
    Q_OBJECT
public:
    explicit TdmFileViewer(QWidget *parent = 0);
    void setTDMSFile(QTDM* tdm);
signals:
	//! Emitted when the window was closed
	void closedWindow(TdmFileViewer *);
protected:
	void closeEvent( QCloseEvent *);
private slots:
	void on_treeView_clicked(const QModelIndex &index);
private:
	std::shared_ptr<myUI::TdmFileViewerUI> ui;
	QMap<QString,QIcon> m_publicIconMap;///<
	bool m_confirm_close;
private:
	bool activeTab(DDCChannelGroupHandle gh);
};



#endif // TDMFILEVIEWER_H
