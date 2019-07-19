PointGrey Tango device
======================

This is the reference documentation of the PointGrey Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`PointGrey camera plugin <camera-pointgrey>` section.


Properties
----------

=============== =============== =============== =========================================================================
Property name	Mandatory	Default value	Description
=============== =============== =============== =========================================================================
camera_serial   Yes		N/A		The serial number of the camera, used to get the connection
packet_size     No              -1              The packet size, in byte
packet_delay    No              -1              The packet inter delay , in us
                                                last both parameters can be used to tune the camera GigE bandwidth, please
                                                refer to the camera documentation for more information
=============== =============== =============== =========================================================================

Attributes
----------
======================= ======= ======================= ======================================================================
Attribute name		RW	Type			Description
======================= ======= ======================= ======================================================================
gain			rw	DevDouble	 	The camera gain factor, in dB
auto_gain               rw      DevBoolean              Auto gain mode can be switched on or off
auto_exp_time           rw      DevBoolean              The camera can be set to auto-exposure mode
auto_frame_mode         rw      DevBoolean              The camera can be set to auto frame rate mode	
frame_rate              rw      DevDouble               The frame rate, in fps
packet_size             rw      DevLong                 See the corresponding property
packet_delay            rw      DevLong                 See the corresponding property
exp_time_range          ro      DevDouble[]             Return the exposure time range (min,max) in ms
gain_range              ro      DevDouble[]             Return the gain range (min,max) in dB
frame_rate_range        ro      DevDouble[]             Return the frame rate range (min,max) in fps
======================= ======= ======================= ======================================================================

Commands
--------

=======================	=============== =======================	===========================================
Command name		Arg. in		Arg. out		Description
=======================	=============== =======================	===========================================
Init			DevVoid 	DevVoid			Do not use
State			DevVoid		DevLong			Return the device state
Status			DevVoid		DevString		Return the device state as a string
getAttrStringValueList	DevString:	DevVarStringArray:	Return the authorized string value list for
			Attribute name	String value list	a given attribute name
=======================	=============== =======================	===========================================


