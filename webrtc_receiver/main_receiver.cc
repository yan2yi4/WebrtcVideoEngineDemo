
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
			return_value = socket_transport_->StartReceiving(500);
	}
	return return_value;
}

int VideoChannelTransport::SetSendDestination(const char* ip_address,
	uint16_t rtp_port) {
	return socket_transport_->InitializeSendSockets(ip_address, rtp_port);
}

void Receiving(uint16_t rtpPort, void *window2){

	int error = 0;
	//
	// Create a VideoEngine instance
	//

	webrtc::VideoEngine* ptrViE = NULL;
	ptrViE = webrtc::VideoEngine::Create();
	if (ptrViE == NULL){printf("ERROR in VideoEngine::Create\n");error = 0;}

	//
	// Init VideoEngine and create a channel
	//

	webrtc::ViEBase* ptrViEBase = webrtc::ViEBase::GetInterface(ptrViE);
	if (ptrViEBase == NULL){ printf("ERROR in ViEBase::GetInterface\n"); error = 0; }

	error = ptrViEBase->Init();
	if (error == -1){ printf("ERROR in ViEBase::Init\n"); error = 0; }

	int videoChannel = -1;
	error = ptrViEBase->CreateChannel(videoChannel);
	if (error == -1){ printf("ERROR in ViEBase::CreateChannel\n"); error = 0; }
	else{
		printf("Ussing Channel: %d\n", videoChannel);
	}

	//
	// RTP/RTCP settings
	//
	webrtc::ViERTP_RTCP* ptrViERtpRtcp =
	webrtc::ViERTP_RTCP::GetInterface(ptrViE);
	if (ptrViERtpRtcp == NULL){ printf("ERROR in ViERTP_RTCP::GetInterface\n"); error = 0; }
	error = ptrViERtpRtcp->SetRTCPStatus(videoChannel,
	webrtc::kRtcpCompound_RFC4585);
	if (error == -1){ printf("ERROR in ViERTP_RTCP::SetRTCPStatus\n"); error = 0; }

	error = ptrViERtpRtcp->SetKeyFrameRequestMethod(
	videoChannel, webrtc::kViEKeyFrameRequestPliRtcp);
	if (error == -1){ printf("ERROR in ViERTP_RTCP::SetKeyFrameRequestMethod\n"); error = 0; }

	error = ptrViERtpRtcp->SetRembStatus(videoChannel, true, true);
	if (error == -1){ printf("ERROR in ViERTP_RTCP::SetTMMBRStatus\n"); error = 0; }

	//
	// Setup codecs
	//

	webrtc::ViECodec* ptrViECodec = webrtc::ViECodec::GetInterface(ptrViE);
	if (ptrViECodec == NULL){ printf("ERROR in ViECodec::GetInterface\n"); error = 0; }

	VideoCodec videoCodec;

	int numOfVeCodecs = ptrViECodec->NumberOfCodecs();
	for (int i = 0; i < numOfVeCodecs; ++i)
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

	error = ptrViECodec->SetReceiveCodec(videoChannel, videoCodec);
	assert(error != -1);

	/*
	//error = ptrViECodec->SetSendCodec(videoChannel, videoCodec);
	//assert(error != -1);
	*/

	//
	// Address settings
	//
	webrtc::ViENetwork* ptrViENetwork =
	webrtc::ViENetwork::GetInterface(ptrViE);
	if (ptrViENetwork == NULL){ printf("ERROR in ViENetwork::GetInterface\n");	error = 0; }

	VideoChannelTransport* video_channel_transport = NULL;
	video_channel_transport = new VideoChannelTransport(ptrViENetwork, videoChannel);

	std::cout << "Using rtp port: " << rtpPort << std::endl;
	std::cout << std::endl;


	error = video_channel_transport->SetLocalReceiver(rtpPort);
	if (error == -1){ printf("ERROR in SetLocalReceiver: %d\n", error); error = 0; }
	error = ptrViEBase->StartReceive(videoChannel);
	if (error == -1){ printf("ERROR in ViENetwork::StartReceive\n"); error = 0; }

	//
	// Set up rendering
	//
	webrtc::ViERender* ptrViERender = webrtc::ViERender::GetInterface(ptrViE);
	if (ptrViERender == NULL) { printf("ERROR in ViERender::GetInterface\n"); error = 0; }

	error = ptrViERender->AddRenderer(videoChannel, window2, 1, 0.0, 0.0, 1.0,
		1.0);
	if (error == -1){ printf("ERROR in ViERender::AddRenderer\n"); error = 0; }

	error = ptrViERender->StartRender(videoChannel);
	if (error == -1){ printf("ERROR in ViERender::StartRender\n"); error = 0; }
	
	printf("Press enter to stop...");
	while ((getchar()) != '\n')
		;

	error = ptrViEBase->StopReceive(videoChannel);
	if (error == -1){ printf("ERROR in ViEBase::StopReceive\n"); error = 0; }

	error = ptrViERender->StopRender(videoChannel);
	if (error == -1){ printf("ERROR in ViERender::StopRender\n"); error = 0; }

	error = ptrViERender->RemoveRenderer(videoChannel);
	if (error == -1){ printf("ERROR in ViERender::RemoveRenderer\n"); error = 0; }

	error = ptrViEBase->DeleteChannel(videoChannel);
	if (error == -1){ printf("ERROR in ViEBase::DeleteChannel\n"); error = 0; }

	//delete video_channel_transport;
	int remainingInterfaces = 0;
	remainingInterfaces = ptrViECodec->Release();
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
	mod_ViEAutoTestWindowManagerInterface* windowManagerReceiver = windowCreator.CreateOneWindow("2");
	void *window2 = windowManagerReceiver->GetWindow1();
	uint16_t localPort = 7000;

	Receiving(localPort, window2);
	/*
	ViEWindowCreator windowCreator;

	ViEAutoTestWindowManagerInterface* windowManager =
	windowCreator.CreateTwoWindows();

	VideoEngineSample(windowManager->GetWindow1(), windowManager->GetWindow2());
	*/
	return 0;
}
