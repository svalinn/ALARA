# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _FEIND

def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


class StringVector(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, StringVector, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, StringVector, name)
    def __repr__(self):
        return "<C std::vector<(std::string)> instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, StringVector, 'this', _FEIND.new_StringVector(*args))
        _swig_setattr(self, StringVector, 'thisown', 1)
    def __len__(*args): return _FEIND.StringVector___len__(*args)
    def __nonzero__(*args): return _FEIND.StringVector___nonzero__(*args)
    def clear(*args): return _FEIND.StringVector_clear(*args)
    def append(*args): return _FEIND.StringVector_append(*args)
    def pop(*args): return _FEIND.StringVector_pop(*args)
    def __getitem__(*args): return _FEIND.StringVector___getitem__(*args)
    def __getslice__(*args): return _FEIND.StringVector___getslice__(*args)
    def __setitem__(*args): return _FEIND.StringVector___setitem__(*args)
    def __setslice__(*args): return _FEIND.StringVector___setslice__(*args)
    def __delitem__(*args): return _FEIND.StringVector___delitem__(*args)
    def __delslice__(*args): return _FEIND.StringVector___delslice__(*args)
    def __del__(self, destroy=_FEIND.delete_StringVector):
        try:
            if self.thisown: destroy(self)
        except: pass

class StringVectorPtr(StringVector):
    def __init__(self, this):
        _swig_setattr(self, StringVector, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, StringVector, 'thisown', 0)
        _swig_setattr(self, StringVector,self.__class__,StringVector)
_FEIND.StringVector_swigregister(StringVectorPtr)

VITAMIN_J = _FEIND.VITAMIN_J
IEAF = _FEIND.IEAF
CINDER_NEUTRON = _FEIND.CINDER_NEUTRON
CINDER_GAMMA = _FEIND.CINDER_GAMMA
DECAY_ENDF_6 = _FEIND.DECAY_ENDF_6
EAF_4_1 = _FEIND.EAF_4_1
CINDER = _FEIND.CINDER
ENDF_IEAF = _FEIND.ENDF_IEAF
FEIND_UNKNOWN_ERROR = _FEIND.FEIND_UNKNOWN_ERROR
FEIND_FORMAT_ERROR = _FEIND.FEIND_FORMAT_ERROR
FEIND_DECAYMODE_ERROR = _FEIND.FEIND_DECAYMODE_ERROR
FEIND_FILEOPEN_ERROR = _FEIND.FEIND_FILEOPEN_ERROR
FEIND_INVALIDOPTION_ERROR = _FEIND.FEIND_INVALIDOPTION_ERROR
FEIND_XSECSIZE_ERROR = _FEIND.FEIND_XSECSIZE_ERROR
FEIND_EMPTYXSEC_ERROR = _FEIND.FEIND_EMPTYXSEC_ERROR
HEAVY_PARTICLES = _FEIND.HEAVY_PARTICLES
LIGHT_PARTICLES = _FEIND.LIGHT_PARTICLES
EM_RADIATION = _FEIND.EM_RADIATION
UNKNOWN = _FEIND.UNKNOWN
GAMMA_DECAY = _FEIND.GAMMA_DECAY
BETA_DECAY = _FEIND.BETA_DECAY
ELECTRON_CAPTURE = _FEIND.ELECTRON_CAPTURE
ISOMERIC_TRANSITION = _FEIND.ISOMERIC_TRANSITION
ALPHA_DECAY = _FEIND.ALPHA_DECAY
NEUTRON_EMISSION = _FEIND.NEUTRON_EMISSION
SPONTANEOUS_FISSION = _FEIND.SPONTANEOUS_FISSION
PROTON_EMISSION = _FEIND.PROTON_EMISSION
BETA_NEUTRON_EMIT = _FEIND.BETA_NEUTRON_EMIT
BETA_ALPHA_EMIT = _FEIND.BETA_ALPHA_EMIT
POSITRON_ALPHA_EMIT = _FEIND.POSITRON_ALPHA_EMIT
POSITRON_PROTON_EMIT = _FEIND.POSITRON_PROTON_EMIT
IT_ALPHA_EMIT = _FEIND.IT_ALPHA_EMIT
NO_FISSION = _FEIND.NO_FISSION
FISSION_FAST = _FEIND.FISSION_FAST
FISSION_THERMAL = _FEIND.FISSION_THERMAL
FISSION_HOT = _FEIND.FISSION_HOT
FISSION_SF = _FEIND.FISSION_SF
TOTAL_CS = _FEIND.TOTAL_CS
NEUTRON_FISSION_CS = _FEIND.NEUTRON_FISSION_CS
CHARGED_CS = _FEIND.CHARGED_CS
class LibDefine(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, LibDefine, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, LibDefine, name)
    def __repr__(self):
        return "<C FEIND::LibDefine instance at %s>" % (self.this,)
    __swig_setmethods__["Args"] = _FEIND.LibDefine_Args_set
    __swig_getmethods__["Args"] = _FEIND.LibDefine_Args_get
    if _newclass:Args = property(_FEIND.LibDefine_Args_get, _FEIND.LibDefine_Args_set)
    __swig_setmethods__["Format"] = _FEIND.LibDefine_Format_set
    __swig_getmethods__["Format"] = _FEIND.LibDefine_Format_get
    if _newclass:Format = property(_FEIND.LibDefine_Format_get, _FEIND.LibDefine_Format_set)
    def __init__(self, *args):
        _swig_setattr(self, LibDefine, 'this', _FEIND.new_LibDefine(*args))
        _swig_setattr(self, LibDefine, 'thisown', 1)
    def __del__(self, destroy=_FEIND.delete_LibDefine):
        try:
            if self.thisown: destroy(self)
        except: pass

