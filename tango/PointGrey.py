############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2011
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################
#=============================================================================
#
# file :        PointGrey.py
#
# description : Python source for the PointGrey and its commands.
#                The class is derived from Device. It represents the
#                CORBA servant object which will be accessed from the
#                network. All commands which can be executed on the
#                Pilatus are implemented in this file.
#
# project :     TANGO Device Server
#
# copyleft :    MAX IV Laboratory
#               Lund, Sweden
#
#=============================================================================
#         (c) - Bliss - ESRF
#=============================================================================
#
import PyTango
from Lima import Core
from Lima import PointGrey as PointGreyAcq
from Lima.Server import AttrHelper

class PointGrey(PyTango.Device_4Impl):

    Core.DEB_CLASS(Core.DebModApplication, 'LimaCCDs')

#------------------------------------------------------------------
#    Device constructor
#------------------------------------------------------------------
    def __init__(self,*args) :
        PyTango.Device_4Impl.__init__(self,*args)

        self.init_device()

        self.__Attribute2FunctionBase = { 'gain'            : 'Gain'
                                        , 'auto_gain'       : 'AutoGain'
                                        , 'exp_time'        : 'ExpTime'
                                        , 'auto_exp_time'   : 'AutoExpTime'
                                        , 'frame_rate'      : 'FrameRate'
                                        , 'auto_frame_rate' : 'AutoFrameRate'
                                        , 'packet_size'     : 'PacketSize'
                                        , 'packet_delay'    : 'PacketDelay'
                                        }

#------------------------------------------------------------------
#    Device destructor
#------------------------------------------------------------------
    def delete_device(self):
        pass

#------------------------------------------------------------------
#    Device initialization
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def init_device(self):
        self.set_state(PyTango.DevState.ON)
        self.get_device_properties(self.get_device_class())

#------------------------------------------------------------------
#    getAttrStringValueList command:
#
#    Description: return a list of authorized values if any
#    argout: DevVarStringArray   
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def getAttrStringValueList(self, attr_name):
        #use AttrHelper
        return AttrHelper.get_attr_string_value_list(self, attr_name)
    
#==================================================================
#
#    PointGrey read/write attribute methods
#
#==================================================================
    def __getattr__(self,name) :
        #use AttrHelper
        return AttrHelper.get_attr_4u(self, name, _PointGreyInterface)

    def read_exp_time_range(self, attr):
        min_, max_ = _PointGreyInterface.getExpTimeRange()
        attr.set_value([min_, max_])

    def read_gain_range(self, attr):
        min_, max_ = _PointGreyInterface.getGainRange()
        attr.set_value([min_, max_])

    def read_frame_rate_range(self, attr):
        min_, max_ = _PointGreyInterface.getFrameRateRange()
        attr.set_value([min_, max_])

#==================================================================
#
#    PointGreyClass class definition
#
#==================================================================
class PointGreyClass(PyTango.DeviceClass):

    class_property_list = {}

    device_property_list = {
        'camera_serial':
        [PyTango.DevLong,
         "Camera serial number", []],
        'packet_size':
        [PyTango.DevLong,
         "GigE packet size", [-1]],
        'packet_delay':                            
        [PyTango.DevLong,
         "GigE packet delay", [-1]],                            
        }

    cmd_list = {
        'getAttrStringValueList':
        [[PyTango.DevString, "Attribute name"],
         [PyTango.DevVarStringArray, "Authorized String value list"]],
        }

    attr_list = {
        'gain':
        [[PyTango.DevDouble,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
          {
              'unit': "dB",
          }],
        'auto_gain':
        [[PyTango.DevBoolean,
          PyTango.SCALAR,
          PyTango.READ_WRITE]],
        'exp_time':
        [[PyTango.DevDouble,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
          {
              'unit': "ms",
          }],
        'auto_exp_time':
        [[PyTango.DevBoolean,
          PyTango.SCALAR,
          PyTango.READ_WRITE]],
        'frame_rate':
        [[PyTango.DevDouble,
          PyTango.SCALAR,
          PyTango.READ_WRITE],
          {
              'unit': "fps",
          }],
        'auto_frame_rate':
        [[PyTango.DevBoolean,
          PyTango.SCALAR,
          PyTango.READ_WRITE]],
        'packet_size':
        [[PyTango.DevLong,
          PyTango.SCALAR,
          PyTango.READ_WRITE]],
        'packet_delay':
        [[PyTango.DevLong,
          PyTango.SCALAR,
          PyTango.READ_WRITE]],
        'exp_time_range':
        [[PyTango.DevDouble,
          PyTango.SPECTRUM,
          PyTango.READ,2],
          {
              'unit': "ms",
              'description':"min exposure, max exposure",
          }],
        'gain_range':
        [[PyTango.DevDouble,
          PyTango.SPECTRUM,
          PyTango.READ,2],
          {
              'unit': "dB",
              'description':"min gain, max gain",
          }],
        'frame_rate_range':
        [[PyTango.DevDouble,
          PyTango.SPECTRUM,
          PyTango.READ,2],
          {
              'unit': "fps",
              'description':"min frame rate, max frame rate",
          }],
        }

    def __init__(self,name) :
        PyTango.DeviceClass.__init__(self,name)
        self.set_type(name)

#----------------------------------------------------------------------------
# Plugins
#----------------------------------------------------------------------------
_PointGreyCam = None
_PointGreyInterface = None

def get_control(camera_serial, packet_size, packet_delay):
    global _PointGreyCam, _PointGreyInterface 

    if _PointGreyCam is None:
        _PointGreyCam = PointGreyAcq.Camera(int(camera_serial),
                                            int(packet_size),
                                            int(packet_delay))
        _PointGreyInterface = PointGreyAcq.Interface(_PointGreyCam)
            
    return Core.CtControl(_PointGreyInterface)

def get_tango_specific_class_n_device():
    return PointGreyClass, PointGrey
