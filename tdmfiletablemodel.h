#ifndef TDMFILETABLEMODEL_H
#define TDMFILETABLEMODEL_H
#include <QAbstractTableModel>
#include <QTDM.h>
class TDMFileTableModel : public QAbstractTableModel
{
public:
    TDMFileTableModel(QObject *parent = 0):QAbstractTableModel(parent){}
	///
	/// \brief设置组句柄给模型
	/// \param group_handle
	void setChannelGroupHandle(DDCChannelGroupHandle group_handle){
		this->m_groupHandle = group_handle;
		m_channelHandles.clear();
		QTDM::getChannels_s(group_handle,m_channelHandles);
		m_header.clear();
		QString name;
		//获取最大行数和记录各个通道名称作为表头
		QVector<unsigned int> rows;
		for (auto i=0;i<m_channelHandles.size();++i)
		{
			QTDM::getChannelName_s(m_channelHandles[i],name);
			rows.append(QTDM::getChannelDataNums_s(m_channelHandles[i]));
			m_header.append(name);
		}
		m_maxRowCount = *(std::max_element(rows.begin(),rows.end()));
	}
	QVariant headerData(int section, Qt::Orientation orientation,int role) const
	{
		if (role != Qt::DisplayRole)
			return QVariant();
		if(Qt::Horizontal == orientation){//说明是水平表头
			if(section<m_header.size())
				return m_header.at(section);
			return QVariant();
		}
		else if(Qt::Vertical == orientation){//垂直表头
			return section+1;
		}
		return QVariant();
	}

	int rowCount(const QModelIndex &parent) const
	{
		return m_maxRowCount;
	}
	int columnCount(const QModelIndex &parent) const
	{
		return m_channelHandles.size();
	}
	QVariant data(const QModelIndex &index, int role) const
	{
		if (!index.isValid())  
			return QVariant();  

		if (role == Qt::TextAlignmentRole) {  
			return int(Qt::AlignRight | Qt::AlignVCenter);  
		} 
		else if (role == Qt::DisplayRole) {  
			if (index.row() >= m_maxRowCount
				||
				index.column() >= m_channelHandles.size())
			{
				return QVariant(); 
			}
			return QTDM::getChannelDataValue_s(
				m_channelHandles[index.column()]
				,index.row());
		}  
		return QVariant();  
	}
	DDCChannelGroupHandle groupHandle(){return m_groupHandle;}
private:
	DDCChannelGroupHandle m_groupHandle;
	unsigned int m_maxRowCount;
	QStringList m_header;
	QList<DDCChannelHandle> m_channelHandles;
 };

#endif // TDMFILETABLEMODEL_H
