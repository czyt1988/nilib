/*c++读取labview的tdms、tdm文件
 *日期：2014-02-03
 *作者：尘中远
 *email：czy.t@163.com
 *blog:http://blog.csdn.net/czyt1988
 */

#ifndef QTDM_H
#define QTDM_H

#include <QObject>
#include <QDebug>
#include <QMap>
#include <QList>
#include <list>
//#include "nilib/nilibddc.h"
#include <nilibddc.h>
#include <QVariant>
#include <QDateTime>
#include <qstandarditemmodel.h>
#include <czyQArrayEx.h>
#define TREE_ITEM_ROLE_DDC_Handle Qt::UserRole + 90
#define TREE_ITEM_ROLE_DDC_TYPE Qt::UserRole + 95 //用于区分是组还是通道DDC_TYPE_GROUP,DDC_TYPE_CHANNEL
#define TREE_ITEM_ROLE_DDC_DESCRIPTION Qt::UserRole + 100
#define TREE_ITEM_ROLE_DDC_DATA_STRUCT_TYPE Qt::UserRole + 101//标定数据的结构,DataTypeMark,等同DDCDataType

#define DDC_MY_TYPE_NO -1
#define DDC_MY_TYPE_FILE 0
#define DDC_MY_TYPE_GROUP 1
#define DDC_MY_TYPE_CHANNEL 2

typedef DDCDataType DataTypeMark;

class tdm_group_struct
{
public:
	tdm_group_struct(){}
	~tdm_group_struct(){}
	DDCChannelGroupHandle getGroupHandle(){
		return m_HGroup;
	}
	void setGroupHandle(DDCChannelGroupHandle group){
		m_HGroup = group;
	}
	QList<DDCChannelHandle> getChannels(){
		return m_channels;
	}
	void setChannels(QList<DDCChannelHandle> channelHandles){
		m_channels = channelHandles;
	}
	void appendChannelHandle(DDCChannelHandle channelHandle){
		m_channels.append(channelHandle);
	}
	size_t channelCounts(){
		return m_channels.size();
	}
private:
	DDCChannelGroupHandle m_HGroup;
	QList<DDCChannelHandle> m_channels;
};

class tdmFileStruct 
{
public:
	tdmFileStruct(){}
	~tdmFileStruct(){}
	DDCFileHandle getFileHandle() const{
		return m_HFile;
	}
	void setFileHandle(DDCFileHandle file){
		m_HFile = file;
	}
	size_t groupCounts() const{
		return m_Group.size();
	}
	void setGroupsStruct(QList<tdm_group_struct> groups){
		m_Group = groups;
	}
	tdm_group_struct& appendGroup(DDCChannelGroupHandle group){
		tdm_group_struct gs;
		gs.setGroupHandle(group);
		m_Group.append(gs);
		return m_Group.last();
	}
	tdm_group_struct groupStruct(size_t index){
		return m_Group.value(index);
	}
	QList<tdm_group_struct> groupStructs(){
		return m_Group;
	}
	QList<DDCChannelGroupHandle> groups(){
		QList<DDCChannelGroupHandle> gs;
		size_t count = m_Group.size();
		gs.reserve(count);
		for (size_t i=0;i<count;++i)
		{
			gs.append(m_Group[i].getGroupHandle());
		}
		return gs;
	}
	//根据DDCChannelGroupHandle获取对应的channels
	QList<DDCChannelHandle> channels(DDCChannelGroupHandle g)
	{
		size_t count = m_Group.size();
		for (size_t i=0;i<count;++i)
		{
			if(m_Group[i].getGroupHandle() == g)
				return m_Group[i].getChannels();
		}
		return QList<DDCChannelHandle>();
	}
	//根据index获取对应的channels
	QList<DDCChannelHandle> channelsByIndex(size_t index)
	{
		return m_Group[index].getChannels();
	}
	tdm_group_struct& operator[](int i)
	{
		return m_Group[i];
	}
	tdm_group_struct& lastGroup()
	{
		return m_Group.last();
	}
private:
	DDCFileHandle m_HFile;
	QList<tdm_group_struct> m_Group;
};
///
/// \brief The QTDM class
///pro加入 LIBS+= "f:/Net Disk/qt/tdmsTest/nilib/nilibddc.lib"
class QTDM
{
public:
    QTDM();
	~QTDM();
public:
    bool load(const QString& filePath, bool readOnly = true);
    bool loadTdms(const QString& filePath,bool readOnly = true);
    bool loadTdm(const QString& filePath,bool readOnly = true);
	void closeFile();
	///
    /// \brief 直接使用现有的DDCFileHandle，不用从文件加载
    /// \param file 文件句柄
	/// \param isCloseOld 如果有旧的已经打开的文件，则关闭
    ///
	void setFileHandle(DDCFileHandle file,bool isCloseOld=true)
	{
		if(isCloseOld)
			closeFile();
		m_openFile = file;
	}
	DDCFileHandle getFileHandle()
	{
		return m_openFile;
	}
    ///
    /// \brief 获取最后一次错误信息
    /// \return
    ///
    QString lastErrorString();

