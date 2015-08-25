#include "TdmFileViewer.h"
#include <tdmfiletablemodel.h>
#include "../def_str.h"
#include <QMessageBox>
#include <QCloseEvent>
myUI::TdmFileViewerUI::TdmFileViewerUI(){
}

void myUI::TdmFileViewerUI::setupUI(QMainWindow* TdmFileViewer){
	if (TdmFileViewer->objectName().isEmpty())
		TdmFileViewer->setObjectName(QStringLiteral("TdmFileViewer"));
	TdmFileViewer->resize(576, 422);
	actionSave = new QAction(TdmFileViewer);
	actionSave->setObjectName(QStringLiteral("actionSave"));
	centralWidget = new QWidget(TdmFileViewer);
	centralWidget->setObjectName(QStringLiteral("centralWidget"));
	verticalLayout = new QVBoxLayout(centralWidget);
	verticalLayout->setSpacing(1);
	verticalLayout->setContentsMargins(11, 11, 11, 11);
	verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
	verticalLayout->setContentsMargins(1, 1, 1, 1);
	tabWidget = new QTabWidget(centralWidget);
	tabWidget->setObjectName(QStringLiteral("tabWidget"));
	tabWidget->setTabPosition(QTabWidget::South);
	addTab(QStringLiteral("组1"));

	verticalLayout->addWidget(tabWidget);

	TdmFileViewer->setCentralWidget(centralWidget);
	mainToolBar = new QToolBar(TdmFileViewer);
	mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
	TdmFileViewer->addToolBar(Qt::TopToolBarArea, mainToolBar);
	menuBar = new QMenuBar(TdmFileViewer);
	menuBar->setObjectName(QStringLiteral("menuBar"));
	menuBar->setGeometry(QRect(0, 0, 576, 23));
	menuFile = new QMenu(menuBar);
	menuFile->setObjectName(QStringLiteral("menuFile"));
	TdmFileViewer->setMenuBar(menuBar);
	dockWidget = new QDockWidget(TdmFileViewer);
	dockWidget->setObjectName(QStringLiteral("dockWidget"));
	dockWidgetContents = new QWidget();
	dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
	verticalLayout_2 = new QVBoxLayout(dockWidgetContents);
	verticalLayout_2->setSpacing(1);
	verticalLayout_2->setContentsMargins(11, 11, 11, 11);
	verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
	verticalLayout_2->setContentsMargins(1, 1, 1, 1);
	treeView = new QTreeView(dockWidgetContents);
	treeView->setObjectName(QStringLiteral("treeView"));
	treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	verticalLayout_2->addWidget(treeView);

	dockWidget->setWidget(dockWidgetContents);
	TdmFileViewer->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);

	mainToolBar->addAction(actionSave);
	menuBar->addAction(menuFile->menuAction());
	menuFile->addAction(actionSave);


	tabWidget->setCurrentIndex(0);
	TdmFileViewer->setWindowTitle(QApplication::translate("TdmFileViewer", "TdmFileViewer", 0));
	actionSave->setText(QApplication::translate("TdmFileViewer", "\344\277\235\345\255\230", 0));
	actionSave->setIconText(QApplication::translate("TdmFileViewer", "\344\277\235\345\255\230", 0));
	//tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("TdmFileViewer", "\347\273\2041", 0));
	menuFile->setTitle(QApplication::translate("TdmFileViewer", "\346\226\207\344\273\266", 0));
	dockWidget->setWindowTitle(QApplication::translate("TdmFileViewer", "\346\226\207\344\273\266\347\273\223\346\236\204", 0));

	QMetaObject::connectSlotsByName(TdmFileViewer);
}

QTableView* myUI::TdmFileViewerUI::addTab(const QString& tableName)
{
	int n = tabWidget->count();
	QTableView* tab = new QTableView();
	tab->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tab->setSelectionBehavior(QAbstractItemView::SelectRows);
	tab->setSelectionMode(QAbstractItemView::SingleSelection);
	tab->setCornerButtonEnabled(false);

	tabWidget->addTab(tab, tableName);
	return tab;
}

QTableView* myUI::TdmFileViewerUI::tableIndex(size_t index)
{
	return qobject_cast<QTableView*>(tabWidget->widget(index));
}

