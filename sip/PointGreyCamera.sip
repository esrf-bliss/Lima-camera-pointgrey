
namespace PointGrey
{
  class Camera
  {
%TypeHeaderCode
#include <PointGreyCamera.h>
%End

  public:

    enum Status {
      Ready, Exposure, Readout, Latency, Fault
    };

    Camera(std::string& camera_ip, const int packet_size = -1, const int packet_delay = -1);
    Camera(const int camera_serial, const int packet_size = -1, const int packet_delay = -1);
    ~Camera();

    void prepareAcq();
    void startAcq();
    void stopAcq();

    void getStatus(PointGrey::Camera::Status& status /Out/);
    
    // -- detector info
    void getDetectorType(std::string& type /Out/);
    void getDetectorModel(std::string& model /Out/);
    void getDetectorImageSize(Size& size /Out/);
	
    // -- sync
    void getTrigMode(TrigMode& mode /Out/);
    void setTrigMode(TrigMode  mode);

    void getExpTime(double& exp_time /Out/);	
    void setExpTime(double  exp_time);
    void getExpTimeRange(double& min_exp_time /Out/, double& max_exp_time /Out/);

    void getNbFrames(int& nb_frames /Out/);
    void setNbFrames(int  nb_frames);

    // -- roi	
    void checkRoi(const Roi& set_roi, Roi& hw_roi /Out/);
    void getRoi(Roi& hw_roi /Out/); 
    void setRoi(const Roi& set_roi);
    bool isRoiAvailable() const;

    // -- bin
    void checkBin(Bin& /In,Out/);
    void getBin(Bin& /Out/);
    void setBin(const Bin&);
    bool isBinningAvailable() const;

    // -- camera specific
    // packet size control
    void getPacketSize(int& packet_size /Out/);
    void setPacketSize(int  packet_size);

    // packet delay control
    void getPacketDelay(int& packet_delay /Out/);
    void setPacketDelay(int  packet_delay);

    // exposure control
    void getAutoExpTime(bool& auto_exp_time /Out/);
    void setAutoExpTime(bool auto_exp_time);

    // gain control
    void getGain(double& gain /Out/);
    void setGain(double gain);
    void getAutoGain(bool& auto_gain /Out/);
    void setAutoGain(bool auto_gain);
    void getGainRange(double& min_gain /Out/, double& max_gain /Out/);
 
    // frame rate control
    void getFrameRate(double& frame_rate /Out/);	
    void setFrameRate(double  frame_rate);
    void getAutoFrameRate(bool& auto_frame_rate /Out/);
    void setAutoFrameRate(bool auto_frame_rate);
    void getFrameRateRange(double& min_frame_rate /Out/, double& max_frame_rate /Out/);
    void getFrameRateOnOff(bool& onOff /Out/);
    void setFrameRateOnOff(bool onOff);
  };
};
