/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** ServerMediaSubsession.cpp
** 
** -------------------------------------------------------------------------*/

// live555
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <Base64.hh>

// project
#include "ServerMediaSubsession.h"

// -----------------------------------------
//    ServerMediaSubsession for Multicast
// -----------------------------------------
MulticastServerMediaSubsession* MulticastServerMediaSubsession::createNew(UsageEnvironment& env
									, struct in_addr destinationAddress
									, Port rtpPortNum, Port rtcpPortNum
									, int ttl
									, unsigned char rtpPayloadType
									, StreamReplicator* replicator
									, int format) 
{ 
	// Create a source
	FramedSource* source = replicator->createStreamReplica();			
	FramedSource* videoSource = createSource(env, source, format);

	// Create RTP/RTCP groupsock
	Groupsock* rtpGroupsock = new Groupsock(env, destinationAddress, rtpPortNum, ttl);
	Groupsock* rtcpGroupsock = new Groupsock(env, destinationAddress, rtcpPortNum, ttl);

	// Create a RTP sink
	RTPSink* videoSink = createSink(env, rtpGroupsock, rtpPayloadType, format);

	// Create 'RTCP instance'
	const unsigned maxCNAMElen = 100;
	unsigned char CNAME[maxCNAMElen+1];
	gethostname((char*)CNAME, maxCNAMElen);
	CNAME[maxCNAMElen] = '\0'; 
	RTCPInstance* rtcpInstance = RTCPInstance::createNew(env, rtcpGroupsock,  500, CNAME, videoSink, NULL);

	// Start Playing the Sink
	videoSink->startPlaying(*videoSource, NULL, NULL);
	
	return new MulticastServerMediaSubsession(replicator, videoSink, rtcpInstance);
}
		
char const* MulticastServerMediaSubsession::sdpLines() 
{
	if (m_SDPLines.empty())
	{
		// Ugly workaround to give SPS/PPS that are get from the RTPSink 
		m_SDPLines.assign(PassiveServerMediaSubsession::sdpLines());
		m_SDPLines.append(getAuxSDPLine(m_rtpSink,NULL));
	}
	return m_SDPLines.c_str();
}

char const* MulticastServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink,FramedSource* inputSource)
{
	return this->getAuxLine(dynamic_cast<V4L2DeviceSource*>(m_replicator->inputSource()), rtpSink->rtpPayloadType());
}
		
// -----------------------------------------
//    ServerMediaSubsession for Unicast
// -----------------------------------------
UnicastServerMediaSubsession* UnicastServerMediaSubsession::createNew(UsageEnvironment& env, StreamReplicator* replicator, int format) 
{ 
	return new UnicastServerMediaSubsession(env,replicator,format);
}
					
FramedSource* UnicastServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
	FramedSource* source = m_replicator->createStreamReplica();
	return createSource(envir(), source, m_format);
}
		
RTPSink* UnicastServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	return createSink(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, m_format);
}
		
char const* UnicastServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink,FramedSource* inputSource)
{
	return this->getAuxLine(dynamic_cast<V4L2DeviceSource*>(m_replicator->inputSource()), rtpSink->rtpPayloadType());
}
