#include "PointGreyCamera.h"

using namespace lima;
using namespace lima::PointGrey;
using namespace std;

//-----------------------------------------------------
// _AcqThread class
//-----------------------------------------------------
class Camera::_AcqThread : public Thread
{
    DEB_CLASS_NAMESPC(DebModCamera, "Camera", "_AcqThread");
public:
    _AcqThread(Camera &aCam);
    virtual ~_AcqThread();
protected:
    virtual void threadFunction();
private:
    Camera &m_cam;
};

//-----------------------------------------------------
//
//-----------------------------------------------------
Camera::Camera(const int camera_serial,
               const int packet_size,
               const int packet_delay)
    : m_nb_frames(1)
    , m_status(Ready)
    , m_quit(false)
    , m_acq_started(false)
    , m_thread_running(true)
    , m_image_number(0)
    , m_camera(NULL)
{
    DEB_CONSTRUCTOR();

    FlyCapture2::BusManager busmgr;
    FlyCapture2::PGRGuid pgrguid;

    unsigned int nb_cameras;
    m_camera = new Camera_t();

    m_error = busmgr.GetNumOfCameras(&nb_cameras);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to create bus manager: " << m_error.GetDescription();

    if (nb_cameras < 1)
        THROW_HW_ERROR(Error) << "No cameras found";

    m_error = busmgr.GetCameraFromSerialNumber(camera_serial, &pgrguid);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Camera not found: " << m_error.GetDescription();

    m_error = m_camera->Connect(&pgrguid);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to connect to camera: " << m_error.GetDescription();

    m_error = m_camera->GetCameraInfo(&m_camera_info);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to get camera info: " << m_error.GetDescription();

    if (packet_size > 0)
        setPacketSize(packet_size);

    if (packet_delay > 0)
        setPacketDelay(packet_delay);

    _getImageSettingsInfo();

    // Setup default image format
    m_image_settings.offsetX = 0;
    m_image_settings.offsetY = 0;
    m_image_settings.width = m_image_settings_info.maxWidth;
    m_image_settings.height = m_image_settings_info.maxHeight;
    m_image_settings.pixelFormat = FlyCapture2::PIXEL_FORMAT_MONO8;

    _applyImageSettings();

    //Acquisition  Thread
    m_acq_thread = new _AcqThread(*this);
    m_acq_thread->start();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
Camera::~Camera()
{
    DEB_DESTRUCTOR();
    delete m_acq_thread;
    m_camera->Disconnect();
    delete m_camera;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::_getImageSettingsInfo()
{
    DEB_MEMBER_FUNCT();
#ifdef USE_GIGE
    m_error = m_camera->GetGigEImageSettingsInfo(&m_image_settings_info);
#else
    bool fmt7_supported;
    m_image_settings_info.mode = FlyCapture2::MODE_0;
    m_error = m_camera->GetFormat7Info(&m_image_settings_info, &fmt7_supported);
    if (!fmt7_supported)
        THROW_HW_ERROR(Error) << "Format7 is not supported";
#endif
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to get Format7 info: " << m_error.GetDescription();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::_applyImageSettings()
{
    DEB_MEMBER_FUNCT();
#ifdef USE_GIGE
    m_error = m_camera->SetGigEImageSettings(&m_image_settings);
#else
    bool valid;
    FlyCapture2::Format7PacketInfo packet_info;
    m_error = m_camera->ValidateFormat7Settings(&m_image_settings, &valid, &packet_info);

    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Unable to validate image format settings: " << m_error.GetDescription();
    if (!valid)
        THROW_HW_ERROR(Error) << "Unsupported image format settings";

    m_error = m_camera->SetFormat7Configuration(&m_image_settings,
                                                packet_info.recommendedBytesPerPacket);
#endif
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Unable to apply image format settings: " << m_error.GetDescription();

    if (m_image_settings.pixelFormat == FlyCapture2::PIXEL_FORMAT_MONO16)
        // Force the camera to PGR's Y16 endianness
        _forcePGRY16Mode();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getPacketSize(int& packet_size)
{
    DEB_MEMBER_FUNCT();
#ifdef USE_GIGE
    FlyCapture2::GigEProperty property;
    property.propType = FlyCapture2::PACKET_SIZE;

    m_error = m_camera->GetGigEProperty(&property);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to get PACKET_SIZE property: " << m_error.GetDescription();

    packet_size = property.value;
    DEB_RETURN() << DEB_VAR1(packet_size);
#endif
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setPacketSize(int packet_size)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(packet_size);
#ifdef USE_GIGE
    FlyCapture2::GigEProperty property;
    property.propType = FlyCapture2::PACKET_SIZE;
    property.value = packet_size;

    m_error = m_camera->SetGigEProperty(&property);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to set PACKET_SIZE property: " << m_error.GetDescription();
#endif
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getPacketDelay(int& packet_delay)
{
    DEB_MEMBER_FUNCT();
#ifdef USE_GIGE
    FlyCapture2::GigEProperty property;
    property.propType = FlyCapture2::PACKET_DELAY;

    m_error = m_camera->GetGigEProperty(&property);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to get PACKET_DELAY property: " << m_error.GetDescription();

    packet_delay = property.value;
    DEB_RETURN() << DEB_VAR1(packet_delay);
#endif
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setPacketDelay(int packet_delay)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(packet_delay);
#ifdef USE_GIGE
    FlyCapture2::GigEProperty property;
    property.propType = FlyCapture2::PACKET_DELAY;
    property.value = packet_delay;

    m_error = m_camera->SetGigEProperty(&property);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to set PACKET_DELAY property: " << m_error.GetDescription();
#endif
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::prepareAcq()
{
    DEB_MEMBER_FUNCT();
    m_image_number = 0;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::startAcq()
{
    DEB_MEMBER_FUNCT();

    DEB_TRACE() << "Start acquisition";

    StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_obj.getBuffer();
    buffer_mgr.setStartTimestamp(Timestamp::now());

    m_error = m_camera->StartCapture();
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Unable to start image capture: " << m_error.GetDescription();

    // Start acquisition thread
    AutoMutex lock(m_cond.mutex());
    m_acq_started = true;
    m_cond.broadcast();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::stopAcq()
{
    DEB_MEMBER_FUNCT();
    AutoMutex lock(m_cond.mutex());
    if (!m_acq_started)
        return;
    m_acq_started = false;
    lock.unlock();

    DEB_TRACE() << "Stop acquisition";
    m_error = m_camera->StopCapture();
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Unable to stop image capture: " << m_error.GetDescription();

    _setStatus(Camera::Ready, false);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getDetectorImageSize(Size& size)
{
    DEB_MEMBER_FUNCT();
    size = Size(m_image_settings_info.maxWidth, m_image_settings_info.maxHeight);
    DEB_RETURN() << DEB_VAR1(size);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getDetectorType(string& type)
{
    DEB_MEMBER_FUNCT();
    type = m_camera_info.vendorName;
    DEB_RETURN() << DEB_VAR1(type);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getDetectorModel(string& model)
{
    DEB_MEMBER_FUNCT();
    model = m_camera_info.modelName;
    DEB_RETURN() << DEB_VAR1(model);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getImageType(ImageType& type)
{
    DEB_MEMBER_FUNCT();
    switch (m_image_settings.pixelFormat)
    {
    case FlyCapture2::PIXEL_FORMAT_MONO8:
        type = Bpp8;
        break;
    case FlyCapture2::PIXEL_FORMAT_MONO16:
        type = Bpp16;
        break;
    default:
        THROW_HW_ERROR(Error) << "Unable to determine the image type";
    }
    DEB_RETURN() << DEB_VAR1(type);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setImageType(ImageType type)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(type);

    FlyCapture2::PixelFormat old_format, new_format;
    bool valid;

    old_format = m_image_settings.pixelFormat;

    switch (type)
    {
    case Bpp8:
        new_format = FlyCapture2::PIXEL_FORMAT_MONO8;
        break;
    case Bpp16:
        new_format = FlyCapture2::PIXEL_FORMAT_MONO16;
        break;
    default:
        THROW_HW_ERROR(Error) << "Unsupported image type";
    }

    if (new_format == old_format)
        // nothing to do
        return;

    if (m_acq_started)
        THROW_HW_ERROR(Error) << "Acquisition in progress";

    m_image_settings.pixelFormat = new_format;
    try
    {
        _applyImageSettings();
    }
    catch (Exception &e)
    {
        m_image_settings.pixelFormat = old_format;
        THROW_HW_ERROR(Error) << e.getErrDesc();
    }

    maxImageSizeChanged(Size(m_image_settings_info.maxWidth, m_image_settings_info.maxHeight), type);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getTrigMode(TrigMode& mode)
{
    DEB_MEMBER_FUNCT();

    cout << "getTrigMode" << endl;

    // Get current trigger settings
    FlyCapture2::TriggerMode triggerMode;
    m_error = m_camera->GetTriggerMode(&triggerMode);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Unable to get trigger mode settings: " << m_error.GetDescription();

    mode = triggerMode.onOff ? ExtTrigSingle : IntTrig;

    DEB_RETURN() << DEB_VAR1(mode);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setTrigMode(TrigMode mode)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(mode);

    // Check for external trigger support
    FlyCapture2::TriggerModeInfo triggerModeInfo;
    m_error = m_camera->GetTriggerModeInfo(&triggerModeInfo);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Unable to get trigger mode info from camera: " << m_error.GetDescription();

    if (!triggerModeInfo.present)
    {
        DEB_ERROR() << "Camera does not support external trigger";
        return;
    }

    // Get current trigger settings
    FlyCapture2::TriggerMode triggerMode;
    m_error = m_camera->GetTriggerMode(&triggerMode);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Unable to get trigger mode settings: " << m_error.GetDescription();

    switch (mode)
    {
    case IntTrig:
        triggerMode.onOff = false;
        break;
    case ExtTrigSingle:
        triggerMode.onOff = true;
        triggerMode.mode = 0;
        triggerMode.source = 0;
        break;
    default:
        THROW_HW_ERROR(Error) << "Trigger mode " << mode << " is not supported";
    }

    m_error = m_camera->SetTriggerMode(&triggerMode);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Unable to set trigger mode settings: " << m_error.GetDescription();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getNbFrames(int& nb_frames)
{
    DEB_MEMBER_FUNCT();
    nb_frames = m_nb_frames;
    DEB_RETURN() << DEB_VAR1(nb_frames);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setNbFrames(int nb_frames)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(nb_frames);
    m_nb_frames = nb_frames;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getNbHwAcquiredFrames(int &nb_acq_frames)
{
    DEB_MEMBER_FUNCT();
    nb_acq_frames = m_image_number;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getStatus(Camera::Status& status)
{
    DEB_MEMBER_FUNCT();
    AutoMutex lock(m_cond.mutex());
    status = m_status;
    DEB_RETURN() << DEB_VAR1(DEB_HEX(status));
}

//-----------------------------------------------------
//
//-----------------------------------------------------
HwBufferCtrlObj* Camera::getBufferCtrlObj()
{
    return &m_buffer_ctrl_obj;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::checkRoi(const Roi& set_roi, Roi& hw_roi)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(set_roi);
    DEB_RETURN() << DEB_VAR1(hw_roi);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getRoi(Roi& hw_roi)
{
    DEB_MEMBER_FUNCT();
    DEB_RETURN() << DEB_VAR1(hw_roi);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setRoi(const Roi& ask_roi)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(ask_roi);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::checkBin(Bin &aBin)
{
    DEB_MEMBER_FUNCT();
    DEB_RETURN() << DEB_VAR1(aBin);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getBin(Bin &aBin)
{
    DEB_MEMBER_FUNCT();
    DEB_RETURN() << DEB_VAR1(aBin);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setBin(const Bin &aBin)
{
    DEB_MEMBER_FUNCT();
    DEB_RETURN() << DEB_VAR1(aBin);
}

//-----------------------------------------------------
// exposure
//-----------------------------------------------------
void Camera::getExpTime(double& exp_time)
{
    DEB_MEMBER_FUNCT();
    _getPropertyValue(FlyCapture2::SHUTTER, exp_time);
    DEB_RETURN() << DEB_VAR1(exp_time);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setExpTime(double exp_time)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(exp_time);
    _setPropertyValue(FlyCapture2::SHUTTER, exp_time);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getExpTimeRange(double& min_exp_time, double& max_exp_time)
{
    DEB_MEMBER_FUNCT();
    _getPropertyRange(FlyCapture2::SHUTTER, min_exp_time, max_exp_time);
    DEB_RETURN() << DEB_VAR2(min_exp_time, max_exp_time);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getAutoExpTime(bool &auto_exp_time)
{
    DEB_MEMBER_FUNCT();
    _getPropertyAutoMode(FlyCapture2::SHUTTER, auto_exp_time);
    DEB_RETURN() << DEB_VAR1(auto_exp_time);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setAutoExpTime(bool auto_exp_time)
{
    DEB_MEMBER_FUNCT();
    _setPropertyAutoMode(FlyCapture2::SHUTTER, auto_exp_time);
    DEB_RETURN() << DEB_VAR1(auto_exp_time);
}

//-----------------------------------------------------
// gain
//-----------------------------------------------------
void Camera::getGain(double& gain)
{
    DEB_MEMBER_FUNCT();
    _getPropertyValue(FlyCapture2::GAIN, gain);
    DEB_RETURN() << DEB_VAR1(gain);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setGain(double gain)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(gain);
    _setPropertyValue(FlyCapture2::GAIN, gain);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getGainRange(double& min_gain, double& max_gain)
{
    DEB_MEMBER_FUNCT();
    _getPropertyRange(FlyCapture2::GAIN, min_gain, max_gain);
    DEB_RETURN() << DEB_VAR2(min_gain, max_gain);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getAutoGain(bool& auto_gain)
{
    DEB_MEMBER_FUNCT();
    _getPropertyAutoMode(FlyCapture2::GAIN, auto_gain);
    DEB_RETURN() << DEB_VAR1(auto_gain);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setAutoGain(bool auto_gain)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(auto_gain);
    _setPropertyAutoMode(FlyCapture2::GAIN, auto_gain);
}

//-----------------------------------------------------
// frame rate
//-----------------------------------------------------
void Camera::getFrameRate(double& frame_rate)
{
    DEB_MEMBER_FUNCT();
    _getPropertyValue(FlyCapture2::FRAME_RATE, frame_rate);
    DEB_RETURN() << DEB_VAR1(frame_rate);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setFrameRate(double frame_rate)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(frame_rate);
    _setPropertyValue(FlyCapture2::FRAME_RATE, frame_rate);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getFrameRateRange(double& min_frame_rate, double& max_frame_rate)
{
    DEB_MEMBER_FUNCT();
    _getPropertyRange(FlyCapture2::FRAME_RATE, min_frame_rate, max_frame_rate);
    DEB_RETURN() << DEB_VAR2(min_frame_rate, max_frame_rate);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::getAutoFrameRate(bool& auto_frame_rate)
{
    DEB_MEMBER_FUNCT();
    _getPropertyAutoMode(FlyCapture2::FRAME_RATE, auto_frame_rate);
    DEB_RETURN() << DEB_VAR1(auto_frame_rate);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::setAutoFrameRate(bool auto_frame_rate)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(auto_frame_rate);
    _setPropertyAutoMode(FlyCapture2::FRAME_RATE, auto_frame_rate);
}

//-----------------------------------------------------
// property management
//-----------------------------------------------------
void Camera::_getPropertyValue(FlyCapture2::PropertyType type, double& value)
{
    DEB_MEMBER_FUNCT();
    FlyCapture2::Property property(type);

    m_error = m_camera->GetProperty(&property);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to get camera property: " << m_error.GetDescription();

    value = property.absValue;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::_setPropertyValue(FlyCapture2::PropertyType type, double value)
{
    DEB_MEMBER_FUNCT();
    FlyCapture2::Property property(type);

    property.onOff = true;
    property.autoManualMode = false;
    property.absControl = true;
    property.absValue = value;

    m_error = m_camera->SetProperty(&property);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to set camera property: " << m_error.GetDescription();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::_getPropertyRange(FlyCapture2::PropertyType type, double& min_value, double& max_value)
{
    DEB_MEMBER_FUNCT();
    FlyCapture2::PropertyInfo property_info(type);

    m_error = m_camera->GetPropertyInfo(&property_info);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to get camera property info: " << m_error.GetDescription();

    min_value = property_info.absMin;
    max_value = property_info.absMax;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::_getPropertyAutoMode(FlyCapture2::PropertyType type, bool& auto_mode)
{
    DEB_MEMBER_FUNCT();
    FlyCapture2::Property property(type);

    m_error = m_camera->GetProperty(&property);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to get camera property: " << m_error.GetDescription();

    auto_mode = property.autoManualMode;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::_setPropertyAutoMode(FlyCapture2::PropertyType type, bool auto_mode)
{
    DEB_MEMBER_FUNCT();
    FlyCapture2::Property property(type);

    property.onOff = not auto_mode;
    property.autoManualMode = auto_mode;

    m_error = m_camera->SetProperty(&property);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to set camera property: " << m_error.GetDescription();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::_setStatus(Camera::Status status, bool force)
{
    DEB_MEMBER_FUNCT();
    AutoMutex alock(m_cond.mutex());
    if (force || m_status != Camera::Fault)
        m_status = status;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Camera::_forcePGRY16Mode()
{
    DEB_MEMBER_FUNCT();
    const unsigned int k_imageDataFmtReg = 0x1048;
    unsigned int value = 0;
    m_error = m_camera->ReadRegister(k_imageDataFmtReg, &value);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to read camera register: " << m_error.GetDescription();

    value &= ~0x1;

    m_error = m_camera->WriteRegister(k_imageDataFmtReg, value);
    if (m_error != FlyCapture2::PGRERROR_OK)
        THROW_HW_ERROR(Error) << "Failed to write camera register: " << m_error.GetDescription();
}

//-----------------------------------------------------
// acquisition thread
//-----------------------------------------------------
Camera::_AcqThread::_AcqThread(Camera &cam) : m_cam(cam)
{
    pthread_attr_setscope(&m_thread_attr, PTHREAD_SCOPE_PROCESS);
}

Camera::_AcqThread::~_AcqThread()
{
    AutoMutex lock(m_cam.m_cond.mutex());
    m_cam.m_quit = true;
    m_cam.m_cond.broadcast();
    lock.unlock();

    join();
}

void Camera::_AcqThread::threadFunction()
{
    DEB_MEMBER_FUNCT();
    FlyCapture2::Error error;
    FlyCapture2::Image image;

    sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param))
    {
        DEB_ERROR() << "Could not set FIFO scheduling for acquisition thread";
    }

    AutoMutex lock(m_cam.m_cond.mutex());
    StdBufferCbMgr& buffer_mgr = m_cam.m_buffer_ctrl_obj.getBuffer();

    while (true)
    {
        while (!m_cam.m_acq_started && !m_cam.m_quit)
        {
            DEB_TRACE() << "Wait";
            m_cam.m_thread_running = false;
            m_cam.m_cond.broadcast();
            m_cam.m_cond.wait();
        }
        if (m_cam.m_quit) return;

        m_cam.m_thread_running = true;
        m_cam.m_status = Camera::Exposure;
        lock.unlock();

        DEB_TRACE() << "Run";
        bool continue_acq = true;

        continue_acq = true;

        while (continue_acq && (!m_cam.m_nb_frames || m_cam.m_image_number < m_cam.m_nb_frames))
        {
            error = m_cam.m_camera->RetrieveBuffer(&image);
            if (error == FlyCapture2::PGRERROR_OK)
            {
                // Grabbing was successful, process image
                m_cam._setStatus(Camera::Readout, false);

                DEB_TRACE() << "image# " << m_cam.m_image_number << " acquired";
                void* framePt = buffer_mgr.getFrameBufferPtr(m_cam.m_image_number);
                const FrameDim& fDim = buffer_mgr.getFrameDim();
                memcpy(framePt, image.GetData(), fDim.getMemSize());

                HwFrameInfoType frame_info;
                frame_info.acq_frame_nb = m_cam.m_image_number;
                continue_acq = buffer_mgr.newFrameReady(frame_info);
                m_cam.m_image_number++;
            }
            else if (error == FlyCapture2::PGRERROR_ISOCH_NOT_STARTED)
            {
                DEB_TRACE() << "Acquisition aborted";
                continue_acq = false;
            }
            else if (error == FlyCapture2::PGRERROR_IMAGE_CONSISTENCY_ERROR)
            {
                DEB_WARNING() << "No image acquired: " << error.GetDescription();
            }
            else
            {
                DEB_ERROR() << "No image acquired: " << error.GetDescription();
                m_cam._setStatus(Camera::Fault, false);
                continue_acq = false;
            }
        }
        m_cam.stopAcq();
        lock.lock();
    }
}

