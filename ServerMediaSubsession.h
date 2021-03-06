/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** ServerMediaSubsession.h
** 
** -------------------------------------------------------------------------*/

#ifndef SERVER_MEDIA_SUBSESSION
#define SERVER_MEDIA_SUBSESSION

#include <string>
#include <sstream>

// V4L2
#include <linux/videodev2.h>

// live555
#include <liveMedia.hh>

// project
#include "V4l2DeviceSource.h"

// ---------------------------------
//   BaseServerMediaSubsession
// ---------------------------------
class BaseServerMediaSubsession
{
	public:
		BaseServerMediaSubsession(StreamReplicator* replicator): m_replicator(replicator) {};
	
	public:
		static FramedSource* createSource(UsageEnvironment& env, FramedSource * videoES, int format)
		{
			FramedSource* source = NULL;
			switch (format)
			{
				case V4L2_PIX_FMT_H264 : source = H264VideoStreamDiscreteFramer::createNew(env, videoES); break;
			}
			return source;
		}

		static RTPSink* createSink(UsageEnvironment& env, Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, int format)
		{
			RTPSink* videoSink = NULL;
			switch (format)
			{
				case V4L2_PIX_FMT_H264 : videoSink = H264VideoRTPSink::createNew(env, rtpGroupsock,rtpPayloadTypeIfDynamic); break;
			}
			return videoSink;
		}

		char const* getAuxLine(V4L2DeviceSource* source,unsigned char rtpPayloadType)
		{
			const char* auxLine = NULL;
			if (source)
			{
				std::ostringstream os; 
				os << "a=fmtp:" << int(rtpPayloadType) << " ";				
				os << source->getAuxLine();				
				os << "\r\n";				
				auxLine = strdup(os.str().c_str());
			} 
			return auxLine;
		}
		
	protected:
		StreamReplicator* m_replicator;
};

// -----------------------------------------
//    ServerMediaSubsession for Multicast
// -----------------------------------------
class MulticastServerMediaSubsession : public PassiveServerMediaSubsession , public BaseServerMediaSubsession
{
	public:
		static MulticastServerMediaSubsession* createNew(UsageEnvironment& env
								, struct in_addr destinationAddress
								, Port rtpPortNum, Port rtcpPortNum
								, int ttl
								, unsigned char rtpPayloadType
								, StreamReplicator* replicator
								, int format);
		
	protected:
		MulticastServerMediaSubsession(StreamReplicator* replicator, RTPSink* rtpSink, RTCPInstance* rtcpInstance) 
				: PassiveServerMediaSubsession(*rtpSink, rtcpInstance), BaseServerMediaSubsession(replicator), m_rtpSink(rtpSink) {};			

		virtual char const* sdpLines() ;
		virtual char const* getAuxSDPLine(RTPSink* rtpSink,FramedSource* inputSource);
		
	protected:
		RTPSink* m_rtpSink;
		std::string m_SDPLines;
};

// -----------------------------------------
//    ServerMediaSubsession for Unicast
// -----------------------------------------
class UnicastServerMediaSubsession : public OnDemandServerMediaSubsession , public BaseServerMediaSubsession
{
	public:
		static UnicastServerMediaSubsession* createNew(UsageEnvironment& env, StreamReplicator* replicator, int format);
		
	protected:
		UnicastServerMediaSubsession(UsageEnvironment& env, StreamReplicator* replicator, int format) 
				: OnDemandServerMediaSubsession(env, False), BaseServerMediaSubsession(replicator), m_format(format) {};
			
		virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
		virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);		
		virtual char const* getAuxSDPLine(RTPSink* rtpSink,FramedSource* inputSource);
					
	protected:
		int m_format;
};

#endif