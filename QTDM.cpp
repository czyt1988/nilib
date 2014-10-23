#include "QTDM.h"
#include <QDir>
#include <memory>
#include "czyQArrayEx.h"

//Q_DECLARE_METATYPE(DDCFile)
//Q_DECLARE_METATYPE(DDCChannelGroup)
//Q_DECLARE_METATYPE(DDCChannel)

QTDM::QTDM()
{
    m_openFile = 0;
    m_errCodesNum = 100;
}
QTDM::~QTDM()
{
	
}
///
/// \brief 加载文件，文件需要以tdm或tdms为后缀，自动判断
/// \param filePath 文件路径
/// \param readOnly 只读模式打开，默认为true
/// \return 成功打开返回true
///
bool QTDM::load(const QString& filePath,bool readOnly)
{
    QFileInfo temDir(filePath);
    QString strSuffix = temDir.suffix();
    strSuffix = strSuffix.toLower();
    if(QStringLiteral("tdms") == strSuffix)
    {
        return loadTdms(filePath,readOnly);
    }
    else if(QStringLiteral("tdm") == strSuffix)
    {
        return loadTdm(filePath,readOnly);
    }
    return false;
}
///
/// \brief 加载tdm文件
/// \param filePath tdm文件路径
/// \param readOnly 只读模式打开，默认为true
/// \return 成功返回true，错误返回false，若需要获取最后错误信息，可以使用lastErrorString函数
///
bool QTDM::loadTdm(const QString& filePath, bool readOnly)
{
    if(m_openFile != 0)
        DDC_CloseFile(m_openFile);
    m_openFile = 0;
    QString path = QDir::toNativeSeparators(filePath);
	qDebug()<<"tdmFile:"<<path;
    int errCode = DDC_OpenFileEx (path.toLocal8Bit().constData(), "TDM",readOnly, &m_openFile);
    remember_err_code(errCode);
    if(errCode < 0)
        return false;
	m_file_struct = createFileStruct();
    return true;
}
///
/// \brief 加载tdms文件
/// \param filePath tdms文件路径
/// \param readOnly 只读模式打开，默认为true
/// \return 成功返回true，错误返回false，若需要获取最后错误信息，可以使用lastErrorString函数
///
bool QTDM::loadTdms(const QString& filePath, bool readOnly)
{

//    DDCFileHandle	ddcFile = 0;
    if(m_openFile != 0)
        DDC_CloseFile(m_openFile);
    m_openFile = 0;
    QString path = QDir::toNativeSeparators(filePath);
	qDebug()<<"tdmsFile:"<<path;
    int errCode = DDC_OpenFileEx (path.toLocal8Bit().constData(), "TDMS",readOnly, &m_openFile);
    remember_err_code(errCode);
    if(errCode < 0)
        return false;
	m_file_struct = createFileStruct();
    return true;
}
/// 
/// \brief 用完一定要关闭，否则不会释放内存
///
void QTDM::closeFile()
{
	if(0 != m_openFile)
        remember_err_code(DDC_CloseFile(m_openFile));
}
/// 
/// \brief 获取整个tdm文件的结构，结构内部包括所有的组合通道的句柄
/// \return tdmFileStruct结构
/// \see tdmFileStruct tdm_group_struct
tdmFileStruct QTDM::createFileStruct()
{
	if(!isFileOPen())
		return tdmFileStruct();
	tdmFileStruct fileStruct;
	fileStruct.setFileHandle(m_openFile);
	
	QList<DDCChannelGroupHandle> groups = getGroups();
	for(auto i=groups.begin();i!= groups.end();++i)
	{
		fileStruct.appendGroup(*i)
			.setChannels(getChannels(*i));
	}
	return fileStruct;
}

QString QTDM::getFilePropertyByName(DDCFileHandle file , const char* propertyName)
{
    unsigned int length(0);
    //获取文件属性 - DDC_FILE_NAME
    int errCode = DDC_GetFileStringPropertyLength (file, propertyName, &length);
    if(errCode < 0)
    {
        remember_err_code(errCode);
        return QString("");
    }
    char* property = nullptr;
    length = (8)*(length + 1);//让它长度有足够
    property = new char[length];
    memset(property,0,sizeof(char)*length);

    errCode = DDC_GetFileProperty (file, propertyName, property, length);
    if(errCode < 0)
        remember_err_code(errCode);

    QString str = QString::fromLocal8Bit(property);

    if(property)
        delete[] property;
    return str;
}

bool QTDM::getFilePropertyByName_s(DDCFileHandle file,const char* propertyName,QString& out_res)
{
	unsigned int length(0);
	//获取文件属性 - DDC_FILE_NAME
	int errCode = DDC_GetFileStringPropertyLength (file, propertyName, &length);
	if(errCode < 0)
	{
		return false;
	}
	std::unique_ptr<char> property = nullptr;
	//char* property = nullptr;
	length = (8)*(length + 1);//让它长度有足够
	property.reset(new char[length]);
	memset(property.get(),0,sizeof(char)*length);

	errCode = DDC_GetFileProperty (file, propertyName, property.get(), length);
	if(errCode < 0)
		return false;

	out_res = QString::fromLocal8Bit(property.get());

// 	if(property)
// 		delete[] property;
	return true;
}

QMap<char*,QString> QTDM::GetFilePropertys()
{
    QMap<char*,QString> filePropertys;
    if(m_openFile<=0)
        return filePropertys;

    filePropertys[DDC_FILE_NAME] = getFilePropertyByName(m_openFile,DDC_FILE_NAME);
    filePropertys[DDC_FILE_DESCRIPTION] = getFilePropertyByName(m_openFile,DDC_FILE_DESCRIPTION);
    filePropertys[DDC_FILE_TITLE] = getFilePropertyByName(m_openFile,DDC_FILE_TITLE);
    filePropertys[DDC_FILE_AUTHOR] = getFilePropertyByName(m_openFile,DDC_FILE_AUTHOR);
    return filePropertys;
}
QString QTDM::getFileProperty_Name()
{
	return getFilePropertyByName(m_openFile,DDC_FILE_NAME);
}
QString QTDM::getFileProperty_Description()
{
	return getFilePropertyByName(m_openFile,DDC_FILE_DESCRIPTION);
}
QString QTDM::getFileProperty_Title()
{
	return getFilePropertyByName(m_openFile,DDC_FILE_TITLE);
}
QString QTDM::getFileProperty_Author()
{
	return getFilePropertyByName(m_openFile,DDC_FILE_AUTHOR);
}

