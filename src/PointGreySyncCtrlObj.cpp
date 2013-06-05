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

#include "PointGreySyncCtrlObj.h"
#include "PointGreyCamera.h"

using namespace lima;
using namespace lima::PointGrey;

/*******************************************************************
 * \brief SyncCtrlObj constructor
 *******************************************************************/
SyncCtrlObj::SyncCtrlObj(Camera& cam)
    : m_cam(cam)
{
    DEB_CONSTRUCTOR();
    double exp_time_ms, min_exp_time_ms, max_exp_time_ms;
    double min_frame_rate, max_frame_rate;

    m_cam.setAutoFrameRate(true);
    m_cam.getExpTime(exp_time_ms);
    m_cam.getExpTimeRange(min_exp_time_ms, max_exp_time_ms);
    m_cam.getFrameRateRange(min_frame_rate, max_frame_rate);

    m_exp_time = exp_time_ms * 1E-3;
    m_lat_time = 0;
    m_max_acq_period = 1.0 / min_frame_rate;

    m_valid_ranges.min_exp_time = min_exp_time_ms * 1E-3;
    m_valid_ranges.max_exp_time = m_max_acq_period;
    m_valid_ranges.min_lat_time = 0;
    m_valid_ranges.max_lat_time = m_max_acq_period - m_exp_time;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
bool SyncCtrlObj::checkTrigMode(TrigMode trig_mode)
{
    DEB_MEMBER_FUNCT();
    return true;
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::setTrigMode(TrigMode trig_mode)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(trig_mode);
    if (!checkTrigMode(trig_mode))
        THROW_HW_ERROR(InvalidValue) << "Invalid " << DEB_VAR1(trig_mode);
    m_cam.setTrigMode(trig_mode);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::getTrigMode(TrigMode& trig_mode)
{
    DEB_MEMBER_FUNCT();
    m_cam.getTrigMode(trig_mode);
    DEB_RETURN() << DEB_VAR1(trig_mode);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::setExpTime(double exp_time)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(exp_time);
    m_exp_time = exp_time;
    _adjustFrameRate();
    m_cam.setExpTime(exp_time * 1E3);

    m_valid_ranges.max_lat_time = m_max_acq_period - m_exp_time;
    validRangesChanged(m_valid_ranges);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::getExpTime(double& exp_time)
{
    DEB_MEMBER_FUNCT();
    exp_time = m_exp_time;
    DEB_RETURN() << DEB_VAR1(exp_time);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::setLatTime(double lat_time)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(lat_time);
    m_lat_time = lat_time;    
    _adjustFrameRate();

    m_valid_ranges.max_exp_time = m_max_acq_period - m_lat_time;
    validRangesChanged(m_valid_ranges);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::getLatTime(double& lat_time)
{
    DEB_MEMBER_FUNCT();
    lat_time = m_lat_time;
    DEB_RETURN() << DEB_VAR1(lat_time);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::setNbHwFrames(int nb_frames)
{
    DEB_MEMBER_FUNCT();
    DEB_PARAM() << DEB_VAR1(nb_frames);
    m_cam.setNbFrames(nb_frames);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::getNbHwFrames(int& nb_frames)
{
    DEB_MEMBER_FUNCT();
    m_cam.getNbFrames(nb_frames);
    DEB_RETURN() << DEB_VAR1(nb_frames);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::getValidRanges(ValidRangesType& valid_ranges)
{
    DEB_MEMBER_FUNCT();
    valid_ranges = m_valid_ranges;
    DEB_RETURN() << DEB_VAR1(valid_ranges);
}

//-----------------------------------------------------
//
//-----------------------------------------------------
void SyncCtrlObj::_adjustFrameRate()
{
    DEB_MEMBER_FUNCT();
    if (m_lat_time > 1E-6)
    {
        double period = m_exp_time + m_lat_time;
        m_cam.setFrameRate(1.0 / period);
    }
    else
        m_cam.setAutoFrameRate(true);
}
