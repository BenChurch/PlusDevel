/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/ 

#ifndef __igtlPlusClientInfoMessage_h
#define __igtlPlusClientInfoMessage_h

#include "vtkPlusOpenIGTLinkExport.h"

#include "igtlObject.h"
#include "igtlStringMessage.h"
#include "PlusIgtlClientInfo.h" 

namespace igtl
{

/*! 
  \class PlusClientInfoMessage 
  \brief IGTL message helper class for PlusServer ClientInfo class
  \ingroup PlusLibOpenIGTLink
*/
class vtkPlusOpenIGTLinkExport PlusClientInfoMessage: public StringMessage
{
public:
  typedef PlusClientInfoMessage                 Self;
  typedef StringMessage                    Superclass;
  typedef SmartPointer<Self>             Pointer;
  typedef SmartPointer<const Self>       ConstPointer;

  igtlTypeMacro( igtl::PlusClientInfoMessage, igtl::StringMessage );
  igtlNewMacro( igtl::PlusClientInfoMessage );

public:
  /*! Override to use the plus igtl factory */
  virtual igtl::MessageBase::Pointer Clone();

  /*! Set Plus IGTL Client Info */ 
  void SetClientInfo( const PlusIgtlClientInfo& clientInfo ); 

  /*! Get Plus IGTL Client Info */ 
  PlusIgtlClientInfo GetClientInfo(); 
  
protected:
  PlusClientInfoMessage();
  ~PlusClientInfoMessage();

  PlusIgtlClientInfo m_ClientInfo; 
};


} // namespace igtl

#endif 