    ///
    /// \brief 设置/获取记录的最大错误数
    /// \return
    ///
    int getErrCodesNum()
    {
        return m_errCodesNum;
    }
    void setErrCodesNum(int errCodesNum)
    {
        m_errCodesNum = errCodesNum;
    }
    ///
    /// \brief 判断文件是否打开
    /// \return
    ///
    bool isFileOPen()
    {
        return (m_openFile != 0);
    }

	tdmFileStruct createFileStruct();
	tdmFileStruct getFileStruct(){
		return m_file_struct;
	}
    ///
    /// \brief 获取文件的属性
    /// \param file 文件句柄
    /// \param propertyName 属性名字
    /// \return 返回属性内容
    /// \see GetFilePropertys
    QString getFilePropertyByName(DDCFileHandle file,const char* propertyName);
	QString getFileProperty_Name();
	QString getFileProperty_Description();
	QString getFileProperty_Title();
	QString getFileProperty_Author();
	static bool getFilePropertyByName_s(DDCFileHandle file,const char* propertyName,QString& out_res);
    ///
    /// \brief 获取文件属性内容
    /// \param file 文件句柄
    /// \return
    ///
    QMap<char*,QString> GetFilePropertys();

    ///
    /// \brief 获取组数目
    /// \return
    ///
    unsigned int getGroupNums();
    ///
    /// \brief 获取所有组的句柄
	/// 
	/// 注意！每次使用getgroup或者getchannel这样的函数，返回的句柄是不一样的，因为句柄获取API每次都会分配一个新句柄
	/// 以便在多线程中使用，所有定义了tdm_group_struct等结构体给予对某次获取的保存
    /// \return 获取所有的组句柄
    /// \see tdm_group_struct
    QList<DDCChannelGroupHandle> getGroups();


    ///
    /// \brief 根据属性名获取组的信息
    /// \param grouphandle 组的句柄
    /// \param propertyName 属性名DDC_CHANNELGROUP_NAME和DDC_CHANNELGROUP_DESCRIPTION
    /// \return 信息
    ///
    QString getGroupPropertyByName(DDCChannelGroupHandle grouphandle,const char* propertyName);
    static QString getGroupPropertyByName_s(DDCChannelGroupHandle grouphandle,const char* propertyName);
    ///
    /// \brief 获取组的属性
    /// \param grouphandle 组的句柄
    /// \return 对应DDC_CHANNELGROUP_NAME和DDC_CHANNELGROUP_DESCRIPTION为主键的map
    ///
    QMap<char*,QString> getGroupPropertys(DDCChannelGroupHandle grouphandle);
    QMap<char*,QString> getGroupPropertys(const unsigned int groupIndex);
    ///
    /// \brief 获取某组的通道数
    /// \param group 组序号
    /// \return 通道总数
    ///
    unsigned int getChannelNums(unsigned int groupIndex);
    unsigned int getChannelNums(DDCChannelGroupHandle grouphandle);
	static unsigned int getChannelNums_s(DDCChannelGroupHandle grouphandle);

