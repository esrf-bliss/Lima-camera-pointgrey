namespace PointGrey
{
  class Interface : HwInterface
  {
%TypeHeaderCode
#include <PointGreyInterface.h>
%End
  public:
    Interface(PointGrey::Camera& cam);
    virtual ~Interface();

    //- from HwInterface
    virtual void	getCapList(std::vector<HwCap> &cap_list /Out/) const;
    virtual void	reset(ResetLevel reset_level);
    virtual void 	prepareAcq();
    virtual void 	startAcq();
    virtual void 	stopAcq();
    virtual void 	getStatus(StatusType& status /Out/);
    virtual int 	getNbHwAcquiredFrames();
    
    // -- camera specific

    // packet size control
    void getPacketSize(int& packet_size /Out/);
%MethodCode
        sipCpp->getCamera().getPacketSize(a0);
%End
    
    void setPacketSize(int  packet_size);
%MethodCode
        sipCpp->getCamera().setPacketSize(a0);
%End

    // packet delay control
    void getPacketDelay(int& packet_delay /Out/);
%MethodCode
        sipCpp->getCamera().getPacketDelay(a0);
%End    

    void setPacketDelay(int  packet_delay);
%MethodCode
        sipCpp->getCamera().setPacketDelay(a0);
%End    

    // gain control
    void getGain(double& gain /Out/);
%MethodCode
        sipCpp->getCamera().getGain(a0);
%End    
    
    void setGain(double gain);
%MethodCode
        sipCpp->getCamera().setGain(a0);
%End    

    void getAutoGain(bool& auto_gain /Out/);
%MethodCode
        sipCpp->getCamera().getAutoGain(a0);
%End    

    void setAutoGain(bool auto_gain);
%MethodCode
        sipCpp->getCamera().setAutoGain(a0);
%End    

    void getGainRange(double& min_gain /Out/, double& max_gain /Out/);
%MethodCode
        sipCpp->getCamera().getGainRange(a0, a1);
%End    

    // exposure control
    void getExpTime(double& exp_time /Out/);
%MethodCode
        sipCpp->getCamera().getExpTime(a0);
%End    
    
    void setExpTime(double exp_time);
%MethodCode
        sipCpp->getCamera().setExpTime(a0);
%End    

    void getAutoExpTime(bool& auto_exp_time /Out/);
%MethodCode
        sipCpp->getCamera().getAutoExpTime(a0);
%End    

    void setAutoExpTime(bool auto_exp_time);
%MethodCode
        sipCpp->getCamera().setAutoExpTime(a0);
%End    

    void getExpTimeRange(double& min_exp_time /Out/, double& max_exp_time /Out/);
%MethodCode
        sipCpp->getCamera().getExpTimeRange(a0, a1);
%End    

    // frame rate control
    void getFrameRate(double& frame_rate /Out/);	
%MethodCode
        sipCpp->getCamera().getFrameRate(a0);
%End    
    
    void setFrameRate(double  frame_rate);
%MethodCode
        sipCpp->getCamera().setFrameRate(a0);
%End    

    void getAutoFrameRate(bool& auto_frame_rate /Out/);
%MethodCode
        sipCpp->getCamera().getAutoFrameRate(a0);
%End    

    void setAutoFrameRate(bool auto_frame_rate);
%MethodCode
        sipCpp->getCamera().setAutoFrameRate(a0);
%End    
    
    void getFrameRateRange(double& min_frame_rate /Out/, double& max_frame_rate /Out/);
%MethodCode
        sipCpp->getCamera().getFrameRateRange(a0, a1);
%End    

  };
};
