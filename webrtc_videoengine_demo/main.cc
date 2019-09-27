
#include <iostream>
#include <thread>
#include <windows.h>

#include "webrtc/video_engine/vie_base.h"
#include "webrtc/video_engine/vie_network.h"
#include "webrtc/video_engine/vie_codec.h"
#include "webrtc/video_engine/vie_network.h"
#include "webrtc/video_engine/vie_rtp_rtcp.h"
#include "webrtc/video_engine/vie_image_process.h"
#include "webrtc/video_engine/vie_capture.h"
#include "webrtc/modules/video_capture/include/video_capture_factory.h"
#include "webrtc/video_engine/vie_render.h"
#include "webrtc/video_engine/vie_external_codec.h"
#include "webrtc/test/channel_transport/udp_transport.h"
#include "webrtc/video_engine/vie_autotest_window_manager_interface.h"
#include "webrtc/video_engine/vie_window_creator.h"

#pragma comment(lib,"audio_coding_module.lib")
#pragma comment(lib,"audio_conference_mixer.lib")
#pragma comment(lib,"audio_device.lib")
#pragma comment(lib,"audio_processing.lib")
#pragma comment(lib,"audio_processing_sse2.lib")
#pragma comment(lib,"audioproc_debug_proto.lib")
//#pragma comment(lib,"bitrate_controller.lib")

#pragma comment(lib,"CNG.lib")
#pragma comment(lib,"common_audio.lib")
#pragma comment(lib,"common_audio_sse2.lib")
#pragma comment(lib,"common_video.lib")
#pragma comment(lib,"channel_transport.lib")
#pragma comment(lib,"directshow_baseclasses.lib")
#pragma comment(lib,"field_trial_default.lib")
#pragma comment(lib,"G711.lib")
#pragma comment(lib,"G722.lib")
#pragma comment(lib,"icui18n.lib")
#pragma comment(lib,"icuuc.lib")
#pragma comment(lib,"iLBC.lib")
#pragma comment(lib,"iSAC.lib")
#pragma comment(lib,"iSACFix.lib")
#pragma comment(lib,"jsoncpp.lib")
#pragma comment(lib,"libjpeg.lib")
#pragma comment(lib,"libvpx.lib")
#pragma comment(lib,"libvpx_asm_offsets_vp8.lib")
#pragma comment(lib,"libvpx_intrinsics_mmx.lib")
#pragma comment(lib,"libvpx_intrinsics_sse2.lib")
#pragma comment(lib,"libvpx_intrinsics_sse4_1.lib")
#pragma comment(lib,"libvpx_intrinsics_ssse3.lib")
#pragma comment(lib,"libyuv.lib")
#pragma comment(lib,"media_file.lib")
#pragma comment(lib,"neteq.lib")
#pragma comment(lib,"opus.lib")
#pragma comment(lib,"paced_sender.lib")
#pragma comment(lib,"PCM16B.lib")
#pragma comment(lib,"protobuf_lite.lib")
#pragma comment(lib,"rbe_components.lib")
#pragma comment(lib,"remote_bitrate_estimator.lib")
#pragma comment(lib,"rtp_rtcp.lib")
#pragma comment(lib,"sqlite3.lib")
#pragma comment(lib,"system_wrappers.lib")
#pragma comment(lib,"usrsctplib.lib")

#pragma comment(lib,"video_capture_module.lib")
#pragma comment(lib,"video_coding_utility.lib")
#pragma comment(lib,"video_engine_core.lib")
#pragma comment(lib,"video_processing.lib")
#pragma comment(lib,"video_processing_sse2.lib")
#pragma comment(lib,"video_render_module.lib")

#pragma comment(lib,"voice_engine.lib")

#pragma comment(lib,"webrtc_i420.lib")
#pragma comment(lib,"webrtc_opus.lib")
#pragma comment(lib,"webrtc_utility.lib")
#pragma comment(lib,"webrtc_video_coding.lib")
#pragma comment(lib,"webrtc_vp8.lib")
#pragma comment(lib,"webrtc_base.lib")