unsigned int QTDM::getGroupNums()
{
    if(!isFileOPen())
        return 0;
    unsigned int groupNum = 0;
    //获取通道组的数目
    remember_err_code(DDC_GetNumChannelGroups(m_openFile, &groupNum));
    return groupNum;
}

QList<DDCChannelGroupHandle> QTDM::getGroups()
{
    QList<DDCChannelGroupHandle> groups;
    if(!isFileOPen())
        return groups;
    unsigned int groupNums = getGroupNums();
    if(groupNums <= 0)
        return groups;
	std::vector<DDCChannelGroupHandle> pGroupHandles(groupNums,0);
    remember_err_code(DDC_GetChannelGroups (m_openFile, pGroupHandles.data(), groupNums));
    //转移
    for(unsigned int i=0;i<groupNums;++i){
        groups.append(pGroupHandles[i]);
    }
    return groups;
}

QString QTDM::getGroupPropertyByName(DDCChannelGroupHandle grouphandle,const char* propertyName)
{
    unsigned int length(0);

    //获取文件属性
    int errCode = DDC_GetChannelGroupStringPropertyLength (grouphandle
		, propertyName
		, &length);
    if(errCode < 0)
    {
        remember_err_code(errCode);
        return QString("");
    }
    char* property = nullptr;
    length = (8)*(length+1);
    property = new char[length];
    memset(property,0,sizeof(char)*length);
    errCode = DDC_GetChannelGroupProperty (grouphandle, propertyName, property, length);
    if(errCode < 0)
        remember_err_code(errCode);
    QString str = QString::fromLocal8Bit(property);
    if(property)
        delete[] property;
    return str;
}

QMap<char*,QString> QTDM::getGroupPropertys(DDCChannelGroupHandle grouphandle)
{
    QMap<char*,QString> groupPropertys;
    if(m_openFile<=0)
        return groupPropertys;
    groupPropertys[DDC_CHANNELGROUP_NAME] = getGroupPropertyByName(grouphandle,DDC_CHANNELGROUP_NAME);
    groupPropertys[DDC_CHANNELGROUP_DESCRIPTION] = getGroupPropertyByName(grouphandle,DDC_CHANNELGROUP_DESCRIPTION);
    return groupPropertys;
}
QMap<char*,QString> QTDM::getGroupPropertys(const unsigned int groupIndex)
{
    QMap<char*,QString> groupPropertys;
    if(m_openFile<=0)
        return groupPropertys;
    QList<DDCChannelGroupHandle> groups = getFileStruct().groups();
    if(groupIndex >= groups.size())
        return groupPropertys;
    DDCChannelGroupHandle groupHandle = groups[groupIndex];
    return getGroupPropertys(groupHandle);
}

unsigned int QTDM::getChannelNums(DDCChannelGroupHandle grouphandle)
{
    if(!isFileOPen())
        return 0;
    unsigned int channelNums(0);
    remember_err_code(DDC_GetNumChannels(grouphandle, &channelNums));//获取通道的数目
    return channelNums;
}
unsigned int QTDM::getChannelNums_s(DDCChannelGroupHandle grouphandle)
{
	unsigned int channelNums(0);
	DDC_GetNumChannels(grouphandle, &channelNums);//获取通道的数目
	return channelNums;
}
unsigned int QTDM::getChannelNums(unsigned int groupIndex)
{
    if(!isFileOPen())
        return 0;
    QList<DDCChannelGroupHandle> groups = getFileStruct().groups();
    if(groupIndex >= groups.size())
        return 0;
    return getChannelNums(groups[groupIndex]);//获取通道的数目
}


QList<DDCChannelHandle> QTDM::getChannels(DDCChannelGroupHandle grouphandle)
{
    QList<DDCChannelHandle> channels;
    if(!isFileOPen())
        return channels;
    unsigned int channelNums = getChannelNums(grouphandle);//获取通道数
    if(channelNums <= 0)
        return channels;
    DDCChannelHandle* channelHandles = new DDCChannelHandle[channelNums];
    remember_err_code(DDC_GetChannels(grouphandle,channelHandles,channelNums));
    for(unsigned int i = 0;i<channelNums;++i){
        channels.append(channelHandles[i]);
    }
    delete[] channelHandles;
    return channels;
}
QList<DDCChannelHandle> QTDM::getChannels(unsigned int groupIndex)
{
    QList<DDCChannelHandle> channels;
    if(m_openFile<=0)
        return channels;
    QList<DDCChannelGroupHandle> groups = getFileStruct().groups();
    if(groupIndex >= groups.size())
        return channels;
    return getChannels(groups[groupIndex]);
}
int QTDM::getChannels_s(DDCChannelGroupHandle grouphandle,QList<DDCChannelHandle>& channels)
{
	channels.clear();
	unsigned int channelNums = getChannelNums_s(grouphandle);//获取通道数
	std::vector<DDCChannelHandle> arr_c(channelNums,nullptr);
	int r_code = DDC_GetChannels(grouphandle,arr_c.data(),channelNums);
	for(unsigned int i = 0;i<arr_c.size();++i){
		channels.append(arr_c[i]);
	}
	return r_code;
}
DDCDataType QTDM::getChannelPropertyType(DDCChannelHandle channelHandle,const char* propertyName)
{
    DDCDataType type;
    if(m_openFile<=0)
        return DDC_String;
    remember_err_code(DDC_GetChannelPropertyType(channelHandle,propertyName,&type));
    return type;
}

