/*=Plus=header=begin======================================================
Program: Plus
Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
See License.txt for details.
=========================================================Plus=header=end*/

#include "PlusConfigure.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMatrix4x4.h"
#include "vtkMultiThreader.h"
#include "vtkObjectFactory.h"
#include "vtkPlusBuffer.h"
#include "vtkPlusChannel.h"
#include "vtkPlusDataSource.h"
#include "vtkPlusDevice.h"
#include "vtkPlusRecursiveCriticalSection.h"
#include "vtkPlusSequenceIO.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPlusTrackedFrameList.h"
#include "vtkWindows.h"
#include "vtksys/SystemTools.hxx"
#include <ctype.h>
#include <time.h>

#if ( _MSC_VER >= 1300 ) // Visual studio .NET
#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4312 )
#endif

#define LOCAL_LOG_ERROR(msg) \
{ \
  std::ostringstream msgStream; \
  if( this->DeviceId == NULL ) \
  { \
    msgStream << " " << msg << std::ends; \
  } \
  else \
  { \
    msgStream << this->DeviceId << ": " << msg << std::ends; \
  } \
  std::string finalStr(msgStream.str()); \
  LOG_ERROR(finalStr); \
}
#define LOCAL_LOG_WARNING(msg) \
{ \
  std::ostringstream msgStream; \
  if( this->DeviceId == NULL ) \
{ \
  msgStream << " " << msg << std::ends; \
} \
  else \
{ \
  msgStream << this->DeviceId << ": " << msg << std::ends; \
} \
  std::string finalStr(msgStream.str()); \
  LOG_WARNING(finalStr); \
}
#define LOCAL_LOG_INFO(msg) \
{ \
  std::ostringstream msgStream; \
  if( this->DeviceId == NULL ) \
{ \
  msgStream << " " << msg << std::ends; \
} \
  else \
{ \
  msgStream << this->DeviceId << ": " << msg << std::ends; \
} \
  std::string finalStr(msgStream.str()); \
  LOG_INFO(finalStr); \
}
#define LOCAL_LOG_DEBUG(msg) \
{ \
  std::ostringstream msgStream; \
  if( this->DeviceId == NULL ) \
{ \
  msgStream << " " << msg << std::ends; \
} \
  else \
{ \
  msgStream << this->DeviceId << ": " << msg << std::ends; \
} \
  std::string finalStr(msgStream.str()); \
  LOG_DEBUG(finalStr); \
}
#define LOCAL_LOG_TRACE(msg) \
{ \
  std::ostringstream msgStream; \
  if( this->DeviceId == NULL ) \
{ \
  msgStream << " " << msg << std::ends; \
} \
  else \
{ \
  msgStream << this->DeviceId << ": " << msg << std::ends; \
} \
  std::string finalStr(msgStream.str()); \
  LOG_TRACE(finalStr); \
}


//----------------------------------------------------------------------------

vtkStandardNewMacro( vtkPlusDevice );

const int vtkPlusDevice::VIRTUAL_DEVICE_FRAME_RATE = 50;
static const int FRAME_RATE_AVERAGING = 10;
const char* vtkPlusDevice::DEFAULT_TRACKER_REFERENCE_FRAME_NAME = "Tracker";
const char* vtkPlusDevice::BMODE_PORT_NAME = "B";
const char* vtkPlusDevice::RFMODE_PORT_NAME = "Rf";
const std::string vtkPlusDevice::PROBE_SWITCH_ATTRIBUTE_NAME = "ProbeId";
const std::string vtkPlusDevice::DEPTH_SWITCH_ATTRIBUTE_NAME = "Depth";
const std::string vtkPlusDevice::MODE_SWITCH_ATTRIBUTE_NAME = "Mode";

//----------------------------------------------------------------------------
const double vtkPlusDevice::ParamIndexKey::NO_DEPTH = -1.0;

vtkPlusDevice::ParamIndexKey::ParamIndexKey()
  : Depth( NO_DEPTH )
  , ProbeId( "" )
  , Mode( Plus_UnknownMode )
{

}

//----------------------------------------------------------------------------
vtkPlusDevice::vtkPlusDevice()
  : ThreadAlive( false )
  , Connected( 0 )
  , Threader( vtkMultiThreader::New() )
  , ThreadId( -1 )
  , CurrentStreamBufferItem( new StreamBufferItem() )
  , ToolReferenceFrameName( NULL )
  , DeviceId( NULL )
  , DataCollector( NULL )
  , AcquisitionRate( 30 )
  , Recording( 0 )
  , DesiredTimestamp( -1 )
  , UpdateWithDesiredTimestamp( 0 )
  , TimestampClosestToDesired( -1 )
  , FrameNumber( 0 )
  , FrameTimeStamp( 0 )
  , OutputNeedsInitialization( 1 )
  , CorrectlyConfigured( true )
  , StartThreadForInternalUpdates( false )
  , LocalTimeOffsetSec( 0.0 )
  , MissingInputGracePeriodSec( 0.0 )
  , RequireImageOrientationInConfiguration( false )
  , RequirePortNameInDeviceSetConfiguration( false )
{
  this->SetNumberOfInputPorts( 0 );

  this->SetToolReferenceFrameName( DEFAULT_TRACKER_REFERENCE_FRAME_NAME );

  // For threaded capture of transformations
  this->UpdateMutex = vtkPlusRecursiveCriticalSection::New();
}

//----------------------------------------------------------------------------
vtkPlusDevice::~vtkPlusDevice()
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::~vtkPlusDevice()" );
  if ( this->Connected )
  {
    Disconnect();
  }

  for( ChannelContainerIterator it = this->OutputChannels.begin(); it != this->OutputChannels.end(); ++it )
  {
    ( *it )->UnRegister( this );
  }
  this->InputChannels.clear();
  this->OutputChannels.clear();
  for( DataSourceContainerIterator it = this->Tools.begin(); it != this->Tools.end(); ++it )
  {
    it->second->UnRegister( this );
  }
  this->Tools.clear();
  for( DataSourceContainerIterator it = this->Fields.begin(); it != this->Fields.end(); ++it )
  {
    it->second->UnRegister( this );
  }
  this->Fields.clear();
  for( DataSourceContainerIterator it = this->VideoSources.begin(); it != this->VideoSources.end(); ++it )
  {
    it->second->UnRegister( this );
  }
  this->VideoSources.clear();

  delete this->CurrentStreamBufferItem;
  this->CurrentStreamBufferItem = NULL;

  delete this->DeviceId;
  this->DeviceId = NULL;

  DELETE_IF_NOT_NULL( this->Threader );

  DELETE_IF_NOT_NULL( this->UpdateMutex );

  LOCAL_LOG_TRACE( "vtkPlusDevice::~vtkPlusDevice() completed" );
}

//----------------------------------------------------------------------------
void vtkPlusDevice::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );

  os << indent << "Connected: " << ( this->Connected ? "Yes\n" : "No\n" );
  os << indent << "SDK version: " << this->GetSdkVersion() << "\n";
  os << indent << "AcquisitionRate: " << this->AcquisitionRate << "\n";
  os << indent << "Recording: " << ( this->Recording ? "On\n" : "Off\n" );

  for( ChannelContainerConstIterator it = this->OutputChannels.begin(); it != this->OutputChannels.end(); ++it )
  {
    vtkPlusChannel* str = *it;
    str->PrintSelf( os, indent );
  }

  if ( this->ToolReferenceFrameName )
  {
    os << indent << "ToolReferenceFrameName: " << this->ToolReferenceFrameName << "\n";
  }

  for( DataSourceContainerConstIterator it = this->VideoSources.begin(); it != this->VideoSources.end(); ++it )
  {
    vtkPlusDataSource* dataSource = it->second;
    dataSource->PrintSelf( os, indent );
  }

  for( DataSourceContainerConstIterator it = this->Tools.begin(); it != this->Tools.end(); ++it )
  {
    vtkPlusDataSource* dataSource = it->second;
    dataSource->PrintSelf( os, indent );
  }

  for( DataSourceContainerConstIterator it = this->Fields.begin(); it != this->Fields.end(); ++it )
  {
    vtkPlusDataSource* dataSource = it->second;
    dataSource->PrintSelf( os, indent );
  }
}

//----------------------------------------------------------------------------
std::string vtkPlusDevice::GetSdkVersion()
{
  // Base class version is the same as the Plus library version
  std::string ver = std::string( "Plus-" ) + std::string( PLUSLIB_VERSION );
  return ver;
}