    ///
    /// \brief 获取grouphandle下的所有通道
    /// \return 通道的list
    ///
    QList<DDCChannelHandle> getChannels(DDCChannelGroupHandle grouphandle);
    QList<DDCChannelHandle> getChannels(unsigned int groupIndex);
	static int getChannels_s(DDCChannelGroupHandle grouphandle,QList<DDCChannelHandle>& channels);
    ///
    /// \brief 通过定义的名字，获取通道的属性
    /// \param channelHandle 通道句柄
    /// \param propertyName 属性名字，DDC_CHANNEL_NAME，DDC_CHANNEL_DESCRIPTION，DDC_CHANNEL_UNIT_STRING
    /// \return
    ///
    QString getChannelStringPropertyByName(DDCChannelHandle channelHandle,const char* propertyName);
    static QString getChannelStringPropertyByName_s(DDCChannelHandle channelHandle,const char* propertyName);
    ///
    /// \brief 获取通道的所有默认属性
    /// \param channelHandle 通道句柄
    /// \return DDC_CHANNEL_NAME，DDC_CHANNEL_DESCRIPTION，DDC_CHANNEL_UNIT_STRING
    ///
    QMap<char*,QString> getChannelStringPropertys(DDCChannelHandle channelHandle);
	QString getChannelName(DDCChannelHandle channelHandle);
	static int getChannelName_s(DDCChannelHandle channelHandle,QString& output_name);

    ///
    /// \brief 获取通道属性类型
    /// \param channelHandle 通道句柄
    /// \param propertyName 属性名
    /// \return 返回DDCDataType的属性类型标志
    ///
    DDCDataType getChannelPropertyType(DDCChannelHandle channelHandle,const char* propertyName);
    ///
    /// \brief 获取通道的数据总数
    /// \param channelHandle 通道句柄
    /// \return 返回通道的数据总数
    ///
    unsigned __int64 getChannelDataNums(DDCChannelHandle channelHandle);
    ///
    /// \brief 获取通道数据，这个函数将会自动对数据的类型进行判断
    /// \param channelHandle通道句柄
    /// \return
    ///
    QVector<QVariant> getChannelDataValues(DDCChannelHandle channelHandle);
    QVector<QVariant> getChannelDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length);
	QVariant getChannelDataValue(DDCChannelHandle channelHandle,size_t index);
	static QVariant getChannelDataValue_s(DDCChannelHandle channelHandle,size_t index);
	static int getChannelDataValues_s(DDCChannelHandle channelHandle,QVector<QVariant>& outputDatas);
	static int getChannelDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<QVariant>& outputDatas);   
	//静态模板函数，实现需要在头文件中写
	template <typename T>
    static	QVector<T> getChannelDataValues_s(DDCChannelHandle channelHandle
		,size_t firstIndex
		,size_t length)
	{
		QVector<QVariant> datas;
		if(DDC_NoError == getChannelDataValues_s(channelHandle,firstIndex,length,datas))
            return czy::QArray::vectorVariant2vectorType<T>(datas);
		return QVector<T>();
	}
		
	template <typename T>
    static QVector<T> getChannelDataValues_s(DDCChannelHandle channelHandle)
	{
		unsigned __int64 dataLength = getChannelDataNums_s(channelHandle);
        return getChannelDataValues_s<T>(channelHandle,0,dataLength);
	}
	///
    /// \brief 获取通道的double数据,若使用没有 firstIndex和length的函数，则是全部获取
    /// \param channelHandle通道句柄
    /// \return
    ///    
    QVector<double> getChannelDoubleDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length);
    QVector<double> getChannelDoubleDataValues(DDCChannelHandle channelHandle);
	static int getChannelDoubleDataValues_s(DDCChannelHandle channelHandle,QVector<double>& outputDatas);
	static int getChannelDoubleDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<double>& outputDatas);

    QVector<float> getChannelFloatDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length);
    QVector<float> getChannelFloatDataValues(DDCChannelHandle channelHandle);
	static int getChannelFloatDataValues_s(DDCChannelHandle channelHandle,QVector<float>& outputDatas);
	static int getChannelFloatDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<float>& outputDatas);

    QVector<QString> getChannelStringDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length);
    QVector<QString> getChannelStringDataValues(DDCChannelHandle channelHandle);
	static int getChannelStringDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<QString>& outputDatas);
	static int getChannelStringDataValues_s(DDCChannelHandle channelHandle,QVector<QString>& outputDatas);

    QVector<long> getChannelLongDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length);
    QVector<long> getChannelLongDataValues(DDCChannelHandle channelHandle);
	static int getChannelLongDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<long>& outputDatas);
	static int getChannelLongDataValues_s(DDCChannelHandle channelHandle,QVector<long>& outputDatas);

    QVector<short> getChannelShortDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length);
    QVector<short> getChannelShortDataValues(DDCChannelHandle channelHandle);
	static int getChannelShortDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<short>& outputDatas);
	static int getChannelShortDataValues_s(DDCChannelHandle channelHandle,QVector<short>& outputDatas);

    QVector<unsigned char> getChannelUCharDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length);
    QVector<unsigned char> getChannelUCharDataValues(DDCChannelHandle channelHandle);
	static int getChannelUCharDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<unsigned char>& outputDatas);
	static int getChannelUCharDataValues_s(DDCChannelHandle channelHandle,QVector<unsigned char>& outputDatas);

    QVector<QDateTime> getChannelDateTimeDataValues(DDCChannelHandle channelHandle);
    QVector<QDateTime> getChannelDateTimeDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length);
	static int getChannelDateTimeDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<QDateTime>& outputDatas);
	static int getChannelDateTimeDataValues_s(DDCChannelHandle channelHandle,QVector<QDateTime>& outputDatas);
    ///
    /// \brief 获取数据类型
    /// \param channelHandle通道句柄
    /// \return
    ///
    DDCDataType getDataType(DDCChannelHandle channelHandle);
    static DDCDataType getDataType_s(DDCChannelHandle channelHandle);
	QString getDataTypeToString(DDCChannelHandle channelHandle);