QString QTDM::getChannelStringPropertyByName(DDCChannelHandle channelHandle,const char* propertyName)
{
    unsigned int length(0);
    //获取文件属性 - DDC_FILE_NAME
    int errCode = DDC_GetChannelStringPropertyLength (channelHandle, propertyName, &length);
    if(errCode < 0)
    {
        remember_err_code(errCode);
        return QString("");
    }
    char* property = nullptr;
    length = (8)*(length+1);
    property = new char[length];
    memset(property,0,sizeof(char)*length);
    errCode = DDC_GetChannelProperty (channelHandle, propertyName, property, length);

    if(errCode < 0)
        remember_err_code(errCode);
    QString str = QString::fromLocal8Bit(property);
    if(property)
        delete[] property;
    return str;
}

QString QTDM::getChannelName(DDCChannelHandle channelHandle)
{
	return getChannelStringPropertyByName(channelHandle,DDC_CHANNEL_NAME);
}
int QTDM::getChannelName_s(DDCChannelHandle channelHandle,QString& output_name)
{
	unsigned int length(0);
	//获取文件属性 - DDC_FILE_NAME
	int errCode = DDC_GetChannelStringPropertyLength (channelHandle, DDC_CHANNEL_NAME, &length);
	if(errCode < 0)
	{
		return errCode;
	}
	char* name_property = nullptr;
	length = (8)*(length+1);
	name_property = new char[length];
	memset(name_property,0,sizeof(char)*length);
	errCode = DDC_GetChannelProperty (channelHandle, DDC_CHANNEL_NAME, name_property, length);

	if(errCode < 0)
		return errCode;
	output_name = QString::fromLocal8Bit(name_property);
	if(name_property)
		delete[] name_property;
	return DDC_NoError;
}

QMap<char*,QString> QTDM::getChannelStringPropertys(DDCChannelHandle channelHandle)
{
    QMap<char*,QString> channelPropertys;
    if(m_openFile<=0)
        return channelPropertys;
    channelPropertys[DDC_CHANNEL_NAME] = getChannelStringPropertyByName(channelHandle,DDC_CHANNEL_NAME);
    channelPropertys[DDC_CHANNEL_DESCRIPTION] = getChannelStringPropertyByName(channelHandle,DDC_CHANNEL_DESCRIPTION);
    channelPropertys[DDC_CHANNEL_UNIT_STRING] = getChannelStringPropertyByName(channelHandle,DDC_CHANNEL_UNIT_STRING);
    return channelPropertys;
}

unsigned __int64 QTDM::getChannelDataNums(DDCChannelHandle channelHandle)
{
    if(m_openFile<=0)
        return 0;
    unsigned __int64 dataSize;
    remember_err_code(DDC_GetNumDataValues(channelHandle,&dataSize));
    return dataSize;
}

unsigned __int64 QTDM::getChannelDataNums_s(DDCChannelHandle channelHandle)
{
	unsigned __int64 dataSize(0);
	DDC_GetNumDataValues(channelHandle,&dataSize);
	return dataSize;
}

DDCDataType QTDM::getDataType(DDCChannelHandle channelHandle)
{
    DDCDataType dataType;
    remember_err_code(DDC_GetDataType(channelHandle,&dataType));
    return dataType;
}

DDCDataType QTDM::getDataType_s(DDCChannelHandle channelHandle)
{
	DDCDataType dataType;
	DDC_GetDataType(channelHandle,&dataType);
	return dataType;
}

QString QTDM::getDataTypeToString(DDCChannelHandle channelHandle)
{
	return dataTypeToString(getDataType(channelHandle));
}

QVector<double> QTDM::getChannelDoubleDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length)
{
    QVector<double> datas;
    datas.resize(length);
    remember_err_code(DDC_GetDataValuesDouble(channelHandle,firstIndex,length,datas.data()));
    return datas;
}
QVector<double> QTDM::getChannelDoubleDataValues(DDCChannelHandle channelHandle)
{
    unsigned __int64 dataLength = getChannelDataNums(channelHandle);
    return getChannelDoubleDataValues(channelHandle,0,dataLength);
}
int QTDM::getChannelDoubleDataValues_s(DDCChannelHandle channelHandle,QVector<double>& outputDatas)
{
	unsigned __int64 dataLength = getChannelDataNums_s(channelHandle);
	return getChannelDoubleDataValues_s(channelHandle,0,dataLength,outputDatas);
}
int QTDM::getChannelDoubleDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<double>& outputDatas)
{
	outputDatas.resize(length);
	int type = getDataType_s(channelHandle);
	if(DDCDataType::DDC_Double == type)
	{
		int r = DDC_GetDataValuesDouble(channelHandle,firstIndex,length,outputDatas.data());
		return r;
	}
	else if (DDCDataType::DDC_Float == type)
	{
		QVector<float> datas;
		int r = getChannelFloatDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<float,double>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_Int16 == type)
	{
		QVector<short> datas;
		int r = getChannelShortDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<short,double>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_Int32 == type)
	{
		QVector<long> datas;
		int r = getChannelLongDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<long,double>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_UInt8 == type)
	{
		QVector<unsigned char> datas;
		int r = getChannelUCharDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<unsigned char,double>(datas,outputDatas);
		return r;
	}
	return DDC_InvalidDataType;
}
QVector<float> QTDM::getChannelFloatDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length)
{
    QVector<float> datas;
    datas.resize(length);
    remember_err_code(DDC_GetDataValuesFloat(channelHandle,firstIndex,length,datas.data()));
    return datas;
}
QVector<float> QTDM::getChannelFloatDataValues(DDCChannelHandle channelHandle)
{
    unsigned __int64 dataLength = getChannelDataNums(channelHandle);
    return getChannelFloatDataValues(channelHandle,0,dataLength);
}
int QTDM::getChannelFloatDataValues_s(DDCChannelHandle channelHandle,QVector<float>& outputDatas)
{
	unsigned __int64 dataLength = getChannelDataNums_s(channelHandle);
	return getChannelFloatDataValues_s(channelHandle,0,dataLength,outputDatas);
}
int QTDM::getChannelFloatDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<float>& outputDatas)
{
	outputDatas.resize(length);
	int type = getDataType_s(channelHandle);
	if(DDCDataType::DDC_Float == type)
	{
		int r = DDC_GetDataValuesFloat(channelHandle,firstIndex,length,outputDatas.data());
		return r;
	}
	else if (DDCDataType::DDC_Double == type)
	{
		QVector<double> datas;
		int r = getChannelDoubleDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<double,float>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_Int16 == type)
	{
		QVector<short> datas;
		int r = getChannelShortDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<short,float>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_Int32 == type)
	{
		QVector<long> datas;
		int r = getChannelLongDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<long,float>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_UInt8 == type)
	{
		QVector<unsigned char> datas;
		int r = getChannelUCharDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<unsigned char,float>(datas,outputDatas);
		return r;
	}
	return DDC_InvalidDataType;
}