using namespace webrtc;

class VideoChannelTransport : public webrtc::test::UdpTransportData {
public:
	VideoChannelTransport(ViENetwork* vie_network, int channel);

	virtual  ~VideoChannelTransport();

	// Start implementation of UdpTransportData.
	virtual void IncomingRTPPacket(const int8_t* incoming_rtp_packet,
		const int32_t packet_length,
		const char* /*from_ip*/,
		const uint16_t /*from_port*/) OVERRIDE;

	virtual void IncomingRTCPPacket(const int8_t* incoming_rtcp_packet,
		const int32_t packet_length,
		const char* /*from_ip*/,
		const uint16_t /*from_port*/) OVERRIDE;
	// End implementation of UdpTransportData.

	// Specifies the ports to receive RTP packets on.
	int SetLocalReceiver(uint16_t rtp_port);
	//int SetLocalReceiver(const char* ipAddress,uint16_t rtp_port);

	// Specifies the destination port and IP address for a specified channel.
	int SetSendDestination(const char* ip_address, uint16_t rtp_port);

private:
	int channel_;
	ViENetwork* vie_network_;
	webrtc::test::UdpTransport* socket_transport_;
};

VideoChannelTransport::VideoChannelTransport(ViENetwork* vie_network,
	int channel)
	: channel_(channel),
	vie_network_(vie_network) {
	uint8_t socket_threads = 1;
	socket_transport_ = webrtc::test::UdpTransport::Create(channel, socket_threads);
	int registered = vie_network_->RegisterSendTransport(channel,
		*socket_transport_);
}

VideoChannelTransport::~VideoChannelTransport() {
	vie_network_->DeregisterSendTransport(channel_);
	webrtc::test::UdpTransport::Destroy(socket_transport_);
}

void VideoChannelTransport::IncomingRTPPacket(
	const int8_t* incoming_rtp_packet,
	const int32_t packet_length,
	const char* /*from_ip*/,
	const uint16_t /*from_port*/) {
	vie_network_->ReceivedRTPPacket(
		channel_, incoming_rtp_packet, packet_length, PacketTime());
}

void VideoChannelTransport::IncomingRTCPPacket(
	const int8_t* incoming_rtcp_packet,
	const int32_t packet_length,
	const char* /*from_ip*/,
	const uint16_t /*from_port*/) {
	vie_network_->ReceivedRTCPPacket(channel_, incoming_rtcp_packet,
		packet_length);
}

int VideoChannelTransport::SetLocalReceiver(uint16_t rtp_port) {
//int VideoChannelTransport::SetLocalReceiver(const char* ipAddress, uint16_t rtp_port) {
	int return_value = socket_transport_->InitializeReceiveSockets(this,
		rtp_port);
	if (return_value == 0) {//return socket_transport_->StartReceiving(500);
		return socket_transport_->StartReceiving(500);
	}
	return return_value;
}

int VideoChannelTransport::SetSendDestination(const char* ip_address,
	uint16_t rtp_port) {
	return socket_transport_->InitializeSendSockets(ip_address, rtp_port);
}

