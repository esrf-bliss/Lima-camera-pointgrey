//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################

#include "PointGreyInterface.h"
#include "PointGreyCamera.h"
#include "PointGreyDetInfoCtrlObj.h"
#include "PointGreySyncCtrlObj.h"

using namespace lima;
using namespace lima::PointGrey;
using namespace std;

/*******************************************************************
 * \brief Hw Interface constructor
 *******************************************************************/
Interface::Interface(Camera& cam)
    : m_cam(cam)
{
    DEB_CONSTRUCTOR();
    m_det_info = new DetInfoCtrlObj(cam);
    m_sync = new SyncCtrlObj(cam);

    m_cap_list.push_back(HwCap(m_det_info));
    m_cap_list.push_back(HwCap(m_sync));

    HwBufferCtrlObj *buffer = cam.getBufferCtrlObj();
    m_cap_list.push_back(HwCap(buffer));
}

//-----------------------------------------------------
//
//-----------------------------------------------------

Interface::~Interface()
{
    DEB_DESTRUCTOR();
    delete m_det_info;
    delete m_sync;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::getCapList(HwInterface::CapList &cap_list) const
{
    DEB_MEMBER_FUNCT();
    cap_list = m_cap_list;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::reset(ResetLevel reset_level)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(reset_level);
    stopAcq();
    m_cam._setStatus(Camera::Ready, true);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::prepareAcq()
{
    DEB_MEMBER_FUNCT();
    m_cam.prepareAcq();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::startAcq()
{
    DEB_MEMBER_FUNCT();
    m_cam.startAcq();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::stopAcq()
{
    DEB_MEMBER_FUNCT();
    m_cam.stopAcq();
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void Interface::getStatus(StatusType& status)
{
    DEB_MEMBER_FUNCT();
    Camera::Status pg_status = Camera::Ready;
    m_cam.getStatus(pg_status);
    switch (pg_status)
    {
    case Camera::Ready:
        status.det = DetIdle;
        status.acq = AcqReady;
        break;
    case Camera::Exposure:
        status.det = DetExposure;
        status.acq = AcqRunning;
        break;
    case Camera::Readout:
        status.det = DetReadout;
        status.acq = AcqRunning;
        break;
    case Camera::Latency:
        status.det = DetLatency;
        status.acq = AcqRunning;
        break;
    case Camera::Fault:
        status.det = DetFault;
        status.acq = AcqFault;
    }
    status.det_mask = DetExposure | DetReadout | DetLatency;
    DEB_RETURN() << DEB_VAR1(status);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
int Interface::getNbHwAcquiredFrames()
{
    DEB_MEMBER_FUNCT();
    int acq_frames;
    m_cam.getNbHwAcquiredFrames(acq_frames);
    return acq_frames;
}