QVector<QString> QTDM::getChannelStringDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length)
{
    //char *values[length];
    char **values = new char*[length];
    remember_err_code(DDC_GetDataValuesString(channelHandle,firstIndex,length,values));
    QVector<QString> datas;
    datas.reserve(length);
    for(int i=0;i<length;++i)
    {
        datas.append(QString::fromLocal8Bit(values[i]));
    }
    //释放lib自己创建的内存
    for(int i = 0; i < length; ++i)
    {
        DDC_FreeMemory (values[i]);
    }
    if(values)
        delete[] values;
    return datas;
}
QVector<QString> QTDM::getChannelStringDataValues(DDCChannelHandle channelHandle)
{
    unsigned __int64 dataLength = getChannelDataNums(channelHandle);
    return getChannelStringDataValues(channelHandle,0,dataLength);
}
int QTDM::getChannelStringDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<QString>& outputDatas)
{
	int r(DDC_NoError);
	char **values = new char*[length];
	r = DDC_GetDataValuesString(channelHandle,firstIndex,length,values);

	outputDatas.resize(length);
	for(int i=0;i<length;++i)
	{
		outputDatas[i] = (QString::fromLocal8Bit(values[i]));
	}
	//释放lib自己创建的内存
	for(int i = 0; i < length; ++i)
	{
		DDC_FreeMemory (values[i]);
	}
	if(values)
		delete[] values;
	return r;
}
int QTDM::getChannelStringDataValues_s(DDCChannelHandle channelHandle,QVector<QString>& outputDatas)
{
	unsigned __int64 dataLength = getChannelDataNums_s(channelHandle);
	return getChannelStringDataValues_s(channelHandle,0,dataLength,outputDatas);
}

QVector<long> QTDM::getChannelLongDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length)
{
    QVector<long> datas;
    datas.resize(length);
    remember_err_code(DDC_GetDataValuesInt32(channelHandle,firstIndex,length,datas.data()));
    return datas;
}
QVector<long> QTDM::getChannelLongDataValues(DDCChannelHandle channelHandle)
{
    unsigned __int64 dataLength = getChannelDataNums(channelHandle);
    return getChannelLongDataValues(channelHandle,0,dataLength);
}
int QTDM::getChannelLongDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<long>& outputDatas)
{
	outputDatas.resize(length);
	int type = getDataType_s(channelHandle);
	if(DDCDataType::DDC_Int32 == type)
	{
		int r = DDC_GetDataValuesInt32(channelHandle,firstIndex,length,outputDatas.data());
		return r;
	}
	else if (DDCDataType::DDC_Int16 == type)
	{
		QVector<short> datas;
		int r = getChannelShortDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<short,long>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_Double == type)
	{
		QVector<double> datas;
		int r = getChannelDoubleDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<double,long>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_Float == type)
	{
		QVector<float> datas;
		int r = getChannelFloatDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<float,long>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_UInt8 == type)
	{
		QVector<unsigned char> datas;
		int r = getChannelUCharDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<unsigned char,long>(datas,outputDatas);
		return r;
	}
	return DDC_InvalidDataType;
}
int QTDM::getChannelLongDataValues_s(DDCChannelHandle channelHandle,QVector<long>& outputDatas)
{
	unsigned __int64 dataLength = getChannelDataNums_s(channelHandle);
	return getChannelLongDataValues_s(channelHandle,0,dataLength,outputDatas);
}


QVector<short> QTDM::getChannelShortDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length)
{
    QVector<short> datas;
    datas.resize(length);
    remember_err_code(DDC_GetDataValuesInt16(channelHandle,firstIndex,length,datas.data()));
    return datas;
}
QVector<short> QTDM::getChannelShortDataValues(DDCChannelHandle channelHandle)
{
    unsigned __int64 dataLength = getChannelDataNums(channelHandle);
    return getChannelShortDataValues(channelHandle,0,dataLength);
}
int QTDM::getChannelShortDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<short>& outputDatas)
{
	outputDatas.resize(length);
	int type = getDataType_s(channelHandle);
	if (DDCDataType::DDC_Int16 == type)
	{
		int r = DDC_GetDataValuesInt16(channelHandle,firstIndex,length,outputDatas.data());
		return r;
	}
	else if(DDCDataType::DDC_Float == type)
	{
		QVector<float> datas;
		int r = getChannelFloatDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<float,short>(datas,outputDatas);
		return r;
	}
    else if (DDC_Double == type)
	{
		QVector<double> datas;
		int r = getChannelDoubleDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<double,short>(datas,outputDatas);
		return r;
	}
	else if(DDCDataType::DDC_Int32 == type)
	{
		QVector<long> datas;
		int r = getChannelLongDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<long,short>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_UInt8 == type)
	{
		QVector<unsigned char> datas;
		int r = getChannelUCharDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<unsigned char,short>(datas,outputDatas);
		return r;
	}
	return DDC_InvalidDataType;
}
int QTDM::getChannelShortDataValues_s(DDCChannelHandle channelHandle,QVector<short>& outputDatas)
{
	unsigned __int64 dataLength = getChannelDataNums_s(channelHandle);
	return getChannelShortDataValues_s(channelHandle,0,dataLength,outputDatas);
}