public:
	DDCFileHandle setInStandardItem(tdmFileStruct fileStruct
		,QStandardItem* item
		,bool isCheckabled = false
		,QIcon* icoGroup = nullptr
		,QIcon* icoChannel = nullptr);
	
	static bool isStandardItemHaveTypeMark(const QStandardItem* item);
	static bool isStandardItemHaveHandleMark(const QStandardItem* item);
	static bool isStandardItemFile(const QStandardItem* item);
	static bool isStandardItemGroup(const QStandardItem* item);
	static bool isStandardItemChannel(const QStandardItem* item);
	static int  getStandardItemTypeMark(const QStandardItem* item);
	static DDCFileHandle getStandardItemFileHandle(const QStandardItem* item);
	static DDCChannelGroupHandle getStandardItemGroupHandle(const QStandardItem* item);
	static DDCChannelHandle getStandardItemChannelHandle(const QStandardItem* item);

	static bool isStandardItemHaveTypeMark(const QModelIndex item);
	static bool isStandardItemHaveHandleMark(const QModelIndex item);
	static bool isStandardItemFile(const QModelIndex item);
	static bool isStandardItemGroup(const QModelIndex item);
	static bool isStandardItemChannel(const QModelIndex item);
	static int  getStandardItemTypeMark(const QModelIndex item);
	static DDCFileHandle getStandardItemFileHandle(const QModelIndex item);
	static DDCChannelGroupHandle getStandardItemGroupHandle(const QModelIndex item);
	static DDCChannelHandle getStandardItemChannelHandle(const QModelIndex item);

	static QString dataTypeToString(const DDCDataType dataType);
public:

	static unsigned __int64 getChannelDataNums_s(DDCChannelHandle channelHandle);
signals:
    
public slots:
	void printAllError()
	{
		for (auto ite = m_errCodes.begin();ite != m_errCodes.end();++ite)
		{
			qDebug()<<"errCode:"<<*ite<<":"<<errcode2String(*ite);
		}
	}
private:
    ///
    /// \brief 捕获错误代码
    /// \param errcode
    ///
    void remember_err_code(int errcode)
    {
        if(0 == errcode)
            return;
        m_errCodes.push_back(errcode);
        if(m_errCodes.size()>m_errCodesNum)
            m_errCodes.pop_front();
        //print_errcode(errcode);
    }

    ///
    /// \brief 把错误代码转换为文字
    /// \param errcode
    /// \return
    ///
    QString errcode2String(int errcode);
    ///
    /// \brief 打印错误代码 release模式下不输出
    /// \param errcode
    ///
    void print_errcode(int errcode)
    {
        qDebug()<<"errCode:"<<errcode;
        qDebug()<<errcode2String(errcode);
    }

private:
    std::list<int> m_errCodes;///< 记录出现的错误代码
    size_t m_errCodesNum;///< 记录的错误代码数量
    DDCFileHandle m_openFile;///<打开的文件句柄
	tdmFileStruct m_file_struct;
};

#endif // QTDM_H