class LibDefinePtr(LibDefine):
    def __init__(self, this):
        _swig_setattr(self, LibDefine, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, LibDefine, 'thisown', 0)
        _swig_setattr(self, LibDefine,self.__class__,LibDefine)
_FEIND.LibDefine_swigregister(LibDefinePtr)
cvar = _FEIND.cvar
NUM_ELEMENTS = cvar.NUM_ELEMENTS
PROTON = cvar.PROTON
NEUTRON = cvar.NEUTRON
DEUTERON = cvar.DEUTERON
TRITON = cvar.TRITON
HELIUM3 = cvar.HELIUM3
HELIUM4 = cvar.HELIUM4
ALPHA = cvar.ALPHA
GAMMA = cvar.GAMMA
BETA = cvar.BETA
ELECTRON = cvar.ELECTRON
XRAY = cvar.XRAY
SF_FRAGMENTS = cvar.SF_FRAGMENTS
POSITRON = cvar.POSITRON
FISSION_DAUGHTER = cvar.FISSION_DAUGHTER


LoadLibrary = _FEIND.LoadLibrary
class RamLib(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, RamLib, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, RamLib, name)
    def __repr__(self):
        return "<C FEIND::RamLib instance at %s>" % (self.this,)
    def GetTotalDecayEnergy(*args): return _FEIND.RamLib_GetTotalDecayEnergy(*args)
    def __init__(self, *args):
        _swig_setattr(self, RamLib, 'this', _FEIND.new_RamLib(*args))
        _swig_setattr(self, RamLib, 'thisown', 1)
    def __del__(self, destroy=_FEIND.delete_RamLib):
        try:
            if self.thisown: destroy(self)
        except: pass

class RamLibPtr(RamLib):
    def __init__(self, this):
        _swig_setattr(self, RamLib, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, RamLib, 'thisown', 0)
        _swig_setattr(self, RamLib,self.__class__,RamLib)
_FEIND.RamLib_swigregister(RamLibPtr)