QVector<unsigned char> QTDM::getChannelUCharDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length)
{
    QVector<unsigned char> datas;
    datas.resize(length);
    remember_err_code(DDC_GetDataValuesUInt8(channelHandle,firstIndex,length,datas.data()));
    return datas;
}
QVector<unsigned char> QTDM::getChannelUCharDataValues(DDCChannelHandle channelHandle)
{
    unsigned __int64 dataLength = getChannelDataNums(channelHandle);
    return getChannelUCharDataValues(channelHandle,0,dataLength);
}
int QTDM::getChannelUCharDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<unsigned char>& outputDatas)
{
	outputDatas.resize(length);
	int type = getDataType_s(channelHandle);
	if (DDCDataType::DDC_UInt8 == type)
	{
		int r = DDC_GetDataValuesUInt8(channelHandle,firstIndex,length,outputDatas.data());
		return r;
	}
	else if (DDCDataType::DDC_Int16 == type)
	{
		QVector<short> datas;
		int r = getChannelShortDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<short,unsigned char>(datas,outputDatas);
		return r;
	}
	else if(DDCDataType::DDC_Int32 == type)
	{
		QVector<long> datas;
		int r = getChannelLongDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<long,unsigned char>(datas,outputDatas);
		return r;
	}
	else if(DDCDataType::DDC_Float == type)
	{
		QVector<float> datas;
		int r = getChannelFloatDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<float,unsigned char>(datas,outputDatas);
		return r;
	}
	else if (DDCDataType::DDC_Double == type)
	{
		QVector<double> datas;
		int r = getChannelDoubleDataValues_s(channelHandle,firstIndex,length,datas);
		czy::QArrayEx::static_cast_VectorA2VectorB<double,unsigned char>(datas,outputDatas);
		return r;
	}
	
	return DDC_InvalidDataType;
}
int QTDM::getChannelUCharDataValues_s(DDCChannelHandle channelHandle,QVector<unsigned char>& outputDatas)
{
	unsigned __int64 dataLength = getChannelDataNums_s(channelHandle);
	return getChannelUCharDataValues_s(channelHandle,0,dataLength,outputDatas);
}


QVector<QDateTime> QTDM::getChannelDateTimeDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length)
{
    unsigned int *year,*month,*day,*hour,*minute,*second,*weekDay;
    double *milliSecond;
    year = new unsigned int[length];
    month = new unsigned int[length];
    day = new unsigned int[length];
    hour = new unsigned int[length];
    minute = new unsigned int[length];
    second = new unsigned int[length];
    milliSecond = new double[length];
    weekDay = new unsigned int[length];
    remember_err_code(DDC_GetDataValuesTimestampComponents(channelHandle,firstIndex,length
                                                           ,year,month,day,hour
                                                           ,minute,second,milliSecond,weekDay));
    QVector<QDateTime> datas;
    for(size_t i=0;i<length;++i)
    {
        datas.append(QDateTime(QDate(year[i],month[i],day[i])
                               ,QTime(hour[i],minute[i],second[i])));
    }
    if(year)
        delete[] year;
    if(month)
        delete[] month;
    if(day)
        delete[] day;
    if(hour)
        delete[] hour;
    if(minute)
        delete[] minute;
    if(second)
        delete[] second;
    if(milliSecond)
        delete[] milliSecond;
    if(weekDay)
        delete[] weekDay;

    return datas;
}
QVector<QDateTime> QTDM::getChannelDateTimeDataValues(DDCChannelHandle channelHandle)
{
    unsigned __int64 dataLength = getChannelDataNums(channelHandle);
    return getChannelDateTimeDataValues(channelHandle,0,dataLength);
}
int QTDM::getChannelDateTimeDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<QDateTime>& outputDatas)
{
	unsigned int *year,*month,*day,*hour,*minute,*second,*weekDay;
	double *milliSecond;
	year = new unsigned int[length];
	month = new unsigned int[length];
	day = new unsigned int[length];
	hour = new unsigned int[length];
	minute = new unsigned int[length];
	second = new unsigned int[length];
	milliSecond = new double[length];
	weekDay = new unsigned int[length];
	int r = DDC_GetDataValuesTimestampComponents(channelHandle,firstIndex,length
		,year,month,day,hour
		,minute,second,milliSecond,weekDay);
	outputDatas.resize(length);
	for(size_t i=0;i<length;++i)
	{
		outputDatas[i] = (QDateTime(QDate(year[i],month[i],day[i])
			,QTime(hour[i],minute[i],second[i])));
	}
	if(year)
		delete[] year;
	if(month)
		delete[] month;
	if(day)
		delete[] day;
	if(hour)
		delete[] hour;
	if(minute)
		delete[] minute;
	if(second)
		delete[] second;
	if(milliSecond)
		delete[] milliSecond;
	if(weekDay)
		delete[] weekDay;

	return r;
}
int QTDM::getChannelDateTimeDataValues_s(DDCChannelHandle channelHandle,QVector<QDateTime>& outputDatas)
{
	unsigned __int64 dataLength = getChannelDataNums_s(channelHandle);
	return getChannelDateTimeDataValues_s(channelHandle,0,dataLength,outputDatas);
}