//----------------------------------------------------------------------------
bool vtkPlusDevice::IsResettable()
{
  // By default, devices cannot be reset.
  return false;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::Reset()
{
  // By default, Reset has no effect.
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
DataSourceContainerConstIterator vtkPlusDevice::GetToolIteratorBegin() const
{
  return this->Tools.begin();
}

//----------------------------------------------------------------------------
DataSourceContainerConstIterator vtkPlusDevice::GetToolIteratorEnd() const
{
  return this->Tools.end();
}

//----------------------------------------------------------------------------
int vtkPlusDevice::GetNumberOfTools() const
{
  return this->Tools.size();
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::AddTool( vtkPlusDataSource* tool, bool requireUniquePortName )
{
  if ( tool == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to add tool to tracker, tool is NULL!" );
    return PLUS_FAIL;
  }

  if ( tool->GetSourceId() == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to add tool to tracker, tool source ID must be defined!" );
    return PLUS_FAIL;
  }

  if ( this->Tools.find( tool->GetSourceId() ) != this->GetToolIteratorEnd() )
  {
    LOCAL_LOG_ERROR( "Tool with ID '" << tool->GetSourceId() << "' is already in the tool container!" );
    return PLUS_FAIL;
  }

  if( requireUniquePortName && tool->GetPortName() != NULL )
  {
    // Check tool port names, it should be unique too
    for ( DataSourceContainerConstIterator it = this->GetToolIteratorBegin(); it != this->GetToolIteratorEnd(); ++it )
    {
      if ( it->second->GetPortName() == NULL )
      {
        continue;
      }
      if ( STRCASECMP( tool->GetPortName(), it->second->GetPortName() ) == 0 )
      {
        LOCAL_LOG_ERROR( "Failed to add '" << tool->GetSourceId() << "' tool to container: tool with name '" << it->second->GetSourceId()
                         << "' is already defined on port '" << tool->GetPortName() << "'!" );
        return PLUS_FAIL;
      }
    }
  }

  tool->Register( this );
  tool->SetDevice( this );
  this->Tools[tool->GetSourceId()] = tool;

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetFieldDataSource( const char* aSourceId, vtkPlusDataSource*& aSource ) const
{
  if ( aSourceId == NULL )
  {
    LOCAL_LOG_ERROR( "vtkPlusDevice::GetTool failed: aToolSourceId is invalid" );
    return PLUS_FAIL;
  }

  if( this->Fields.find( aSourceId ) != this->Fields.end() )
  {
    aSource = this->Fields.find( aSourceId )->second;
    return PLUS_SUCCESS;
  }

  return PLUS_FAIL;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetFieldDataSource( const std::string& aSourceId, vtkPlusDataSource*& aSource ) const
{
  return this->GetFieldDataSource( aSourceId.c_str(), aSource );
}

//----------------------------------------------------------------------------
DataSourceContainerConstIterator vtkPlusDevice::GetFieldDataSourcessIteratorBegin() const
{
  return this->Fields.begin();
}

//----------------------------------------------------------------------------
DataSourceContainerConstIterator vtkPlusDevice::GetFieldDataSourcessIteratorEnd() const
{
  return this->Fields.end();
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::AddFieldDataSource( vtkPlusDataSource* aSource )
{
  if ( aSource == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to add field data source to device, source is NULL!" );
    return PLUS_FAIL;
  }

  if ( aSource->GetSourceId() == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to add field data to device, field data source ID must be defined!" );
    return PLUS_FAIL;
  }

  if ( this->Fields.find( aSource->GetSourceId() ) != this->GetFieldDataSourcessIteratorEnd() )
  {
    LOCAL_LOG_ERROR( "Field data with ID '" << aSource->GetSourceId() << "' is already in the field data container!" );
    return PLUS_FAIL;
  }

  aSource->Register( this );
  aSource->SetDevice( this );
  this->Fields[aSource->GetSourceId()] = aSource;

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
int vtkPlusDevice::GetNumberOfFieldDataSources() const
{
  return this->Fields.size();
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetFirstActiveTool( vtkPlusDataSource*& aTool ) const
{
  if ( this->Tools.size() == 0 )
  {
    LOCAL_LOG_ERROR( "Failed to get first active tool - there are no tools!" );
    return PLUS_FAIL;
  }

  // Get the first tool
  aTool = this->Tools.begin()->second;

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetTool( const char* aToolSourceId, vtkPlusDataSource*& aTool ) const
{
  if ( aToolSourceId == NULL )
  {
    LOCAL_LOG_ERROR( "vtkPlusDevice::GetTool failed: aToolSourceId is invalid" );
    return PLUS_FAIL;
  }

  if( this->Tools.find( aToolSourceId ) != this->Tools.end() )
  {
    aTool = this->Tools.find( aToolSourceId )->second;
    return PLUS_SUCCESS;
  }

  return PLUS_FAIL;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetTool( const std::string& aToolSourceId, vtkPlusDataSource*& aTool ) const
{
  return this->GetTool( aToolSourceId.c_str(), aTool );
}

//-----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetToolByPortName( const char* portName, vtkPlusDataSource*& aTool )
{
  if ( portName == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to get tool - port name is NULL!" );
    return PLUS_FAIL;
  }

  for ( DataSourceContainerIterator it = this->Tools.begin(); it != this->Tools.end(); ++it )
  {
    if ( STRCASECMP( portName, it->second->GetPortName() ) == 0 )
    {
      aTool = it->second;
      return PLUS_SUCCESS;
    }
  }

  return PLUS_FAIL;
}

//-----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetVideoSourcesByPortName( const char* aPortName, std::vector<vtkPlusDataSource*>& sources )
{
  sources.clear();
  if ( aPortName == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to get video source - port name is NULL!" );
    return PLUS_FAIL;
  }

  for ( DataSourceContainerIterator it = this->VideoSources.begin(); it != this->VideoSources.end(); ++it )
  {
    if ( it->second->GetPortName() == NULL )
    {
      LOCAL_LOG_DEBUG( "Port name is not defined for video source " << ( it->second->GetSourceId() != NULL ? it->second->GetSourceId() : "unknown" ) );
      continue;
    }
    if ( STRCASECMP( aPortName, it->second->GetPortName() ) == 0 )
    {
      sources.push_back( it->second );
    }
  }

  return PLUS_SUCCESS;
}

//-----------------------------------------------------------------------------
void vtkPlusDevice::SetToolsBufferSize( int aBufferSize )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::SetToolsBufferSize(" << aBufferSize << ")" );
  for ( DataSourceContainerConstIterator it = this->GetToolIteratorBegin(); it != this->GetToolIteratorEnd(); ++it )
  {
    it->second->SetBufferSize( aBufferSize );
  }
}

//----------------------------------------------------------------------------
void vtkPlusDevice::SetFieldDataSourcesBufferSize( int aBufferSize )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::SetFieldDataSourcesBufferSize(" << aBufferSize << ")" );
  for ( DataSourceContainerConstIterator it = this->GetFieldDataSourcessIteratorBegin(); it != this->GetFieldDataSourcessIteratorEnd(); ++it )
  {
    it->second->SetBufferSize( aBufferSize );
  }
}

//----------------------------------------------------------------------------
void vtkPlusDevice::SetLocalTimeOffsetSec( double aTimeOffsetSec )
{
  for( DataSourceContainerIterator it = this->VideoSources.begin(); it != this->VideoSources.end(); ++it )
  {
    vtkPlusDataSource* image = it->second;
    image->SetLocalTimeOffsetSec( aTimeOffsetSec );
  }
  // local tools
  for( DataSourceContainerIterator it = this->Tools.begin(); it != this->Tools.end(); ++it )
  {
    vtkPlusDataSource* tool = it->second;
    tool->SetLocalTimeOffsetSec( aTimeOffsetSec );
  }
  this->LocalTimeOffsetSec = aTimeOffsetSec;
}

//----------------------------------------------------------------------------
double vtkPlusDevice::GetLocalTimeOffsetSec()
{
  return this->LocalTimeOffsetSec;
}

//-----------------------------------------------------------------------------
void vtkPlusDevice::DeepCopy( vtkPlusDevice* device )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::DeepCopy" );
  for ( DataSourceContainerConstIterator it = device->Tools.begin(); it != device->Tools.end(); ++it )
  {
    LOCAL_LOG_DEBUG( "Copy the buffer of tracker tool: " << it->first );
    if ( this->AddTool( it->second, false ) != PLUS_SUCCESS )
    {
      LOCAL_LOG_ERROR( "Copy of tool '" << it->first << "' failed - unable to add tool to the container!" );
      continue;
    }

    vtkPlusDataSource* tool = NULL;
    if ( this->GetTool( it->first.c_str(), tool ) != PLUS_SUCCESS )
    {
      LOCAL_LOG_ERROR( "Copy of tool '" << it->first << "' failed - unable to get tool from container!" );
      continue;
    }

    tool->DeepCopy( it->second );
  }

  for ( DataSourceContainerConstIterator it = device->Fields.begin(); it != device->Fields.end(); ++it )
  {
    LOCAL_LOG_DEBUG( "Copy the buffer of field data source: " << it->first );
    if ( this->AddFieldDataSource( it->second ) != PLUS_SUCCESS )
    {
      LOCAL_LOG_ERROR( "Copy of field data source '" << it->first << "' failed - unable to add field data source to the container!" );
      continue;
    }

    vtkPlusDataSource* aSource = NULL;
    if ( this->GetFieldDataSource( it->first.c_str(), aSource ) != PLUS_SUCCESS )
    {
      LOCAL_LOG_ERROR( "Copy of field data source '" << it->first << "' failed - unable to get field data source from container!" );
      continue;
    }

    aSource->DeepCopy( it->second );
  }

  this->InternalUpdateRate = device->GetInternalUpdateRate();
  this->SetAcquisitionRate( device->GetAcquisitionRate() );
}

//-----------------------------------------------------------------------------
double vtkPlusDevice::GetInternalUpdateRate() const
{
  return this->InternalUpdateRate;
}

//-----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::SetAcquisitionRate( double aRate )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::SetAcquisitionRate(" << aRate << ")" );

  if ( this->AcquisitionRate == aRate )
  {
    return PLUS_SUCCESS;
  }

  this->AcquisitionRate = aRate;
  this->Modified();

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::WriteToolsToSequenceFile( const char* filename, bool useCompression /*= false*/ )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::WriteToolsToSequenceFile: " << filename );

  if ( this->GetNumberOfTools() == 0 )
  {
    LOCAL_LOG_ERROR( "Failed to write tracker to metafile - there are no active tools!" );
    return PLUS_FAIL;
  }

  // Get the number of items from buffers and use the lowest
  int numberOfItems( -1 );
  for ( DataSourceContainerConstIterator it = this->Tools.begin(); it != this->Tools.end(); ++it )
  {
    if ( numberOfItems < 0 || numberOfItems > it->second->GetNumberOfItems() )
    {
      numberOfItems = it->second->GetNumberOfItems();
    }
  }

  vtkSmartPointer<vtkPlusTrackedFrameList> trackedFrameList = vtkSmartPointer<vtkPlusTrackedFrameList>::New();

  PlusStatus status = PLUS_SUCCESS;

  // Get the first source
  vtkPlusDataSource* firstActiveTool = this->Tools.begin()->second;

  for ( int i = 0 ; i < numberOfItems; i++ )
  {
    //Create fake image
    PlusTrackedFrame trackedFrame;
    PlusVideoFrame videoFrame;
    int frameSize[3] = {1, 1, 1};
    // Don't waste space, create a greyscale image
    videoFrame.AllocateFrame( frameSize, VTK_UNSIGNED_CHAR, 1 );
    trackedFrame.SetImageData( videoFrame );

    StreamBufferItem bufferItem;
    BufferItemUidType uid = firstActiveTool->GetOldestItemUidInBuffer() + i;

    if ( firstActiveTool->GetStreamBufferItem( uid, &bufferItem ) != ITEM_OK )
    {
      LOCAL_LOG_ERROR( "Failed to get tracker buffer item with UID: " << uid );
      continue;
    }

    const double frameTimestamp = bufferItem.GetFilteredTimestamp( firstActiveTool->GetLocalTimeOffsetSec() );

    // Add main source timestamp
    std::ostringstream timestampFieldValue;
    timestampFieldValue << std::fixed << frameTimestamp;
    trackedFrame.SetCustomFrameField( "Timestamp", timestampFieldValue.str() );

    // Add main source unfiltered timestamp
    std::ostringstream unfilteredtimestampFieldValue;
    unfilteredtimestampFieldValue << std::fixed << bufferItem.GetUnfilteredTimestamp( firstActiveTool->GetLocalTimeOffsetSec() );
    trackedFrame.SetCustomFrameField( "UnfilteredTimestamp", unfilteredtimestampFieldValue.str() );

    // Add main source frameNumber
    std::ostringstream frameNumberFieldValue;
    frameNumberFieldValue << std::fixed << bufferItem.GetIndex();
    trackedFrame.SetCustomFrameField( "FrameNumber", frameNumberFieldValue.str() );

    // Add transforms
    for ( DataSourceContainerConstIterator it = this->Tools.begin(); it != this->Tools.end(); ++it )
    {
      StreamBufferItem toolBufferItem;
      if ( it->second->GetStreamBufferItemFromTime( frameTimestamp, &toolBufferItem, vtkPlusBuffer::EXACT_TIME ) != ITEM_OK )
      {
        LOCAL_LOG_ERROR( "Failed to get tracker buffer item from time: " << std::fixed << frameTimestamp );
        continue;
      }

      vtkSmartPointer<vtkMatrix4x4> toolMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      if ( toolBufferItem.GetMatrix( toolMatrix ) != PLUS_SUCCESS )
      {
        LOCAL_LOG_ERROR( "Failed to get toolMatrix" );
        return PLUS_FAIL;
      }

      PlusTransformName toolToTrackerTransform( it->second->GetSourceId(), this->ToolReferenceFrameName );
      trackedFrame.SetCustomFrameTransform( toolToTrackerTransform, toolMatrix );

      // Add source status
      trackedFrame.SetCustomFrameTransformStatus( toolToTrackerTransform, vtkPlusDevice::ConvertToolStatusToTrackedFrameFieldStatus( toolBufferItem.GetStatus() ) );
    }

    // Add tracked frame to the list
    // This is a debugging/test function, so the additional copying in AddTrackedFrame compared to TakeTrackedFrame is not relevant.
    trackedFrameList->AddTrackedFrame( &trackedFrame );
  }

  // Save tracked frames to metafile
  if( vtkPlusSequenceIO::Write( filename, trackedFrameList, trackedFrameList->GetImageOrientation(), useCompression ) != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Failed to save tracked frames to sequence metafile!" );
    return PLUS_FAIL;
  }

  return status;
}

//----------------------------------------------------------------------------
TrackedFrameFieldStatus vtkPlusDevice::ConvertToolStatusToTrackedFrameFieldStatus( ToolStatus status )
{
  TrackedFrameFieldStatus fieldStatus = FIELD_INVALID;
  if ( status == TOOL_OK )
  {
    fieldStatus = FIELD_OK;
  }

  return fieldStatus;
}

//----------------------------------------------------------------------------
ToolStatus vtkPlusDevice::ConvertTrackedFrameFieldStatusToToolStatus( TrackedFrameFieldStatus fieldStatus )
{
  ToolStatus status = TOOL_MISSING;
  if ( fieldStatus == FIELD_OK )
  {
    status = TOOL_OK ;
  }

  return status;
}

//----------------------------------------------------------------------------
std::string vtkPlusDevice::ConvertToolStatusToString( ToolStatus status )
{
  std::string flagFieldValue;
  if ( status == TOOL_OK )
  {
    flagFieldValue = "OK";
  }
  else if ( status == TOOL_MISSING )
  {
    flagFieldValue = "TOOL_MISSING";
  }
  else if ( status == TOOL_OUT_OF_VIEW )
  {
    flagFieldValue = "TOOL_OUT_OF_VIEW";
  }
  else if ( status == TOOL_OUT_OF_VOLUME )
  {
    flagFieldValue = "TOOL_OUT_OF_VOLUME";
  }
  else if ( status == TOOL_REQ_TIMEOUT )
  {
    flagFieldValue = "TOOL_REQ_TIMEOUT";
  }
  else
  {
    LOG_ERROR( "Unknown tracker status received - set TOOL_INVALID by default!" );
    flagFieldValue = "TOOL_INVALID";
  }

  return flagFieldValue;
}


//----------------------------------------------------------------------------
vtkXMLDataElement* vtkPlusDevice::FindThisDeviceElement( vtkXMLDataElement* rootXMLElement )
{
  if( rootXMLElement == NULL )
  {
    LOCAL_LOG_ERROR( "Unable to find device XML element for this device." );
    return NULL;
  }

  vtkXMLDataElement* dataCollectionElement = rootXMLElement->FindNestedElementWithName( "DataCollection" );
  if ( dataCollectionElement == NULL )
  {
    LOCAL_LOG_ERROR( "Unable to find data collection element in XML tree!" );
    return NULL;
  }

  vtkXMLDataElement* deviceXMLElement = NULL;
  for ( int i = 0; i < dataCollectionElement->GetNumberOfNestedElements(); ++i )
  {
    deviceXMLElement = dataCollectionElement->GetNestedElement( i );

    if ( deviceXMLElement->GetName() != NULL && deviceXMLElement->GetAttribute( "Id" ) != NULL &&
         STRCASECMP( deviceXMLElement->GetName(), "Device" ) == 0 && STRCASECMP( deviceXMLElement->GetAttribute( "Id" ), this->GetDeviceId() ) == 0 )
    {
      return deviceXMLElement;
    }
  }

  return NULL;
}

//----------------------------------------------------------------------------
vtkXMLDataElement* vtkPlusDevice::FindOutputChannelElement( vtkXMLDataElement* rootXMLElement, const char* aChannelId )
{
  if( rootXMLElement == NULL || aChannelId == NULL )
  {
    LOCAL_LOG_ERROR( "Unable to find XML element for this channel. Bad inputs." );
    return NULL;
  }

  vtkXMLDataElement* deviceXMLElement = this->FindThisDeviceElement( rootXMLElement );

  vtkXMLDataElement* outputChannelsElement = deviceXMLElement->FindNestedElementWithName( "OutputChannels" );
  if( outputChannelsElement != NULL )
  {
    for ( int i = 0; i < outputChannelsElement->GetNumberOfNestedElements(); ++i )
    {
      vtkXMLDataElement* anXMLElement = outputChannelsElement->GetNestedElement( i );

      if ( anXMLElement->GetName() != NULL && anXMLElement->GetAttribute( "Id" ) != NULL &&
           STRCASECMP( anXMLElement->GetName(), "OutputChannel" ) == 0 && STRCASECMP( anXMLElement->GetAttribute( "Id" ), aChannelId ) == 0 )
      {
        return anXMLElement;
      }
    }
  }

  return NULL;
}

//----------------------------------------------------------------------------
vtkXMLDataElement* vtkPlusDevice::FindInputChannelElement( vtkXMLDataElement* rootXMLElement, const char* aChannelId )
{
  if( rootXMLElement == NULL || aChannelId == NULL )
  {
    LOCAL_LOG_ERROR( "Unable to find XML element for this channel. Bad inputs." );
    return NULL;
  }

  vtkXMLDataElement* deviceXMLElement = this->FindThisDeviceElement( rootXMLElement );

  vtkXMLDataElement* inputChannelsElement = deviceXMLElement->FindNestedElementWithName( "InputChannels" );
  if( inputChannelsElement != NULL )
  {
    for ( int i = 0; i < inputChannelsElement->GetNumberOfNestedElements(); ++i )
    {
      vtkXMLDataElement* anXMLElement = inputChannelsElement->GetNestedElement( i );

      if ( anXMLElement->GetName() != NULL && anXMLElement->GetAttribute( "Id" ) != NULL &&
           STRCASECMP( anXMLElement->GetName(), "InputChannel" ) == 0 && STRCASECMP( anXMLElement->GetAttribute( "Id" ), aChannelId ) == 0 )
      {
        return anXMLElement;
      }
    }
  }

  return NULL;
}

//-----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::ReadConfiguration( vtkXMLDataElement* rootXMLElement )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::ReadConfiguration" );

  if( rootXMLElement == NULL )
  {
    LOCAL_LOG_ERROR( "Unable to find device XML element for this device." );
    return PLUS_FAIL;
  }

  vtkXMLDataElement* deviceXMLElement = this->FindThisDeviceElement( rootXMLElement );

  if( deviceXMLElement == NULL )
  {
    LOCAL_LOG_ERROR( "Unable to find device XML element for device" );
    return PLUS_FAIL;
  }

  // Continue with device configuration
  const char* referenceName = deviceXMLElement->GetAttribute( "ToolReferenceFrame" );
  if ( referenceName != NULL )
  {
    this->SetToolReferenceFrameName( referenceName );
  }
  else if( this->IsTracker() )
  {
    this->SetToolReferenceFrameName( this->GetDeviceId() );
    LOCAL_LOG_WARNING( "ToolReferenceFrame is undefined. Default \"" << ( this->GetToolReferenceFrameName() ? this->GetToolReferenceFrameName() : "(undefined)" ) << "\" will be used." );
  }

  if( deviceXMLElement->GetAttribute( "MissingInputGracePeriodSec" ) != NULL )
  {
    deviceXMLElement->GetScalarAttribute( "MissingInputGracePeriodSec", this->MissingInputGracePeriodSec );
  }

  vtkXMLDataElement* dataSourcesElement = deviceXMLElement->FindNestedElementWithName( "DataSources" );
  if( dataSourcesElement != NULL )
  {
    // Read source configurations
    for ( int source = 0; source < dataSourcesElement->GetNumberOfNestedElements(); source++ )
    {
      vtkXMLDataElement* dataSourceElement = dataSourcesElement->GetNestedElement( source );

      if ( STRCASECMP( dataSourceElement->GetName(), "DataSource" ) != 0 )
      {
        // if this is not a Tool element, skip it
        continue;
      }

      vtkSmartPointer<vtkPlusDataSource> aDataSource = vtkSmartPointer<vtkPlusDataSource>::New();
      if( dataSourceElement->GetAttribute( "Type" ) != NULL && STRCASECMP( dataSourceElement->GetAttribute( "Type" ), "Tool" ) == 0 )
      {
        aDataSource->SetReferenceCoordinateFrameName( this->ToolReferenceFrameName );

        if ( aDataSource->ReadConfiguration( dataSourceElement, this->RequirePortNameInDeviceSetConfiguration, this->RequireImageOrientationInConfiguration, this->GetDeviceId() ) != PLUS_SUCCESS )
        {
          LOCAL_LOG_ERROR( "Unable to add tool to tracker - failed to read tool configuration" );
          continue;
        }

        if ( this->AddTool( aDataSource ) != PLUS_SUCCESS )
        {
          LOCAL_LOG_ERROR( "Failed to add tool '" << ( aDataSource->GetSourceId() ? aDataSource->GetSourceId() : "(unspecified)" ) << "' to device on port " << ( aDataSource->GetPortName() ? aDataSource->GetPortName() : "(unspecified)" ) );
        }
      }
      else if( dataSourceElement->GetAttribute( "Type" ) != NULL && STRCASECMP( dataSourceElement->GetAttribute( "Type" ), "FieldData" ) == 0 )
      {
        if ( aDataSource->ReadConfiguration( dataSourceElement, this->RequirePortNameInDeviceSetConfiguration, this->RequireImageOrientationInConfiguration, this->GetDeviceId() ) != PLUS_SUCCESS )
        {
          LOCAL_LOG_ERROR( "Unable to add field data source to device - failed to read field data source configuration" );
          continue;
        }

        if ( this->AddFieldDataSource( aDataSource ) != PLUS_SUCCESS )
        {
          LOCAL_LOG_ERROR( "Failed to add field data source '" << ( aDataSource->GetSourceId() ? aDataSource->GetSourceId() : "(unspecified)" ) << "' to device." );
        }
      }
      else if( dataSourceElement->GetAttribute( "Type" ) != NULL && STRCASECMP( dataSourceElement->GetAttribute( "Type" ), "Video" ) == 0 )
      {
        aDataSource->ReadConfiguration( dataSourceElement, this->RequirePortNameInDeviceSetConfiguration, this->RequireImageOrientationInConfiguration, this->GetDeviceId() );

        if ( this->AddVideo( aDataSource ) != PLUS_SUCCESS )
        {
          LOCAL_LOG_ERROR( "Failed to add video source '" << ( aDataSource->GetSourceId() ? aDataSource->GetSourceId() : "(unspecified)" ) << "' to device." );
        }
      }
    }
  }

  // Continue with device configuration
  int acquisitionRate = 0;
  if ( deviceXMLElement->GetScalarAttribute( "AcquisitionRate", acquisitionRate ) )
  {
    this->SetAcquisitionRate( acquisitionRate );
  }
  else
  {
    LOCAL_LOG_DEBUG( "Unable to find acquisition rate in device element when it is required, using default " << this->GetAcquisitionRate() );
  }

  vtkXMLDataElement* outputChannelsElement = deviceXMLElement->FindNestedElementWithName( "OutputChannels" );
  if( outputChannelsElement != NULL )
  {
    // Now that we have the tools and images, we can create the output streams and connect things as necessary
    for ( int channel = 0; channel < outputChannelsElement->GetNumberOfNestedElements(); channel++ )
    {
      vtkXMLDataElement* channelElement = outputChannelsElement->GetNestedElement( channel );
      if ( STRCASECMP( channelElement->GetName(), "OutputChannel" ) != 0 )
      {
        // if this is not a channel element, skip it
        continue;
      }

      if( channelElement->GetAttribute( "Id" ) == NULL )
      {
        LOCAL_LOG_ERROR( "No channel Id present. Skipping channel configuration." );
        continue;
      }

      bool skip( false );
      for( ChannelContainerIterator it = this->OutputChannels.begin(); it != this->OutputChannels.end(); ++it )
      {
        if( STRCASECMP( ( *it )->GetChannelId(), channelElement->GetAttribute( "Id" ) ) == 0 )
        {
          LOCAL_LOG_ERROR( "Channel with duplicate channel Id \'" << ( *it )->GetChannelId() << "\'. Skipping channel configuration." );
          skip = true;
          break;
        }
      }
      if( skip )
      {
        continue;
      }

      vtkSmartPointer<vtkPlusChannel> aChannel = vtkSmartPointer<vtkPlusChannel>::New();
      aChannel->SetOwnerDevice( this );
      aChannel->ReadConfiguration( channelElement, this->RequireImageOrientationInConfiguration );
      AddOutputChannel( aChannel );
    }
  }

  double localTimeOffsetSec = 0;
  if ( deviceXMLElement->GetScalarAttribute( "LocalTimeOffsetSec", localTimeOffsetSec ) )
  {
    // DeviceId is never null, if it is, there is memory stomping occurring elsewhere
    LOCAL_LOG_INFO( "Local time offset: " << 1000 * localTimeOffsetSec << "ms" );
    this->SetLocalTimeOffsetSec( localTimeOffsetSec );
  }
  else
  {
    LOCAL_LOG_DEBUG( "Local time offset was not defined in device configuration" );
  }

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::WriteConfiguration( vtkXMLDataElement* config )
{
  if ( config == NULL )
  {
    LOCAL_LOG_ERROR( "Unable to write configuration: xml data element is NULL!" );
    return PLUS_FAIL;
  }

  vtkXMLDataElement* deviceDataElement = this->FindThisDeviceElement( config );

  if ( deviceDataElement == NULL )
  {
    LOCAL_LOG_ERROR( "Unable to write configuration: xml data element is NULL!" );
    return PLUS_FAIL;
  }

  vtkXMLDataElement* dataSourcesElement = deviceDataElement->FindNestedElementWithName( "DataSources" );
  if( dataSourcesElement != NULL )
  {
    for ( int i = 0; i < dataSourcesElement->GetNumberOfNestedElements(); i++ )
    {
      vtkXMLDataElement* dataSourceElement = dataSourcesElement->GetNestedElement( i );
      if ( STRCASECMP( dataSourceElement->GetName(), "DataSource" ) != 0 )
      {
        // if this is not a data source element, skip it
        continue;
      }
      vtkPlusDataSource* aDataSource = NULL;
      if( dataSourceElement->GetAttribute( "Type" ) != NULL && STRCASECMP( dataSourceElement->GetAttribute( "Type" ), vtkPlusDataSource::DATA_SOURCE_TYPE_TOOL_TAG.c_str() ) == 0 )
      {
        PlusTransformName toolId( dataSourceElement->GetAttribute( "Id" ), this->GetToolReferenceFrameName() );
        if( dataSourceElement->GetAttribute( "Id" ) == NULL || this->GetTool( toolId.GetTransformName(), aDataSource ) != PLUS_SUCCESS )
        {
          LOCAL_LOG_ERROR( "Unable to retrieve tool when saving config." );
          return PLUS_FAIL;
        }
        aDataSource->WriteConfiguration( dataSourceElement );
      }
      if( dataSourceElement->GetAttribute( "Type" ) != NULL && STRCASECMP( dataSourceElement->GetAttribute( "Type" ), vtkPlusDataSource::DATA_SOURCE_TYPE_FIELDDATA_TAG.c_str() ) == 0 )
      {
        if( dataSourceElement->GetAttribute( "Id" ) == NULL || this->GetFieldDataSource( dataSourceElement->GetAttribute( "Id" ), aDataSource ) != PLUS_SUCCESS )
        {
          LOCAL_LOG_ERROR( "Unable to retrieve field data source when saving config." );
          return PLUS_FAIL;
        }
        aDataSource->WriteConfiguration( dataSourceElement );
      }
      else if( dataSourceElement->GetAttribute( "Type" ) != NULL && STRCASECMP( dataSourceElement->GetAttribute( "Type" ), vtkPlusDataSource::DATA_SOURCE_TYPE_VIDEO_TAG.c_str() ) == 0 )
      {
        if( dataSourceElement->GetAttribute( "Id" ) == NULL || this->GetVideoSource( dataSourceElement->GetAttribute( "Id" ), aDataSource ) != PLUS_SUCCESS )
        {
          LOCAL_LOG_ERROR( "Unable to retrieve video source when saving config." );
          return PLUS_FAIL;
        }
        aDataSource->WriteConfiguration( dataSourceElement );
      }
    }
  }

  this->InternalWriteOutputChannels( config );

  this->InternalWriteInputChannels( config );

  if( this->GetLocalTimeOffsetSec() != 0.0 )
  {
    deviceDataElement->SetDoubleAttribute( "LocalTimeOffsetSec", this->GetLocalTimeOffsetSec() );
  }

  return PLUS_SUCCESS;
}


//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::Connect()
{
  LOCAL_LOG_DEBUG( "vtkPlusDevice::Connect: " << ( this->DeviceId ? this->DeviceId : "undefined" ) );

  if ( this->Connected )
  {
    LOCAL_LOG_DEBUG( "Already connected to the data source" );
    return PLUS_SUCCESS;
  }

  // We will report unknown tools after each Connect
  this->ReportedUnknownTools.clear();

  if ( this->InternalConnect() != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Cannot connect to data source, ConnectInternal failed" );
    return PLUS_FAIL;
  }

  this->Connected = 1;

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::Disconnect()
{
  LOCAL_LOG_DEBUG( "vtkPlusDevice::Disconnect" );

  if ( !this->Connected )
  {
    LOCAL_LOG_DEBUG( "Data source is already disconnected" );
    return PLUS_SUCCESS;
  }
  if ( this->Recording )
  {
    this->StopRecording();
  }

  this->Connected = 0;

  InternalDisconnect();

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
// Set the source to acquire data continuously.
// You should override this as appropriate for your device.
PlusStatus vtkPlusDevice::StartRecording()
{
  LOCAL_LOG_DEBUG( "vtkPlusDevice::StartRecording" );

  if ( this->Recording )
  {
    LOCAL_LOG_DEBUG( "Recording is already active" );
    return PLUS_SUCCESS;
  }

  if ( !this->Connected )
  {
    if ( this->Connect() != PLUS_SUCCESS )
    {
      LOCAL_LOG_ERROR( "Cannot start recording, connection failed" );
      return PLUS_FAIL;
    }
  }

  if ( this->InternalStartRecording() != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Cannot start recording, internal StartRecording failed" );
    return PLUS_FAIL;
  }

  this->RecordingStartTime = vtkPlusAccurateTimer::GetSystemTime();
  this->Recording = 1;

  if( this->StartThreadForInternalUpdates )
  {
    this->ThreadId =
      this->Threader->SpawnThread( ( vtkThreadFunctionType )\
                                   &vtkDataCaptureThread, this );
  }

  this->Modified();

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
// Stop continuous acquisition.  You will have to override this
// if your class overrides Record()
PlusStatus vtkPlusDevice::StopRecording()
{
  LOCAL_LOG_DEBUG( "vtkPlusDevice::StopRecording" );

  if ( !this->Recording )
  {
    LOCAL_LOG_DEBUG( "Recording is already inactive" );
    return PLUS_SUCCESS;
  }

  this->ThreadId = -1;
  this->Recording = 0;

  if( this->GetStartThreadForInternalUpdates() )
  {
    LOCAL_LOG_DEBUG( "Wait for internal update thread to terminate" );
    // Let's give a chance to the thread to stop before we kill the connection
    while ( this->ThreadAlive )
    {
      vtkPlusAccurateTimer::Delay( 0.1 );
    }
    this->ThreadId = -1;
    LOCAL_LOG_DEBUG( "Internal update thread terminated" );
  }

  if ( this->InternalStopRecording() != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Failed to stop tracking thread!" );
    return PLUS_FAIL;
  }

  this->Modified();

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
// this function runs in an alternate thread to asynchronously acquire data
void* vtkPlusDevice::vtkDataCaptureThread( vtkMultiThreader::ThreadInfo* data )
{
  vtkPlusDevice* self = ( vtkPlusDevice* )( data->UserData );

  double rate = self->GetAcquisitionRate();
  double currtime[FRAME_RATE_AVERAGING] = {0};
  unsigned long updatecount = 0;
  self->ThreadAlive = true;

  while ( self->IsRecording() && self->GetCorrectlyConfigured() )
  {
    double newtime = vtkPlusAccurateTimer::GetSystemTime();
    // get current tracking rate over last few updates
    double difftime = newtime - currtime[updatecount % FRAME_RATE_AVERAGING];
    currtime[updatecount % FRAME_RATE_AVERAGING] = newtime;
    if ( updatecount > FRAME_RATE_AVERAGING && difftime != 0 )
    {
      self->InternalUpdateRate = ( FRAME_RATE_AVERAGING / difftime );
    }

    {
      // Lock before update
      PlusLockGuard<vtkPlusRecursiveCriticalSection> updateMutexGuardedLock( self->UpdateMutex );
      if ( !self->Recording )
      {
        // recording has been stopped
        break;
      }
      self->InternalUpdate();
      self->UpdateTime.Modified();
    }

    double delay = ( newtime + 1.0 / rate - vtkPlusAccurateTimer::GetSystemTime() );
    if ( delay > 0 )
    {
      vtkPlusAccurateTimer::Delay( delay );
    }

    updatecount++;
  }

  self->ThreadAlive = false;
  return NULL;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetBufferSize( vtkPlusChannel& aChannel, int& outVal, const char* aSourceId /*= NULL*/ )
{
  LOCAL_LOG_TRACE( "vtkPlusDeviceg::GetBufferSize" );

  if( aSourceId == NULL )
  {
    vtkPlusDataSource* aSource( NULL );
    if( aChannel.GetVideoSource( aSource ) != PLUS_SUCCESS )
    {
      LOCAL_LOG_ERROR( "Unable to retrieve the buffer size of the video source in channel: " << aChannel.GetChannelId() << "." );
      return PLUS_FAIL;
    }

    outVal = aSource->GetBufferSize();
    return PLUS_SUCCESS;
  }

  for ( DataSourceContainerConstIterator it = this->GetToolIteratorBegin(); it != this->GetToolIteratorEnd(); ++it )
  {
    if( STRCASECMP( it->second->GetSourceId(), aSourceId ) == 0 )
    {
      outVal = it->second->GetBufferSize();
      return PLUS_SUCCESS;
    }
  }

  for ( DataSourceContainerConstIterator it = this->GetFieldDataSourcessIteratorBegin(); it != this->GetFieldDataSourcessIteratorEnd(); ++it )
  {
    if( STRCASECMP( it->second->GetSourceId(), aSourceId ) == 0 )
    {
      outVal = it->second->GetBufferSize();
      return PLUS_SUCCESS;
    }
  }

  LOCAL_LOG_ERROR( "Unable to find tool " << aSourceId << " when requesting its buffer size." );
  return PLUS_FAIL;
}

//----------------------------------------------------------------------------
// set or change the circular buffer size
// you will have to override this if you want the buffers
// to be device-specific (i.e. something other than vtkDataArray)
PlusStatus vtkPlusDevice::SetBufferSize( vtkPlusChannel& aChannel, int FrameBufferSize, const char* aSourceId /*= NULL*/ )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::SetBufferSize(" << FrameBufferSize << ")" );

  if ( FrameBufferSize < 0 )
  {
    LOCAL_LOG_ERROR( "vtkPlusDevice::SetBufferSize: There must be at least one framebuffer" );
    return PLUS_FAIL;
  }

  if( aSourceId == NULL )
  {
    vtkPlusDataSource* aSource( NULL );
    if( aChannel.GetVideoSource( aSource ) != PLUS_SUCCESS )
    {
      LOCAL_LOG_ERROR( "Unable to retrieve the video source." );
      return PLUS_FAIL;
    }

    // update the buffer size
    if ( aSource->SetBufferSize( FrameBufferSize ) != PLUS_SUCCESS )
    {
      LOCAL_LOG_ERROR( "Failed to set buffer size!" );
      return PLUS_FAIL;
    }
    return PLUS_SUCCESS;
  }

  for ( DataSourceContainerConstIterator it = this->GetToolIteratorBegin(); it != this->GetToolIteratorEnd(); ++it )
  {
    if( STRCASECMP( it->second->GetSourceId(), aSourceId ) == 0 )
    {
      it->second->SetBufferSize( FrameBufferSize );
    }
  }

  for ( DataSourceContainerConstIterator it = this->GetFieldDataSourcessIteratorBegin(); it != this->GetFieldDataSourcessIteratorEnd(); ++it )
  {
    if( STRCASECMP( it->second->GetSourceId(), aSourceId ) == 0 )
    {
      it->second->SetBufferSize( FrameBufferSize );
    }
  }

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
void vtkPlusDevice::SetStartTime( double startTime )
{
  for ( DataSourceContainerConstIterator it = this->GetVideoIteratorBegin(); it != this->GetVideoIteratorEnd(); ++it )
  {
    it->second->SetStartTime( startTime );
  }

  for ( DataSourceContainerConstIterator it = this->GetToolIteratorBegin(); it != this->GetToolIteratorEnd(); ++it )
  {
    it->second->SetStartTime( startTime );
  }

  for ( DataSourceContainerConstIterator it = this->GetFieldDataSourcessIteratorBegin(); it != this->GetFieldDataSourcessIteratorEnd(); ++it )
  {
    it->second->SetStartTime( startTime );
  }
}

//----------------------------------------------------------------------------
double vtkPlusDevice::GetStartTime()
{
  // TODO : should this return the buffer start time or should the buffer start time
  //        simply be included in the average
  double sumStartTime = 0.0;
  double numberOfBuffers( 0 );
  for ( DataSourceContainerConstIterator it = this->GetVideoIteratorBegin(); it != this->GetVideoIteratorEnd(); ++it )
  {
    sumStartTime += it->second->GetStartTime();
    numberOfBuffers++;
  }

  for ( DataSourceContainerConstIterator it = this->GetToolIteratorBegin(); it != this->GetToolIteratorEnd(); ++it )
  {
    sumStartTime += it->second->GetStartTime();
    numberOfBuffers++;
  }

  for ( DataSourceContainerConstIterator it = this->GetFieldDataSourcessIteratorBegin(); it != this->GetFieldDataSourcessIteratorEnd(); ++it )
  {
    sumStartTime += it->second->GetStartTime();
    numberOfBuffers++;
  }

  return sumStartTime / numberOfBuffers;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::Probe()
{
  PlusLockGuard<vtkPlusRecursiveCriticalSection> updateMutexGuardedLock( this->UpdateMutex );

  if ( this->InternalStartRecording() != PLUS_SUCCESS )
  {
    return PLUS_FAIL;
  }

  if ( this->InternalStopRecording() != PLUS_SUCCESS )
  {
    return PLUS_FAIL;
  }

  return PLUS_SUCCESS;
}

//-----------------------------------------------------------------------------
void vtkPlusDevice::ClearAllBuffers()
{
  for ( DataSourceContainerConstIterator it = this->GetVideoIteratorBegin(); it != this->GetVideoIteratorEnd(); ++it )
  {
    it->second->Clear();
  }
  for ( DataSourceContainerConstIterator it = this->GetToolIteratorBegin(); it != this->GetToolIteratorEnd(); ++it )
  {
    it->second->Clear();
  }
  for ( DataSourceContainerConstIterator it = this->GetFieldDataSourcessIteratorBegin(); it != this->GetFieldDataSourcessIteratorEnd(); ++it )
  {
    it->second->Clear();
  }
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::ForceUpdate()
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::ForceUpdate" );

  if ( !this->Connected )
  {
    if ( this->Connect() != PLUS_SUCCESS )
    {
      LOCAL_LOG_ERROR( "Cannot acquire a single frame, connection failed" );
      return PLUS_FAIL;
    }
  }

  {
    PlusLockGuard<vtkPlusRecursiveCriticalSection> updateMutexGuardedLock( this->UpdateMutex );
    this->InternalUpdate();
  }
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::ToolTimeStampedUpdateWithoutFiltering( const char* aToolSourceId, vtkMatrix4x4* matrix, ToolStatus status, double unfilteredtimestamp, double filteredtimestamp, const PlusTrackedFrame::FieldMapType* customFields /* = NULL */ )
{
  if ( aToolSourceId == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to update tool - tool source ID is NULL!" );
    return PLUS_FAIL;
  }

  vtkPlusDataSource* tool = NULL;
  if ( this->GetTool( aToolSourceId, tool ) != PLUS_SUCCESS )
  {
    if ( this->ReportedUnknownTools.find( aToolSourceId ) == this->ReportedUnknownTools.end() )
    {
      // We have not reported yet that this tool is unknown
      LOCAL_LOG_ERROR( "Failed to update tool - unable to find tool: " << aToolSourceId );
      this->ReportedUnknownTools.insert( std::string( aToolSourceId ) );
    }
    return PLUS_FAIL;
  }

  // This function is for devices has no frame numbering, just auto increment tool frame number if new frame received
  unsigned long frameNumber = tool->GetFrameNumber() + 1 ;
  PlusStatus bufferStatus = tool->AddTimeStampedItem( matrix, status, frameNumber, unfilteredtimestamp, filteredtimestamp, customFields );
  tool->SetFrameNumber( frameNumber );

  return bufferStatus;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::AddVideoItemToVideoSources( const std::vector<vtkPlusDataSource*>& videoSources, const PlusVideoFrame& frame, long frameNumber, double unfilteredTimestamp/*=UNDEFINED_TIMESTAMP*/, double filteredTimestamp/*=UNDEFINED_TIMESTAMP*/, const PlusTrackedFrame::FieldMapType* customFields /*= NULL*/ )
{
  PlusStatus result( PLUS_SUCCESS );
  for( std::vector<vtkPlusDataSource*>::const_iterator it = videoSources.begin(); it != videoSources.end(); ++it )
  {
    vtkPlusDataSource* source = *it;
    if( source->AddItem( &frame, frameNumber, unfilteredTimestamp, filteredTimestamp, customFields ) != PLUS_SUCCESS )
    {
      result = PLUS_FAIL;
    }
  }
  return result;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::AddVideoItemToVideoSources( const std::vector<vtkPlusDataSource*>& videoSources, void* imageDataPtr, US_IMAGE_ORIENTATION usImageOrientation, const int frameSizeInPx[3], PlusCommon::VTKScalarPixelType pixelType, int numberOfScalarComponents, US_IMAGE_TYPE imageType, int numberOfBytesToSkip, long frameNumber, double unfilteredTimestamp/*=UNDEFINED_TIMESTAMP*/, double filteredTimestamp/*=UNDEFINED_TIMESTAMP*/, const PlusTrackedFrame::FieldMapType* customFields/*= NULL*/ )
{
  if ( frameSizeInPx[0] < 0 || frameSizeInPx[1] < 0 || frameSizeInPx[2] < 0 || numberOfScalarComponents < 0 )
  {
    LOG_ERROR( "Invalid negative values sent to vtkPlusUsDevice::AddVideoItemToVideoSources. Aborting." );
    return PLUS_FAIL;
  }

  unsigned int frameSizeInPxUint[3] = { static_cast<unsigned int>( frameSizeInPx[0] ), static_cast<unsigned int>( frameSizeInPx[1] ), static_cast<unsigned int>( frameSizeInPx[2] ) };
  return this->AddVideoItemToVideoSources( videoSources, imageDataPtr, usImageOrientation, frameSizeInPxUint, pixelType, static_cast<unsigned int>( numberOfScalarComponents ), imageType, numberOfBytesToSkip, frameNumber, unfilteredTimestamp, filteredTimestamp, customFields );
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::AddVideoItemToVideoSources( const std::vector<vtkPlusDataSource*>& videoSources, void* imageDataPtr, US_IMAGE_ORIENTATION usImageOrientation, const unsigned int frameSizeInPx[3], PlusCommon::VTKScalarPixelType pixelType, unsigned int numberOfScalarComponents, US_IMAGE_TYPE imageType, int numberOfBytesToSkip, long frameNumber, double unfilteredTimestamp /*= UNDEFINED_TIMESTAMP*/, double filteredTimestamp /*= UNDEFINED_TIMESTAMP*/, const PlusTrackedFrame::FieldMapType* customFields /*= NULL*/ )
{
  PlusStatus result( PLUS_SUCCESS );
  for ( std::vector<vtkPlusDataSource*>::const_iterator it = videoSources.begin(); it != videoSources.end(); ++it )
  {
    vtkPlusDataSource* source = *it;
    if ( source->AddItem( imageDataPtr, usImageOrientation, frameSizeInPx, pixelType, numberOfScalarComponents, imageType, numberOfBytesToSkip, frameNumber, unfilteredTimestamp, filteredTimestamp, customFields ) != PLUS_SUCCESS )
    {
      result = PLUS_FAIL;
    }
  }
  return result;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::ToolTimeStampedUpdate( const char* aToolSourceId, vtkMatrix4x4* matrix, ToolStatus status, unsigned long frameNumber, double unfilteredtimestamp, const PlusTrackedFrame::FieldMapType* customFields/*= NULL*/ )
{
  if ( aToolSourceId == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to update tool - tool source ID is NULL!" );
    return PLUS_FAIL;
  }

  vtkPlusDataSource* tool = NULL;
  if ( this->GetTool( aToolSourceId, tool ) != PLUS_SUCCESS )
  {
    if ( this->ReportedUnknownTools.find( aToolSourceId ) == this->ReportedUnknownTools.end() )
    {
      // We have not reported yet that this tool is unknown
      LOCAL_LOG_ERROR( "Failed to update tool - unable to find tool: " << aToolSourceId );
      this->ReportedUnknownTools.insert( std::string( aToolSourceId ) );
    }
    return PLUS_FAIL;
  }

  PlusStatus bufferStatus = tool->AddTimeStampedItem( matrix, status, frameNumber, unfilteredtimestamp, UNDEFINED_TIMESTAMP, customFields );
  tool->SetFrameNumber( frameNumber );

  return bufferStatus;
}

//----------------------------------------------------------------------------
// This method returns the largest data that can be generated.
int vtkPlusDevice::RequestInformation( vtkInformation* vtkNotUsed( request ),
                                       vtkInformationVector** vtkNotUsed( inputVector ),
                                       vtkInformationVector* outputVector )
{
  //LOCAL_LOG_TRACE("vtkPlusDevice::RequestInformation");

  if ( !this->Connected )
  {
    this->Connect();
  }

  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject( 0 );

  // Find a video source and set extent
  vtkPlusDataSource* aSource( NULL );
  for ( ChannelContainerIterator it = this->OutputChannels.begin(); it != this->OutputChannels.end(); ++it )
  {
    if ( ( *it )->GetVideoSource( aSource ) == PLUS_SUCCESS )
    {
      break;
    }
  }
  if ( aSource == NULL )
  {
    return 0;
  }

  unsigned int* frameSize = aSource->GetOutputFrameSize();
  // TODO : determine another mechanism to see if device has data yet or not

  int extent[6] = { 0, static_cast<int>( frameSize[0] ) - 1, 0, static_cast<int>( frameSize[1] ) - 1, 0, static_cast<int>( frameSize[2] ) - 1 };
  outInfo->Set( vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6 );

  // Set the origin and spacing. The video source provides raw pixel output,
  // therefore the spacing is (1,1,1) and the origin is (0,0,0)
  double spacing[3] = { 1, 1, 1 };
  outInfo->Set( vtkDataObject::SPACING(), spacing, 3 );
  double origin[3] = { 0, 0, 0 };
  outInfo->Set( vtkDataObject::ORIGIN(), origin, 3 );

  // set default data type - unsigned char and number of components 1
  vtkDataObject::SetPointDataActiveScalarInfo( outInfo, aSource->GetPixelType(), 1 );

  return 1;
}

//----------------------------------------------------------------------------
// The Execute method is fairly complex, so I would not recommend overriding
// it unless you have to.  Override the UnpackRasterLine() method instead.
int vtkPlusDevice::RequestData( vtkInformation* vtkNotUsed( request ),
                                vtkInformationVector** vtkNotUsed( inputVector ),
                                vtkInformationVector* vtkNotUsed( outputVector ) )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::RequestData" );

  // Find a video source and set extent
  vtkPlusDataSource* aSource( NULL );
  for ( ChannelContainerIterator it = this->OutputChannels.begin(); it != this->OutputChannels.end(); ++it )
  {
    if ( ( *it )->GetVideoSource( aSource ) == PLUS_SUCCESS )
    {
      break;
    }
  }
  if ( aSource == NULL )
  {
    return 1;
  }

  if ( aSource->GetNumberOfItems() < 1 )
  {
    LOCAL_LOG_DEBUG( "Cannot request data from video source, the video buffer is empty or does not exist!" );
    vtkImageData* data = vtkImageData::SafeDownCast( this->GetOutputDataObject( 0 ) );
    unsigned int frameSize[3] = { 0, 0, 0 };
    aSource->GetOutputFrameSize( frameSize );
    data->SetExtent( 0, frameSize[0] - 1, 0, frameSize[1] - 1, 0, frameSize[2] - 1 );
    data->AllocateScalars( aSource->GetPixelType(), aSource->GetNumberOfScalarComponents() );

    return 1;
  }

  if ( this->UpdateWithDesiredTimestamp && this->DesiredTimestamp != -1 )
  {
    ItemStatus itemStatus = aSource->GetStreamBufferItemFromTime( this->DesiredTimestamp, this->CurrentStreamBufferItem, vtkPlusBuffer::EXACT_TIME );
    if ( itemStatus != ITEM_OK )
    {
      LOCAL_LOG_ERROR( "Unable to copy video data to the requested output!" );
      return 1;
    }
  }
  else
  {
    // get the most recent frame if we are not updating with the desired timestamp
    ItemStatus itemStatus = aSource->GetLatestStreamBufferItem( this->CurrentStreamBufferItem );
    if ( itemStatus != ITEM_OK )
    {
      LOCAL_LOG_ERROR( "Unable to copy video data to the requested output!" );
      return 1;
    }
  }

  this->FrameTimeStamp = this->CurrentStreamBufferItem->GetTimestamp( aSource->GetLocalTimeOffsetSec() );
  this->TimestampClosestToDesired = this->CurrentStreamBufferItem->GetTimestamp( aSource->GetLocalTimeOffsetSec() );

  void* sourcePtr = this->CurrentStreamBufferItem->GetFrame().GetScalarPointer();
  int bytesToCopy = this->CurrentStreamBufferItem->GetFrame().GetFrameSizeInBytes();

  // The whole image buffer is copied, regardless of the UPDATE_EXTENT value to make the copying implementation simpler
  // For a more efficient implementation, we should only update the requested part of the image.
  vtkImageData* data = vtkImageData::SafeDownCast( this->GetOutputDataObject( 0 ) );
  unsigned int frameSize[3] = { 0, 0, 0 };
  this->CurrentStreamBufferItem->GetFrame().GetFrameSize( frameSize );
  data->SetExtent( 0, frameSize[0] - 1, 0, frameSize[1] - 1, 0, frameSize[2] - 1 );
  data->AllocateScalars( aSource->GetPixelType(), aSource->GetNumberOfScalarComponents() );

  unsigned char* outPtr = ( unsigned char* )data->GetScalarPointer();
  memcpy( outPtr, sourcePtr, bytesToCopy );

  return 1;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::SetInputFrameSize( vtkPlusDataSource& aSource, int x, int y, int z )
{
  if ( x < 0 || y < 0 || z < 0 )
  {
    LOCAL_LOG_ERROR( "Negative frame size sent to SetInputFrameSize. Aborting." );
    return PLUS_FAIL;
  }

  return this->SetInputFrameSize( aSource, static_cast<unsigned int>( x ), static_cast<unsigned int>( y ), static_cast<unsigned int>( z ) );
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::SetInputFrameSize( vtkPlusDataSource& aSource, unsigned int x, unsigned int y, unsigned int z )
{
  unsigned int* frameSize = aSource.GetInputFrameSize();
  if ( x == frameSize[0] &&
       y == frameSize[1] &&
       z == frameSize[2] )
  {
    return PLUS_SUCCESS;
  }

  if ( x != 0 && y != 0 && z == 0 )
  {
    LOCAL_LOG_WARNING( "Single slice images should have a dimension of z=1" );
    z = 1;
  }

  if ( x < 1 || y < 1 || z < 1 )
  {
    LOCAL_LOG_ERROR( "SetInputFrameSize: Illegal frame size " << x << "x" << y << "x" << z );
    return PLUS_FAIL;
  }

  aSource.SetInputFrameSize( x, y, z );
  aSource.Modified();

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetOutputFrameSize( vtkPlusChannel& aChannel, unsigned int& x, unsigned int& y, unsigned int& z )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::GetOutputFrameSize" );

  unsigned int dim[3] = {0, 0, 0};
  if( this->GetOutputFrameSize( aChannel, dim ) != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Unable to get frame size from the device." );
    return PLUS_FAIL;
  }
  x = dim[0];
  y = dim[1];
  z = dim[2];

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetOutputFrameSize( vtkPlusChannel& aChannel, unsigned int dim[3] )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::GetOutputFrameSize" );

  vtkPlusDataSource* aSource( NULL );
  if( aChannel.GetVideoSource( aSource ) != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Unable to retrieve the video source." );
    return PLUS_FAIL;
  }

  return aSource->GetOutputFrameSize( dim );
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetInputFrameSize( vtkPlusChannel& aChannel, unsigned int& x, unsigned int& y, unsigned int& z )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::GetInputFrameSize" );

  unsigned int dim[3] = {0, 0, 0};
  if( this->GetInputFrameSize( aChannel, dim ) != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Unable to get frame size from the device." );
    return PLUS_FAIL;
  }
  x = dim[0];
  y = dim[1];
  z = dim[2];

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetInputFrameSize( vtkPlusChannel& aChannel, unsigned int dim[3] )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::GetInputFrameSize" );

  vtkPlusDataSource* aSource( NULL );
  if( aChannel.GetVideoSource( aSource ) != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Unable to retrieve the video source." );
    return PLUS_FAIL;
  }

  aSource->GetInputFrameSize( dim );
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::SetPixelType( vtkPlusChannel& aChannel, PlusCommon::VTKScalarPixelType pixelType )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::SetPixelType" );

  vtkPlusDataSource* aSource( NULL );
  if( aChannel.GetVideoSource( aSource ) != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Unable to retrieve the video source." );
    return PLUS_FAIL;
  }

  return aSource->SetPixelType( pixelType );
}

//----------------------------------------------------------------------------
PlusCommon::VTKScalarPixelType vtkPlusDevice::GetPixelType( vtkPlusChannel& aChannel )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::GetPixelType" );

  vtkPlusDataSource* aSource( NULL );
  if( aChannel.GetVideoSource( aSource ) != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Unable to retrieve the video source." );
    return VTK_VOID;
  }

  return aSource->GetPixelType();
}

//----------------------------------------------------------------------------
US_IMAGE_TYPE vtkPlusDevice::GetImageType( vtkPlusChannel& aChannel )
{
  vtkPlusDataSource* aSource( NULL );
  if( aChannel.GetVideoSource( aSource ) != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Unable to retrieve the video source." );
    return US_IMG_TYPE_XX;
  }

  return aSource->GetImageType();
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::SetImageType( vtkPlusChannel& aChannel, US_IMAGE_TYPE imageType )
{
  vtkPlusDataSource* aSource( NULL );
  if( aChannel.GetVideoSource( aSource ) != PLUS_SUCCESS )
  {
    LOCAL_LOG_ERROR( "Unable to retrieve the video source." );
    return PLUS_FAIL;
  }
  return aSource->SetImageType( imageType );
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetOutputChannelByName( vtkPlusChannel*& aChannel, const char* aChannelId )
{
  if( aChannelId == NULL )
  {
    LOCAL_LOG_ERROR( "Null channel name sent to GetOutputChannelByName." );
    return PLUS_FAIL;
  }

  for( ChannelContainerIterator it = this->OutputChannels.begin(); it != this->OutputChannels.end(); ++it )
  {
    vtkPlusChannel* channel = ( *it );
    if( STRCASECMP( channel->GetChannelId(), aChannelId ) == 0 )
    {
      aChannel = channel;
      return PLUS_SUCCESS;
    }
  }

  return PLUS_FAIL;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::AddInputChannel( vtkPlusChannel* aChannel )
{
  if (aChannel == NULL)
  {
    LOCAL_LOG_ERROR("vtkPlusDevice::AddInputChannel failed: input channel is invalid");
    return PLUS_FAIL;
  }
  for( ChannelContainerIterator it = InputChannels.begin(); it != InputChannels.end(); ++it )
  {
    if( STRCASECMP( ( *it )->GetChannelId(), aChannel->GetChannelId() ) == 0 )
    {
      LOCAL_LOG_WARNING( "Duplicate addition of an input aChannel." );
      return PLUS_SUCCESS;
    }
  }

  // aChannel remains valid, as it is owned by another device (which provides it as an output)
  // TODO: it could be safer to increase the reference counter of aChannel here (and decrease the ref counter when aChannel is removed from InputChannels)
  this->InputChannels.push_back( aChannel );
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
double vtkPlusDevice::GetAcquisitionRate() const
{
  return this->AcquisitionRate;
}

//----------------------------------------------------------------------------
void vtkPlusDevice::InternalWriteOutputChannels( vtkXMLDataElement* rootXMLElement )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::InternalWriteOutputChannels( " << rootXMLElement->GetName() << ")" );

  for( ChannelContainerConstIterator it = this->OutputChannels.begin(); it != this->OutputChannels.end(); ++it )
  {
    vtkPlusChannel* aStream = *it;
    vtkXMLDataElement* streamElement = this->FindOutputChannelElement( rootXMLElement, aStream->GetChannelId() );
    aStream->WriteConfiguration( streamElement );
  }
}

//----------------------------------------------------------------------------
void vtkPlusDevice::InternalWriteInputChannels( vtkXMLDataElement* rootXMLElement )
{
  LOCAL_LOG_TRACE( "vtkPlusDevice::InternalWriteInputChannels( " << rootXMLElement->GetName() << ")" );

  for( ChannelContainerConstIterator it = this->InputChannels.begin(); it != this->InputChannels.end(); ++it )
  {
    vtkPlusChannel* aStream = *it;
    vtkXMLDataElement* streamElement = this->FindInputChannelElement( rootXMLElement, aStream->GetChannelId() );
    aStream->WriteConfiguration( streamElement );
  }
}

//----------------------------------------------------------------------------
DataSourceContainerConstIterator vtkPlusDevice::GetVideoIteratorBegin() const
{
  return this->VideoSources.begin();
}

//----------------------------------------------------------------------------
DataSourceContainerConstIterator vtkPlusDevice::GetVideoIteratorEnd() const
{
  return this->VideoSources.end();
}

//----------------------------------------------------------------------------
int vtkPlusDevice::GetNumberOfVideoSources() const
{
  return this->VideoSources.size();
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::AddVideo( vtkPlusDataSource* aVideo )
{
  if ( aVideo == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to add video to device, video is NULL!" );
    return PLUS_FAIL;
  }

  if ( aVideo->GetSourceId() == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to add video to device, image Id must be defined!" );
    return PLUS_FAIL;
  }

  if ( this->VideoSources.find( aVideo->GetSourceId() ) == this->GetVideoIteratorEnd() )
  {
    // Check image port names, it should be unique too
    for ( DataSourceContainerConstIterator it = this->GetVideoIteratorBegin(); it != this->GetVideoIteratorEnd(); ++it )
    {
      if ( STRCASECMP( aVideo->GetSourceId(), it->second->GetSourceId() ) == 0 )
      {
        LOCAL_LOG_ERROR( "Failed to add '" << aVideo->GetSourceId() << "' video to container: video with Id '" << it->second->GetSourceId()
                         << "' is already defined'!" );
        return PLUS_FAIL;
      }
    }

    aVideo->Register( this );
    aVideo->SetDevice( this );
    this->VideoSources[aVideo->GetSourceId()] = aVideo;
  }
  else
  {
    LOCAL_LOG_ERROR( "Image with Id '" << aVideo->GetSourceId() << "' is already in the image container!" );
    return PLUS_FAIL;
  }

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetFirstVideoSource( vtkPlusDataSource*& aVideoSource )
{
  if ( this->VideoSources.size() == 0 )
  {
    LOCAL_LOG_ERROR( "Failed to get first active video source - there are no video sources!" );
    return PLUS_FAIL;
  }

  aVideoSource = this->VideoSources.begin()->second;

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetVideoSource( const char* aSourceId, vtkPlusDataSource*& aVideoSource )
{
  if ( aSourceId == NULL )
  {
    LOCAL_LOG_ERROR( "Failed to get image, image name is NULL" );
    return PLUS_FAIL;
  }

  if( this->VideoSources.find( aSourceId ) != this->VideoSources.end() )
  {
    aVideoSource = this->VideoSources.find( aSourceId )->second;
    return PLUS_SUCCESS;
  }

  return PLUS_FAIL;
}

//----------------------------------------------------------------------------
std::vector<vtkPlusDataSource*> vtkPlusDevice::GetVideoSources() const
{
  std::vector<vtkPlusDataSource*> results;
  for( DataSourceContainerConstIterator it = this->VideoSources.begin(); it != this->VideoSources.end(); ++it )
  {
    results.push_back( it->second );
  }
  return results;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetVideoSourceByIndex( const unsigned int index, vtkPlusDataSource*& aVideoSource )
{
  if ( index > this->VideoSources.size() )
  {
    LOCAL_LOG_ERROR( "Failed to get video source, index is outside of range." );
    aVideoSource = NULL;
    return PLUS_FAIL;
  }

  DataSourceContainerIterator it = this->VideoSources.begin();
  for( unsigned int i = 0 ; i < index; ++i )
  {
    ++it;
  }
  aVideoSource = it->second;
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetDataSource( const char* aSourceId, vtkPlusDataSource*& aSource )
{
  if( this->GetVideoSource( aSourceId, aSource ) == PLUS_SUCCESS )
  {
    return PLUS_SUCCESS;
  }
  if( this->GetTool( aSourceId, aSource ) == PLUS_SUCCESS )
  {
    return PLUS_SUCCESS;
  }
  if( this->GetFieldDataSource( aSourceId, aSource ) == PLUS_SUCCESS )
  {
    return PLUS_SUCCESS;
  }
  else
  {
    return PLUS_FAIL;
  }
}

//----------------------------------------------------------------------------
ChannelContainerConstIterator vtkPlusDevice::GetOutputChannelsStart() const
{
  return this->OutputChannels.begin();
}

//----------------------------------------------------------------------------
ChannelContainerIterator vtkPlusDevice::GetOutputChannelsStart()
{
  return this->OutputChannels.begin();
}

//----------------------------------------------------------------------------
ChannelContainerConstIterator vtkPlusDevice::GetOutputChannelsEnd() const
{
  return this->OutputChannels.end();
}

//----------------------------------------------------------------------------
ChannelContainerIterator vtkPlusDevice::GetOutputChannelsEnd()
{
  return this->OutputChannels.end();
}

//------------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetToolReferenceFrameFromTrackedFrame( PlusTrackedFrame& aFrame, std::string& aToolReferenceFrameName )
{
  LOG_TRACE( "vtkPlusDataCollectorFile::GetTrackerToolReferenceFrame" );

  // Try to find it out from the custom transforms that are stored in the tracked frame
  std::vector<PlusTransformName> transformNames;
  aFrame.GetCustomFrameTransformNameList( transformNames );

  if ( transformNames.size() == 0 )
  {
    LOG_ERROR( "No transforms found in tracked frame!" );
    return PLUS_FAIL;
  }

  std::string frameName = "";
  for ( std::vector<PlusTransformName>::iterator it = transformNames.begin(); it != transformNames.end(); ++it )
  {
    if ( frameName == "" )
    {
      frameName = it->To();
    }
    else if ( frameName != it->To() )
    {
      LOG_ERROR( "Destination coordinate frame names are not the same!" );
      return PLUS_FAIL;
    }
  }

  aToolReferenceFrameName = frameName;

  return PLUS_SUCCESS;
}

//------------------------------------------------------------------------------
bool vtkPlusDevice::IsTracker() const
{
  LOCAL_LOG_ERROR( "Calling base IsTracker. Override in the derived classes." );
  return false;
}

//------------------------------------------------------------------------------
PlusStatus vtkPlusDevice::AddOutputChannel( vtkPlusChannel* aChannel )
{
  if ( aChannel == NULL )
  {
    LOCAL_LOG_ERROR( "Cannot add device, aChannel is invalid" );
    return PLUS_FAIL;
  }
  if ( aChannel->GetOwnerDevice() == NULL )
  {
    aChannel->SetOwnerDevice( this );
  }
  this->OutputChannels.push_back( aChannel );
  aChannel->Register( this );
  return PLUS_SUCCESS;
}

//------------------------------------------------------------------------------
void vtkPlusDevice::SetDataCollector( vtkPlusDataCollector* _arg )
{
  this->DataCollector = _arg;
}

//------------------------------------------------------------------------------
PlusStatus vtkPlusDevice::BuildParameterIndexList( const ChannelContainer& channels, bool& depthSwitchingEnabled, bool& modeSwitchingEnabled, bool& probeSwitchingEnabled, std::vector<ParamIndexKey*>& output )
{
  std::vector<double> depthList;
  std::vector<std::string> modeList;
  std::vector<std::string> probeList;

  for ( ChannelContainerConstIterator it = channels.begin(); it != channels.end(); ++it )
  {
    std::string value;
    if( ( *it )->GetCustomAttribute( std::string( vtkPlusDevice::DEPTH_SWITCH_ATTRIBUTE_NAME ), value ) == PLUS_SUCCESS )
    {
      double depth = 0.0;
      PlusCommon::StringToDouble( value.c_str(), depth );
      depthList.push_back( depth );
    }

    if( ( *it )->GetCustomAttribute( std::string( vtkPlusDevice::PROBE_SWITCH_ATTRIBUTE_NAME ), value ) == PLUS_SUCCESS )
    {
      probeList.push_back( value );
    }

    if( ( *it )->GetCustomAttribute( std::string( vtkPlusDevice::MODE_SWITCH_ATTRIBUTE_NAME ), value ) == PLUS_SUCCESS )
    {
      modeList.push_back( value );
    }
  }

  if( depthList.size() == 0 &&
      modeList.size() == 0 &&
      probeList.size() == 0 )
  {
    // We have no switching going on, we're done here
    return PLUS_SUCCESS;
  }

  depthSwitchingEnabled = depthList.size() > 1;
  modeSwitchingEnabled = modeList.size() > 1;
  probeSwitchingEnabled = probeList.size() > 1;

  int numErrors( 0 );
  for ( ChannelContainerConstIterator it = channels.begin(); it != channels.end(); ++it )
  {
    // Create a key to index this channel element
    ParamIndexKey* key = new ParamIndexKey();
    // set the depth
    std::string value;
    if( ( *it )->GetCustomAttribute( std::string( vtkPlusDevice::DEPTH_SWITCH_ATTRIBUTE_NAME ), value ) == PLUS_SUCCESS )
    {
      PlusCommon::StringToDouble( value.c_str(), key->Depth );
    }
    else if( depthList.size() > 0 )
    {
      LOCAL_LOG_ERROR( "Channel " << ( *it )->GetChannelId() << " does not have a \'" << vtkPlusDevice::DEPTH_SWITCH_ATTRIBUTE_NAME << "\' sub-attribute specified. Please specify the depth on all channels." );
      numErrors++;
      continue;
    }

    // set the probe id
    if( ( *it )->GetCustomAttribute( std::string( vtkPlusDevice::PROBE_SWITCH_ATTRIBUTE_NAME ), key->ProbeId ) != PLUS_SUCCESS && probeList.size() > 0 )
    {
      LOCAL_LOG_ERROR( "Channel " << ( *it )->GetChannelId() << " does not have a \'" << vtkPlusDevice::PROBE_SWITCH_ATTRIBUTE_NAME << "\' sub-attribute specified. Please specify the probe details for all channels." );
      numErrors++;
      continue;
    }

    // determine and set the mode
    if( ( *it )->GetCustomAttribute( std::string( vtkPlusDevice::MODE_SWITCH_ATTRIBUTE_NAME ), value ) == PLUS_SUCCESS )
    {
      if( value.compare( std::string( vtkPlusDevice::RFMODE_PORT_NAME ) ) == 0 )
      {
        key->Mode = Plus_RfMode;
      }
      else if( value.compare( std::string( vtkPlusDevice::BMODE_PORT_NAME ) ) == 0 )
      {
        key->Mode = Plus_BMode;
      }
      else
      {
        LOCAL_LOG_WARNING( "Unable to determine Mode. Please specify a sub-attribute with Id \'" << vtkPlusDevice::MODE_SWITCH_ATTRIBUTE_NAME << "\'=" << vtkPlusDevice::BMODE_PORT_NAME << " or " << vtkPlusDevice::RFMODE_PORT_NAME << ". Defaulting to " << vtkPlusDevice::BMODE_PORT_NAME );
        key->Mode = Plus_BMode;
      }
    }
    else if( ( *it )->GetRfProcessor() == NULL )
    {
      key->Mode = Plus_BMode;
      LOCAL_LOG_WARNING( "No \'" << vtkPlusDevice::MODE_SWITCH_ATTRIBUTE_NAME << "\' tag found on channel " << ( *it )->GetChannelId() << ". Defaulting to " << vtkPlusDevice::BMODE_PORT_NAME << "." );
    }
    else
    {
      key->Mode = Plus_RfMode;
      LOCAL_LOG_INFO( "No \'" << vtkPlusDevice::MODE_SWITCH_ATTRIBUTE_NAME << "\' tag found on channel " << ( *it )->GetChannelId() << ". Found tag " << vtkPlusRfProcessor::GetRfProcessorTagName() << " so setting channel Mode to " << vtkPlusDevice::RFMODE_PORT_NAME << "." );
    }
    output.push_back( key );
  }

  return PLUS_SUCCESS;
}

//------------------------------------------------------------------------------
bool vtkPlusDevice::HasGracePeriodExpired()
{
  return ( vtkPlusAccurateTimer::GetSystemTime() - this->RecordingStartTime ) > this->MissingInputGracePeriodSec;
}

//------------------------------------------------------------------------------
PlusStatus vtkPlusDevice::CreateDefaultOutputChannel( bool addSource/*=true*/ )
{
  // Create output channel
  vtkSmartPointer<vtkPlusChannel> aChannel = vtkSmartPointer<vtkPlusChannel>::New();
  aChannel->SetOwnerDevice( this );
  aChannel->SetChannelId( "VideoStream" );
  if ( this->AddOutputChannel( aChannel ) != PLUS_SUCCESS )
  {
    return PLUS_FAIL;
  }
  if ( !addSource )
  {
    // no need to add sources, so we are done
    return PLUS_SUCCESS;
  }

  if ( this->IsTracker() )
  {
    // TODO: add default tools
  }
  else
  {
    // Create an output video stream for this channel
    vtkPlusDataSource* aDataSource = vtkPlusDataSource::New();
    if ( aDataSource->SetBufferSize( 30 ) != PLUS_SUCCESS )
    {
      LOG_ERROR( "Failed to set video buffer size!" );
      return PLUS_FAIL;
    }
    aDataSource->SetInputImageOrientation( US_IMG_ORIENT_MF );
    if( aDataSource->SetSourceId( "Video" ) != PLUS_SUCCESS )
    {
      return PLUS_FAIL;
    }
    if( this->AddVideo( aDataSource ) != PLUS_SUCCESS )
    {
      return PLUS_FAIL;
    }
    aChannel->SetVideoSource( aDataSource );
  }
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetFirstActiveOutputVideoSource( vtkPlusDataSource*& aVideoSource )
{
  aVideoSource = NULL;
  if ( this->OutputChannels.empty() )
  {
    LOCAL_LOG_ERROR( "Failed to get first active output video source - there are no output channels" );
    return PLUS_FAIL;
  }
  for( ChannelContainerIterator it = this->OutputChannels.begin(); it != this->OutputChannels.end(); ++it )
  {
    if ( ( *it )->GetVideoSource( aVideoSource ) == PLUS_SUCCESS && aVideoSource != NULL )
    {
      // found a video source in the output channel
      return PLUS_SUCCESS;
    }
  }

  LOG_ERROR( "There is no active video sources in the output channel(s)" );
  return PLUS_FAIL;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetImageMetaData( PlusCommon::ImageMetaDataList& imageMetaDataItems )
{
  LOCAL_LOG_DEBUG( "No ImageMetaData is available" );
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::GetImage( const std::string& requestedImageId, std::string& assignedImageId, const std::string& imageReferencFrameName, vtkImageData* imageData, vtkMatrix4x4* ijkToReferenceTransform )
{
  LOCAL_LOG_ERROR( "vtkPlusDevice::GetImage is not implemented" );
  return PLUS_FAIL;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusDevice::SendText( const std::string& textToSend, std::string* textReceived/*=NULL*/ )
{
  LOCAL_LOG_ERROR( "vtkPlusDevice::SendText is not implemented" );
  return PLUS_FAIL;
}
