#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <unistd.h>
#include "lima/CtControl.h"
#include "lima/CtSaving.h"
#include "lima/CtAcquisition.h"
#include "lima/CtImage.h"
#include "lima/HwInterface.h"
#include "PointGreyCamera.h"
#include "PointGreyInterface.h"

using namespace std;
using namespace lima;
using namespace lima::PointGrey;

DEB_GLOBAL(DebModTest);

const int camera_serial = 123456;
const int packet_size = 9000;
const int packet_delay = 0;

int main () {
  DEB_GLOBAL_FUNCT();
  DebParams::setModuleFlags(DebParams::AllFlags);
  DebParams::setTypeFlags(DebParams::AllFlags);
  DebParams::setFormatFlags(DebParams::AllFlags);

  Camera *m_camera;
  Interface *m_interface;
  CtControl* m_control;

  int nframes = 25;
  double time = 0.1;

  try {
    m_camera = new Camera(camera_serial, packet_size, packet_delay);
    m_interface = new Interface(*m_camera);
    m_control = new CtControl(m_interface);

    // do acquisition
    m_control->acquisition()->setAcqExpoTime(time);
    m_control->acquisition()->setLatencyTime(0.5);
    m_control->acquisition()->setAcqNbFrames(nframes);
    m_control->prepareAcq();
    m_control->startAcq();
    while (1) {
      CtControl::Status ctStatus;
      m_control->getStatus(ctStatus);
      if (ctStatus.AcquisitionStatus == lima::AcqReady) {
	break;
      } else if (ctStatus.AcquisitionStatus != lima::AcqRunning) {
	THROW_HW_ERROR(Error) << "status fault ";
      } else {
	sleep(1);
	std::cout << "wait while still running" << std::endl;
      }
    }
    std::cout << "Complete" << std::endl;

  } catch (Exception &e) {

  }
}