int VideoEngineSample(void* window1, void* window2)
{

	int error = 0;

	//
	// Create a VideoEngine instance
	//
	webrtc::VideoEngine* ptrViE = NULL;
	ptrViE = webrtc::VideoEngine::Create();
	if (ptrViE == NULL)
	{
		printf("ERROR in VideoEngine::Create\n");
		return -1;
	}

	//
	// Init VideoEngine and create a channel
	//
	webrtc::ViEBase* ptrViEBase = webrtc::ViEBase::GetInterface(ptrViE);
	if (ptrViEBase == NULL)
	{
		printf("ERROR in ViEBase::GetInterface\n");
		return -1;
	}

	error = ptrViEBase->Init();
	if (error == -1)
	{
		printf("ERROR in ViEBase::Init\n");
		return -1;
	}

	webrtc::ViERTP_RTCP* ptrViERtpRtcp =
		webrtc::ViERTP_RTCP::GetInterface(ptrViE);
	if (ptrViERtpRtcp == NULL)
	{
		printf("ERROR in ViERTP_RTCP::GetInterface\n");
		return -1;
	}

	int videoChannel = -1;
	error = ptrViEBase->CreateChannel(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViEBase::CreateChannel\n");
		return -1;
	}

	//
	// List available capture devices, allocate and connect.
	//
	webrtc::ViECapture* ptrViECapture =
		webrtc::ViECapture::GetInterface(ptrViE);
	if (ptrViEBase == NULL)
	{
		printf("ERROR in ViECapture::GetInterface\n");
		return -1;
	}

	const unsigned int KMaxDeviceNameLength = 128;
	const unsigned int KMaxUniqueIdLength = 256;
	char deviceName[KMaxDeviceNameLength];
	memset(deviceName, 0, KMaxDeviceNameLength);
	char uniqueId[KMaxUniqueIdLength];
	memset(uniqueId, 0, KMaxUniqueIdLength);

	printf("Available capture devices:\n");
	int captureIdx = 0;
	for (captureIdx = 0;
		captureIdx < ptrViECapture->NumberOfCaptureDevices();
		captureIdx++)
	{
		memset(deviceName, 0, KMaxDeviceNameLength);
		memset(uniqueId, 0, KMaxUniqueIdLength);

		error = ptrViECapture->GetCaptureDevice(captureIdx, deviceName,
			KMaxDeviceNameLength, uniqueId,
			KMaxUniqueIdLength);
		if (error == -1)
		{
			printf("ERROR in ViECapture::GetCaptureDevice\n");
			return -1;
		}
		printf("\t %d. %s\n", captureIdx + 1, deviceName);
	}
	printf("\nChoose capture device: ");

	if (scanf("%d", &captureIdx) != 1)
	{
		printf("Error in scanf()\n");
		return -1;
	}
	getchar();
	captureIdx = captureIdx - 1; // Compensate for idx start at 1.

	error = ptrViECapture->GetCaptureDevice(captureIdx, deviceName,
		KMaxDeviceNameLength, uniqueId,
		KMaxUniqueIdLength);
	if (error == -1)
	{
		printf("ERROR in ViECapture::GetCaptureDevice\n");
		return -1;
	}

	int captureId = 0;
	error = ptrViECapture->AllocateCaptureDevice(uniqueId, KMaxUniqueIdLength,
		captureId);
	if (error == -1)
	{
		printf("ERROR in ViECapture::AllocateCaptureDevice\n");
		return -1;
	}

	error = ptrViECapture->ConnectCaptureDevice(captureId, videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViECapture::ConnectCaptureDevice\n");
		return -1;
	}

	error = ptrViECapture->StartCapture(captureId);
	if (error == -1)
	{
		printf("ERROR in ViECapture::StartCapture\n");
		return -1;
	}

	//
	// RTP/RTCP settings
	//

	error = ptrViERtpRtcp->SetRTCPStatus(videoChannel,
		webrtc::kRtcpCompound_RFC4585);
	if (error == -1)
	{
		printf("ERROR in ViERTP_RTCP::SetRTCPStatus\n");
		return -1;
	}

	error = ptrViERtpRtcp->SetKeyFrameRequestMethod(
		videoChannel, webrtc::kViEKeyFrameRequestPliRtcp);
	if (error == -1)
	{
		printf("ERROR in ViERTP_RTCP::SetKeyFrameRequestMethod\n");
		return -1;
	}

	error = ptrViERtpRtcp->SetRembStatus(videoChannel, true, true);
	if (error == -1)
	{
		printf("ERROR in ViERTP_RTCP::SetTMMBRStatus\n");
		return -1;
	}


	//
	// Set up rendering
	//
	webrtc::ViERender* ptrViERender = webrtc::ViERender::GetInterface(ptrViE);
	if (ptrViERender == NULL) {
		printf("ERROR in ViERender::GetInterface\n");
		return -1;
	}


	error
		= ptrViERender->AddRenderer(captureId, window1, 0, 0.0, 0.0, 1.0, 1.0);
	if (error == -1)
	{
		printf("ERROR in ViERender::AddRenderer\n");
		return -1;
	}

	error = ptrViERender->StartRender(captureId);
	if (error == -1)
	{
		printf("ERROR in ViERender::StartRender\n");
		return -1;
	}

	error = ptrViERender->AddRenderer(videoChannel, window2, 1, 0.0, 0.0, 1.0,1.0);
	if (error == -1){printf("ERROR in ViERender::AddRenderer\n");return -1;}

	error = ptrViERender->StartRender(videoChannel);
	if (error == -1){printf("ERROR in ViERender::StartRender\n");return -1;
	}

	//
	// Setup codecs
	//
	webrtc::ViECodec* ptrViECodec = webrtc::ViECodec::GetInterface(ptrViE);
	if (ptrViECodec == NULL)
	{
		printf("ERROR in ViECodec::GetInterface\n");
		return -1;
	}

	VideoCodec videoCodec;

	int numOfVeCodecs = ptrViECodec->NumberOfCodecs();
	for (int i = 0; i<numOfVeCodecs; ++i)
	{
		if (ptrViECodec->GetCodec(i, videoCodec) != -1)
		{
			if (videoCodec.codecType == kVideoCodecVP8)
				break;
		}
	}

	videoCodec.targetBitrate = 64;
	//videoCodec.minBitrate = 200;
	videoCodec.minBitrate = 32;
	//videoCodec.maxBitrate = 300;
	videoCodec.maxBitrate = 128;
	//videoCodec.maxFramerate = 25;
	videoCodec.maxFramerate = 60;

	error = ptrViECodec->SetSendCodec(videoChannel, videoCodec);
	assert(error != -1);

	error = ptrViECodec->SetReceiveCodec(videoChannel, videoCodec);
	assert(error != -1);
	//
	// Address settings
	//
	webrtc::ViENetwork* ptrViENetwork =
		webrtc::ViENetwork::GetInterface(ptrViE);
	if (ptrViENetwork == NULL)
	{
		printf("ERROR in ViENetwork::GetInterface\n");
		return -1;
	}

	VideoChannelTransport* video_channel_transport = NULL;
	video_channel_transport = new VideoChannelTransport(
		ptrViENetwork, videoChannel);


	const char* ipAddress = "127.0.0.1";
	uint16_t rtpPort = 6000;
	std::cout << std::endl;
	std::cout << "IP: " << ipAddress << std::endl;
	std::cout << "Using rtp port: " << rtpPort << std::endl;
	std::cout << std::endl;


	error = video_channel_transport->SetLocalReceiver(rtpPort);
	if (error == -1)
	{
		printf("ERROR in SetLocalReceiver\n");
		return -1;
	}

	error = video_channel_transport->SetSendDestination(ipAddress, rtpPort);
	if (error == -1)
	{
		printf("ERROR in SetSendDestination\n");
		return -1;
	}

	error = ptrViEBase->StartReceive(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViENetwork::StartReceive\n");
		return -1;
	}

	error = ptrViEBase->StartSend(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViENetwork::StartSend\n");
		return -1;
	}

	printf("\n call started\n\n");
	unsigned int total_bitrate_sent = 0;
	unsigned int video_bitrate_sent = 0;
	unsigned int fec_bitrate_sent = 0;
	unsigned int nackBitrateSent = 0;

	while (true){
		int bandWidth = ptrViERtpRtcp->GetBandwidthUsage(videoChannel,
			total_bitrate_sent, video_bitrate_sent, fec_bitrate_sent, nackBitrateSent);

		printf("total_bitrate_sent: %4d kbits/s, video_bitrate_sent: %4d kbits/s\r",
			total_bitrate_sent / 1024, video_bitrate_sent / 1024);

	}

	error = ptrViEBase->StopReceive(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViEBase::StopReceive\n");
		return -1;
	}

	error = ptrViEBase->StopSend(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViEBase::StopSend\n");
		return -1;
	}

	error = ptrViERender->StopRender(captureId);
	if (error == -1)
	{
		printf("ERROR in ViERender::StopRender\n");
		return -1;
	}

	error = ptrViERender->RemoveRenderer(captureId);
	if (error == -1)
	{
		printf("ERROR in ViERender::RemoveRenderer\n");
		return -1;
	}

	error = ptrViERender->StopRender(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViERender::StopRender\n");
		return -1;
	}

	error = ptrViERender->RemoveRenderer(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViERender::RemoveRenderer\n");
		return -1;
	}

	error = ptrViECapture->StopCapture(captureId);
	if (error == -1)
	{
		printf("ERROR in ViECapture::StopCapture\n");
		return -1;
	}

	error = ptrViECapture->DisconnectCaptureDevice(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViECapture::DisconnectCaptureDevice\n");
		return -1;
	}

	error = ptrViECapture->ReleaseCaptureDevice(captureId);
	if (error == -1)
	{
		printf("ERROR in ViECapture::ReleaseCaptureDevice\n");
		return -1;
	}

	error = ptrViEBase->DeleteChannel(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViEBase::DeleteChannel\n");
		return -1;
	}

	delete video_channel_transport;
	int remainingInterfaces = 0;
	remainingInterfaces = ptrViECodec->Release();
	remainingInterfaces += ptrViECapture->Release();
	remainingInterfaces += ptrViERtpRtcp->Release();
	remainingInterfaces += ptrViERender->Release();
	remainingInterfaces += ptrViENetwork->Release();
	remainingInterfaces += ptrViEBase->Release();

	if (remainingInterfaces > 0)
	{
		printf("ERROR: Could not release all interfaces\n");
		return -1;
	}

	bool deleted = webrtc::VideoEngine::Delete(ptrViE);
	if (deleted == false)
	{
		printf("ERROR in VideoEngine::Delete\n");
		return -1;
	}

	return 0;

}