TdmFileViewer::TdmFileViewer(QWidget *parent) :
    QMainWindow(parent)
{
	ui = std::make_shared<myUI::TdmFileViewerUI>();
	ui->setupUI(this);
	setAttribute(Qt::WA_DeleteOnClose);
	m_confirm_close = true;
	m_publicIconMap[TREE_ITEM_ICON_Project] = QIcon(QStringLiteral(":/treeItemIcon/res_treeItemIcon/Project.png"));
	m_publicIconMap[TREE_ITEM_ICON_folder] = QIcon(QStringLiteral(":/treeItemIcon/res_treeItemIcon/folder.png"));
	m_publicIconMap[TREE_ITEM_ICON_folderAnsys] = QIcon(QStringLiteral(":/treeItemIcon/res_treeItemIcon/folder-ansys.png"));
	m_publicIconMap[TREE_ITEM_ICON_TdmsGroup] = QIcon(QStringLiteral(":/treeItemIcon/res_treeItemIcon/group.png"));
	m_publicIconMap[TREE_ITEM_ICON_TdmsChannel] = QIcon(QStringLiteral(":/treeItemIcon/res_treeItemIcon/channel.png"));
	m_publicIconMap[TREE_ITEM_ICON_folderOriginal] = QIcon(QStringLiteral(":/treeItemIcon/res_treeItemIcon/folder_original.png"));
	m_publicIconMap[TREE_ITEM_ICON_DataItem] = QIcon(QStringLiteral(":/treeItemIcon/res_treeItemIcon/dataItem.png"));
	m_publicIconMap[TREE_ITEM_ICON_Spectrum] = QIcon(QStringLiteral(":/treeItemIcon/res_treeItemIcon/spectrum.png"));

	//connect(ui->treeView,&QAbstractItemView::clicked,this,&TdmFileViewer::on_treeView_clicked);
}

void TdmFileViewer::setTDMSFile(QTDM* tdm)
{
	if (nullptr == tdm)
	{
		Q_CHECK_PTR(tdm);
		return;
	}
	ui->tabWidget->clear();

	QList<DDCChannelGroupHandle> gh = tdm->getFileStruct().groups();
	for (int i=0;i<gh.size();++i)
	{
		QTableView* table 
			= ui->addTab(tdm->getGroupPropertyByName(gh[i],DDC_CHANNELGROUP_NAME));
		if(table){
			TDMFileTableModel* model = new TDMFileTableModel(table);
			model->setGroupHandle(gh[i]);
			table->setModel(model);
		}
	}
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->treeView->model());
	if (nullptr == model)
	{
		model = new QStandardItemModel(ui->treeView);
		ui->treeView->setModel(model);
	}
	QStandardItem* topItem = new QStandardItem(m_publicIconMap[TREE_ITEM_ICON_Project],tdm->getFileProperty_Name());
	DDCFileHandle file = tdm->setInStandardItem(
		tdm->getFileStruct()
		,topItem
		,false
		,&m_publicIconMap[TREE_ITEM_ICON_TdmsGroup]
		,&m_publicIconMap[TREE_ITEM_ICON_TdmsChannel]
	);
	topItem->setData(QVariant(DDC_MY_TYPE_FILE),TREE_ITEM_ROLE_DDC_TYPE);
    topItem->setData(QVariant::fromValue(reinterpret_cast<quintptr>(file)),TREE_ITEM_ROLE_DDC_Handle);
	model->appendRow(topItem);

}

void TdmFileViewer::on_treeView_clicked(const QModelIndex &index)
{
	qDebug()<<"!";
	QVariant var = index.data(TREE_ITEM_ROLE_DDC_TYPE);
	if(!var.isValid())
		return;
	int type = var.toInt();
    quintptr p = 0;
	DDCChannelGroupHandle gH = nullptr;
	DDCChannelHandle cH = nullptr;
	switch(type)
	{
	case DDC_MY_TYPE_FILE:
		break;
	case DDC_MY_TYPE_GROUP:
		{
            p = index.data(TREE_ITEM_ROLE_DDC_Handle).value<quintptr>();
            gH = reinterpret_cast<DDCChannelGroupHandle>(p);
			activeTab(gH);
		}
		break;
	case DDC_MY_TYPE_CHANNEL:
		{
            p = index.data(TREE_ITEM_ROLE_DDC_Handle).value<quintptr>();
			cH = DDCChannelHandle(p);
            quintptr gp = index.parent().data(TREE_ITEM_ROLE_DDC_Handle).value<quintptr>();
            gH = reinterpret_cast<DDCChannelGroupHandle>(gp);
			activeTab(gH);
		}
		break;
	default:
		break;
	}

}
//没实现，发现model索引的group和tree里的不一致
//todo :把tree里的标示出来
bool TdmFileViewer::activeTab(DDCChannelGroupHandle gh)
{
	int tabNums = ui->tabWidget->count();
	for (int i=0;i<tabNums;++i)
	{
		QTableView* tView = qobject_cast<QTableView*>(ui->tabWidget->widget(i));
		if (tView)
		{
			TDMFileTableModel* model = static_cast<TDMFileTableModel*>(tView->model());
			if(model->groupHandle() == gh){
				ui->tabWidget->setCurrentWidget(ui->tabWidget->widget(i));
				return true;
			}
		
		}
	}
	return false;
}


void TdmFileViewer::closeEvent( QCloseEvent *e )
{
	if (m_confirm_close){
		switch( 
			QMessageBox::question(this
			, windowTitle()
			,QStringLiteral("是否关闭窗口？"))
			)
		{
		case QMessageBox::Yes:
			emit closedWindow(this);
			e->accept();
			break;
		default:
			e->ignore();
			break;
		}
	} 
	else {
		emit closedWindow(this);
		e->accept();
	}
}