QVector<QVariant> QTDM::getChannelDataValues(DDCChannelHandle channelHandle)
{
	unsigned __int64 dataLength = getChannelDataNums(channelHandle);
	return getChannelDataValues(channelHandle,0,dataLength);
}
QVector<QVariant> QTDM::getChannelDataValues(DDCChannelHandle channelHandle,size_t firstIndex,size_t length)
{
    DDCDataType type = getDataType(channelHandle);
    switch(type)
    {
    case DDCDataType::DDC_Float:
        return czy::QArrayEx::VectorType2VectorVariant(getChannelFloatDataValues(channelHandle,firstIndex,length));
    case DDCDataType::DDC_Double:
        return czy::QArrayEx::VectorType2VectorVariant(getChannelDoubleDataValues(channelHandle,firstIndex,length));
    case DDCDataType::DDC_UInt8:
        return czy::QArrayEx::VectorType2VectorVariant(getChannelUCharDataValues(channelHandle,firstIndex,length));
    case DDCDataType::DDC_Int16:
        return czy::QArrayEx::VectorType2VectorVariant(getChannelShortDataValues(channelHandle,firstIndex,length));
    case DDCDataType::DDC_Int32:
        return czy::QArrayEx::VectorType2VectorVariant(getChannelLongDataValues(channelHandle,firstIndex,length));
    case DDCDataType::DDC_String:
        return czy::QArrayEx::VectorType2VectorVariant(getChannelStringDataValues(channelHandle,firstIndex,length));
    case DDCDataType::DDC_Timestamp:
        return czy::QArrayEx::VectorType2VectorVariant(getChannelDateTimeDataValues(channelHandle,firstIndex,length));
    }
    return QVector<QVariant>();
}
QVariant QTDM::getChannelDataValue(DDCChannelHandle channelHandle,size_t index)
{
	DDCDataType type = getDataType(channelHandle);
	switch(type)
	{
	case DDCDataType::DDC_Float:
		{
			float d;
			remember_err_code(
				DDC_GetDataValuesFloat(channelHandle,index,1,&d));
			return d;
		}
	case DDCDataType::DDC_Double:
		{
			double d;
			remember_err_code(
				DDC_GetDataValuesDouble(channelHandle,index,1,&d));
			return d;
		}
	case DDCDataType::DDC_UInt8:
		{
			unsigned char d;
			remember_err_code(
				DDC_GetDataValuesUInt8(channelHandle,index,1,&d));
			return d;
		}
	case DDCDataType::DDC_Int16:
		{
			short d;
			remember_err_code(
				DDC_GetDataValuesInt16(channelHandle,index,1,&d));
			return d;
		}
	case DDCDataType::DDC_Int32:
		{
            long d;
			remember_err_code(
				DDC_GetDataValuesInt32(channelHandle,index,1,&d));
            return qlonglong(d);
		}
	case DDCDataType::DDC_String:
		{
			char **values = new char*[1];
			remember_err_code(DDC_GetDataValuesString(channelHandle,index,1,values));
			QString d;
			d.append(QString::fromLocal8Bit(values[0]));

			//释放lib自己创建的内存
			DDC_FreeMemory (values[0]);
			if(values)
				delete[] values;
			return d;
		}
	case DDCDataType::DDC_Timestamp:
		{
			QDateTime d;
			unsigned int year,month,day,hour,minute,second,weekDay;
			double milliSecond;
			remember_err_code(DDC_GetDataValuesTimestampComponents(channelHandle,index,1
				,&year,&month,&day,&hour
				,&minute,&second,&milliSecond,&weekDay));
			return QDateTime(QDate(year,month,day),QTime(hour,minute,second));
		}
	}
	return QVariant();
}

QVariant QTDM::getChannelDataValue_s(DDCChannelHandle channelHandle,size_t index)
{
	DDCDataType type = getDataType_s(channelHandle);
	switch(type)
	{
	case DDCDataType::DDC_Float:
		{
			float d;
			DDC_GetDataValuesFloat(channelHandle,index,1,&d);
			return d;
		}
	case DDCDataType::DDC_Double:
		{
			double d;
			DDC_GetDataValuesDouble(channelHandle,index,1,&d);
			return d;
		}
	case DDCDataType::DDC_UInt8:
		{
			unsigned char d;
			DDC_GetDataValuesUInt8(channelHandle,index,1,&d);
			return d;
		}
	case DDCDataType::DDC_Int16:
		{
			short d;
			DDC_GetDataValuesInt16(channelHandle,index,1,&d);
			return d;
		}
	case DDCDataType::DDC_Int32:
		{
            long d;
			DDC_GetDataValuesInt32(channelHandle,index,1,&d);
            return qlonglong(d);
		}
	case DDCDataType::DDC_String:
		{
			char **values = new char*[1];
			DDC_GetDataValuesString(channelHandle,index,1,values);
			QString d;
			d.append(QString::fromLocal8Bit(values[0]));
			//释放lib自己创建的内存
			DDC_FreeMemory (values[0]);
			if(values)
				delete[] values;
			return d;
		}
	case DDCDataType::DDC_Timestamp:
		{
			QDateTime d;
			unsigned int year,month,day,hour,minute,second,weekDay;
			double milliSecond;
			DDC_GetDataValuesTimestampComponents(channelHandle,index,1
				,&year,&month,&day,&hour
				,&minute,&second,&milliSecond,&weekDay);
			return QDateTime(QDate(year,month,day),QTime(hour,minute,second));
		}
	}
	return QVariant();
}