void SetUpVideoEngineSampleSender(const char* ipAddress, uint16_t rtpPort, uint16_t localPort, void *window1)
{

	int error = 0;

	//
	// Create a VideoEngine instance
	//

	webrtc::VideoEngine* ptrViE = webrtc::VideoEngine::Create();
	if (ptrViE == NULL)	{ printf("ERROR in VideoEngine::Create\n"); error = 0; }

	//
	// Init VideoEngine and create a channel
	//

	webrtc::ViEBase* ptrViEBase = webrtc::ViEBase::GetInterface(ptrViE);
	if (ptrViEBase == NULL)	{ printf("ERROR in ViEBase::GetInterface\n"); error = 0; }

	error = ptrViEBase->Init();
	if (error == -1){printf("ERROR in ViEBase::Init\n");}

	webrtc::ViERTP_RTCP* ptrViERtpRtcp =
		webrtc::ViERTP_RTCP::GetInterface(ptrViE);
	if (ptrViERtpRtcp == NULL){ printf("ERROR in ViERTP_RTCP::GetInterface\n"); error = 0; }

	int videoChannel = -1;
	error = ptrViEBase->CreateChannel(videoChannel);
	if (error == -1){ printf("ERROR in ViEBase::CreateChannel\n"); error = 0; }
	else{
		printf("Using Channel: %d\n", videoChannel);
	}

	//
	// List available capture devices, allocate and connect.
	//
	webrtc::ViECapture* ptrViECapture =	webrtc::ViECapture::GetInterface(ptrViE);
	if (ptrViEBase == NULL){ printf("ERROR in ViECapture::GetInterface\n"); error = 0; }

	const unsigned int KMaxDeviceNameLength = 128;
	const unsigned int KMaxUniqueIdLength = 256;
	char deviceName[KMaxDeviceNameLength];
	memset(deviceName, 0, KMaxDeviceNameLength);
	char uniqueId[KMaxUniqueIdLength];
	memset(uniqueId, 0, KMaxUniqueIdLength);

	printf("Available capture devices:\n");
	int captureIdx = 0;
	for (captureIdx = 0;
		captureIdx < ptrViECapture->NumberOfCaptureDevices();
		captureIdx++)
	{
		memset(deviceName, 0, KMaxDeviceNameLength);
		memset(uniqueId, 0, KMaxUniqueIdLength);

		error = ptrViECapture->GetCaptureDevice(captureIdx, deviceName,
			KMaxDeviceNameLength, uniqueId,
			KMaxUniqueIdLength);
		if (error == -1)
		{
			printf("ERROR in ViECapture::GetCaptureDevice\n");
			error = 0;
		}
		printf("\t %d. %s\n", captureIdx + 1, deviceName);
	}
	printf("\nChoose capture device: ");

	if (scanf("%d", &captureIdx) != 1)
	{
		printf("Error in scanf()\n");
		error = 0;
	}
	getchar();
	captureIdx = captureIdx - 1; // Compensate for idx start at 1.

	error = ptrViECapture->GetCaptureDevice(captureIdx, deviceName,	KMaxDeviceNameLength, uniqueId,	KMaxUniqueIdLength);
	if (error == -1){ printf("ERROR in ViECapture::GetCaptureDevice\n"); error = 0; }

	int captureId = 0;
	error = ptrViECapture->AllocateCaptureDevice(uniqueId, KMaxUniqueIdLength,captureId);
	if (error == -1){ printf("ERROR in ViECapture::AllocateCaptureDevice\n"); error = 0; }

	error = ptrViECapture->ConnectCaptureDevice(captureId, videoChannel);
	if (error == -1){ printf("ERROR in ViECapture::ConnectCaptureDevice\n"); error = 0; }

	error = ptrViECapture->StartCapture(captureId);
	if (error == -1){ printf("ERROR in ViECapture::StartCapture\n"); error = 0; }

	//
	// RTP/RTCP settings
	//

	error = ptrViERtpRtcp->SetRTCPStatus(videoChannel, webrtc::kRtcpCompound_RFC4585);
	if (error == -1){ printf("ERROR in ViERTP_RTCP::SetRTCPStatus\n"); error = 0; }

	
	error = ptrViERtpRtcp->SetKeyFrameRequestMethod(videoChannel, webrtc::kViEKeyFrameRequestPliRtcp);
	if (error == -1){ printf("ERROR in ViERTP_RTCP::SetKeyFrameRequestMethod\n"); error = 0; }

	error = ptrViERtpRtcp->SetRembStatus(videoChannel, true, true);
	if (error == -1){ printf("ERROR in ViERTP_RTCP::SetTMMBRStatus\n"); error = 0; }
	


	//
	// Set up rendering
	//
	webrtc::ViERender* ptrViERender = webrtc::ViERender::GetInterface(ptrViE);
	if (ptrViERender == NULL) { printf("ERROR in ViERender::GetInterface\n"); error = 0; }

	error = ptrViERender->AddRenderer(captureId, window1, 0, 0.0, 0.0, 1.0, 1.0);
	if (error == -1){ printf("ERROR in ViERender::AddRenderer\n"); error = 0; }

	error = ptrViERender->StartRender(captureId);
	if (error == -1){ printf("ERROR in ViERender::StartRender\n"); error = 0; }

	//
	// Setup codecs
	//

	webrtc::ViECodec* ptrViECodec = webrtc::ViECodec::GetInterface(ptrViE);
	if (ptrViECodec == NULL){ printf("ERROR in ViECodec::GetInterface\n"); error = 0; }
	
	VideoCodec videoCodec;
	
	int numOfVeCodecs = ptrViECodec->NumberOfCodecs();
	for (int i = 0; i<numOfVeCodecs; ++i)
	{
		if (ptrViECodec->GetCodec(i, videoCodec) != -1)
		{
			if (videoCodec.codecType == kVideoCodecVP8)
				break;
		}
	}

	videoCodec.targetBitrate = 64;
	videoCodec.minBitrate = 32;
	videoCodec.maxBitrate = 128;
	videoCodec.maxFramerate = 60;

	error = ptrViECodec->SetSendCodec(videoChannel, videoCodec);
	assert(error != -1);
	error = ptrViECodec->SetReceiveCodec(videoChannel, videoCodec);
	assert(error != -1);
	
	//
	// Address settings
	//

	webrtc::ViENetwork* ptrViENetwork =	webrtc::ViENetwork::GetInterface(ptrViE);
	if (ptrViENetwork == NULL){ printf("ERROR in ViENetwork::GetInterface\n"); error = 0; }

	VideoChannelTransport* video_channel_transport = NULL;
	video_channel_transport = new VideoChannelTransport(ptrViENetwork, videoChannel);

	//error = video_channel_transport->SetLocalReceiver(rtpPort);
	error = video_channel_transport->SetLocalReceiver(localPort);
	if (error == -1){ printf("ERROR in SetLocalReceiver\n"); error = 0; }
	
	std::cout << std::endl;
	std::cout << "Sending to IP: " << ipAddress << std::endl;
	std::cout << "Sending to rtp port: " << rtpPort << std::endl;
	std::cout << std::endl;


	//error = ptrViEBase->StartReceive(videoChannel);
	//if (error == -1){ printf("ERROR in ViENetwork::StartReceive\n"); }
	error = video_channel_transport->SetSendDestination(ipAddress, rtpPort);
	if (error == -1){ printf("ERROR in SetSendDestination\n"); error = 0; }

	printf("StartSend\n");error = ptrViEBase->StartSend(videoChannel);
	if (error == -1){ printf("ERROR in ViENetwork::StartSend\n"); error = 0; }

	printf("Press enter to stop...");
	while ((getchar()) != '\n')	;

	error = ptrViEBase->StopSend(videoChannel);
	if (error == -1){ printf("ERROR in ViEBase::StopSend\n"); error = 0; }

	error = ptrViERender->StopRender(captureId);
	if (error == -1){ printf("ERROR in ViERender::StopRender\n"); error = 0; }

	error = ptrViERender->RemoveRenderer(captureId);
	if (error == -1){ printf("ERROR in ViERender::RemoveRenderer\n"); error = 0; }

	error = ptrViECapture->StopCapture(captureId);
	if (error == -1){ printf("ERROR in ViECapture::StopCapture\n"); error = 0; }

	error = ptrViECapture->DisconnectCaptureDevice(videoChannel);
	if (error == -1){ printf("ERROR in ViECapture::DisconnectCaptureDevice\n"); error = 0; }

	error = ptrViECapture->ReleaseCaptureDevice(captureId);
	if (error == -1){ printf("ERROR in ViECapture::ReleaseCaptureDevice\n"); error = 0; }

	delete video_channel_transport;
	int remainingInterfaces = 0;
	remainingInterfaces = ptrViECodec->Release();
	remainingInterfaces += ptrViECapture->Release();
	remainingInterfaces += ptrViERtpRtcp->Release();
	remainingInterfaces += ptrViERender->Release();
	remainingInterfaces += ptrViENetwork->Release();
	remainingInterfaces += ptrViEBase->Release();

	if (remainingInterfaces > 0){ printf("ERROR: Could not release all interfaces\n"); error = 0; }

	bool deleted = webrtc::VideoEngine::Delete(ptrViE);
	if (deleted == false){ printf("ERROR in VideoEngine::Delete\n"); error = 0; }

}

int main(int argc, char* argvc[])
{

	mod_ViEWindowCreator windowCreator;
	mod_ViEAutoTestWindowManagerInterface* windowManagerSender = windowCreator.CreateOneWindow("1");
	void *window1 = windowManagerSender->GetWindow1();
	const char* destinationIpAddress = "127.0.0.1";
	uint16_t rtpPort = 7000;
	uint16_t localPort = 6000;

	SetUpVideoEngineSampleSender(destinationIpAddress, rtpPort, localPort, window1);



	/*
	ViEWindowCreator windowCreator;

	ViEAutoTestWindowManagerInterface* windowManager =
	windowCreator.CreateTwoWindows();

	VideoEngineSample(windowManager->GetWindow1(), windowManager->GetWindow2());
	*/

	
	return 0;
}