int QTDM::getChannelDataValues_s(DDCChannelHandle channelHandle,QVector<QVariant>& outputDatas)
{
	unsigned __int64 dataLength = getChannelDataNums_s(channelHandle);
	return getChannelDataValues_s(channelHandle,0,dataLength,outputDatas);
}
int QTDM::getChannelDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length,QVector<QVariant>& outputDatas)
{
	DDCDataType type = getDataType_s(channelHandle);
	int r(DDC_NoError);
	switch(type)
	{
	case DDCDataType::DDC_Float:
		{
			QVector<float> dataF;
			r = getChannelFloatDataValues_s(channelHandle,firstIndex,length,dataF);
			outputDatas = czy::QArrayEx::VectorType2VectorVariant(dataF);
		}
		break;
	case DDCDataType::DDC_Double:
		{
			QVector<double> dataD;
			r = getChannelDoubleDataValues_s(channelHandle,firstIndex,length,dataD);
			outputDatas = czy::QArrayEx::VectorType2VectorVariant(dataD);
		}	
		break;
	case DDCDataType::DDC_UInt8:
		{
			QVector<unsigned char> dataU8;
			r = getChannelUCharDataValues_s(channelHandle,firstIndex,length,dataU8);
			outputDatas =  czy::QArrayEx::VectorType2VectorVariant(dataU8);
		}
		break;
	case DDCDataType::DDC_Int16:
		{
			QVector<short> dataI16;
			r = getChannelShortDataValues_s(channelHandle,firstIndex,length,dataI16);
			outputDatas = czy::QArrayEx::VectorType2VectorVariant(dataI16);
		}
		break;
	case DDCDataType::DDC_Int32:
		{
			QVector<long> dataI32;
			r = getChannelLongDataValues_s(channelHandle,firstIndex,length,dataI32);
			outputDatas =  czy::QArrayEx::VectorType2VectorVariant(dataI32);
		}
		break;
	case DDCDataType::DDC_String:
		{
			QVector<QString> dataS;
			r = getChannelStringDataValues_s(channelHandle,firstIndex,length,dataS);
			outputDatas = czy::QArrayEx::VectorType2VectorVariant(dataS);
		}
		break;
	case DDCDataType::DDC_Timestamp:
		{
			QVector<QDateTime> dataT;
			r = getChannelDateTimeDataValues_s(channelHandle,firstIndex,length,dataT);
			outputDatas = czy::QArrayEx::VectorType2VectorVariant(dataT);
		}
		break;
	default:
		r = DDC_InvalidDataType;
	}
	return r;
}
// template <typename T>
// QVector<typename T> QTDM::getChannelDataValues_s(DDCChannelHandle channelHandle,size_t firstIndex,size_t length)
// {
// 	QVector<QVariant> datas;
// 	if(DDC_NoError == getChannelDataValues_s(channelHandle,firstIndex,length,datas))
// 	{
// 		return czy::QArrayEx::VectorVariant2VectorType<typename T>(datas);
// 	}
// 	return QVector<T>();
// }
// template <typename T>
// QVector<typename T> QTDM::getChannelDataValues_s(DDCChannelHandle channelHandle)
// {
// 	unsigned __int64 dataLength = getChannelDataNums(channelHandle);
// 	return getChannelDataValues_s<typename T>(channelHandle,0,dataLength);
// }

//--------------------------------------------------------------------------

QString QTDM::lastErrorString()
{
    if(m_errCodes.size()>0)
        return errcode2String(m_errCodes.back());
    return QString("");
}

QString QTDM::errcode2String(int errcode)
{
    return QString(DDC_GetLibraryErrorDescription(errcode));
}



//-------------------------------------------------------------------------------
///
/// \brief 把文件所有信息存到QStandardItem*中
/// \param channelHandle通道句柄
/// \param icoGroup 对应的是groups图标
/// \return 返回文件的句柄
///
DDCFileHandle QTDM::setInStandardItem(tdmFileStruct fileStruct,QStandardItem* item,bool isCheckabled,QIcon* icoGroup ,QIcon* icoChannel)
{
	QList<DDCChannelGroupHandle> groups = fileStruct.groups();
	QString str;
	for(auto ite = groups.begin();ite != groups.end();++ite)
	{
		QStandardItem* itemGroup =
			(icoGroup == nullptr 
			? new QStandardItem(getGroupPropertyByName(*ite,DDC_CHANNELGROUP_NAME))
			: new QStandardItem(*icoGroup,getGroupPropertyByName(*ite,DDC_CHANNELGROUP_NAME)));
		//存放组的自身句柄
		itemGroup->setData(QVariant::fromValue((void*)(*ite)),TREE_ITEM_ROLE_DDC_Handle);	//由于Q_DECLARE_METATYPE(DDCChannelGroup)编译不过，只能通过转换为void*存放
		itemGroup->setData(QVariant(DDC_MY_TYPE_GROUP),TREE_ITEM_ROLE_DDC_TYPE);
		if(isCheckabled)
		{
			itemGroup->setCheckable(true);
			itemGroup->setTristate(true);
		}
		//存放组的额外的信息
		str = getGroupPropertyByName(*ite,DDC_CHANNELGROUP_DESCRIPTION);
		if(!str.isEmpty())
			itemGroup->setData(QVariant::fromValue(str),TREE_ITEM_ROLE_DDC_DESCRIPTION);
		
		QList<DDCChannelHandle> channels = fileStruct.channels(*ite);
		for(auto ite_c = channels.begin();ite_c != channels.end();++ite_c)
		{
			QStandardItem* itemChannel =
				(icoChannel == nullptr 
				? new QStandardItem(getChannelStringPropertyByName(*ite_c,DDC_CHANNEL_NAME))
				: new QStandardItem(*icoChannel,getChannelStringPropertyByName(*ite_c,DDC_CHANNEL_NAME)));
			//存放通道的自身句柄
			itemChannel->setData(QVariant::fromValue((void*)(*ite_c)),TREE_ITEM_ROLE_DDC_Handle);	//由于Q_DECLARE_METATYPE(DDCChannelGroup)编译不过，只能通过转换为void*存放
			itemChannel->setData(QVariant(DDC_MY_TYPE_CHANNEL),TREE_ITEM_ROLE_DDC_TYPE);
			if(isCheckabled)
				itemChannel->setCheckable(true);
			//存放通道的额外的信息
			str = getChannelStringPropertyByName(*ite_c,DDC_CHANNEL_DESCRIPTION);
			if(!str.isEmpty())
				itemChannel->setData(QVariant::fromValue(str),TREE_ITEM_ROLE_DDC_DESCRIPTION);
			itemChannel->setData(QVariant(getDataType(*ite_c)),TREE_ITEM_ROLE_DDC_DATA_STRUCT_TYPE);

			itemGroup->appendRow(itemChannel);
		}

		item->appendRow(itemGroup);
	}
	return fileStruct.getFileHandle();
}
///
/// \brief 判断QStandardItem是否存在文件类型标记位TREE_ITEM_ROLE_DDC_TYPE
///
bool QTDM::isStandardItemHaveTypeMark(const QStandardItem* item)
{
	return item->data(TREE_ITEM_ROLE_DDC_TYPE).isValid();
}
bool QTDM::isStandardItemHaveTypeMark(const QModelIndex item)
{
	return item.data(TREE_ITEM_ROLE_DDC_TYPE).isValid();
}
bool QTDM::isStandardItemHaveHandleMark(const QStandardItem* item)
{
	return item->data(TREE_ITEM_ROLE_DDC_Handle).isValid();
}
bool QTDM::isStandardItemHaveHandleMark(const QModelIndex item)
{
	return item.data(TREE_ITEM_ROLE_DDC_Handle).isValid();
}
bool QTDM::isStandardItemFile(const QStandardItem* item)
{
	QVariant var = item->data(TREE_ITEM_ROLE_DDC_TYPE);
	if(!var.isValid())
	{
		return false;
	}
	return (DDC_MY_TYPE_FILE == var.toInt());
}
bool QTDM::isStandardItemFile(const QModelIndex item)
{
	QVariant var = item.data(TREE_ITEM_ROLE_DDC_TYPE);
	if(!var.isValid())
	{
		return false;
	}
	return (DDC_MY_TYPE_FILE == var.toInt());
}
bool QTDM::isStandardItemGroup(const QStandardItem* item)
{
	QVariant var = item->data(TREE_ITEM_ROLE_DDC_TYPE);
	if(!var.isValid())
	{
		return false;
	}
	return (DDC_MY_TYPE_GROUP == var.toInt());
}
bool QTDM::isStandardItemGroup(const QModelIndex item)
{
	QVariant var = item.data(TREE_ITEM_ROLE_DDC_TYPE);
	if(!var.isValid())
	{
		return false;
	}
	return (DDC_MY_TYPE_GROUP == var.toInt());
}
bool QTDM::isStandardItemChannel(const QStandardItem* item)
{
	QVariant var = item->data(TREE_ITEM_ROLE_DDC_TYPE);
	if(!var.isValid())
	{
		return false;
	}
	return (DDC_MY_TYPE_CHANNEL == var.toInt());
}
bool QTDM::isStandardItemChannel(const QModelIndex item)
{
	QVariant var = item.data(TREE_ITEM_ROLE_DDC_TYPE);
	if(!var.isValid())
	{
		return false;
	}
	return (DDC_MY_TYPE_CHANNEL == var.toInt());
}
int  QTDM::getStandardItemTypeMark(const QStandardItem* item)
{
	QVariant var = item->data(TREE_ITEM_ROLE_DDC_TYPE);
	if(!var.isValid())
	{
		return DDC_MY_TYPE_NO;
	}
	return var.toInt();
}
int  QTDM::getStandardItemTypeMark(const QModelIndex item)
{
	QVariant var = item.data(TREE_ITEM_ROLE_DDC_TYPE);
	if(!var.isValid())
	{
		return DDC_MY_TYPE_NO;
	}
	return var.toInt();
}
DDCFileHandle QTDM::getStandardItemFileHandle(const QStandardItem* item)
{
	QVariant var = item->data(TREE_ITEM_ROLE_DDC_Handle);
	if(!var.isValid())
	{
		return nullptr;
	}
	return DDCFileHandle(var.value<void*>());
}
DDCFileHandle QTDM::getStandardItemFileHandle(const QModelIndex item)
{
	QVariant var = item.data(TREE_ITEM_ROLE_DDC_Handle);
	if(!var.isValid())
	{
		return nullptr;
	}
	return DDCFileHandle(var.value<void*>());
}
DDCChannelGroupHandle QTDM::getStandardItemGroupHandle(const QStandardItem* item)
{
	QVariant var = item->data(TREE_ITEM_ROLE_DDC_Handle);
	if(!var.isValid())
	{
		return nullptr;
	}
	return DDCChannelGroupHandle(var.value<void*>());
}
DDCChannelGroupHandle QTDM::getStandardItemGroupHandle(const QModelIndex item)
{
	QVariant var = item.data(TREE_ITEM_ROLE_DDC_Handle);
	if(!var.isValid())
	{
		return nullptr;
	}
	return DDCChannelGroupHandle(var.value<void*>());
}
DDCChannelHandle QTDM::getStandardItemChannelHandle(const QStandardItem* item)
{
	QVariant var = item->data(TREE_ITEM_ROLE_DDC_Handle);
	if(!var.isValid())
	{
		return nullptr;
	}
	return DDCChannelHandle(var.value<void*>());
}
DDCChannelHandle QTDM::getStandardItemChannelHandle(const QModelIndex item)
{
	QVariant var = item.data(TREE_ITEM_ROLE_DDC_Handle);
	if(!var.isValid())
	{
		return nullptr;
	}
	return DDCChannelHandle(var.value<void*>());
}
///
/// \brief 把数据类型转换为字符串
/// \param
///
QString QTDM::dataTypeToString(const DDCDataType dataType)
{
	switch(dataType)
	{
	case DDCDataType::DDC_UInt8:
		return QStringLiteral("UInt8");
	case DDCDataType::DDC_Int16:
		return QStringLiteral("Int16");
	case DDCDataType::DDC_Int32:
		return QStringLiteral("Int32");
	case DDCDataType::DDC_Float:
		return QStringLiteral("Float");
	case DDCDataType::DDC_Double:
		return QStringLiteral("Double");
	case DDCDataType::DDC_String:
		return QStringLiteral("String");
	case DDCDataType::DDC_Timestamp:
		return QStringLiteral("Timestamp");
	}
	return QStringLiteral("UnKnow");
}
/*
typedef enum {
	DDC_UInt8		= 5,	// unsigned char
	DDC_Int16		= 2,	// short
	DDC_Int32		= 3,	// int
	DDC_Float		= 9,	// float
	DDC_Double		= 10,	// double
	DDC_String		= 23,	// string
	DDC_Timestamp	= 30,	// timestamp (Year/Month/Day/Hour/Minute/Second/Millisecond components)
} DDCDataType;
*/
